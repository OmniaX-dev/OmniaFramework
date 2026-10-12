/*
	OmniaFramework - A collection of useful functionality
	Copyright (C) 2026  OmniaX-Dev

	This file is part of OmniaFramework.

	OmniaFramework is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	OmniaFramework is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with OmniaFramework.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <ogfx/gui/widgets/Widget.hpp>
#include <ogfx/gui/widgets/PianoRollStyle.hpp>
#include <ostd/utils/Signals.hpp>
#include <ostd/utils/Time.hpp>
#include <osnd/osnd.hpp>
#include <memory>

namespace ogfx
{
	namespace gui
	{
		// Signal payload for NoteOn/NoteOff - carries everything a listener would plausibly
		// need without having to re-derive it (pitch -> NoteInfo, which voice, when).
		class PianoRollNoteEventData : public ostd::Object
		{
			public:
				inline PianoRollNoteEventData(void) { setTypeName("ogfx::gui::PianoRollNoteEventData"); validate(); }
				i32 pitch { 0 };
				i32 velocity { 0 };
				i32 channel { 0 };
				u32 voiceIndex { 0 };
				ostd::MidiParser::NoteInfo noteInfo;
				f64 timestamp { 0.0 };
		};

		// A Synthesia-style falling-note piano visualization, synced to audio. Rewritten from
		// scratch (not ported) from two earlier, admittedly fragile prototypes - the one
		// deliberate behavioral departure from both is how "now" is tracked: every frame reads
		// the audio device's actual consumed-sample count (via osnd::AudioPlayer), rather than
		// a wall-clock aligned to audio once at start and left to drift. Seeking is therefore
		// exact and instant (a cursor reset), not an approximation.
		class PianoRoll : public Widget
		{
			public: struct Voice
			{
				String name;
				String midiFilePath;
				Color color { Colors::Transparent }; // Transparent = let PianoRollStyle's other tiers decide
				bool visible { true };
				stdvec<ostd::MidiParser::NoteEvent> notes; // time-sorted (MidiParser's parse order)
				// Sustain/sostenuto/soft CCs from this voice's own file - kept per-voice (not
				// flattened away) so a future per-voice preview can "merge in the pedal events
				// too" even when isolating just this one voice; ordinary multi-voice playback
				// (build_synth_schedule()) merges every visible voice's pedalEvents into one
				// timeline regardless, since pedals are global to whatever's actually sounding.
				stdvec<ostd::MidiParser::ControlChangeEvent> pedalEvents;
			};

			public:
				inline PianoRoll(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				PianoRoll& create(void);
				// Explicitly virtual (not just inheriting Widget::applyTheme's virtual-ness
				// implicitly) so a subclass's own override reads unambiguously as one - see
				// PianoRollEditor's.
				virtual void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(BasicRenderer2D& gfx) override;
				void onUpdate(void) override;

				// --- style ---
				inline PianoRollStyle& getStyle(void) { return m_style; }
				inline const PianoRollStyle& getStyle(void) const { return m_style; }
				inline void setStyle(const PianoRollStyle& style) { m_style = style; }
				// Convenience forwarding to getStyle() - the music key determines how both note-
				// name label options (PianoRollStyle's ShowNoteNamesOnFallingNotes/OnKeys) spell
				// each pitch, e.g. GbMajor spells pitch class 11 as "Cb" rather than "B". The
				// state itself lives in the style (so it saves/loads with everything else);
				// this just saves a ".getStyle()" for what's otherwise an ordinary playback-
				// adjacent setting.
				inline void setMusicKey(ostd::MidiParser::eMusicKey key) { m_style.setMusicKey(key); }
				inline ostd::MidiParser::eMusicKey getMusicKey(void) const { return m_style.getMusicKey(); }

				// --- voices ---
				// Parses midiFilePath via ostd::MidiParser and adds it as a new voice. color ==
				// Colors::Transparent (the default) lets PianoRollStyle's per-note/per-key tiers
				// decide the color instead of forcing a flat per-voice one. Returns (u32)-1 and
				// logs a warning if the file can't be parsed.
				// virtual so a subclass (e.g. one adding per-note selection) can keep its own
				// parallel per-voice bookkeeping in sync without duplicating voice management -
				// behavior/signature for every existing caller is unchanged either way.
				virtual u32 loadVoice(const String& midiFilePath, const Color& color = Colors::Transparent, const String& name = "");
				virtual bool removeVoice(u32 index);
				virtual void clearVoices(void);
				inline u32 getVoiceCount(void) const { return (u32)m_voices.size(); }
				Voice& getVoice(u32 index);
				const Voice& getVoice(u32 index) const;
				// rebuild_synth_schedule() keeps the fallback synth's own audible schedule
				// consistent with visibility - without it, muting a voice visually wouldn't mute
				// it audibly whenever the synth (rather than a real audio file) is the one
				// actually sounding.
				inline void setVoiceVisible(u32 index, bool visible) { if (index < m_voices.size()) { m_voices[index].visible = visible; recompute_pressed_keys(getCurrentTimeSeconds()); rebuild_synth_schedule(); } }
				inline void setVoiceColor(u32 index, const Color& color) { if (index < m_voices.size()) { m_voices[index].color = color; recompute_pressed_keys(getCurrentTimeSeconds()); } }

				// --- audio ---
				// WAV or MP3, picked by extension (see osnd::AudioClip::load). Without an audio
				// file loaded, playback still works, timed off an internal clock instead.
				// virtual for the same reason as loadVoice()/removeVoice()/clearVoices() - lets a
				// subclass track its own bookkeeping (e.g. remembering the loaded path) without
				// duplicating audio loading itself.
				virtual bool loadAudio(const String& audioFilePath);
				virtual void clearAudio(void);
				inline bool hasAudio(void) const { return m_audioPlayer && m_audioPlayer->isLoaded(); }

				// --- fallback MIDI synth ---
				// Takes over as the actual sound (and, via has_synth_clock(), timing) source
				// whenever hasAudio() is false and there's at least one voice loaded - the embedded
				// soundfont (osnd::MidiSynth's own default) needs nothing set up to work; these just
				// let a caller point it at a different one instead. See getVoice()/pedalEvents - a
				// real soundfont is only ever swapped, never required.
				bool setSoundfont(const String& soundfontFilePath);
				void useEmbeddedSoundfont(void);
				inline bool hasSynthDevice(void) const { return m_synth && m_synth->hasDevice(); }

				// Whether MIDI pitch (0-127) is currently highlighted on the keyboard - the same
				// state driving draw_keyboard()'s own highlighting, exposed for anything an app
				// might want to build on top (a custom overlay, an external keyboard display, ...).
				inline bool isKeyPressed(i32 pitch) const { return pitch >= 0 && pitch < 128 && m_keyPressed[(size_t)pitch]; }

				// What color MIDI pitch (0-127) is actually drawn with right now - pressed, mid
				// release-fade, or its plain key color - the same thing draw_keyboard() uses,
				// exposed for the same reason as isKeyPressed() (mirroring the keyboard elsewhere).
				inline Color getKeyDisplayColor(i32 pitch) const
				{
					if (pitch < 0 || pitch >= 128)
						return Colors::Transparent;
					return resolve_key_color(pitch, ostd::MidiParser::getNoteInfo(pitch).isBlackKey(), get_raw_time());
				}

				// The full 88-key keyboard's natural ("design") pixel width at the style's own key
				// dimensions, unscaled - i.e. what you'd get if the widget were exactly this wide.
				// The widget itself can be set to any width via setSize(); the keyboard (and key
				// heights, which stay derived from width so proportions aren't distorted) scale up
				// or down from this natural size to exactly fill whatever width it's actually
				// given, the same way the Godot reference scales a fixed preferred layout to fit
				// its canvas. Useful for picking a size that renders at exactly 1:1 (scale == 1).
				f32 getKeyboardWidth(void) const;

			protected:
				// Reused by anything subclassing PianoRoll to add its own per-note overlay (e.g.
				// a selection highlight) without re-deriving PianoRoll's own layout/timing math -
				// if that math ever changes here, a subclass using these picks up the change for
				// free instead of silently drifting out of sync.
				f32 key_x_position(i32 midiPitch, f32 scale) const; // relative to the widget's left edge
				// bounds.w / getKeyboardWidth() - the same scale onDraw() computes for its own
				// draw_keyboard()/draw_falling_notes() calls.
				inline f32 compute_scale(const Rectangle& bounds) const
				{
					f32 naturalWidth = getKeyboardWidth();
					return naturalWidth > 0.0f ? bounds.w / naturalWidth : 1.0f;
				}
				// getCurrentTimeSeconds() - m_startPadSeconds - the same "raw" (file-native,
				// pre-roll-pad-adjusted) clock update_active_notes()/draw_falling_notes()/
				// draw_keyboard() compare note.startTime/endTime against internally.
				inline f64 get_raw_time(void) const { return getCurrentTimeSeconds() - m_startPadSeconds; }
				// Whether note.startTime/endTime puts it inside the current fall-area window
				// right now - the same check update_active_notes() uses to populate m_activeNotes.
				inline bool is_note_in_fall_window(const ostd::MidiParser::NoteEvent& note) const
				{
					f64 now = get_raw_time();
					f64 lookahead = (f64)m_style.getLookaheadSeconds();
					return now >= note.startTime - lookahead && now <= note.endTime;
				}
				// The exact falling-note rectangle draw_falling_notes() draws `note` at, in the
				// same bounds/scale terms onDraw() already computes - the single source of truth
				// for both drawing the falling note and anything hit-testing or highlighting it.
				Rectangle compute_falling_note_rect(const ostd::MidiParser::NoteEvent& note, bool isBlackKey, const Rectangle& bounds, f32 scale) const;

			private:
				// hasAudio() only means "a file decoded successfully" - it can be true with no
				// live output device (unreachable hardware, audio subsystem unavailable, ...),
				// in which case AudioPlayer::getPositionSeconds() never advances on its own.
				// This is the actual "can I trust AudioPlayer as the clock right now" check,
				// used by onUpdate()/getCurrentTimeSeconds() - falls back to the internal clock
				// whenever it's false, even if hasAudio() is true.
				inline bool has_real_audio_clock(void) const { return m_audioPlayer && m_audioPlayer->isLoaded() && m_audioPlayer->hasDevice(); }
				// Same idea, but for the fallback MIDI synth: only ever the live clock when
				// there's no real audio file to prefer instead (m_synth exists lazily, see
				// ensure_synth()/play()).
				inline bool has_synth_clock(void) const { return !hasAudio() && m_synth && m_synth->hasDevice(); }
				// Lazily creates m_synth (first use only) and (re)builds its schedule from every
				// currently visible voice's notes + pedalEvents, flattened and sample-sorted -
				// called whenever the voice list changes or the synth is about to start.
				void ensure_synth(void);
				void rebuild_synth_schedule(void);

			public:

				// --- playback control ---
				void play(void);
				void pause(void);
				void stop(void); // pause + seek to 0
				bool isPlaying(void) const;
				void seekSeconds(f64 seconds);
				inline void seekNormalized(f32 normalized01) { seekSeconds((f64)normalized01 * getTotalDurationSeconds()); }
				f64 getCurrentTimeSeconds(void) const;
				inline f32 getCurrentTimeNormalized(void) const { f64 d = getTotalDurationSeconds(); return d > 0.0 ? (f32)(getCurrentTimeSeconds() / d) : 0.0f; }
				f64 getTotalDurationSeconds(void) const;

				// --- audio volume ---
				// 0..1. Cached locally and re-applied on loadAudio() too, so it can be set before
				// any audio is loaded (or while there's no live device) without losing the setting.
				void setVolume(f32 volume01);
				inline f32 getVolume(void) const { return m_volume; }

				// --- save/load (style + voice list - not transient playback position) ---
				bool saveStateToJson(const String& path) const;
				bool loadStateFromJson(const String& path);

				// Fired with a PianoRollNoteEventData*; MidiStart/MidiEnd fire with nullptr.
				inline static const u32 NoteOnSignal    { ostd::SignalHandler::newCustomSignal(12000) };
				inline static const u32 NoteOffSignal   { ostd::SignalHandler::newCustomSignal(12001) };
				inline static const u32 MidiStartSignal { ostd::SignalHandler::newCustomSignal(12002) };
				inline static const u32 MidiEndSignal   { ostd::SignalHandler::newCustomSignal(12003) };

			private:
				struct ActiveNote
				{
					u32 voiceIndex;
					const ostd::MidiParser::NoteEvent* note; // points into m_voices[voiceIndex].notes - stable (deque-free vector, never reallocated after load)
				};

			private:
				void recompute_total_duration(void);
				void update_active_notes(f64 now, f64 previousNow);
				// Full from-scratch recompute of which keys are pressed at a given instant -
				// needed anywhere state changes discontinuously (a seek, a voice becoming
				// hidden/removed, ...) rather than by a continuous frame-to-frame advance, since
				// update_active_notes()'s NoteOn/NoteOff edge-detection never fires when
				// previousNow == now (a jump) and so never clears/sets stale key state on its own.
				void recompute_pressed_keys(f64 now);
				// scale: bounds.w / getKeyboardWidth() (computed once in onDraw) - lets the whole
				// 88-key layout grow/shrink to whatever width the widget actually has instead of
				// requiring the widget to be exactly getKeyboardWidth() wide. Key heights scale
				// along with it automatically (still width * heightMultiplier), so proportions -
				// including the style's own preferred ones - are preserved rather than distorted.
				void draw_keyboard(BasicRenderer2D& gfx, const Rectangle& bounds, f32 scale);
				void draw_falling_notes(BasicRenderer2D& gfx, const Rectangle& bounds, f32 scale);
				// What color a key should actually be drawn right now - pressed, mid-release-fade,
				// or its plain normal color, in that priority order.
				Color resolve_key_color(i32 pitch, bool isBlack, f64 now) const;
				// Shared by the falling-note and keyboard-key note-name labels - centered
				// horizontally, anchored a small margin above bounds' own bottom edge (so it sits
				// inside the note/key it's labeling, not below it).
				void draw_bottom_centered_label(BasicRenderer2D& gfx, const String& text, const Rectangle& bounds, const Color& color, f32 fontSize, f32 scale);
				void emit_note_signal(u32 signalId, const ostd::MidiParser::NoteEvent& note, u32 voiceIndex);

			private:
				PianoRollStyle m_style;
				stdvec<Voice> m_voices;
				std::unique_ptr<osnd::AudioPlayer> m_audioPlayer;
				// Fallback sound (and, via has_synth_clock(), timing) source - only ever relevant
				// while !hasAudio(); created lazily by ensure_synth() on first actual need (play()/
				// seekSeconds() with voices loaded and no audio file), not at construction, so a
				// PianoRoll that always has real audio never pays for a FluidSynth instance/device.
				std::unique_ptr<osnd::MidiSynth> m_synth;
				// m_fallbackClockSeconds/m_lastUpdateSeconds/update_active_notes()/draw_falling_notes()
				// all work in "raw" time - the MIDI/audio files' own native clock, where 0 is the
				// first actual MIDI/audio sample, same as before this field existed. m_startPadSeconds
				// is the lead-in silence (see recompute_total_duration()) inserted before that: raw
				// time starts at -m_startPadSeconds instead of 0, so the earliest note is already
				// lookaheadSeconds away (fully offscreen) right when getCurrentTimeSeconds() == 0,
				// instead of popping in already-partially-fallen. getCurrentTimeSeconds() (the public,
				// presentation-facing clock that starts at 0 and only ever counts up) is just
				// raw + m_startPadSeconds.
				f64 m_startPadSeconds { 0.0 };
				// Whether whichever live clock is actually in play right now (m_audioPlayer, or
				// m_synth when there's no audio file) has had its own position trustworthy -
				// false during the pre-roll pad (before it's ever been positioned/started this
				// session), in which case raw time is tracked by m_fallbackClockSeconds instead,
				// same as when there's no live clock of either kind at all.
				bool m_audioStarted { false };
				f64 m_fallbackClockSeconds { 0.0 }; // raw time; used whenever audio isn't (yet) driving it
				ostd::Counter m_updateClock; // drives m_fallbackClockSeconds's delta time each frame
				bool m_playing { false };
				f64 m_totalDurationSeconds { 0.0 }; // presentation time (pad-inclusive)
				f64 m_lastUpdateSeconds { 0.0 }; // raw time; for edge-detecting note-on/off and MidiStart/MidiEnd between frames
				bool m_midiStartFired { false };
				f32 m_volume { 1.0f };
				stdvec<ActiveNote> m_activeNotes;
				// Per-pitch key-pressed state for keyboard highlighting - index by MIDI pitch directly.
				stdvec<bool> m_keyPressed;       // index = MIDI pitch
				stdvec<Color> m_pressedKeyColor; // index = MIDI pitch; meaningful only where m_keyPressed is true
				// Pressed-key release fade (style.isUseKeyFadeEnabled()/getKeyFadeDurationSeconds())
				// - set at the moment a key releases (see update_active_notes()), read back at draw
				// time in draw_keyboard(). A seek/voice change (recompute_pressed_keys()) clears
				// these outright instead of trying to animate across a discontinuous jump.
				stdvec<bool> m_keyFading;         // index = MIDI pitch; a fade-out is in progress
				stdvec<f64> m_keyReleaseTime;     // raw time of release; meaningful only where m_keyFading is true
				stdvec<Color> m_keyReleaseColor;  // color shown right before release; meaningful only where m_keyFading is true
		};
	}
}
