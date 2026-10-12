#include "MidiSynth.hpp"
#include <osnd/resources/UprightPianoKWSF.hpp>
#include <fluidsynth.h>
#include <SDL3/SDL_audio.h>
#include <SDL3/SDL_init.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <cstdio>

namespace osnd
{
	namespace
	{
		// osnd doesn't depend on ogfx (and ogfx's own SDL init only requests SDL_INIT_VIDEO -
		// SDL3 does not auto-initialize other subsystems on first use), so this has to be self-
		// sufficient here, same as AudioPlayer's own copy of this helper.
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

		// A virtual "filename" that only embedded_sf_open() ever recognizes - any other path
		// (a real setSoundfont() call) falls straight through to FluidSynth's own always-
		// registered default file loader, since embedded_sf_open() returns nullptr (meaning
		// "I can't open this") for anything else.
		constexpr const char* kEmbeddedVirtualPath = "__osnd_embedded_upright_piano__";

		struct EmbeddedSfHandle { const unsigned char* data; size_t size; size_t pos; };

		void* embedded_sf_open(const char* filename)
		{
			if (filename == nullptr || std::strcmp(filename, kEmbeddedVirtualPath) != 0)
				return nullptr;
			return new EmbeddedSfHandle{ osnd::upright_piano_kw_sf2, osnd::upright_piano_kw_sf2_size, 0 };
		}

		int embedded_sf_read(void* buf, fluid_long_long_t count, void* handle)
		{
			auto* h = static_cast<EmbeddedSfHandle*>(handle);
			if (count < 0 || h->pos + (size_t)count > h->size)
				return FLUID_FAILED;
			std::memcpy(buf, h->data + h->pos, (size_t)count);
			h->pos += (size_t)count;
			return FLUID_OK;
		}

		int embedded_sf_seek(void* handle, fluid_long_long_t offset, int origin)
		{
			auto* h = static_cast<EmbeddedSfHandle*>(handle);
			i64 base = origin == SEEK_SET ? 0 : origin == SEEK_CUR ? (i64)h->pos : (i64)h->size;
			i64 newPos = base + offset;
			if (newPos < 0 || (size_t)newPos > h->size)
				return FLUID_FAILED;
			h->pos = (size_t)newPos;
			return FLUID_OK;
		}

		fluid_long_long_t embedded_sf_tell(void* handle)
		{
			return (fluid_long_long_t)static_cast<EmbeddedSfHandle*>(handle)->pos;
		}

		int embedded_sf_close(void* handle)
		{
			delete static_cast<EmbeddedSfHandle*>(handle);
			return FLUID_OK;
		}

		// Sustain, sostenuto, soft - the only controllers seeksamples() needs to re-derive and
		// resend when jumping mid-piece (see MidiSynth::seekSamples()).
		constexpr i32 kPedalControllers[3] = { 64, 66, 67 };
	}

