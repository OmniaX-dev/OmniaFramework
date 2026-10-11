#include "Midi.hpp"

#include <stdexcept>
#include <string>
#include <array>
#include "../vendor/midifile/MidiFile.h"

namespace ostd
{
	//TODO: Errors
	stdvec<MidiParser::NoteEvent> MidiParser::parseFile(const String& filePath)
	{
	    smf::MidiFile midi;

	    if (!midi.read(filePath))
	        throw std::runtime_error("Failed to read MIDI file: " + filePath.cpp_str());

	    // Merge multi-track files into a single track (a no-op if the file already has only
	    // one) - every event keeps its original channel, so multi-track files exported by a
	    // real DAW (the common case) are no longer rejected outright.
	    midi.joinTracks();

	    // Prepare time analysis and note pairing
	    midi.doTimeAnalysis();
	    midi.linkNotePairs();

	    stdvec<NoteEvent> notes;
	    notes.reserve(256);

	    for (i32 e = 0; e < midi[0].size(); ++e)
		{
	        auto& ev = midi[0][e];
	        if (ev.isNoteOn())
			{
	            NoteEvent note;
	            note.pitch     = ev.getKeyNumber();
	            note.startTime = ev.seconds;
	            note.duration  = ev.getDurationInSeconds();
	            note.endTime   = note.startTime + note.duration;
	            note.velocity  = ev.getVelocity();
	            note.channel   = ev.getChannel();
	            note.rightHand = false;
	            note.last	   = false;
	            note.first	   = false;
	            notes.push_back(note);
	        }
	    }

	    NoteEvent* notePtr = nullptr;
	    f64 noteTime = 0.0;
	    for (auto& note : notes)
	    {
	    	if (note.endTime > noteTime)
		    {
	   			notePtr = &note;
	    		noteTime = note.endTime;
		    }
	    }
	    if (notePtr != nullptr)
	   		notePtr->last = true;

	    notePtr = nullptr;
	    noteTime = 999999.0;
	    for (auto& note : notes)
	    {
	       	if (note.startTime < noteTime)
		    {
		     	notePtr = &note;
		      	noteTime = note.startTime;
		    }
	    }
	    if (notePtr != nullptr)
	    	notePtr->first = true;

	    return notes;
	}

	MidiParser::NoteInfo MidiParser::getNoteInfo(i32 midiPitch)
	{
	    static const std::array<std::string, 12> noteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

	    NoteInfo info;

	    // Calculate name and octave
	    info.noteInOctave = midiPitch % 12;
	    info.name = noteNames[info.noteInOctave];
	    info.octave = (midiPitch / 12) - 1; // MIDI 0 = C-1

	    // Calculate 88-key index (A0 = MIDI 21, C8 = MIDI 108)
	    if (midiPitch >= 21 && midiPitch <= 108)
	        info.keyIndex = midiPitch - 21;
	    else
	        info.keyIndex = -1; // Outside standard piano range

	    return info;
	}

	String MidiParser::getNoteNameForKey(i32 noteInOctave, eMusicKey key)
	{
	    noteInOctave = ((noteInOctave % 12) + 12) % 12; // tolerate a raw MIDI pitch or a negative value too

	    // Letter order A..G, matched index-for-index across these three tables.
	    static const char letters[7]   = { 'A', 'B', 'C', 'D', 'E', 'F', 'G' };
	    static const i32 naturalPC[7]  = {  9,   11,  0,   2,   4,   5,   7  };
	    static const i32 sharpOrder[7] = {  5,   2,   6,   3,   0,   4,   1  }; // F,C,G,D,A,E,B
	    static const i32 flatOrder[7]  = {  1,   4,   0,   3,   6,   2,   5  }; // B,E,A,D,G,C,F

	    struct KeyDef { i32 direction; i32 count; }; // direction: +1 sharp, -1 flat, 0 = CMajor (neither)
	    static const KeyDef keyDefs[15] = {
	        {  0, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 1, 6 }, { 1, 7 },
	        { -1, 1 }, { -1, 2 }, { -1, 3 }, { -1, 4 }, { -1, 5 }, { -1, 6 }, { -1, 7 }
	    };
	    const KeyDef& def = keyDefs[(size_t)key];

	    i32 accidental[7] = { 0, 0, 0, 0, 0, 0, 0 };
	    if (def.direction != 0)
	    {
	        const i32* order = def.direction > 0 ? sharpOrder : flatOrder;
	        for (i32 i = 0; i < def.count; i++)
	            accidental[order[i]] = def.direction;
	    }

	    auto letterString = [](char c) -> String { char buf[2] = { c, '\0' }; return String(buf); };

	    // 1) A match against one of the key's own 7 diatonic scale degrees - always correct and
	    //    unambiguous.
	    for (i32 i = 0; i < 7; i++)
	    {
	        i32 pc = ((naturalPC[i] + accidental[i]) % 12 + 12) % 12;
	        if (pc == noteInOctave)
	            return accidental[i] == 0 ? letterString(letters[i]) : letterString(letters[i]).add(accidental[i] > 0 ? "#" : "b");
	    }

	    // 2) Not a scale degree, but it's exactly the un-altered pitch of one of the key's own
	    //    altered letters - spelled as that letter's plain natural (standard convention: e.g.
	    //    G major's natural F is "F", not the enharmonic "E#").
	    for (i32 i = 0; i < 7; i++)
	    {
	        if (accidental[i] != 0 && naturalPC[i] == noteInOctave)
	            return letterString(letters[i]);
	    }

	    // 3) A genuine chromatic passing tone, outside the key altogether - sharp of the lower
	    //    natural neighbor for a sharp (or C major) key, flat of the upper natural neighbor
	    //    for a flat key. Index into letters[]/naturalPC[], by pitch class 0-11.
	    static const i32 lowerLetterOf[12] = { 2, 2, 3, 3, 4, 5, 5, 6, 6, 0, 0, 1 };
	    static const i32 upperLetterOf[12] = { 2, 3, 3, 4, 4, 5, 6, 6, 0, 0, 1, 1 };
	    if (def.direction >= 0)
	        return letterString(letters[lowerLetterOf[noteInOctave]]).add("#");
	    else
	        return letterString(letters[upperLetterOf[noteInOctave]]).add("b");
	}
}
