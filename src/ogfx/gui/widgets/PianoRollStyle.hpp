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

#include <ostd/data/Color.hpp>
#include <ostd/io/Midi.hpp>
#include <ostd/utils/Defines.hpp>

namespace ogfx
{
	namespace gui
	{
		// Standalone, serializable visual/layout config for PianoRoll - separate from the
		// widget itself the same way the Godot reference kept its "VirtualPianoData" resource
		// apart from the visualization node. Owned by PianoRoll via getStyle()/setStyle(), but
		// usable (and save/load-able) independently of any widget instance.
		class PianoRollStyle
		{
			public:
				// Resolves highest-priority-first: a caller-supplied voice color (if given,
				// e.g. from PianoRoll::Voice::color) -> per-pitch-class color (if enabled for
				// that pitch class) -> the 88-key gradient (if one is set) -> plain black/white
				// fallback. voiceColor may be nullptr (no voice-tier override).
				Color resolveNoteColor(i32 midiPitch, const Color* voiceColor = nullptr) const;

				// Builds the 88-entry per-key gradient by linearly interpolating the given
				// color stops across all 88 keys. An empty list disables this tier entirely
				// (resolveNoteColor falls through to the next one) - see clearPerKeyGradient().
				void setPerKeyGradient(const stdvec<Color>& stops);
				inline void clearPerKeyGradient(void) { m_perKeyColors.clear(); }
				inline bool hasPerKeyGradient(void) const { return m_perKeyColors.size() == 88; }
				static stdvec<Color> rainbowGradientStops(void); // canned 7-stop ROYGBIV preset, just an ordinary input to setPerKeyGradient()

				inline void setPerNoteColor(u8 noteInOctave /* 0=C..11=B */, const Color& color, bool enabled = true)
				{
					if (noteInOctave < 12) { m_perNoteColors[noteInOctave] = color; m_perNoteColorsEnabled[noteInOctave] = enabled; }
				}
				inline Color getPerNoteColor(u8 noteInOctave) const { return noteInOctave < 12 ? m_perNoteColors[noteInOctave] : Colors::White; }
				inline bool isPerNoteColorEnabled(u8 noteInOctave) const { return noteInOctave < 12 && m_perNoteColorsEnabled[noteInOctave]; }

				OSTD_BOOL_PARAM_GETSET_E(UsePerNoteColors, m_usePerNoteColors);
				OSTD_BOOL_PARAM_GETSET_E(UseFilledNotes, m_useFilledNotes);
				OSTD_BOOL_PARAM_GETSET_E(UseNoteColorOnPressedKey, m_useNoteColorOnPressedKey);

				OSTD_PARAM_GETSET(Color, FallingWhiteNoteColor, m_fallingWhiteNoteColor);
				OSTD_PARAM_GETSET(Color, FallingBlackNoteColor, m_fallingBlackNoteColor);
				OSTD_PARAM_GETSET(Color, WhiteKeyColor, m_whiteKeyColor);
				OSTD_PARAM_GETSET(Color, WhiteKeyPressedColor, m_whiteKeyPressedColor);
				OSTD_PARAM_GETSET(Color, BlackKeyColor, m_blackKeyColor);
				OSTD_PARAM_GETSET(Color, BlackKeyPressedColor, m_blackKeyPressedColor);
				OSTD_PARAM_GETSET(Color, BackgroundColor, m_backgroundColor);
				OSTD_PARAM_GETSET(Color, KeyboardLineColor, m_keyboardLineColor);

				OSTD_PARAM_GETSET(f32, WhiteKeyWidth, m_whiteKeyWidth);
				OSTD_PARAM_GETSET(f32, WhiteKeyHeightMultiplier, m_whiteKeyHeightMultiplier);
				OSTD_PARAM_GETSET(f32, BlackKeyWidth, m_blackKeyWidth);
				OSTD_PARAM_GETSET(f32, BlackKeyHeightMultiplier, m_blackKeyHeightMultiplier);
				OSTD_PARAM_GETSET(f32, BlackKeyOffset, m_blackKeyOffset);
				OSTD_PARAM_GETSET(f32, WhiteKeyGap, m_whiteKeyGap);
				OSTD_PARAM_GETSET(f32, BlackKeyGap, m_blackKeyGap);
				OSTD_PARAM_GETSET(f32, FallingNoteRadiusWhite, m_fallingNoteRadiusWhite);
				OSTD_PARAM_GETSET(f32, FallingNoteRadiusBlack, m_fallingNoteRadiusBlack);
				// The one authoritative timing/speed knob: how many seconds of lead time are
				// visible in the fall area at once. Both a note's on-screen height and its
				// scroll speed are derived from this plus the widget's actual pixel height at
				// draw time (fallAreaHeightPixels / lookaheadSeconds) - deliberately NOT two
				// separately-settable numbers (an explicit "pixels per second" alongside this),
				// which is exactly the kind of pair that can be set inconsistently with itself
				// and was part of what made the Godot version fragile under parameter changes.
				OSTD_PARAM_GETSET(f32, LookaheadSeconds, m_lookaheadSeconds);
				// Extra silence added on top of whatever lead-in is already needed for the
				// earliest note to start fully offscreen (see PianoRoll::recompute_total_duration())
				// - so the first note doesn't start falling the instant play() is pressed, but only
				// after it's had a moment to actually appear from offscreen first.
				OSTD_PARAM_GETSET(f32, ExtraOffscreenSeconds, m_extraOffscreenSeconds);

