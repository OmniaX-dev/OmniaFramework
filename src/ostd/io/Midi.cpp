#include "Midi.hpp"

#include <stdexcept>
#include <string>
#include <array>
#include <cmath>
#include <map>
#include <utility>
#include <algorithm>
#include "../vendor/midifile/MidiFile.h"

namespace ostd
{
	//TODO: Errors
	stdvec<MidiParser::NoteEvent> MidiParser::parseFile(const String& filePath)
	{
	    stdvec<NoteEvent> notes;
	    i32 ticksPerQuarterNote = 0;
	    stdvec<TempoEvent> tempoMap;
	    parseFileEx(filePath, notes, ticksPerQuarterNote, tempoMap);
	    return notes;
	}

	bool MidiParser::parseFileEx(const String& filePath, stdvec<NoteEvent>& outNotes, i32& outTicksPerQuarterNote, stdvec<TempoEvent>& outTempoMap)
	{
	    stdvec<ControlChangeEvent> pedalEvents;
	    return parseFileEx(filePath, outNotes, outTicksPerQuarterNote, outTempoMap, pedalEvents);
	}

	bool MidiParser::parseFileEx(const String& filePath, stdvec<NoteEvent>& outNotes, i32& outTicksPerQuarterNote, stdvec<TempoEvent>& outTempoMap, stdvec<ControlChangeEvent>& outPedalEvents)
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

	    outNotes.clear();
	    outNotes.reserve(256);
	    outTicksPerQuarterNote = midi.getTicksPerQuarterNote();
	    outTempoMap.clear();
	    outPedalEvents.clear();

	    for (i32 e = 0; e < midi[0].size(); ++e)
		{
	        auto& ev = midi[0][e];
	        if (ev.isTempo())
	        {
	            TempoEvent tempo;
	            tempo.tick = ev.tick;
	            tempo.microsecondsPerQuarter = (f64)ev.getTempoMicroseconds();
	            outTempoMap.push_back(tempo);
	            continue;
	        }
	        if (ev.isController())
	        {
	            i32 controller = ev.getControllerNumber();
	            if (controller == 64 || controller == 66 || controller == 67) // sustain, sostenuto, soft
	            {
	                ControlChangeEvent pedal;
	                pedal.tick = ev.tick;
	                pedal.time = ev.seconds;
	                pedal.controller = controller;
	                pedal.value = ev.getControllerValue();
	                pedal.channel = ev.getChannel();
	                outPedalEvents.push_back(pedal);
	            }
	            continue;
	        }
	        if (ev.isNoteOn())
			{
	            NoteEvent note;
	            note.pitch     = ev.getKeyNumber();
	            note.startTime = ev.seconds;
	            note.duration  = ev.getDurationInSeconds();
	            note.endTime   = note.startTime + note.duration;
	            note.velocity  = ev.getVelocity();
	            note.channel   = ev.getChannel();
	            note.startTick = ev.tick;
	            smf::MidiEvent* offEvent = ev.getLinkedEvent();
	            if (offEvent != nullptr)
	            {
	                note.endTick     = offEvent->tick;
	                note.offVelocity = offEvent->getVelocity();
	            }
	            else
	            {
	                // No paired note-off found (malformed/truncated file) - fall back to the
	                // already-computed seconds-based duration converted back to ticks, rather
	                // than leaving endTick at 0 (which would make the note zero/negative length).
	                f64 ticksPerSecond = outTicksPerQuarterNote > 0 && !outTempoMap.empty()
	                    ? (f64)outTicksPerQuarterNote / (outTempoMap.back().microsecondsPerQuarter / 1000000.0)
	                    : (f64)outTicksPerQuarterNote / (500000.0 / 1000000.0); // default 120 BPM
	                note.endTick = note.startTick + (i32)std::round(note.duration * ticksPerSecond);
	                note.offVelocity = 0;
	            }
	            note.rightHand = false;
	            note.last	   = false;
	            note.first	   = false;
	            outNotes.push_back(note);
	        }
	    }

	    if (outTempoMap.empty())
	    {
	        TempoEvent defaultTempo;
	        defaultTempo.tick = 0;
	        defaultTempo.microsecondsPerQuarter = 500000.0; // 120 BPM, smf::MidiFile's own default
	        outTempoMap.push_back(defaultTempo);
	    }

	    NoteEvent* notePtr = nullptr;
	    f64 noteTime = 0.0;
	    for (auto& note : outNotes)
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
	    for (auto& note : outNotes)
	    {
	       	if (note.startTime < noteTime)
		    {
		     	notePtr = &note;
		      	noteTime = note.startTime;
		    }
	    }
	    if (notePtr != nullptr)
	    	notePtr->first = true;

