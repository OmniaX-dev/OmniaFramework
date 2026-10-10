#include "PianoRoll.hpp"
#include <ogfx/render/BasicRenderer.hpp>
#include <ostd/io/Logger.hpp>
#include <ostd/io/Json.hpp>
#include <filesystem>
#include <algorithm>
#include <cmath>

namespace ogfx
{
	namespace gui
	{
		namespace
		{
			inline u8 lerpChannel(u8 a, u8 b, f32 t) { return (u8)std::round(a + ((f32)b - (f32)a) * t); }

			// Same approach as PianoRollStyle.cpp's own lerpColor() (its gradient tier) - kept as
			// its own small copy here rather than a shared utility, since that's all either side
			// needs it for.
			Color lerpColor(const Color& a, const Color& b, f32 t)
			{
				Color a2 = a, b2 = b; // Color's channel accessors aren't const-qualified
				return Color(lerpChannel(a2.r, b2.r, t), lerpChannel(a2.g, b2.g, t), lerpChannel(a2.b, b2.b, t), lerpChannel(a2.a, b2.a, t));
			}
		}

		PianoRoll& PianoRoll::create(void)
		{
			setSize(800, 320);
			m_pressedKeyColor.assign(128, Colors::Transparent);
			m_keyPressed.assign(128, false);
			m_keyFading.assign(128, false);
			m_keyReleaseTime.assign(128, 0.0);
			m_keyReleaseColor.assign(128, Colors::Transparent);
			m_updateClock.startCount(ostd::eTimeUnits::Milliseconds);
			setStylesheetCategoryName("pianoRoll");
			validate();
			return *this;
		}

		void PianoRoll::applyTheme(const ostd::Stylesheet& theme)
		{
			// PianoRollStyle (not the global theme system) is the intended customization
			// surface for this widget - see getStyle()/setStyle() and PianoRollStyle's own
			// save/load. Nothing to pull from the theme today.
		}

		u32 PianoRoll::loadVoice(const String& midiFilePath, const Color& color, const String& name)
		{
			Voice voice;
			try
			{
				voice.notes = ostd::MidiParser::parseFile(midiFilePath);
			}
			catch (std::exception& e)
			{
				OX_WARN("PianoRoll: failed to load voice \"%s\": %s", midiFilePath.c_str(), e.what());
				return (u32)-1;
			}
			voice.midiFilePath = midiFilePath;
			voice.color = color;
			voice.visible = true;
			if (!name.empty())
				voice.name = name;
			else
				voice.name = String(std::filesystem::path(midiFilePath.cpp_str()).stem().string());

			m_voices.push_back(std::move(voice));
			recompute_total_duration();
			return (u32)m_voices.size() - 1;
		}

		bool PianoRoll::removeVoice(u32 index)
		{
			if (index >= m_voices.size())
				return false;
			m_voices.erase(m_voices.begin() + index);
			recompute_total_duration();
			// recompute_pressed_keys() compares against notes' raw (unshifted) timestamps, same
			// as everywhere else internal - getCurrentTimeSeconds() is the public, pad-shifted
			// presentation clock, so it needs converting back.
			recompute_pressed_keys(getCurrentTimeSeconds() - m_startPadSeconds);
			return true;
		}

		void PianoRoll::clearVoices(void)
		{
			m_voices.clear();
			m_activeNotes.clear();
			recompute_total_duration();
			recompute_pressed_keys(getCurrentTimeSeconds() - m_startPadSeconds);
		}

		PianoRoll::Voice& PianoRoll::getVoice(u32 index)
		{
			static Voice invalid;
			return index < m_voices.size() ? m_voices[index] : invalid;
		}

		const PianoRoll::Voice& PianoRoll::getVoice(u32 index) const
		{
			static const Voice invalid;
			return index < m_voices.size() ? m_voices[index] : invalid;
		}

		bool PianoRoll::loadAudio(const String& audioFilePath)
		{
			osnd::AudioClip clip = osnd::AudioClip::load(audioFilePath);
			if (!clip.isValid())
			{
				OX_WARN("PianoRoll: failed to load audio \"%s\"", audioFilePath.c_str());
				return false;
			}
			m_audioPlayer = std::make_unique<osnd::AudioPlayer>();
			// Keep going even if the device itself failed to open (no hardware reachable right
			// now, ...) - duration/position/seek all stay valid either way, see AudioPlayer.
			m_audioPlayer->load(std::move(clip));
			m_audioPlayer->setVolume(m_volume);
			m_audioStarted = false; // fresh device - hasn't been positioned/started for this session yet
			recompute_total_duration();
			return true;
		}