	MidiSynth::MidiSynth(void)
	{
		ensure_audio_subsystem();

		m_settings = new_fluid_settings();
		fluid_settings_setnum(m_settings, "synth.sample-rate", (double)SampleRate);
		fluid_settings_setint(m_settings, "synth.polyphony", 256);
		m_synth = new_fluid_synth(m_settings);

		// Registered once - embedded_sf_open() only ever matches kEmbeddedVirtualPath, so this
		// never intercepts a real setSoundfont() file path.
		fluid_sfloader_t* loader = new_fluid_defsfloader(m_settings);
		fluid_sfloader_set_callbacks(loader, embedded_sf_open, embedded_sf_read, embedded_sf_seek, embedded_sf_tell, embedded_sf_close);
		fluid_synth_add_sfloader(m_synth, loader); // synth takes ownership
		useEmbeddedSoundfont();

		SDL_AudioSpec spec;
		spec.format = SDL_AUDIO_F32;
		spec.channels = 2;
		spec.freq = (int)SampleRate;
		m_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, &MidiSynth::sdl_feed_callback, this);
	}

	MidiSynth::~MidiSynth(void)
	{
		if (m_stream)
			SDL_DestroyAudioStream(m_stream);
		if (m_synth)
			delete_fluid_synth(m_synth);
		if (m_settings)
			delete_fluid_settings(m_settings);
	}

	void MidiSynth::useEmbeddedSoundfont(void)
	{
		if (!m_synth)
			return;
		int id = fluid_synth_sfload(m_synth, kEmbeddedVirtualPath, 1);
		if (id < 0)
			return;
		if (m_currentSoundfontId >= 0)
			fluid_synth_sfunload(m_synth, m_currentSoundfontId, true);
		m_currentSoundfontId = id;
		normalize_gain();
	}

	bool MidiSynth::setSoundfont(const String& filePath)
	{
		if (!m_synth)
			return false;
		int id = fluid_synth_sfload(m_synth, filePath.c_str(), 1);
		if (id < 0)
			return false;
		if (m_currentSoundfontId >= 0)
			fluid_synth_sfunload(m_synth, m_currentSoundfontId, true);
		m_currentSoundfontId = id;
		normalize_gain();
		return true;
	}

	void MidiSynth::setSchedule(stdvec<Event> events, u64 totalSampleCount)
	{
		std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) { return a.sampleOffset < b.sampleOffset; });
		std::lock_guard<std::mutex> lock(m_scheduleMutex);
		m_schedule = std::move(events);
		m_totalSamples = totalSampleCount;
		m_nextEventIndex = 0;
		m_frameCursor.store(0, std::memory_order_relaxed);
		m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);
	}

	void MidiSynth::clearSchedule(void)
	{
		std::lock_guard<std::mutex> lock(m_scheduleMutex);
		m_schedule.clear();
		m_nextEventIndex = 0;
		m_totalSamples = 0;
		m_frameCursor.store(0, std::memory_order_relaxed);
		m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);
	}

	void MidiSynth::play(void)
	{
		if (!m_stream)
			return;
		SDL_ResumeAudioStreamDevice(m_stream);
		m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);
		m_playing = true;
	}

	void MidiSynth::pause(void)
	{
		if (!m_stream)
			return;
		SDL_PauseAudioStreamDevice(m_stream);
		m_playing = false;
	}

	void MidiSynth::stop(void)
	{
		pause();
		allNotesOff();
		seekSamples(0);
	}

	bool MidiSynth::isPlaying(void) const
	{
		return m_playing;
	}

	void MidiSynth::seekSamples(u64 sampleOffset)
	{
		std::lock_guard<std::mutex> lock(m_scheduleMutex);
		if (sampleOffset > m_totalSamples)
			sampleOffset = m_totalSamples;

		// Silence whatever was left sounding from before the jump - the events between the old
		// and new position are being skipped outright, never individually applied, so any note
		// still held at the old position would otherwise keep ringing forever.
		if (m_synth)
			fluid_synth_all_notes_off(m_synth, -1);

		size_t newIndex = (size_t)(std::lower_bound(m_schedule.begin(), m_schedule.end(), sampleOffset,
			[](const Event& e, u64 off) { return e.sampleOffset < off; }) - m_schedule.begin());

		// Re-derive and resend the latest sustain/sostenuto/soft value per channel as of this
		// point - those ControlChange events before newIndex are being skipped the same way
		// note events are, so without this a pedal already held at the jump target would
		// silently come back up instead of staying down.
		if (m_synth)
		{
			i32 latestValue[16][3];
			for (auto& row : latestValue)
				for (auto& v : row)
					v = -1;
			for (size_t i = 0; i < newIndex; ++i)
			{
				const Event& e = m_schedule[i];
				if (e.type != Event::Type::ControlChange || e.channel < 0 || e.channel >= 16)
					continue;
				for (i32 p = 0; p < 3; ++p)
					if (e.param1 == kPedalControllers[p])
						latestValue[e.channel][p] = e.param2;
			}
			for (i32 ch = 0; ch < 16; ++ch)
				for (i32 p = 0; p < 3; ++p)
					if (latestValue[ch][p] >= 0)
						fluid_synth_cc(m_synth, ch, kPedalControllers[p], latestValue[ch][p]);
		}

		m_nextEventIndex = newIndex;
		m_frameCursor.store((i64)sampleOffset, std::memory_order_relaxed);
		m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);
	}

	f64 MidiSynth::getPositionSeconds(void) const
	{
		f64 base = (f64)getPositionSamples() / (f64)SampleRate;
		if (!m_playing)
			return base;
		f64 elapsedSeconds = (f64)(now_ns() - m_cursorUpdateTimeNs.load(std::memory_order_relaxed)) / 1e9;
		if (elapsedSeconds < 0.0)
			elapsedSeconds = 0.0;
		std::lock_guard<std::mutex> lock(m_scheduleMutex);
		f64 total = (f64)m_totalSamples / (f64)SampleRate;
		return std::min(base + elapsedSeconds, total);
	}

	void MidiSynth::setVolume(f32 volume01)
	{
		if (m_stream)
			SDL_SetAudioStreamGain(m_stream, std::clamp(volume01, 0.0f, 1.0f));
	}

	f32 MidiSynth::getVolume(void) const
	{
		return m_stream ? SDL_GetAudioStreamGain(m_stream) : 1.0f;
	}

	void MidiSynth::setGain(f32 gain)
	{
		if (m_synth)
			fluid_synth_set_gain(m_synth, std::clamp(gain, 0.0f, 10.0f));
	}

	f32 MidiSynth::getGain(void) const
	{
		return m_synth ? fluid_synth_get_gain(m_synth) : 1.0f;
	}

	void MidiSynth::normalize_gain(void)
	{
		if (!m_synth)
			return;

		bool wasPlaying = m_playing;
		if (m_stream)
			SDL_PauseAudioStreamDevice(m_stream); // keep the measurement note (and the
												   // all-notes-off around it) inaudible

		constexpr f32 targetPeak = 0.5f; // -6dBFS, leaves headroom so real polyphony doesn't clip
		constexpr int measureFrames = (int)(SampleRate * 0.3); // 300ms - long enough to catch the attack
		{
			// Serializes against feed()'s own fluid_synth_write_float calls on the audio
			// thread - concurrent renders on the same synth are not safe, and this reuses the
			// same mutex feed() already holds for its whole render.
			std::lock_guard<std::mutex> lock(m_scheduleMutex);
			fluid_synth_set_gain(m_synth, 1.0f); // measure at a neutral reference gain
			fluid_synth_all_notes_off(m_synth, -1);
			fluid_synth_noteon(m_synth, 0, 60, 100); // middle C, moderate velocity - representative
			stdvec<float> scratch((size_t)measureFrames * 2);
			fluid_synth_write_float(m_synth, measureFrames, scratch.data(), 0, 2, scratch.data(), 1, 2);
			fluid_synth_noteoff(m_synth, 0, 60);
			fluid_synth_all_notes_off(m_synth, -1);

			f32 peak = 0.0f;
			for (f32 sample : scratch)
				peak = std::max(peak, std::fabs(sample));
			f32 gain = peak > 0.001f ? (targetPeak / peak) : 1.0f;
			fluid_synth_set_gain(m_synth, std::clamp(gain, 0.1f, 8.0f));
		}

		if (wasPlaying && m_stream)
			SDL_ResumeAudioStreamDevice(m_stream);
	}

	void MidiSynth::noteOn(i32 channel, i32 pitch, i32 velocity)
	{
		if (m_synth)
			fluid_synth_noteon(m_synth, channel, pitch, velocity);
	}

	void MidiSynth::noteOff(i32 channel, i32 pitch)
	{
		if (m_synth)
			fluid_synth_noteoff(m_synth, channel, pitch);
	}

	void MidiSynth::allNotesOff(void)
	{
		if (!m_synth)
			return;
		fluid_synth_all_notes_off(m_synth, -1);
		// all_notes_off silences voices but doesn't clear controller state - without this, a
		// pedal that was held down would still be "down" as far as the synth is concerned for
		// whatever's played next.
		for (i32 ch = 0; ch < 16; ++ch)
			for (i32 controller : kPedalControllers)
				fluid_synth_cc(m_synth, ch, controller, 0);
	}

	void MidiSynth::apply_event(const Event& event)
	{
		switch (event.type)
		{
			case Event::Type::NoteOn: fluid_synth_noteon(m_synth, event.channel, event.param1, event.param2); break;
			case Event::Type::NoteOff: fluid_synth_noteoff(m_synth, event.channel, event.param1); break;
			case Event::Type::ControlChange: fluid_synth_cc(m_synth, event.channel, event.param1, event.param2); break;
		}
	}

	void MidiSynth::sdl_feed_callback(void* userdata, SDL_AudioStream* stream, int additionalAmount, int totalAmount)
	{
		static_cast<MidiSynth*>(userdata)->feed(stream, additionalAmount);
	}

	void MidiSynth::feed(SDL_AudioStream* stream, int additionalAmount)
	{
		if (additionalAmount <= 0 || !m_synth)
			return;
		constexpr u32 channels = 2;
		constexpr u32 bytesPerFrame = channels * (u32)sizeof(float);
		u64 requestedFrames = (u64)additionalAmount / bytesPerFrame;
		if (requestedFrames == 0)
			return;

		stdvec<float> buffer(requestedFrames * channels);
		u64 framesRendered = 0;

		std::lock_guard<std::mutex> lock(m_scheduleMutex);
		i64 cursor = m_frameCursor.load(std::memory_order_relaxed);

		// Renders in segments bounded by each due event's exact sample offset: apply everything
		// due at the current cursor, render up to whichever comes first (the next event, or the
		// end of this buffer), then repeat - sample-accurate scheduling entirely on this (audio)
		// thread, no cross-thread timers.
		while (framesRendered < requestedFrames)
		{
			while (m_nextEventIndex < m_schedule.size() && m_schedule[m_nextEventIndex].sampleOffset <= (u64)cursor)
			{
				apply_event(m_schedule[m_nextEventIndex]);
				++m_nextEventIndex;
			}

			u64 chunk = requestedFrames - framesRendered;
			if (m_nextEventIndex < m_schedule.size())
				chunk = std::min(chunk, m_schedule[m_nextEventIndex].sampleOffset - (u64)cursor);

			float* seg = buffer.data() + framesRendered * channels;
			fluid_synth_write_float(m_synth, (int)chunk, seg, 0, 2, seg, 1, 2);
			framesRendered += chunk;
			cursor += (i64)chunk;
		}

		m_frameCursor.store(cursor, std::memory_order_relaxed);
		m_cursorUpdateTimeNs.store(now_ns(), std::memory_order_relaxed);

		SDL_PutAudioStreamData(stream, buffer.data(), (int)(requestedFrames * bytesPerFrame));
	}
}
