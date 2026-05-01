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
		String se(color_string);
		se.trim();

		// --- Step 1: Extract optional .lighten(x) or .darken(x) suffix ---
		enum class Modifier { None, Lighten, Darken };
		Modifier mod = Modifier::None;
		f32 modAmount = 0.0f;

		{
			stdvec<String> matches;
			auto positions = se.regexFind("\\.(lighten|darken)\\(\\s*([0-9]*\\.?[0-9]+)\\s*\\)\\s*$", true, &matches);

			if (!positions.empty())
			{
				const String& whole = matches[0];
				mod = whole.contains("lighten") ? Modifier::Lighten : Modifier::Darken;

				// Extract the number between '(' and ')'
				u32 openParen = whole.indexOf("(");
				u32 closeParen = whole.indexOf(")");
				String numStr = whole.new_substr(openParen + 1, closeParen);
				numStr.trim();
				modAmount = numStr.toFloat();

				// Clamp to [0, 1]
				if (modAmount < 0.0f) modAmount = 0.0f;
				if (modAmount > 1.0f) modAmount = 1.0f;

				// Strip the suffix from se
				se = se.substr(0, positions[0]);
				se.trim();
			}
		}

		// --- Step 2: Default values ---
		r = 0;
		g = 0;
		b = 0;
		a = 255;

		// --- Step 3: Normalize "#" prefix to "0x" ---
		if (se.startsWith("#"))
		{
			String tmp = se.new_substr(1);
			tmp.trim();
			se = String("0x") + tmp;
		}

		// --- Step 4: Parse the color body ---
		if (se.startsWith("0x"))
		{
			String hexPart = se.new_substr(2);
			hexPart.trim();
			u32 hexDigits = hexPart.len();

			i64 ic = se.toInt();
			u32 c = static_cast<u32>(ic);

			if (hexDigits <= 6)
			{
				// RGB only — alpha defaults to 255
				r = (c >> 16) & 0xFF;
				g = (c >>  8) & 0xFF;
				b = (c >>  0) & 0xFF;
				a = 255;
			}
			else
			{
				// RGBA — 7 or 8 digits
				r = (c >> 24) & 0xFF;
				g = (c >> 16) & 0xFF;
				b = (c >>  8) & 0xFF;
				a = (c >>  0) & 0xFF;
			}
		}
		else if ((se.startsWith("(") || se.startsWith("rgba(") || se.startsWith("rgb("))
				 && se.endsWith(")") && se.contains(","))
		{
			se = se.substr(se.indexOf("(") + 1, se.len() - 1);
			se.trim();
			auto tokens = se.tokenize(",", true, false);
			if (tokens.count() < 3 || tokens.count() > 4)
			{
				OX_WARN("ostd::Color::set(const String&) -> Invalid rgb string format: %s.", color_string.c_str());
				return *this;
			}
			r = tokens.next().toInt();
			g = tokens.next().toInt();
			b = tokens.next().toInt();
			if (tokens.hasNext())
				a = tokens.next().toInt();
		}
		else
		{
			OX_WARN("ostd::Color::set(const String&) -> Unknown color string format: %s", color_string.c_str());
			return *this;
		}

		// --- Step 5: Apply lighten/darken modifier ---
		if (mod == Modifier::Lighten)
			lighten(modAmount);
		else if (mod == Modifier::Darken)
			darken(modAmount);

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

}
