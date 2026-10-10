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
#include <SDL3/SDL_audio.h>
#include <algorithm>

namespace osnd
{
	// A whole audio file, fully decoded to PCM in memory - deliberately not a streaming
	// decoder. Playback files for this use case (a recorded piano performance) are short
	// enough that decoding up front is cheap, and it's what makes AudioPlayer's seeking exact
	// and instant (a seek is just moving a read cursor over an already-decoded buffer, never a
	// re-decode or an approximate wall-clock offset).
	class AudioClip
	{
		public:
			// Dispatches to loadWAV/loadMP3 by file extension. Returns an empty (isValid()==
			// false) clip on failure - check isValid() rather than relying on exceptions.
			static AudioClip load(const String& path);
			static AudioClip loadWAV(const String& path);  // SDL3's own built-in decoder
			static AudioClip loadMP3(const String& path);  // vendored dr_mp3 (see src/osnd/vendor/)

			inline bool isValid(void) const { return !m_pcm.empty() && m_channels > 0 && m_sampleRate > 0; }
			inline SDL_AudioFormat getFormat(void) const { return m_format; }
			inline u32 getChannels(void) const { return m_channels; }
			inline u32 getSampleRate(void) const { return m_sampleRate; }
			inline u64 getFrameCount(void) const { return m_frameCount; }
			inline f64 getDurationSeconds(void) const { return m_sampleRate > 0 ? (f64)m_frameCount / (f64)m_sampleRate : 0.0; }
			inline u32 getBytesPerFrame(void) const { return SDL_AUDIO_BYTESIZE(m_format) * m_channels; }

			// Raw PCM access for AudioPlayer's feed callback - frameIndex/frameCount are in
			// audio frames (one sample per channel), not bytes.
			inline const u8* getFrameData(u64 frameIndex) const { return m_pcm.data() + (frameIndex * getBytesPerFrame()); }
			inline u64 clampFrameCount(u64 frameIndex, u64 requestedFrames) const
			{
				return (frameIndex >= m_frameCount) ? 0 : std::min<u64>(requestedFrames, m_frameCount - frameIndex);
			}

		private:
			SDL_AudioFormat m_format { SDL_AUDIO_S16 };
			u32 m_channels { 0 };
			u32 m_sampleRate { 0 };
			u64 m_frameCount { 0 };
			stdvec<u8> m_pcm;
	};
}
