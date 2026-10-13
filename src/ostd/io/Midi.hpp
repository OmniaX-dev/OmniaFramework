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

#include <ostd/string/String.hpp>

namespace ostd
{
	class MidiParser
	{
	    public: class NoteEvent
	    {
	        public:
	            i32 pitch               { 0 };          // MIDI note number (0-127)
	            f64 startTime            { 0.0 };        // Start time in seconds
	            f64 endTime              { 0.0 };        // Start time in seconds
	            f64 duration             { 0.0 };        // Duration in seconds
	            i32 velocity            { 0 };          // Attack velocity (1-127)
	            i32 channel             { 0 };          // MIDI channel (0-15)
	            // Exact source-file tick position and note-off velocity, alongside the derived
	            // (tempo-dependent, and so potentially lossy under round-tripping) seconds-based
	            // fields above - what writeFile() uses to reconstruct byte-exact note events,
	            // rather than re-deriving ticks from startTime/endTime and a tempo map.
	            i32 startTick            { 0 };
	            i32 endTick              { 0 };
	            i32 offVelocity          { 0 };          // Release velocity (0-127, usually 0)

	            bool hit { false };
	            bool rightHand { false };
	            bool last { false };
	            bool first { false };

	        public:
	            inline bool operator<(const NoteEvent& other) const
	            {
					if (startTime == other.startTime)
						return pitch < other.pitch;
	                return startTime < other.startTime;
	            }
	            inline String toString(void) const
	            {
	                String str = "Pitch: ";
	                str.add(pitch);
	                str.add("\nstartTime: ").add(startTime);
	                str.add("\nendTime: ").add(endTime);
	                str.add("\nduration: ").add(duration);
	                str.add("\nvelocity: ").add(velocity);
	                str.add("\nchannel: ").add(channel);
	                str.add("\nhit: ").add(STR_BOOL(hit));
	                return str;
	            }
	    };
		public: class NoteInfo
		{
	        public:
	            String name               { "" };     // e.g., "A", "C#", "F"
	            i32 octave                      { 0 };      // e.g., 4 for C4
	            i32 noteInOctave                { 0 };      // 0-11
	            i32 keyIndex                    { 0 };      // 0-based index for 88-key piano (A0=0), -1 if out of range

	        public:
	            inline bool isWhiteKey(void) const
	            {
	                return noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
	                noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
	                noteInOctave == 11;
	            }
	            inline bool isBlackKey(void) const { return !isWhiteKey(); }
	            inline String toString(void) const
	            {
	                String str = "NOTE INFO: ";
	                str.add(name).add(octave);
	                str.add("  -  noteInOctave: ").add(noteInOctave);
	                str.add("  -  keyIndex: ").add(keyIndex);
	                str.add("\n");
	                return str;
	            }

	            static inline bool isWhiteKey(i32 noteInOctave)
	            {
	                return noteInOctave == 0 || noteInOctave == 2 || noteInOctave == 4 ||
	                noteInOctave == 5 || noteInOctave == 7 || noteInOctave == 9 ||
	                noteInOctave == 11;
	            }
	            static inline bool isBlackKey(i32 noteInOctave) { return !NoteInfo::isWhiteKey(noteInOctave); }
		};

		// Major and (natural) minor key signatures (circle of fifths) - what
		// getNoteNameForKey() spells a pitch class as depends on this, e.g. pitch class 11
		// (B/Cb) is "B" in CMajor but "Cb" in GbMajor, since Gb major's key signature already
		// uses Cb as a scale degree. Covers all 15 conventional major key names (including the
		// three enharmonic pairs spelled differently depending on which side of the circle you
		// approach from: FSharpMajor/GbMajor, CSharpMajor/DbMajor, BMajor/CbMajor) plus their
		// 15 relative minors, which share the exact same key signature (same accidentals) as
		// their relative major - e.g. AMinor spells exactly like CMajor, EMinor like GMajor.
		public: enum class eMusicKey
		{
			CMajor, GMajor, DMajor, AMajor, EMajor, BMajor, FSharpMajor, CSharpMajor,
			FMajor, BbMajor, EbMajor, AbMajor, DbMajor, GbMajor, CbMajor,
			AMinor, EMinor, BMinor, FSharpMinor, CSharpMinor, GSharpMinor, DSharpMinor, ASharpMinor,
			DMinor, GMinor, CMinor, FMinor, BbMinor, EbMinor, AbMinor
		};

		// A tempo change, in the same absolute-tick terms as NoteEvent::startTick - what
		// writeFile() needs to reproduce a source file's tempo map exactly, rather than
		// assuming a single constant tempo throughout.
		public: class TempoEvent
		{
			public:
				i32 tick                    { 0 };
				f64 microsecondsPerQuarter   { 500000.0 }; // 120 BPM
		};