		void PianoRoll::clearAudio(void)
		{
			m_audioPlayer.reset();
			m_audioStarted = false;
			recompute_total_duration();
		}

		void PianoRoll::setVolume(f32 volume01)
		{
			m_volume = std::clamp(volume01, 0.0f, 1.0f);
			if (m_audioPlayer)
				m_audioPlayer->setVolume(m_volume);
		}

		void PianoRoll::recompute_total_duration(void)
		{
			f64 d = 0.0;
			f64 earliestStart = 0.0;
			bool anyNotes = false;
			for (auto& voice : m_voices)
				for (auto& note : voice.notes)
				{
					d = std::max(d, note.endTime);
					if (!anyNotes || note.startTime < earliestStart)
						earliestStart = note.startTime;
					anyNotes = true;
				}
			if (hasAudio())
				d = std::max(d, m_audioPlayer->getDurationSeconds());

			// Lead-in silence so the earliest note starts fully offscreen instead of popping in
			// already partway down - only needed if the content itself doesn't already have at
			// least lookaheadSeconds of silence before the first note. extraOffscreenSeconds is
			// added unconditionally on top, so the first note doesn't start falling the instant
			// play() is pressed - it gets a moment to actually appear from offscreen first.
			m_startPadSeconds = anyNotes ? std::max(0.0, (f64)m_style.getLookaheadSeconds() - earliestStart) + (f64)m_style.getExtraOffscreenSeconds() : 0.0;
			m_totalDurationSeconds = m_startPadSeconds + d;

			// Only reset the playhead to the (new) start if nothing has actually played yet this
			// session - loading/removing a voice mid-playback shouldn't yank the position back to
			// 0 out from under whatever's already happening.
			if (!m_midiStartFired && !m_playing)
			{
				m_fallbackClockSeconds = -m_startPadSeconds;
				// Must match m_fallbackClockSeconds, or onUpdate()'s very first tick sees a stale
				// prevRaw (left over from construction/a previous session, e.g. 0.0) instead of
				// the correct -m_startPadSeconds - for a note starting at/near raw time 0 with a
				// small pad, that stale prevRaw can already be >= the note's startTime, so the
				// strict "startTime > prevNow" NoteOn edge never fires for it.
				m_lastUpdateSeconds = -m_startPadSeconds;
				m_audioStarted = false;
				if (hasAudio())
				{
					m_audioPlayer->pause();
					m_audioPlayer->seekSeconds(0.0);
				}
			}
		}

		void PianoRoll::play(void)
		{
			m_playing = true;
			if (has_live_audio_clock() && !m_audioPlayer->isPlaying())
			{
				f64 raw = m_fallbackClockSeconds;
				if (raw >= 0.0)
				{
					// Past the pre-roll already (e.g. resuming after a seek-then-pause) - resume
					// directly from here. If raw is still negative, leave the device alone;
					// onUpdate() starts it itself exactly when raw reaches 0.
					m_audioPlayer->seekSeconds(raw);
					m_audioPlayer->play();
					m_audioStarted = true;
				}
			}
		}

		void PianoRoll::pause(void)
		{
			// Sync the fallback clock to wherever the audio clock actually was - once the device
			// starts actually driving things, m_fallbackClockSeconds is left frozen at whatever it
			// was at that crossing (see onUpdate()); play()'s own "resume directly" branch reads
			// m_fallbackClockSeconds, so without this it would always resume from that stale
			// frozen value - a backward jump on every single pause/play, which also means
			// update_active_notes() sees a backward (non-forward) jump and skips its own edge
			// detection, leaving stale "ghost" pressed keys and rebuilding m_activeNotes around
			// the wrong (far earlier) position.
			m_fallbackClockSeconds = getCurrentTimeSeconds() - m_startPadSeconds;
			m_playing = false;
			if (hasAudio())
				m_audioPlayer->pause();
		}

		void PianoRoll::stop(void)
		{
			pause();
			seekSeconds(0.0);
		}

		bool PianoRoll::isPlaying(void) const
		{
			return m_playing;
		}

