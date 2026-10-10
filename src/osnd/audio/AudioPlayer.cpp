#include "AudioPlayer.hpp"
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include <algorithm>
#include <chrono>

namespace osnd
{
	namespace
	{
		// osnd doesn't depend on ogfx (and ogfx's own SDL init only requests SDL_INIT_VIDEO -
		// SDL3 does not auto-initialize other subsystems on first use), so AudioPlayer has to be
		// self-sufficient here. Safe to call repeatedly/from multiple AudioPlayers - SDL
		// reference-counts subsystem init, and the one-time check itself is thread-safe (C++11
		// magic statics).
		void ensure_audio_subsystem(void)
		{
			static const bool initialized = []() {
				if (!SDL_WasInit(SDL_INIT_AUDIO))
					SDL_InitSubSystem(SDL_INIT_AUDIO);
				return true;
			}();
			(void)initialized;
		}

		inline i64 now_ns(void)
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
		}
	}

	AudioPlayer::~AudioPlayer(void)
	{
		unload();
	}

	bool AudioPlayer::load(AudioClip clip)
	{
		unload();
		if (!clip.isValid())
			return false;
		ensure_audio_subsystem();
		m_clip = std::move(clip);

		SDL_AudioSpec spec;
		spec.format = m_clip.getFormat();
		spec.channels = (int)m_clip.getChannels();
		spec.freq = (int)m_clip.getSampleRate();
		m_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, &AudioPlayer::sdl_feed_callback, this);
		m_frameCursor.store(0, std::memory_order_relaxed);
		m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);
		m_playing = false;
		// Deliberately keep m_clip even if the device failed to open (no hardware reachable,
		// temporarily busy, ...) - duration/position/seek bookkeeping stays valid and queryable
		// either way; only actual audio output needs a live stream. play()/pause() already
		// no-op safely without one.
		return m_stream != nullptr;
	}

	void AudioPlayer::unload(void)
	{
		if (m_stream)
		{
			SDL_DestroyAudioStream(m_stream); // also stops/unbinds the device - safe to call while not inside the feed callback
			m_stream = nullptr;
		}
		m_clip = AudioClip();
		m_frameCursor.store(0, std::memory_order_relaxed);
		m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);
		m_playing = false;
	}

	void AudioPlayer::play(void)
	{
		if (!m_stream)
			return;
		SDL_ResumeAudioStreamDevice(m_stream);
		// Resets the interpolation baseline to right now - otherwise getPositionSeconds() would
		// extrapolate from whenever the cursor last moved (possibly long before this pause/resume),
		// overshooting until the next audio callback corrects it.
		m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);
		m_playing = true;
	}

	void AudioPlayer::pause(void)
	{
		if (!m_stream)
			return;
		SDL_PauseAudioStreamDevice(m_stream);
		m_playing = false;
	}

	bool AudioPlayer::isPlaying(void) const
	{
		return m_playing;
	}

	void AudioPlayer::seekSeconds(f64 seconds)
	{
		seekSamples((i64)(seconds * (f64)m_clip.getSampleRate()));
	}

	void AudioPlayer::seekSamples(i64 frameIndex)
	{
		if (frameIndex < 0) frameIndex = 0;
		i64 total = (i64)m_clip.getFrameCount();
		if (frameIndex > total) frameIndex = total;
		if (m_stream)
			SDL_ClearAudioStream(m_stream); // drop whatever was already queued from the old position
		m_frameCursor.store(frameIndex, std::memory_order_relaxed);
		m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);
	}

	f64 AudioPlayer::getPositionSeconds(void) const
	{
		if (m_clip.getSampleRate() <= 0)
			return 0.0;
		f64 base = (f64)getPositionSamples() / (f64)m_clip.getSampleRate();
		if (!m_playing)
			return base;
		f64 elapsedSeconds = (f64)(now_ns() - m_cursorUpdateTimeNs.load(std::memory_order_relaxed)) / 1e9;
		if (elapsedSeconds < 0.0)
			elapsedSeconds = 0.0; // shouldn't happen, but a clock/ordering hiccup shouldn't run time backwards
		return std::min(base + elapsedSeconds, getDurationSeconds());
	}

	void AudioPlayer::setVolume(f32 volume01)
	{
		if (m_stream)
			SDL_SetAudioStreamGain(m_stream, std::clamp(volume01, 0.0f, 1.0f));
	}

	f32 AudioPlayer::getVolume(void) const
	{
		return m_stream ? SDL_GetAudioStreamGain(m_stream) : 1.0f;
	}

	void AudioPlayer::sdl_feed_callback(void* userdata, SDL_AudioStream* stream, int additionalAmount, int totalAmount)
	{
		static_cast<AudioPlayer*>(userdata)->feed(stream, additionalAmount);
	}

	void AudioPlayer::feed(SDL_AudioStream* stream, int additionalAmount)
	{
		if (additionalAmount <= 0 || !m_clip.isValid())
			return;
		u32 bytesPerFrame = m_clip.getBytesPerFrame();
		if (bytesPerFrame == 0)
			return;

		u64 requestedFrames = (u64)additionalAmount / bytesPerFrame;
		i64 cursor = m_frameCursor.load(std::memory_order_relaxed);
		u64 availableFrames = m_clip.clampFrameCount((u64)cursor, requestedFrames);

		if (availableFrames > 0)
		{
			SDL_PutAudioStreamData(stream, m_clip.getFrameData((u64)cursor), (int)(availableFrames * bytesPerFrame));
			m_frameCursor.store(cursor + (i64)availableFrames, std::memory_order_relaxed);
			m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);
		}

		// Past the end of the clip (or the clip is shorter than one callback's worth of data) -
		// feed silence instead of leaving SDL waiting, so playback stops cleanly rather than
		// glitching; the frame cursor simply stops advancing past getFrameCount().
		u64 remainingFrames = requestedFrames - availableFrames;
		if (remainingFrames > 0)
		{
			stdvec<u8> silence(remainingFrames * bytesPerFrame, 0);
			SDL_PutAudioStreamData(stream, silence.data(), (int)silence.size());
		}
	}
}