		// A sustain/sostenuto/soft pedal control-change (CC64/66/67 only - see parseFileEx()).
		// Kept as the controller's full 0-127 value, not collapsed to on/off, since recordings
		// (e.g. from Pianoteq) use half-pedaling.
		public: class ControlChangeEvent
		{
			public:
				i32 tick        { 0 };
				f64 time        { 0.0 }; // seconds, same clock as NoteEvent::startTime
				i32 controller  { 0 };   // 64 = sustain, 66 = sostenuto, 67 = soft
				i32 value       { 0 };   // 0-127
				i32 channel     { 0 };
		};

	    public:
			// Parses a single-track MIDI file and returns a vector of NoteEvents. A thin
			// wrapper over parseFileEx() that discards the tick/tempo-map data - kept as the
			// simple, original entry point every existing caller already uses.
	        static stdvec<NoteEvent> parseFile(const String& filePath);
			// Same parse, plus the exact tick-resolution data parseFile() discards (each
			// NoteEvent's startTick/endTick/offVelocity are populated either way - this just
			// additionally returns the file's own ticksPerQuarterNote and full tempo map,
			// needed to write a new file back out with writeFile() and have it play back
			// identically, not just contain the same notes).
			static bool parseFileEx(const String& filePath, stdvec<NoteEvent>& outNotes, i32& outTicksPerQuarterNote, stdvec<TempoEvent>& outTempoMap);
			// Same as the 4-arg overload above, plus the file's pedal CCs (sustain/sostenuto/
			// soft - see ControlChangeEvent); the 4-arg overload just calls this one with a
			// throwaway vector, same relationship parseFile() has to parseFileEx().
			static bool parseFileEx(const String& filePath, stdvec<NoteEvent>& outNotes, i32& outTicksPerQuarterNote, stdvec<TempoEvent>& outTempoMap, stdvec<ControlChangeEvent>& outPedalEvents);
			// Writes notes (with their startTick/endTick/channel/velocity/offVelocity) back out
			// as a new, single-track Standard MIDI File, reproducing ticksPerQuarterNote and
			// tempoMap exactly - the counterpart to parseFileEx(), e.g. for exporting a subset
			// of a file's notes (a selection) as its own playable .mid file. pedalEvents (sustain/
			// sostenuto/soft CCs - see ControlChangeEvent) is optional and, unlike notes, written
			// as-is with no further deduplication/merging of its own - callers combining pedal
			// data from more than one source (e.g. more than one Voice) should run it through
			// mergePedalEvents() first, same as PianoRoll's own live playback schedule does.
			static bool writeFile(const String& filePath, const stdvec<NoteEvent>& notes, i32 ticksPerQuarterNote, const stdvec<TempoEvent>& tempoMap, const stdvec<ControlChangeEvent>& pedalEvents = {});
			// Combines pedal CC events from one or more independent sources (one inner vector
			// per source, e.g. per PianoRoll::Voice) into a single, clean stream. This exists
			// because this project's own common case - a performance already split into per-
			// hand/voice .mid files - has each file carry its own near-identical copy of the
			// sustain pedal track, so simply concatenating every source's raw events (sorted by
			// time) can produce a spurious early release: if source A's pedal-up lands before
			// source B's overlapping pedal-down for what both sources actually mean as the same,
			// continuously-held pedal, naive interleaving briefly "releases" it mid-hold.
			// Within each source, consecutive value>0 ("down") / value==0 ("up") events for the
			// same channel+controller are first paired into hold intervals (any further value>0
			// events before the matching "up" just update the interval's own peak value, for
			// half-pedal data); a trailing, never-released "down" is kept open through its own
			// last activity rather than dropped. Across sources, overlapping or touching
			// intervals for the same channel+controller are then merged into one, spanning the
			// earliest down to the latest up (by whichever contributing interval actually ends
			// last - an earlier-ending contributor never retroactively cuts a later one short).
			// The result is a flat, tick-sorted stream of down/up ControlChangeEvent pairs, used
			// identically by PianoRoll's own multi-voice playback schedule and by exporting a
			// selection spanning more than one voice, so "two voices open at once" behaves the
			// same whether the pedal is actually heard or written to a file.
			static stdvec<ControlChangeEvent> mergePedalEvents(const stdvec<stdvec<ControlChangeEvent>>& perSourceEvents);
			// Convert MIDI pitch to NoteInfo
			static NoteInfo getNoteInfo(i32 midiPitch);
			// Note name (no octave) for a pitch class (0-11, C=0, any other i32 is normalized
			// into range) spelled correctly for the given major key - unlike NoteInfo::name
			// (always the plain always-sharp C-major-style spelling), this picks the actual
			// diatonic spelling for the key's 7 scale degrees, falls back to the plain natural
			// letter for a scale degree's own un-altered pitch (the standard convention, e.g.
			// G major's natural F, not "E#"), and only resorts to a generic sharp/flat-of-the-
			// neighbor spelling for a pitch that's genuinely outside the key altogether.
			static String getNoteNameForKey(i32 noteInOctave, eMusicKey key);
	};
}
