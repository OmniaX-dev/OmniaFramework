#include "Midi.hpp"
#include <ostd/vendor/midifile/MidiFile.h>
#include <vector>
#include <stdexcept>
#include <string>
#include <array>



namespace ostd
{
	//TODO: Errors
	std::vector<MidiParser::NoteEvent> MidiParser::parseFile(const ostd::String& filePath)
	{
	    smf::MidiFile midi;

	    if (!midi.read(filePath))
	        throw std::runtime_error("Failed to read MIDI file: " + filePath.cpp_str());

	    // Check track count
	    if (midi.getTrackCount() != 1)
	        throw std::runtime_error("Expected exactly 1 track, but found " + std::to_string(midi.getTrackCount()));

	    // Prepare time analysis and note pairing
	    midi.doTimeAnalysis();
	    midi.linkNotePairs();

	    std::vector<NoteEvent> notes;
	    notes.reserve(256);

	    for (int e = 0; e < midi[0].size(); ++e)
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
	    double noteTime = 0.0;
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

	MidiParser::NoteInfo MidiParser::getNoteInfo(int midiPitch)
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
}
