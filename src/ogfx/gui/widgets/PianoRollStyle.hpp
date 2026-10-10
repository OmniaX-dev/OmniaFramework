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
		};
	}
}