		void PianoRoll::seekSeconds(f64 seconds)
		{
			if (seconds < 0.0) seconds = 0.0;
			if (seconds > m_totalDurationSeconds) seconds = m_totalDurationSeconds;
			f64 raw = seconds - m_startPadSeconds; // may be negative - still within the pre-roll
			// Keep both clocks in sync regardless of which one is actually live right now - if
			// hasAudio() is true but there's no live device yet, this still leaves AudioPlayer's
			// own cursor correctly positioned for if/when it starts actually advancing.
			m_fallbackClockSeconds = raw;
			if (hasAudio())
			{
				if (raw >= 0.0)
				{
					m_audioPlayer->seekSeconds(raw);
					if (m_playing)
						m_audioPlayer->play();
					else
						m_audioPlayer->pause();
					m_audioStarted = true;
				}
				else
				{
					// Back inside the pre-roll - the device has nothing to play yet.
					m_audioPlayer->pause();
					m_audioPlayer->seekSeconds(0.0);
					m_audioStarted = false;
				}
			}
			// Prevents the next onUpdate() from seeing a huge prevNow->now jump and firing a
			// storm of catch-up NoteOn/NoteOff signals for everything in between.
			m_lastUpdateSeconds = raw;
			update_active_notes(raw, raw);
			// update_active_notes()'s NoteOn/NoteOff edge-detection never fires here (prevNow
			// == now - nothing to cross), so whatever was pressed before the seek would
			// otherwise stay highlighted until some other note at the same pitch happened to
			// retrigger it. Recompute key state outright instead of relying on edges.
			recompute_pressed_keys(raw);
			if (seconds <= 0.0)
				m_midiStartFired = false; // rewinding to the very start allows MidiStart to fire again on the next play()
		}

		f64 PianoRoll::getCurrentTimeSeconds(void) const
		{
			f64 raw = (has_live_audio_clock() && m_audioStarted) ? m_audioPlayer->getPositionSeconds() : m_fallbackClockSeconds;
			return raw + m_startPadSeconds;
		}

		f64 PianoRoll::getTotalDurationSeconds(void) const
		{
			return m_totalDurationSeconds;
		}

		void PianoRoll::onUpdate(void)
		{
			f64 prevRaw = m_lastUpdateSeconds;
			f64 deltaMs = (f64)m_updateClock.read();
			m_updateClock.restart(ostd::eTimeUnits::Milliseconds);

			f64 raw;
			if (has_live_audio_clock() && m_audioPlayer->isPlaying())
				raw = m_audioPlayer->getPositionSeconds();
			else
			{
				if (m_playing)
					m_fallbackClockSeconds += deltaMs / 1000.0;
				raw = m_fallbackClockSeconds;
				if (m_playing && has_live_audio_clock() && raw >= 0.0)
				{
					// Crossing out of the pre-roll silence into real time - start the device now
					// so its own position (0 at this instant) lines up exactly with raw == 0.
					m_audioPlayer->seekSeconds(0.0);
					m_audioPlayer->play();
					m_audioStarted = true;
					raw = m_audioPlayer->getPositionSeconds();
				}
			}

			if (m_playing)
			{
				if (!m_midiStartFired)
				{
					m_midiStartFired = true;
					ostd::SignalHandler::emitSignal(MidiStartSignal);
				}
				f64 rawDuration = m_totalDurationSeconds - m_startPadSeconds;
				if (rawDuration > 0.0 && raw >= rawDuration)
				{
					update_active_notes(rawDuration, prevRaw);
					m_lastUpdateSeconds = rawDuration;
					ostd::SignalHandler::emitSignal(MidiEndSignal);
					pause();
					return;
				}
			}

			update_active_notes(raw, prevRaw);
			m_lastUpdateSeconds = raw;
		}

