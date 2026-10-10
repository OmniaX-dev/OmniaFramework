#include "PianoRollStyle.hpp"
#include <ostd/io/Json.hpp>
#include <cmath>

namespace ogfx
{
	namespace gui
	{
		namespace
		{
			inline u8 lerpChannel(u8 a, u8 b, f32 t) { return (u8)std::round(a + ((f32)b - (f32)a) * t); }

			Color lerpColor(const Color& a, const Color& b, f32 t)
			{
				Color a2 = a, b2 = b; // Color's channel accessors aren't const-qualified
				return Color(lerpChannel(a2.r, b2.r, t), lerpChannel(a2.g, b2.g, t), lerpChannel(a2.b, b2.b, t), lerpChannel(a2.a, b2.a, t));
			}

			constexpr const char* kNoteNames[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		}

		Color PianoRollStyle::resolveNoteColor(i32 midiPitch, const Color* voiceColor) const
		{
			if (voiceColor)
				return *voiceColor;

			ostd::MidiParser::NoteInfo info = ostd::MidiParser::getNoteInfo(midiPitch);

			if (m_usePerNoteColors && isPerNoteColorEnabled((u8)info.noteInOctave))
				return getPerNoteColor((u8)info.noteInOctave);

			if (hasPerKeyGradient() && info.keyIndex >= 0 && info.keyIndex < (i32)m_perKeyColors.size())
				return m_perKeyColors[(size_t)info.keyIndex];

			return info.isBlackKey() ? m_fallingBlackNoteColor : m_fallingWhiteNoteColor;
		}

		void PianoRollStyle::setPerKeyGradient(const stdvec<Color>& stops)
		{
			m_perKeyColors.clear();
			if (stops.empty())
				return;
			if (stops.size() == 1)
			{
				m_perKeyColors.assign(88, stops[0]);
				return;
			}
			m_perKeyColors.reserve(88);
			const f32 segments = (f32)(stops.size() - 1);
			for (i32 key = 0; key < 88; key++)
			{
				f32 t = (f32)key / 87.0f * segments; // 0..segments across all 88 keys
				i32 seg = std::min((i32)t, (i32)segments - 1);
				f32 localT = t - (f32)seg;
				m_perKeyColors.push_back(lerpColor(stops[(size_t)seg], stops[(size_t)seg + 1], localT));
			}
		}

		stdvec<Color> PianoRollStyle::rainbowGradientStops(void)
		{
			return {
				Color("#FF0000FF"), Color("#FF7F00FF"), Color("#FFFF00FF"), Color("#00FF00FF"),
				Color("#0000FFFF"), Color("#4B0082FF"), Color("#8F00FFFF")
			};
		}

		bool PianoRollStyle::saveToJson(const String& path) const
		{
			ostd::JsonFile f(path);
			if (!f.isLoaded())
				return false;

			f.set_double("style.dimensions.whiteKeyWidth", m_whiteKeyWidth);
			f.set_double("style.dimensions.whiteKeyHeightMultiplier", m_whiteKeyHeightMultiplier);
			f.set_double("style.dimensions.blackKeyWidth", m_blackKeyWidth);
			f.set_double("style.dimensions.blackKeyHeightMultiplier", m_blackKeyHeightMultiplier);
			f.set_double("style.dimensions.blackKeyOffset", m_blackKeyOffset);
			f.set_double("style.dimensions.whiteKeyGap", m_whiteKeyGap);
			f.set_double("style.dimensions.blackKeyGap", m_blackKeyGap);
			f.set_double("style.dimensions.fallingNoteRadiusWhite", m_fallingNoteRadiusWhite);
			f.set_double("style.dimensions.fallingNoteRadiusBlack", m_fallingNoteRadiusBlack);
			f.set_double("style.dimensions.lookaheadSeconds", m_lookaheadSeconds);

			f.set_bool("style.usePerNoteColors", m_usePerNoteColors);
			f.set_bool("style.useFilledNotes", m_useFilledNotes);
			f.set_bool("style.useNoteColorOnPressedKey", m_useNoteColorOnPressedKey);

			f.set_color("style.colors.fallingWhiteNote", m_fallingWhiteNoteColor);
			f.set_color("style.colors.fallingBlackNote", m_fallingBlackNoteColor);
			f.set_color("style.colors.whiteKey", m_whiteKeyColor);
			f.set_color("style.colors.whiteKeyPressed", m_whiteKeyPressedColor);
			f.set_color("style.colors.blackKey", m_blackKeyColor);
			f.set_color("style.colors.blackKeyPressed", m_blackKeyPressedColor);
			f.set_color("style.colors.background", m_backgroundColor);
			f.set_color("style.colors.keyboardLine", m_keyboardLineColor);

			for (u8 i = 0; i < 12; i++)
			{
				String base = String("style.colors.perNote.").add(kNoteNames[i]);
				f.set_color(String(base).add(".color"), m_perNoteColors[i]);
				f.set_bool(String(base).add(".use"), m_perNoteColorsEnabled[i]);
			}

			f.set_color_array("style.colors.perKey", m_perKeyColors); // empty array if no gradient is set
			return true;
		}

		bool PianoRollStyle::loadFromJson(const String& path)
		{
			ostd::JsonFile f(path);
			if (!f.isLoaded())
				return false;

			m_whiteKeyWidth = f.get_float("style.dimensions.whiteKeyWidth");
			m_whiteKeyHeightMultiplier = f.get_float("style.dimensions.whiteKeyHeightMultiplier");
			m_blackKeyWidth = f.get_float("style.dimensions.blackKeyWidth");
			m_blackKeyHeightMultiplier = f.get_float("style.dimensions.blackKeyHeightMultiplier");
			m_blackKeyOffset = f.get_float("style.dimensions.blackKeyOffset");
			m_whiteKeyGap = f.get_float("style.dimensions.whiteKeyGap");
			m_blackKeyGap = f.get_float("style.dimensions.blackKeyGap");
			m_fallingNoteRadiusWhite = f.get_float("style.dimensions.fallingNoteRadiusWhite");
			m_fallingNoteRadiusBlack = f.get_float("style.dimensions.fallingNoteRadiusBlack");
			m_lookaheadSeconds = f.get_float("style.dimensions.lookaheadSeconds");

			m_usePerNoteColors = f.get_bool("style.usePerNoteColors");
			m_useFilledNotes = f.get_bool("style.useFilledNotes");
			m_useNoteColorOnPressedKey = f.get_bool("style.useNoteColorOnPressedKey");

			m_fallingWhiteNoteColor = f.get_color("style.colors.fallingWhiteNote");
			m_fallingBlackNoteColor = f.get_color("style.colors.fallingBlackNote");
			m_whiteKeyColor = f.get_color("style.colors.whiteKey");
			m_whiteKeyPressedColor = f.get_color("style.colors.whiteKeyPressed");
			m_blackKeyColor = f.get_color("style.colors.blackKey");
			m_blackKeyPressedColor = f.get_color("style.colors.blackKeyPressed");
			m_backgroundColor = f.get_color("style.colors.background");
			m_keyboardLineColor = f.get_color("style.colors.keyboardLine");

			for (u8 i = 0; i < 12; i++)
			{
				String base = String("style.colors.perNote.").add(kNoteNames[i]);
				m_perNoteColors[i] = f.get_color(String(base).add(".color"));
				m_perNoteColorsEnabled[i] = f.get_bool(String(base).add(".use"));
			}

			m_perKeyColors = f.get_color_array("style.colors.perKey");
			return true;
		}
	}
}