	    return true;
	}

	bool MidiParser::writeFile(const String& filePath, const stdvec<NoteEvent>& notes, i32 ticksPerQuarterNote, const stdvec<TempoEvent>& tempoMap, const stdvec<ControlChangeEvent>& pedalEvents)
	{
	    smf::MidiFile midi;
	    // A fresh smf::MidiFile already has one (empty) track at index 0 - no addTrack() needed.
	    midi.setTicksPerQuarterNote(ticksPerQuarterNote > 0 ? ticksPerQuarterNote : 480);

	    for (auto& tempo : tempoMap)
	    {
	        smf::MidiEvent tempoEvent;
	        tempoEvent.tick = tempo.tick;
	        tempoEvent.setTempoMicroseconds((i32)std::round(tempo.microsecondsPerQuarter));
	        midi.addEvent(0, tempoEvent);
	    }

	    for (auto& note : notes)
	    {
	        midi.addNoteOn(0, note.startTick, note.channel, note.pitch, note.velocity);
	        midi.addNoteOff(0, note.endTick, note.channel, note.pitch, note.offVelocity);
	    }

	    for (auto& pedal : pedalEvents)
	        midi.addController(0, pedal.tick, pedal.channel, pedal.controller, pedal.value);

	    midi.sortTracks();
	    return midi.write(filePath.cpp_str());
	}

	stdvec<MidiParser::ControlChangeEvent> MidiParser::mergePedalEvents(const stdvec<stdvec<ControlChangeEvent>>& perSourceEvents)
	{
	    // One paired-up hold interval per (channel, controller) span - see the declaration
	    // comment for the two-pass approach (pair within a source first, then merge across
	    // sources) and why it's split that way.
	    struct Interval
	    {
	        i32 channel; i32 controller;
	        i32 startTick; i32 endTick;
	        f64 startTime; f64 endTime;
	        i32 peakValue;
	        bool closed; // true if a real value==0 "up" was seen, false if still held when its source ran out of events
	    };
	    stdvec<Interval> intervals;

	    for (auto& source : perSourceEvents)
	    {
	        // Keyed by (channel, controller) - a source's own stream can interleave more than
	        // one pedal/channel at once, so each key tracks its own independent open/closed
	        // state rather than assuming one pedal at a time.
	        std::map<std::pair<i32, i32>, Interval> open;
	        for (auto& ev : source)
	        {
	            auto key = std::make_pair(ev.channel, ev.controller);
	            if (ev.value > 0)
	            {
	                auto it = open.find(key);
	                if (it == open.end())
	                    open.emplace(key, Interval{ ev.channel, ev.controller, ev.tick, ev.tick, ev.time, ev.time, ev.value, false });
	                else
	                {
	                    // Still held - a further half-pedal fluctuation (or a duplicate "down")
	                    // before the matching "up". Not a new interval; just extends how far
	                    // this one's known-active so far and tracks its peak depth.
	                    it->second.endTick = ev.tick;
	                    it->second.endTime = ev.time;
	                    it->second.peakValue = std::max(it->second.peakValue, ev.value);
	                }
	            }
	            else
	            {
	                auto it = open.find(key);
	                if (it != open.end())
	                {
	                    it->second.endTick = ev.tick;
	                    it->second.endTime = ev.time;
	                    it->second.closed = true;
	                    intervals.push_back(it->second);
	                    open.erase(it);
	                }
	                // an "up" with nothing open means nothing was ever held - nothing to close
	            }
	        }
	        // Whatever's still open when this source runs out of events never got a real
	        // release in the file - kept as its own (unclosed) interval rather than dropped, so
	        // a trailing held pedal still comes through afterward.
	        for (auto& [key, interval] : open)
	            intervals.push_back(interval);
	    }

	    // Standard "merge overlapping intervals", grouped by (channel, controller): sort by
	    // start, then absorb every following interval whose own start is at or before the
	    // current run's end. closed only carries forward from whichever contributing interval
	    // actually ends latest - an earlier-ending contributor that happened to get a real
	    // release never retroactively cuts short a later, still-overlapping one.
	    std::sort(intervals.begin(), intervals.end(), [](const Interval& a, const Interval& b) {
	        if (a.channel != b.channel) return a.channel < b.channel;
	        if (a.controller != b.controller) return a.controller < b.controller;
	        return a.startTick < b.startTick;
	    });

	    stdvec<ControlChangeEvent> result;
	    for (size_t i = 0; i < intervals.size(); )
	    {
	        Interval merged = intervals[i];
	        size_t j = i + 1;
	        while (j < intervals.size() && intervals[j].channel == merged.channel && intervals[j].controller == merged.controller && intervals[j].startTick <= merged.endTick)
	        {
	            if (intervals[j].endTick > merged.endTick)
	            {
	                merged.endTick = intervals[j].endTick;
	                merged.endTime = intervals[j].endTime;
	                merged.closed = intervals[j].closed;
	            }
	            else if (intervals[j].endTick == merged.endTick && intervals[j].closed)
	                merged.closed = true;
	            merged.peakValue = std::max(merged.peakValue, intervals[j].peakValue);
	            j++;
	        }

	        ControlChangeEvent down;
	        down.tick = merged.startTick; down.time = merged.startTime;
	        down.channel = merged.channel; down.controller = merged.controller; down.value = merged.peakValue;
	        result.push_back(down);

	        // Only synthesize the matching "up" if some contributing interval actually closed
	        // this one for real - otherwise the pedal was still held when every source's data
	        // ran out, and a synthetic release here would be a fabricated event, not a merge.
	        if (merged.closed)
	        {
	            ControlChangeEvent up;
	            up.tick = merged.endTick; up.time = merged.endTime;
	            up.channel = merged.channel; up.controller = merged.controller; up.value = 0;
	            result.push_back(up);
	        }

	        i = j;
	    }

	    std::sort(result.begin(), result.end(), [](const ControlChangeEvent& a, const ControlChangeEvent& b) { return a.tick < b.tick; });
	    return result;
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
	    static const KeyDef keyDefs[30] = {
	        // majors
	        {  0, 0 }, { 1, 1 }, { 1, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 1, 6 }, { 1, 7 },
	        { -1, 1 }, { -1, 2 }, { -1, 3 }, { -1, 4 }, { -1, 5 }, { -1, 6 }, { -1, 7 },
	        // relative minors - identical signatures to the majors above, same order
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