		void PianoRoll::update_active_notes(f64 now, f64 prevNow)
		{
			m_activeNotes.clear();
			const f32 lookahead = m_style.getLookaheadSeconds();
			const bool forward = now >= prevNow;

			for (u32 vi = 0; vi < m_voices.size(); vi++)
			{
				auto& voice = m_voices[vi];
				if (!voice.visible)
					continue;
				for (auto& note : voice.notes)
				{
					if (now >= note.startTime - lookahead && now <= note.endTime)
						m_activeNotes.push_back({ vi, &note });

					if (!forward)
						continue; // a backward seek already re-settled state in seekSeconds() - never re-fire signals here

					if (note.startTime > prevNow && note.startTime <= now)
					{
						if (note.pitch >= 0 && note.pitch < 128)
						{
							m_keyPressed[(size_t)note.pitch] = true;
							m_keyFading[(size_t)note.pitch] = false; // re-pressed - cancel any in-progress release fade
							Color vc = voice.color;
							bool hasVoiceColor = !(vc == Colors::Transparent);
							m_pressedKeyColor[(size_t)note.pitch] = m_style.resolveNoteColor(note.pitch, hasVoiceColor ? &voice.color : nullptr);
						}
						emit_note_signal(NoteOnSignal, note, vi);
					}
					if (note.endTime > prevNow && note.endTime <= now)
					{
						if (note.pitch >= 0 && note.pitch < 128)
						{
							// Starts the release fade from whatever color was actually being shown
							// (which depends on useNoteColorOnPressedKey), not an unconditional one.
							bool isBlack = ostd::MidiParser::getNoteInfo(note.pitch).isBlackKey();
							m_keyReleaseColor[(size_t)note.pitch] = m_style.isUseNoteColorOnPressedKeyEnabled()
								? m_pressedKeyColor[(size_t)note.pitch]
								: (isBlack ? m_style.getBlackKeyPressedColor() : m_style.getWhiteKeyPressedColor());
							m_keyReleaseTime[(size_t)note.pitch] = note.endTime;
							m_keyFading[(size_t)note.pitch] = true;
							m_keyPressed[(size_t)note.pitch] = false;
						}
						emit_note_signal(NoteOffSignal, note, vi);
					}
				}
			}
		}

		void PianoRoll::recompute_pressed_keys(f64 now)
		{
			std::fill(m_keyPressed.begin(), m_keyPressed.end(), false);
			// A seek/voice change is a discontinuous jump with no meaningful "time since release"
			// to animate a fade across - settle instantly instead (either pressed, or plain
			// normal color, never mid-fade).
			std::fill(m_keyFading.begin(), m_keyFading.end(), false);
			for (auto& voice : m_voices)
			{
				if (!voice.visible)
					continue;
				for (auto& note : voice.notes)
				{
					if (note.pitch < 0 || note.pitch >= 128)
						continue;
					if (now >= note.startTime && now < note.endTime) // half-open, matching update_active_notes()'s NoteOn/NoteOff edges
					{
						m_keyPressed[(size_t)note.pitch] = true;
						Color vc = voice.color;
						bool hasVoiceColor = !(vc == Colors::Transparent);
						m_pressedKeyColor[(size_t)note.pitch] = m_style.resolveNoteColor(note.pitch, hasVoiceColor ? &voice.color : nullptr);
					}
				}
			}
		}

		void PianoRoll::emit_note_signal(u32 signalId, const ostd::MidiParser::NoteEvent& note, u32 voiceIndex)
		{
			PianoRollNoteEventData data;
			data.pitch = note.pitch;
			data.velocity = note.velocity;
			data.channel = note.channel;
			data.voiceIndex = voiceIndex;
			data.noteInfo = ostd::MidiParser::getNoteInfo(note.pitch);
			data.timestamp = signalId == NoteOffSignal ? note.endTime : note.startTime;
			ostd::SignalHandler::emitSignal(signalId, ostd::Signal::Priority::RealTime, &data);
		}

		f32 PianoRoll::key_x_position(i32 midiPitch, f32 scale) const
		{
			if (midiPitch < 21 || midiPitch > 108)
				return 0.0f;
			i32 target = midiPitch - 21;
			f32 whiteX = 0.0f;
			for (i32 k = 0; k <= target; k++)
			{
				auto info = ostd::MidiParser::getNoteInfo(21 + k);
				if (info.isBlackKey())
				{
					if (k == target)
					{
						// whiteX is the position just past the preceding white key, including its
						// gap - subtracting the gap back out gives that white key's actual drawn
						// right edge, the real anchor for a black key's position (rather than
						// centering on it, which looks flush/symmetric instead of like a real
						// piano). getBlackKeyOffset() is a small additional nudge on top, same as
						// the Godot reference.
						f32 precedingWhiteKeyRightEdge = whiteX - m_style.getWhiteKeyGap() * scale;
						return precedingWhiteKeyRightEdge - m_style.getBlackKeyWidth() * scale * (3.0f / 4.0f) + m_style.getBlackKeyOffset() * scale;
					}
				}
				else
				{
					if (k == target)
						return whiteX;
					whiteX += (m_style.getWhiteKeyWidth() + m_style.getWhiteKeyGap()) * scale;
				}
			}
			return whiteX;
		}

