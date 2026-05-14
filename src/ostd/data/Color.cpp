#include "Color.hpp"
#include <cmath>
#include "../io/Logger.hpp"
#include "../io/IOHandlers.hpp"

namespace ostd
{
	Color::Color(void) : r(*this), g(*this), b(*this), a(*this)
	{
		set();
		setTypeName("ostd::Color");
		BaseObject::setValid(true);
	}

	Color::Color(u8 rgb_single_value, u8 alpha) : r(*this), g(*this), b(*this), a(*this)
	{
		set(rgb_single_value, alpha);
		setTypeName("ostd::Color");
		BaseObject::setValid(true);
	}

	Color::Color(u8 _r, u8 _g, u8 _b, u8 alpha) : r(*this), g(*this), b(*this), a(*this)
	{
		set(_r, _g, _b, alpha);
		setTypeName("ostd::Color");
		BaseObject::setValid(true);
	}

	Color::Color(const String& color_string) : r(*this), g(*this), b(*this), a(*this)
	{
		set(color_string);
		setTypeName("ostd::Color");
		BaseObject::setValid(true);
	}

	Color::Color(const FloatCol& normalized_color) : r(*this), g(*this), b(*this), a(*this)
	{
		set(normalized_color);
		setTypeName("ostd::Color");
		BaseObject::setValid(true);
	}

	Color::Color(const Color& copy) : BaseObject(copy), r(*this), g(*this), b(*this), a(*this)
	{
		r = cast<u8>(copy.r);
		g = cast<u8>(copy.g);
		b = cast<u8>(copy.b);
		a = cast<u8>(copy.a);
	}

	bool Color::operator==(const Color& col2)
	{
		return (r == col2.r && g == col2.g && b == col2.b && a == col2.a);
	}

	bool Color::operator!=(const Color& col2)
	{
		return !(*this == col2);
	}

	Color& Color::operator=(const Color& copy)
	{
		BaseObject::operator=(copy);
		r = cast<u8>(copy.r);
		g = cast<u8>(copy.g);
		b = cast<u8>(copy.b);
		a = cast<u8>(copy.a);
		return *this;
	}

	Color& Color::set(void)
	{
		r = 0;
		g = 0;
		b = 0;
		a = 255;
		return *this;
	}

	Color& Color::set(u8 rgb_single_value, u8 alpha)
	{
		r = rgb_single_value;
		g = rgb_single_value;
		b = rgb_single_value;
		a = alpha;
		return *this;
	}

	Color& Color::set(u8 _r, u8 _g, u8 _b, u8 alpha)
	{
		r = _r;
		g = _g;
		b = _b;
		a = alpha;
		return *this;
	}

	Color& Color::set(const String& color_string)
	{
		if (!isValidColorString(color_string, this))
		{
			OX_WARN("ostd::Color::set(const String&) -> Invalid color string: %s", color_string.c_str());
			r = 0; g = 0; b = 0; a = 255;  // match original default-on-failure
		}
		return *this;
	}

	Color& Color::set(const FloatCol& normalized_color)
	{
		r = cast<u8>(std::round(normalized_color.r * 255));
		g = cast<u8>(std::round(normalized_color.g * 255));
		b = cast<u8>(std::round(normalized_color.b * 255));
		a = cast<u8>(std::round(normalized_color.a * 255));
		return *this;
	}

	String Color::hexString(bool include_alpha, const String& prefix) const
	{
		String hex = "";
		hex += String::getHexStr(r, false, 1);
		hex += String::getHexStr(g, false, 1);
		hex += String::getHexStr(b, false, 1);
		if (include_alpha)
			hex += String::getHexStr(a, false, 1);
		hex = prefix + String(hex).toUpper();
		return hex;
	}

	String Color::rgbString(bool include_parenthesis, bool include_alpha) const
	{
		String rgb = "";
		if (include_parenthesis)
			rgb.add("(");
		rgb.add(r).add(", ");
		rgb.add(g).add(", ");
		rgb.add(b);
		if (include_alpha)
			rgb.add(", ").add(a);
		if (include_parenthesis)
			rgb.add(")");
		return rgb;
	}

	u32 Color::asInteger(eColorFormat format) const
	{
		union uC32 {
			u8 data[4];
			u32 value;
		} c32_u;
		if (format == eColorFormat::RGBA)
		{
			c32_u.data[0] = a;
			c32_u.data[1] = b;
			c32_u.data[2] = g;
			c32_u.data[3] = r;
		}
		else if (format == eColorFormat::ARGB)
		{
			c32_u.data[0] = r;
			c32_u.data[1] = g;
			c32_u.data[2] = b;
			c32_u.data[3] = a;
		}
		return c32_u.value;
	}

	Color::FloatCol Color::getNormalizedColor(void) const
	{
		if (m_dirty)
		{
			m_cachedFloat = {
				r.value * (1.0f / 255.0f),
				g.value * (1.0f / 255.0f),
				b.value * (1.0f / 255.0f),
				a.value * (1.0f / 255.0f)
			};
			m_dirty = false;
		}
		return m_cachedFloat;
	}

	Color Color::darkened(f32 amount) const
	{
		f32 h, s, v;
		RGBtoHSV(r.value / 255.0f, g.value / 255.0f, b.value / 255.0f, h, s, v);
		v = std::clamp(v - amount, 0.0f, 1.0f);
		f32 nr, ng, nb;
		HSVtoRGB(h, s, v, nr, ng, nb);
		return {
			cast<u8>(nr * 255),
			cast<u8>(ng * 255),
			cast<u8>(nb * 255),
			cast<u8>(a.value)
		};
	}

