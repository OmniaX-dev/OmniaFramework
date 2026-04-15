/*
    OmniaFramework - A collection of useful functionality
    Copyright (C) 2025  OmniaX-Dev

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

	    public:
			// Parses a single-track MIDI file and returns a vector of NoteEvents
	        static stdvec<NoteEvent> parseFile(const String& filePath);
			// Convert MIDI pitch to NoteInfo
			static NoteInfo getNoteInfo(i32 midiPitch);
	};
}
