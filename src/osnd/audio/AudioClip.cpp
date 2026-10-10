#include "AudioClip.hpp"
#include <ostd/io/Logger.hpp>
#include "../vendor/dr_mp3.h"
#include <filesystem>
#include <cstring>
#include <cctype>

namespace osnd
{
	AudioClip AudioClip::load(const String& path)
	{
		std::filesystem::path p(path.cpp_str());
		String ext = p.extension().string();
		for (auto& c : ext)
			c = (char)std::tolower((unsigned char)c);
		if (ext == ".mp3")
			return loadMP3(path);
		return loadWAV(path); // default/fallback - also correct for actual .wav files
	}

	AudioClip AudioClip::loadWAV(const String& path)
	{
		AudioClip clip;
		SDL_AudioSpec spec;
		u8* buffer = nullptr;
		u32 bufferLen = 0;
		if (!SDL_LoadWAV(path.c_str(), &spec, &buffer, &bufferLen))
		{
			OX_ERROR("osnd::AudioClip: failed to load WAV \"%s\": %s", path.c_str(), SDL_GetError());
			return clip;
		}
		clip.m_format = spec.format;
		clip.m_channels = (u32)spec.channels;
		clip.m_sampleRate = (u32)spec.freq;
		clip.m_pcm.assign(buffer, buffer + bufferLen);
		SDL_free(buffer);
		u32 bytesPerFrame = clip.getBytesPerFrame();
		clip.m_frameCount = bytesPerFrame > 0 ? (u64)clip.m_pcm.size() / bytesPerFrame : 0;
		return clip;
	}

	AudioClip AudioClip::loadMP3(const String& path)
	{
		AudioClip clip;
		drmp3_config config;
		drmp3_uint64 frameCount = 0;
		drmp3_int16* pcm = drmp3_open_file_and_read_pcm_frames_s16(path.c_str(), &config, &frameCount, nullptr);
		if (!pcm)
		{
			OX_ERROR("osnd::AudioClip: failed to decode MP3 \"%s\"", path.c_str());
			return clip;
		}
		clip.m_format = SDL_AUDIO_S16;
		clip.m_channels = config.channels;
		clip.m_sampleRate = config.sampleRate;
		clip.m_frameCount = (u64)frameCount;
		u64 byteCount = (u64)frameCount * clip.getBytesPerFrame();
		clip.m_pcm.resize(byteCount);
		std::memcpy(clip.m_pcm.data(), pcm, byteCount);
		drmp3_free(pcm, nullptr);
		return clip;
	}
}
