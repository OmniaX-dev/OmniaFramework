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

#include <ostd/data/Types.hpp>
#include <ostd/string/String.hpp>
#include <atomic>
#include <mutex>

// Forward declarations matching FluidSynth's own typedef chain (fluidsynth/types.h) and SDL3's
// own struct tag exactly, so MidiSynth.cpp's later #include <fluidsynth.h>/<SDL3/SDL_audio.h>
// redeclare the identical types rather than colliding with different ones - keeps this header
// itself free of both heavier includes, same as AudioPlayer.hpp does for SDL_AudioStream.
struct SDL_AudioStream;
typedef struct _fluid_hashtable_t fluid_settings_t;
typedef struct _fluid_synth_t fluid_synth_t;

namespace osnd
{
	// Generic, widget-agnostic MIDI playback via FluidSynth - renders through
	// fluid_synth_write_float into an SDL3 audio device stream opened in callback mode (never
	// FluidSynth's own audio/MIDI drivers - CMakeLists.txt disables all of those at the build
	// level). Defaults to the soundfont embedded in UprightPianoKWSF.hpp/.cpp, loaded straight
	// from memory via a custom FluidSynth VIO loader (no temp file); setSoundfont() switches to
	// a real file instead.
	//
	// Playback is driven by a caller-supplied, sample-accurate schedule (setSchedule()) rather
	// than anything resembling a live MIDI input - the audio callback (feed()) renders in
	// segments bounded by each due event's own sample offset, applies it (noteon/noteoff/cc),
	// and continues, entirely on the audio thread with no other-thread timers. This is what
	// PianoRoll uses as its fallback sound source when no real audio file is loaded; see its
	// own integration for how a set of voices becomes one flat schedule.
	//
	// noteOn()/noteOff()/allNotesOff() bypass the schedule entirely for one-shot use (FluidSynth
	// itself is safe to call concurrently with rendering - this is its documented real-time MIDI
	// input pattern) - not wired into anything yet, but here for PianoRollEditor's future single-
	// note/selection audition work.
	class MidiSynth
	{
		public:
			struct Event
			{
				enum class Type { NoteOn, NoteOff, ControlChange };
				u64 sampleOffset { 0 }; // absolute frame index from the schedule's own start
				Type type { Type::NoteOn };
				i32 channel { 0 };
				i32 param1 { 0 }; // pitch, or controller number
				i32 param2 { 0 }; // velocity, or controller value - unused for NoteOff
			};

		public:
			MidiSynth(void);
			~MidiSynth(void);

			// Real soundfont file on disk - unloads whatever was loaded before (embedded or
			// not) only once the new one has loaded successfully. False (and nothing changed)
			// if filePath can't be loaded as an SF2.
			bool setSoundfont(const String& filePath);
			// Back to the resource embedded in the binary - this is also the state right after
			// construction, so most callers never need to call this explicitly.
			void useEmbeddedSoundfont(void);
			// True only if a real output device is actually open - mirrors AudioPlayer::hasDevice().
			inline bool hasDevice(void) const { return m_stream != nullptr; }

			// Replaces the whole playback schedule and resets position to 0 (seek afterward if
			// resuming somewhere other than the start, e.g. because PianoRoll is already mid-
			// playback when its voice list changes). events need not be pre-sorted.
			void setSchedule(stdvec<Event> events, u64 totalSampleCount);
			void clearSchedule(void);

			void play(void);
			void pause(void);
			void stop(void); // pause + all-notes-off + pedal reset + seek to 0
			bool isPlaying(void) const;

			// Jumps the cursor and re-settles synth state for that instant: silences whatever
			// was left sounding from before the jump (events it's skipping past are never
			// individually replayed), then re-sends the latest sustain/sostenuto/soft (CC64/66/
			// 67) value per channel as of this point, since those are otherwise lost the same
			// way - directly implements "send current pedal state when starting mid-piece".
			void seekSamples(u64 sampleOffset);
			inline void seekSeconds(f64 seconds) { seekSamples(seconds > 0.0 ? (u64)(seconds * (f64)SampleRate) : 0); }
			inline i64 getPositionSamples(void) const { return m_frameCursor.load(std::memory_order_relaxed); }
			// Wall-clock interpolated between audio-callback cursor updates, same pattern as
			// AudioPlayer::getPositionSeconds().
			f64 getPositionSeconds(void) const;

			void setVolume(f32 volume01);
			f32 getVolume(void) const;

			// Raw FluidSynth output gain (0-10, unlike setVolume()'s 0-1 SDL stream gain) -
			// normalize_gain() sets this automatically on every soundfont load to compensate
			// for how differently soundfonts are sampled (e.g. Salamander's samples are
			// recorded much quieter than the embedded upright piano's), but it's exposed here
			// too in case a caller wants to nudge it manually afterward.
			void setGain(f32 gain);
			f32 getGain(void) const;

			// One-shot, bypass-the-schedule helpers - see the class comment.
			void noteOn(i32 channel, i32 pitch, i32 velocity);
			void noteOff(i32 channel, i32 pitch);
			void allNotesOff(void); // also resets CC64/66/67 to 0 on every channel

			static constexpr u32 SampleRate = 44100;

		private:
			static void sdl_feed_callback(void* userdata, struct SDL_AudioStream* stream, int additionalAmount, int totalAmount);
			void feed(struct SDL_AudioStream* stream, int additionalAmount);
			void apply_event(const Event& event);
			// Loads the soundfont named by m_currentSoundfontPath into a throwaway fluid_synth_t
			// (never m_synth itself), plays a reference note on it, measures its peak output, and
			// sets m_synth's own gain so every soundfont settles near the same perceived loudness
			// - this is what actually fixes "soundfont X sounds quieter than soundfont Y", since
			// that's a property of how each soundfont's samples were recorded, not something a
			// single fixed default gain (or the master volume) can compensate for on its own.
			// Measuring on a disposable synth instance, rather than noteon/noteoff-ing m_synth
			// itself, means there's never any real voice state to silence afterward - no click
			// from killing a voice mid-amplitude, no release tail that could bleed into whatever
			// plays next.
			void normalize_gain(void);

		private:
			fluid_settings_t* m_settings { nullptr };
			fluid_synth_t* m_synth { nullptr };
			int m_currentSoundfontId { -1 };
			// Whatever was last loaded (the embedded sentinel path, or a real setSoundfont() file
			// path) - kept around purely so normalize_gain() can load the same soundfont into its
			// own throwaway synth instance.
			String m_currentSoundfontPath;
			struct SDL_AudioStream* m_stream { nullptr };

			// Guards m_schedule/m_nextEventIndex/m_totalSamples across the UI thread
			// (setSchedule/seekSamples/clearSchedule) and the audio thread (feed()) - the only
			// cross-thread synchronization this class needs, since UI-thread calls are rare
			// (a seek, a schedule swap) and feed()'s own hold time is just one buffer's worth of
			// rendering.
			mutable std::mutex m_scheduleMutex;
			stdvec<Event> m_schedule; // sorted by sampleOffset
			size_t m_nextEventIndex { 0 };
			u64 m_totalSamples { 0 };

			std::atomic<i64> m_frameCursor { 0 };
			std::atomic<i64> m_cursorUpdateTimeNs { 0 };
			bool m_playing { false };
	};
}