	Color Color::scaled(f32 factor) const
	{
		return {
			cast<u8>(std::clamp(r.value * factor, 0.0f, 255.0f)),
			cast<u8>(std::clamp(g.value * factor, 0.0f, 255.0f)),
			cast<u8>(std::clamp(b.value * factor, 0.0f, 255.0f)),
			cast<u8>(a.value)
		};
	}

	String Color::toString(void) const
	{
		return hexString(true, "#") + " -> rgba" + rgbString(true, true);
	}

	void Color::print(bool newLine, OutputHandlerBase* __destination) const
	{
		if (__destination == nullptr)
			std::cout << *this << (newLine ? "\n" : "");
		else
		{
			__destination->p(this->toString());
			if (newLine) __destination->nl();
		}
	}

	void Color::RGBtoHSV(f32 r, f32 g, f32 b, f32& h, f32& s, f32& v)
	{
		f32 max = std::max({r, g, b});
		f32 min = std::min({r, g, b});
		f32 d = max - min;

		v = max;

		if (max == 0)
		{
			s = 0;
			h = 0;
			return;
		}

		s = d / max;

		if (max == r)      h = std::fmod((g - b) / d + 6.0f, 6.0f);
		else if (max == g) h = (b - r) / d + 2.0f;
		else               h = (r - g) / d + 4.0f;

		h /= 6.0f;
	}

	void Color::HSVtoRGB(f32 h, f32 s, f32 v, f32& r, f32& g, f32& b)
	{
		i32 i = cast<i32>(h * 6);
		f32 f = h * 6 - i;
		f32 p = v * (1 - s);
		f32 q = v * (1 - f * s);
		f32 t = v * (1 - (1 - f) * s);

		switch (i % 6)
		{
			case 0: r = v, g = t, b = p; break;
			case 1: r = q, g = v, b = p; break;
			case 2: r = p, g = v, b = t; break;
			case 3: r = p, g = q, b = v; break;
			case 4: r = t, g = p, b = v; break;
			case 5: r = v, g = p, b = q; break;
		}
	}

	bool Color::isValidColorString(const String& str, Color* out)
	{
		String se(str);
		se.trim();
		if (se.len() == 0) return false;

		// --- Step 1: Extract optional .lighten(x) / .darken(x) suffix ---
		enum class Modifier { None, Lighten, Darken };
		Modifier mod = Modifier::None;
		f32 modAmount = 0.0f;
		{
			stdvec<String> matches;
			auto positions = se.regexFind(
				"\\.(lighten|darken)\\(\\s*([0-9]*\\.?[0-9]+)\\s*\\)\\s*$",
				true, &matches);
			if (!positions.empty())
			{
				const String& whole = matches[0];
				mod = whole.contains("lighten") ? Modifier::Lighten : Modifier::Darken;

				const u32 openParen  = whole.indexOf("(");
				const u32 closeParen = whole.indexOf(")");
				String numStr = whole.new_substr(openParen + 1, closeParen);
				numStr.trim();
				modAmount = numStr.toFloat();
				if (modAmount < 0.0f) modAmount = 0.0f;
				if (modAmount > 1.0f) modAmount = 1.0f;

				se = se.new_substr(0, positions[0]);
				se.trim();
			}
			if (se.len() == 0) return false;
		}

		// Local working color — only written back to *out if everything validates.
		u8 rr = 0, gg = 0, bb = 0, aa = 255;

		// --- Step 2: Normalize "#" prefix to "0x" ---
		if (se.startsWith("#"))
			se = String("0x") + se.new_substr(1).trim();

		// --- Step 3a: Hex form ---
		if (se.toLower().startsWith("0x"))
		{
			if (!se.isHex()) return false;
			const u32 digits = se.len() - 2;
			if (digits < 1 || digits > 8) return false;

			const u32 c = static_cast<u32>(se.toInt());
			if (digits <= 6)
			{
				rr = (c >> 16) & 0xFF;
				gg = (c >>  8) & 0xFF;
				bb = (c >>  0) & 0xFF;
				aa = 255;
			}
			else
			{
				rr = (c >> 24) & 0xFF;
				gg = (c >> 16) & 0xFF;
				bb = (c >>  8) & 0xFF;
				aa = (c >>  0) & 0xFF;
			}
		}
		// --- Step 3b: Functional form ---
		else if ((se.startsWith("(") || se.startsWith("rgb(") || se.startsWith("rgba("))
				 && se.endsWith(")") && se.contains(","))
		{
			String inner = se.new_substr(se.indexOf("(") + 1, se.len() - 1).trim();
			auto tokens = inner.tokenize(",", true, false);
			const u32 n = tokens.count();
			if (n < 3 || n > 4) return false;

			i64 vals[4] = { 0, 0, 0, 255 };
			for (u32 i = 0; i < n; ++i)
			{
				String tok = tokens.next();
				if (tok.len() == 0 || !tok.isInt()) return false;
				const i64 v = tok.toInt();
				if (v < 0 || v > 255) return false;
				vals[i] = v;
			}
			rr = static_cast<u8>(vals[0]);
			gg = static_cast<u8>(vals[1]);
			bb = static_cast<u8>(vals[2]);
			aa = static_cast<u8>(vals[3]);
		}
		else
		{
			return false;
		}

		// --- Step 4: Build the output color and apply modifier ---
		if (out != nullptr)
		{
			out->set(rr, gg, bb, aa);
			if      (mod == Modifier::Lighten) out->lighten(modAmount);
			else if (mod == Modifier::Darken)  out->darken(modAmount);
		}
		return true;
	}
}
