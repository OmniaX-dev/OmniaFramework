#include "Color.hpp"
#include <cmath>
#include "../io/Logger.hpp"
#include "../io/IOHandlers.hpp"

namespace ostd
{
	Color::Color(void) : r(*this), g(*this), b(*this), a(*this)
	{
		set();
		setTypeName("ox::Color");
		BaseObject::setValid(true);
	}

	Color::Color(uint8_t rgb_single_value, uint8_t alpha) : r(*this), g(*this), b(*this), a(*this)
	{
		set(rgb_single_value, alpha);
		setTypeName("ox::Color");
		BaseObject::setValid(true);
	}

	Color::Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t alpha) : r(*this), g(*this), b(*this), a(*this)
	{
		set(_r, _g, _b, alpha);
		setTypeName("ox::Color");
		BaseObject::setValid(true);
	}

	Color::Color(const String& color_string) : r(*this), g(*this), b(*this), a(*this)
	{
		set(color_string);
		setTypeName("ox::Color");
		BaseObject::setValid(true);
	}

	Color::Color(const FloatCol& normalized_color) : r(*this), g(*this), b(*this), a(*this)
	{
		set(normalized_color);
		setTypeName("ox::Color");
		BaseObject::setValid(true);
	}

	Color::Color(const Color& copy) : BaseObject(copy), r(*this), g(*this), b(*this), a(*this)
	{
		r = static_cast<uint8_t>(copy.r);
		g = static_cast<uint8_t>(copy.g);
		b = static_cast<uint8_t>(copy.b);
		a = static_cast<uint8_t>(copy.a);
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
		r = static_cast<uint8_t>(copy.r);
		g = static_cast<uint8_t>(copy.g);
		b = static_cast<uint8_t>(copy.b);
		a = static_cast<uint8_t>(copy.a);
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

	Color& Color::set(uint8_t rgb_single_value, uint8_t alpha)
	{
		r = rgb_single_value;
		g = rgb_single_value;
		b = rgb_single_value;
		a = alpha;
		return *this;
	}

	Color& Color::set(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t alpha)
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
		r = 0;
		g = 0;
		b = 0;
		a = 255;
		if (se.startsWith("#"))
		{
			String tmp = se.new_substr(1);
			tmp.trim();
			se = String("0x") + tmp;
		}
		if (se.startsWith("0x"))
		{
			int64_t ic = se.toInt();
			union uC32 {
				uint8_t data[4];
				uint32_t value;
			} c32_u;
			c32_u.value = static_cast<uint32_t>(ic);
			a = c32_u.data[0];
			b = c32_u.data[1];
			g = c32_u.data[2];
			r = c32_u.data[3];
		}
		else if ((se.startsWith("(") || se.startsWith("rgba(") || se.startsWith("rgb(")) && se.endsWith(")") && se.contains(","))
		{
			se = se.substr(se.indexOf("(") + 1, se.len() - 1);
			se.trim();
			auto tokens = se.tokenize(",", true, false);
			if (tokens.count() < 3 || tokens.count() > 4)
			{
				OX_WARN("ox::Color::set(const String&) -> Invalid rgb string format: %s.", color_string.c_str());
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
			OX_WARN("ox::Color::set(const String&) -> Unkown color string format: %s", color_string.c_str());
		}
		return *this;
	}

	Color& Color::set(const FloatCol& normalized_color)
	{
		r = static_cast<uint8_t>(std::round(normalized_color.r * 255));
		g = static_cast<uint8_t>(std::round(normalized_color.g * 255));
		b = static_cast<uint8_t>(std::round(normalized_color.b * 255));
		a = static_cast<uint8_t>(std::round(normalized_color.a * 255));
		return *this;
	}

	String Color::hexString(bool include_alpha, String prefix) const
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

	uint32_t Color::asInteger(eColorFormat format) const
	{
		union uC32 {
			uint8_t data[4];
			uint32_t value;
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

	void Color::RGBtoHSV(float r, float g, float b, float& h, float& s, float& v)
	{
	    float max = std::max({r, g, b});
	    float min = std::min({r, g, b});
	    float d = max - min;

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

	void Color::HSVtoRGB(float h, float s, float v, float& r, float& g, float& b)
	{
	    int i = static_cast<int>(h * 6);
	    float f = h * 6 - i;
	    float p = v * (1 - s);
	    float q = v * (1 - f * s);
	    float t = v * (1 - (1 - f) * s);

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