		f32 PianoRoll::getKeyboardWidth(void) const
		{
			return key_x_position(108, 1.0f) + m_style.getWhiteKeyWidth(); // 108 = C8, the highest (white) key
		}

		void PianoRoll::onDraw(BasicRenderer2D& gfx)
		{
			Rectangle bounds = getGlobalBounds();
			gfx.fillRect(bounds, m_style.getBackgroundColor());
			// Scales the whole 88-key layout (width and, since height stays derived from width,
			// height too) to exactly fill whatever width the widget is actually given, rather than
			// requiring it to be set to getKeyboardWidth() - so a style's own preferred sizes stay
			// the single source of truth for proportions, and the widget's actual size (which a
			// layout/scroll container may well be the one deciding) drives everything else.
			f32 naturalWidth = getKeyboardWidth();
			f32 scale = naturalWidth > 0.0f ? bounds.w / naturalWidth : 1.0f;
			// Clips keys/notes that don't fit within whatever height the widget was actually given
			// instead of letting them bleed past the widget's own bounds into whatever is drawn
			// next. additive=true intersects with whatever clip an ancestor (e.g. a scrolled
			// panel's viewport) already has active, instead of overriding it outright - every
			// other widget does the same (see Widget::onDraw()/WidgetManager's own calls).
			gfx.pushClippingRect(bounds, true);
			draw_falling_notes(gfx, bounds, scale);
			draw_keyboard(gfx, bounds, scale);
			gfx.popClippingRect();
		}

		Color PianoRoll::resolve_key_color(i32 pitch, bool isBlack, f64 now) const
		{
			if (m_keyPressed[(size_t)pitch])
			{
				return m_style.isUseNoteColorOnPressedKeyEnabled()
					? m_pressedKeyColor[(size_t)pitch]
					: (isBlack ? m_style.getBlackKeyPressedColor() : m_style.getWhiteKeyPressedColor());
			}
			Color normal = isBlack ? m_style.getBlackKeyColor() : m_style.getWhiteKeyColor();
			if (m_style.isUseKeyFadeEnabled() && m_keyFading[(size_t)pitch])
			{
				f32 duration = m_style.getKeyFadeDurationSeconds();
				if (duration > 0.0f)
				{
					f32 t = (f32)((now - m_keyReleaseTime[(size_t)pitch]) / (f64)duration);
					if (t < 1.0f)
						return lerpColor(m_keyReleaseColor[(size_t)pitch], normal, std::clamp(t, 0.0f, 1.0f));
				}
			}
			return normal;
		}

		void PianoRoll::draw_keyboard(BasicRenderer2D& gfx, const Rectangle& bounds, f32 scale)
		{
			f32 whiteW = m_style.getWhiteKeyWidth() * scale;
			f32 blackW = m_style.getBlackKeyWidth() * scale;
			f32 whiteH = whiteW * m_style.getWhiteKeyHeightMultiplier();
			f32 blackH = blackW * m_style.getBlackKeyHeightMultiplier();
			f32 keyboardTop = bounds.y + bounds.h - whiteH;
			f32 blackRadius = blackW * 0.3f;
			Rectangle blackRadii { 0, 0, blackRadius, blackRadius }; // rounded bottom-right/bottom-left only
			// Raw time, same convention as draw_falling_notes() - m_keyReleaseTime is stamped in
			// raw time too (see update_active_notes()).
			f64 now = getCurrentTimeSeconds() - m_startPadSeconds;

			for (i32 k = 0; k < 88; k++)
			{
				i32 pitch = 21 + k;
				if (ostd::MidiParser::getNoteInfo(pitch).isBlackKey())
					continue;
				f32 x = key_x_position(pitch, scale);
				gfx.fillRect({ bounds.x + x, keyboardTop, whiteW, whiteH }, resolve_key_color(pitch, false, now));
			}
			for (i32 k = 0; k < 88; k++)
			{
				i32 pitch = 21 + k;
				if (!ostd::MidiParser::getNoteInfo(pitch).isBlackKey())
					continue;
				f32 x = key_x_position(pitch, scale);
				gfx.fillRoundRect({ bounds.x + x, keyboardTop, blackW, blackH }, resolve_key_color(pitch, true, now), blackRadii);
			}
			gfx.drawLine({ { bounds.x, keyboardTop }, { bounds.x + bounds.w, keyboardTop } }, m_style.getKeyboardLineColor(), 2);
		}