				// A released key fades back to its normal color over this many seconds instead of
				// switching instantly. useKeyFade = false (or a duration of 0) disables it outright.
				OSTD_BOOL_PARAM_GETSET_E(UseKeyFade, m_useKeyFade);
				OSTD_PARAM_GETSET(f32, KeyFadeDurationSeconds, m_keyFadeDurationSeconds);

				// --- music key - determines note-name spelling for both label options below ---
				OSTD_PARAM_GETSET(ostd::MidiParser::eMusicKey, MusicKey, m_musicKey);
				// Note name (no octave) for a MIDI pitch (or an already-reduced 0-11 pitch class
				// - either works, see getNoteNameForKey()), spelled correctly for the current
				// music key.
				inline String getNoteName(i32 midiPitch) const { return ostd::MidiParser::getNoteNameForKey(midiPitch, m_musicKey); }

				// --- tempo / time signature - purely descriptive (not derived from whatever a
				// loaded voice's own tempo map actually says - see PianoRoll::setTempo()/
				// setTimeSignature()), used only to place the bar/beat grid lines below ---
				// Non-positive is rejected outright (kept as whatever it was before), same as
				// setTimeSignature() already does for its own parts - 0 (or negative) BPM isn't a
				// real tempo, it's indistinguishable from "unset", and loadFromJson() relies on
				// that same rejection to fall back to the compiled-in default (120) instead of
				// silently zeroing out (and so completely disabling - see getBeatDurationSeconds())
				// the whole bar/beat grid whenever an on-disk style predates these fields or was
				// otherwise saved/edited with a placeholder 0.
				inline f32 getTempo(void) const { return m_tempo; }
				inline void setTempo(f32 bpm) { if (bpm > 0.0f) m_tempo = bpm; }
				inline i32 getTimeSignatureNumerator(void) const { return m_timeSignatureNumerator; }
				inline i32 getTimeSignatureDenominator(void) const { return m_timeSignatureDenominator; }
				// Parses "N/D" (e.g. "4/4", "6/8") - anything malformed (no '/', a non-numeric or
				// non-positive part) is ignored outright, leaving the current signature unchanged.
				void setTimeSignature(const String& signature);
				inline String getTimeSignature(void) const { return String("").add(m_timeSignatureNumerator).add("/").add(m_timeSignatureDenominator); }
				// One beat's length in seconds, under the usual "BPM counts quarter notes"
				// convention: a beat is (4/denominator) quarter notes long, so e.g. 6/8 at the
				// same tempo as 4/4 has a beat (an eighth note) half as long as 4/4's (a quarter
				// note) - not simply 60/bpm regardless of the signature's own denominator.
				inline f64 getBeatDurationSeconds(void) const { return m_tempo > 0.0f && m_timeSignatureDenominator > 0 ? (60.0 / (f64)m_tempo) * (4.0 / (f64)m_timeSignatureDenominator) : 0.0; }
				inline f64 getBarDurationSeconds(void) const { return getBeatDurationSeconds() * (f64)m_timeSignatureNumerator; }
				// Which beat of its bar the very first note actually falls on (1-based, default
				// 1 - the ordinary case, no pickup/anacrusis: the first note lands right on its
				// bar's downbeat). A value > 1 means the piece effectively starts mid-bar - e.g.
				// 3 means the first note is preceded by 2 empty beats still belonging to that same
				// first bar - and PianoRoll::draw_bar_beat_lines() anchors the whole bar/beat grid
				// accordingly (see PianoRoll::setStartBeat()). Non-positive is rejected, same
				// reasoning as setTempo() above.
				inline i32 getStartBeat(void) const { return m_startBeat; }
				inline void setStartBeat(i32 beat) { if (beat > 0) m_startBeat = beat; }

				// --- bar/beat grid lines, spanning the widget's full width, plus the bar number
				// drawn just under each bar line at the left edge - both toggles default off
				// (m_showBarLines/m_showBeatLines) since they assume setTempo()/setTimeSignature()
				// were actually called for whatever's loaded; the beat color defaults fainter
				// than the bar color so the bar grid reads as the more prominent of the two. ---
				OSTD_BOOL_PARAM_GETSET_E(ShowBarLines, m_showBarLines);
				OSTD_BOOL_PARAM_GETSET_E(ShowBeatLines, m_showBeatLines);
				OSTD_PARAM_GETSET(Color, BarLineColor, m_barLineColor);
				OSTD_PARAM_GETSET(Color, BeatLineColor, m_beatLineColor);
				OSTD_PARAM_GETSET(Color, BarNumberColor, m_barNumberColor);
				OSTD_PARAM_GETSET(f32, BarNumberFontSize, m_barNumberFontSize);

