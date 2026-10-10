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
			};

			public:
				inline PianoRoll(Window& window) : Widget({ 0, 0, 0, 0 }, window) { create(); }
				PianoRoll& create(void);
				void applyTheme(const ostd::Stylesheet& theme) override;
				void onDraw(BasicRenderer2D& gfx) override;
				void onUpdate(void) override;

				// --- style ---
				inline PianoRollStyle& getStyle(void) { return m_style; }
				inline const PianoRollStyle& getStyle(void) const { return m_style; }
				inline void setStyle(const PianoRollStyle& style) { m_style = style; }

				// --- voices ---
				// Parses midiFilePath via ostd::MidiParser and adds it as a new voice. color ==
				// Colors::Transparent (the default) lets PianoRollStyle's per-note/per-key tiers
				// decide the color instead of forcing a flat per-voice one. Returns (u32)-1 and
				// logs a warning if the file can't be parsed.
				u32 loadVoice(const String& midiFilePath, const Color& color = Colors::Transparent, const String& name = "");
				bool removeVoice(u32 index);
				void clearVoices(void);
				inline u32 getVoiceCount(void) const { return (u32)m_voices.size(); }
				Voice& getVoice(u32 index);
				const Voice& getVoice(u32 index) const;
				inline void setVoiceVisible(u32 index, bool visible) { if (index < m_voices.size()) { m_voices[index].visible = visible; recompute_pressed_keys(getCurrentTimeSeconds()); } }
				inline void setVoiceColor(u32 index, const Color& color) { if (index < m_voices.size()) { m_voices[index].color = color; recompute_pressed_keys(getCurrentTimeSeconds()); } }

				// --- audio ---
				// WAV or MP3, picked by extension (see osnd::AudioClip::load). Without an audio
				// file loaded, playback still works, timed off an internal clock instead.
				bool loadAudio(const String& audioFilePath);
				void clearAudio(void);
				inline bool hasAudio(void) const { return m_audioPlayer && m_audioPlayer->isLoaded(); }

				// Whether MIDI pitch (0-127) is currently highlighted on the keyboard - the same
				// state driving draw_keyboard()'s own highlighting, exposed for anything an app
				// might want to build on top (a custom overlay, an external keyboard display, ...).
				inline bool isKeyPressed(i32 pitch) const { return pitch >= 0 && pitch < 128 && m_keyPressed[(size_t)pitch]; }

				// The full 88-key keyboard's natural ("design") pixel width at the style's own key
				// dimensions, unscaled - i.e. what you'd get if the widget were exactly this wide.
				// The widget itself can be set to any width via setSize(); the keyboard (and key
				// heights, which stay derived from width so proportions aren't distorted) scale up
				// or down from this natural size to exactly fill whatever width it's actually
				// given, the same way the Godot reference scales a fixed preferred layout to fit
				// its canvas. Useful for picking a size that renders at exactly 1:1 (scale == 1).
				f32 getKeyboardWidth(void) const;

			private:
				// hasAudio() only means "a file decoded successfully" - it can be true with no
				// live output device (unreachable hardware, audio subsystem unavailable, ...),
				// in which case AudioPlayer::getPositionSeconds() never advances on its own.
				// This is the actual "can I trust AudioPlayer as the clock right now" check,
				// used by onUpdate()/getCurrentTimeSeconds() - falls back to the internal clock
				// whenever it's false, even if hasAudio() is true.
				inline bool has_live_audio_clock(void) const { return m_audioPlayer && m_audioPlayer->isLoaded() && m_audioPlayer->hasDevice(); }

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
				f32 key_x_position(i32 midiPitch, f32 scale) const; // relative to the widget's left edge
				void emit_note_signal(u32 signalId, const ostd::MidiParser::NoteEvent& note, u32 voiceIndex);

			private:
				PianoRollStyle m_style;
				stdvec<Voice> m_voices;
				std::unique_ptr<osnd::AudioPlayer> m_audioPlayer;
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
				// Whether m_audioPlayer's own position is trustworthy right now - false during the
				// pre-roll pad (before the device has ever been positioned/started for this session),
				// in which case raw time is tracked by m_fallbackClockSeconds instead, same as when
				// there's no live audio clock at all.
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
		};
	}
}