		void PianoRoll::draw_falling_notes(BasicRenderer2D& gfx, const Rectangle& bounds, f32 scale)
		{
			f32 whiteH = m_style.getWhiteKeyWidth() * scale * m_style.getWhiteKeyHeightMultiplier();
			f32 keyboardTop = bounds.y + bounds.h - whiteH;
			f32 fallAreaHeight = bounds.h - whiteH;
			f32 lookahead = std::max(0.01f, m_style.getLookaheadSeconds());
			f32 pxPerSecond = fallAreaHeight / lookahead;
			// Raw time, not the public (pad-shifted) getCurrentTimeSeconds() - note.startTime is
			// in the same raw clock, which is what makes a note sit fully offscreen until raw
			// catches up to startTime - lookahead (see m_startPadSeconds).
			f64 now = getCurrentTimeSeconds() - m_startPadSeconds;

			gfx.pushClippingRect({ bounds.x, bounds.y, bounds.w, fallAreaHeight }, true); // additive - see onDraw()
			for (auto& active : m_activeNotes)
			{
				auto& note = *active.note;
				auto& voice = m_voices[active.voiceIndex];
				bool isBlack = ostd::MidiParser::getNoteInfo(note.pitch).isBlackKey();

				f32 noteHeight = (f32)note.duration * pxPerSecond;
				f32 bottomY = keyboardTop - (f32)(note.startTime - now) * pxPerSecond;
				f32 topY = bottomY - noteHeight;

				f32 gap = (isBlack ? m_style.getBlackKeyGap() : m_style.getWhiteKeyGap()) * scale;
				f32 w = (isBlack ? m_style.getBlackKeyWidth() : m_style.getWhiteKeyWidth()) * scale - gap;
				f32 x = key_x_position(note.pitch, scale) + gap * 0.5f;

				Color vc = voice.color;
				bool hasVoiceColor = !(vc == Colors::Transparent);
				Color col = m_style.resolveNoteColor(note.pitch, hasVoiceColor ? &voice.color : nullptr);
				f32 radius = (isBlack ? m_style.getFallingNoteRadiusBlack() : m_style.getFallingNoteRadiusWhite()) * scale;

				Rectangle rect { bounds.x + x, topY, w, noteHeight };
				if (m_style.isUseFilledNotesEnabled())
					gfx.fillRoundRect(rect, col, radius);
				else
					gfx.outlinedRoundRect(rect, Colors::Transparent, col, radius, 2);
			}
			gfx.popClippingRect();
		}

		bool PianoRoll::saveStateToJson(const String& path) const
		{
			if (!m_style.saveToJson(path))
				return false;
			ostd::JsonFile f(path);
			if (!f.isLoaded())
				return false;

			stdvec<String> names, paths;
			stdvec<Color> colors;
			stdvec<i32> visibleFlags;
			for (auto& v : m_voices)
			{
				names.push_back(v.name);
				paths.push_back(v.midiFilePath);
				colors.push_back(v.color);
				visibleFlags.push_back(v.visible ? 1 : 0);
			}
			f.set_string_array("voices.names", names);
			f.set_string_array("voices.paths", paths);
			f.set_color_array("voices.colors", colors);
			f.set_int_array("voices.visible", visibleFlags);
			return true;
		}

		bool PianoRoll::loadStateFromJson(const String& path)
		{
			if (!m_style.loadFromJson(path))
				return false;
			ostd::JsonFile f(path);
			if (!f.isLoaded())
				return false;

			auto names = f.get_string_array("voices.names");
			auto paths = f.get_string_array("voices.paths");
			auto colors = f.get_color_array("voices.colors");
			auto visibleFlags = f.get_int_array("voices.visible");

			clearVoices();
			for (size_t i = 0; i < paths.size(); i++)
			{
				Color c = i < colors.size() ? colors[i] : Colors::Transparent;
				String n = i < names.size() ? names[i] : String("");
				u32 idx = loadVoice(paths[i], c, n);
				if (idx != (u32)-1 && i < visibleFlags.size())
					setVoiceVisible(idx, visibleFlags[i] != 0);
			}
			return true;
		}
	}
}