				// --- note-name labels drawn inside each falling note, bottom-centered ---
				OSTD_BOOL_PARAM_GETSET_E(ShowNoteNamesOnFallingNotes, m_showNoteNamesOnFallingNotes);
				OSTD_PARAM_GETSET(Color, FallingNoteNameColorWhite, m_fallingNoteNameColorWhite);
				OSTD_PARAM_GETSET(Color, FallingNoteNameColorBlack, m_fallingNoteNameColorBlack);
				OSTD_PARAM_GETSET(f32, FallingNoteNameFontSize, m_fallingNoteNameFontSize);

				// --- note-name labels drawn at the bottom of each keyboard key - a separate
				// option from the falling-note one above, same pattern ---
				OSTD_BOOL_PARAM_GETSET_E(ShowNoteNamesOnKeys, m_showNoteNamesOnKeys);
				OSTD_PARAM_GETSET(Color, KeyNoteNameColorWhite, m_keyNoteNameColorWhite);
				OSTD_PARAM_GETSET(Color, KeyNoteNameColorBlack, m_keyNoteNameColorBlack);
				OSTD_PARAM_GETSET(f32, KeyNoteNameFontSize, m_keyNoteNameFontSize);

				bool saveToJson(const String& path) const;
				bool loadFromJson(const String& path);

			private:
				bool m_usePerNoteColors { false };
				bool m_useFilledNotes { true };
				bool m_useNoteColorOnPressedKey { true };

				Color m_perNoteColors[12];
				bool m_perNoteColorsEnabled[12] { false };
				stdvec<Color> m_perKeyColors; // empty, or exactly 88 entries

				// These defaults mirror the Godot reference's VirtualPianoData._init() (same
				// colors and the same width/height/offset/gap numbers, converted from Color8()
				// and from Godot's width-multiplier pairs to ours) rather than inventing new
				// ones - notably the black keys, which that version draws proportionally taller
				// (height = 9x width) than this widget's old defaults did (4x).
				Color m_fallingWhiteNoteColor { "#146EAAFF" };
				Color m_fallingBlackNoteColor { "#00324BFF" };
				Color m_whiteKeyColor { "#BBBBBBFF" };
				Color m_whiteKeyPressedColor { "#7878D2FF" };
				Color m_blackKeyColor { "#222222FF" };
				Color m_blackKeyPressedColor { "#14145AFF" };
				Color m_backgroundColor { "#141414FF" };
				// Godot draws this as a two-color gradient (piano_line_color1/2) - this widget
				// only has one keyboard-line color, so this uses the brighter of the two.
				Color m_keyboardLineColor { "#A00A0AFF" };

				f32 m_whiteKeyWidth { 40 };
				f32 m_whiteKeyHeightMultiplier { 7 };
				f32 m_blackKeyWidth { 22 };
				f32 m_blackKeyHeightMultiplier { 8 };
				// 0, not Godot's 4 - that value was a small nudge on top of Godot's own *centered*
				// black-key formula; PianoRoll::key_x_position() positions black keys with a
				// built-in ~2/3-to-the-left bias already (closer to a real piano than centered),
				// so reusing Godot's nudge here would dilute that bias back toward centered
				// instead of adding to it. Still a free fine-tune knob on top either way.
				f32 m_blackKeyOffset { 0 };
				f32 m_whiteKeyGap { 4 };
				f32 m_blackKeyGap { 0 };
				f32 m_fallingNoteRadiusWhite { 5 };
				f32 m_fallingNoteRadiusBlack { 5 };
				f32 m_lookaheadSeconds { 4.5f };
				f32 m_extraOffscreenSeconds { 1.0f };
				bool m_useKeyFade { true };
				f32 m_keyFadeDurationSeconds { 0.2f };

				ostd::MidiParser::eMusicKey m_musicKey { ostd::MidiParser::eMusicKey::CMajor };

				f32 m_tempo { 120.0f };
				i32 m_timeSignatureNumerator { 4 };
				i32 m_timeSignatureDenominator { 4 };
				i32 m_startBeat { 1 };

				bool m_showBarLines { false };
				bool m_showBeatLines { false };
				Color m_barLineColor { "#FFFFFF30" }; // faint
				Color m_beatLineColor { "#FFFFFF18" }; // fainter still than m_barLineColor
				Color m_barNumberColor { "#FFFFFF80" };
				f32 m_barNumberFontSize { 12 };

				bool m_showNoteNamesOnFallingNotes { false };
				Color m_fallingNoteNameColorWhite { "#FFFFFFFF" };
				Color m_fallingNoteNameColorBlack { "#FFFFFFFF" };
				f32 m_fallingNoteNameFontSize { 14 };

				bool m_showNoteNamesOnKeys { false };
				// Dark text on white keys, light text on black keys - the same convention real
				// note-naming keyboard stickers use, for the same reason (contrast against each
				// key's own default background color).
				Color m_keyNoteNameColorWhite { "#000000FF" };
				Color m_keyNoteNameColorBlack { "#FFFFFFFF" };
				f32 m_keyNoteNameFontSize { 16 };
		};
	}
}
