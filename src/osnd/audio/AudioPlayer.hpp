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

#include "AudioClip.hpp"
#include <atomic>

namespace osnd
{
	// Plays one AudioClip through an SDL3 audio device stream opened in callback mode - SDL's
	// own audio thread pulls PCM from the clip at m_frameCursor whenever it needs more, so
	// there's no per-frame manual push from the render/main thread. m_frameCursor is the single
	// piece of state shared between threads (atomic; written on the audio thread, read on the
	// main thread for sync), which is what makes PianoRoll's "where are we right now" query
	// continuous and drift-free instead of the wall-clock-aligned-once approach both reference
	// implementations used.
	class AudioPlayer
	{
		public:
			~AudioPlayer(void);

			// Takes ownership of clip (moved) and opens the device stream, stopped/paused.
			// False on failure (device open failed) - check isLoaded() either way.
			bool load(AudioClip clip);
			void unload(void);
			inline bool isLoaded(void) const { return m_clip.isValid(); }
			// True only if a real output device is actually open - isLoaded() can be true (the
			// file decoded fine) while this is false (no device reachable), in which case
			// getPositionSamples() will never advance on its own; callers driving a visual sync
			// off this player should check hasDevice(), not just isLoaded(), before trusting it
			// as an advancing clock.
			inline bool hasDevice(void) const { return m_stream != nullptr; }

			void play(void);
			void pause(void);
			inline void stop(void) { pause(); seekSamples(0); }
			bool isPlaying(void) const;

			void seekSeconds(f64 seconds);
			void seekSamples(i64 frameIndex);
			// Interpolated, not just the raw frame cursor - the cursor itself only advances when
			// SDL's audio callback delivers a new chunk (every ~20-40ms depending on the device),
			// so reading it directly for a per-frame visual sync produces a stepped/frozen-then-
			// jumping motion rather than smooth continuous movement. Between cursor updates, this
			// extrapolates forward using the wall clock since the last update, snapping back to
			// the real (sample-accurate) value every time a new chunk actually lands.
			f64 getPositionSeconds(void) const;
			inline i64 getPositionSamples(void) const { return m_frameCursor.load(std::memory_order_relaxed); }
			inline f64 getDurationSeconds(void) const { return m_clip.getDurationSeconds(); }

			void setVolume(f32 volume01);
			f32 getVolume(void) const;

		private:
			static void sdl_feed_callback(void* userdata, struct SDL_AudioStream* stream, int additionalAmount, int totalAmount);
			void feed(struct SDL_AudioStream* stream, int additionalAmount);

		private:
			AudioClip m_clip;
			struct SDL_AudioStream* m_stream { nullptr };
			std::atomic<i64> m_frameCursor { 0 };
			std::atomic<i64> m_cursorUpdateTimeNs { 0 }; // steady_clock timestamp of the last m_frameCursor write - see getPositionSeconds()
			bool m_playing { false };
	};
}
