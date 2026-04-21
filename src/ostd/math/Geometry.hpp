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

#include <cmath>
#include <algorithm>
#include <ostd/string/String.hpp>

#define PI 3.1415926535898f
#define TWO_PI PI * 2.0f
#define HALF_PI PI / 2.0f
#define QUARTER_PI PI / 4.0f
#define DEG_TO_RAD(deg) (f32)(deg * (PI / 180.0f))
#define RAD_TO_DEG(rad) (f32)(rad * (180.0f / PI))

#define LERP(n1, n2, f) (n2 - n1) * f + n1
#define CAP(n, max) (n > max ? max : n)
#define CAPD(n, min) (n < min ? min : n)
#define CAPB(n, min, max) (n < min ? min : (n > max ? max : n))
#define MAX(n1, n2) std::max(n1, n2)
#define MIN(n1, n2) std::min(n1, n2)
#define PROPORTION(w, x, y) ((x * w) / y)

#define CONVERT_1D_2D(i, width) ostd::IPoint(i % width, i / width)
#define CONVERT_2D_1D(x, y, width) (x + width * y)

namespace ostd
{
	struct Vec2 : public __i_stringeable
	{
		//======================== Data ========================
		f32 x { 0.0f };
		f32 y { 0.0f };
		//======================================================


		//==================== Construction ====================
		inline Vec2(f32 xx = 0, f32 yy = 0) : x(xx), y(yy) {  }
		inline Vec2(const Vec2& v2) { set(v2); }
		inline Vec2& set(const Vec2& v2) { x = v2.x; y = v2.y; return *this; }
		inline Vec2& set(f32 xx, f32 yy) { x = xx; y = yy; return *this; }
		//======================================================


		//================== Static Functions ==================
		inline static Vec2 fromAngle(f32 angle) { return Vec2(std::cos(angle), std::sin(angle)); }
		inline static f32 angleBetween(const Vec2& v1, const Vec2& v2) { return std::acos(v1.dot(v2) / (v1.mag() * v2.mag())); }
		//======================================================


		//===================== Conversion =====================
		inline Vec2 toIsometric(void) const { return Vec2(x - y, (x + y) / 2.0f); }
		inline Vec2 toCartesian(void) const { return Vec2((2 * y + x) / 2.0f, (2 * y - x) / 2.0f); }
		inline String toString(void) const override { return String("{ ").add(x).add(", ").add(y).add(" }"); }
		//======================================================


		//===================== Operations =====================
		inline f32 mag(void) const { return std::sqrt((x * x) + (y * y)); }
		inline Vec2 add(Vec2 v2) const { return { x + v2.x, y + v2.y }; }
		inline Vec2 add(f32 x2, f32 y2) const { return { x + x2, y + y2 }; }
		inline Vec2 sub(Vec2 v2) const { return { x - v2.x, y - v2.y }; }
		inline Vec2 sub(f32 x2, f32 y2) const { return { x - x2, y - y2 }; }
		inline Vec2 mul(f32 scalar) const { return { x * scalar, y * scalar }; }
		inline Vec2 div(f32 scalar) const { return { x / scalar, y / scalar }; }
		inline Vec2 propx(f32 new_x) const { return { new_x, new_x * (y / x) }; }
		inline Vec2 propy(f32 new_y) const { return { new_y * (x / y), new_y }; }

		inline Vec2 normalize(void) const { f32 m = _zp(mag()); return { x / m,     y / m     };    }
		inline f32 dist(Vec2 v2) const { return std::sqrt((f32)((v2.x - x) * (v2.x - x)) + ((v2.y - y) * (v2.y - y))); }
		inline f32 heading(void) const { return std::atan2(y, x); }
		inline Vec2 rotate(f32 angle) const { return Vec2(*this).rotate(angle); }
		inline f32 dot(const Vec2& v2) const { return (x * v2.x) + (y * v2.y); }
		inline f32 cross(const Vec2& v2) const { return (x * v2.y) - (v2.x * y); }
		//======================================================


		//===================== Modifiers ======================
		inline Vec2& addm(const Vec2& v2) { x += v2.x; y += v2.y; return *this; }
		inline Vec2& addm(const f32& x2, const f32& y2) { x += x2; y += y2; return *this; }
		inline Vec2& subm(const Vec2& v2) { x -= v2.x; y -= v2.y; return *this; }
		inline Vec2& subm(const f32& x2, const f32& y2) { x -= x2; y -= y2; return *this; }
		inline Vec2& mulm(const f32& scalar) { x *= scalar; y *= scalar; return *this; }
		inline Vec2& divm(const f32& scalar) { x /= scalar; y /= scalar; return *this; }
		inline Vec2& propxm(const f32& new_x) { x = new_x; y = new_x * (y / x); return *this; }
		inline Vec2& propym(const f32& new_y) { x = new_y * (x / y); y = new_y; return *this; }

		inline Vec2& normalizem(void) { f32 m = _zp(mag()); x /= m; y /= m;     return *this;    }
		inline Vec2& setMag(const f32& mag) { return normalizem().mulm(mag); }
		inline Vec2& setHeading(const f32& angle) { f32 m = mag(); set(m * std::cos(angle), m * std::sin(angle)); return *this; }
		inline Vec2& rotate(const f32& angle) { return setHeading(heading() + angle); }
		inline Vec2& limit(const f32& max) { f32 msq = mag(); msq *= msq; if (msq > (max * max)) divm(std::sqrt(msq)).mulm(max); return *this; }
		//======================================================


		//===================== Operators ======================
		inline bool  operator==(const Vec2& op2) const { return (x == op2.x && y == op2.y); }
		inline bool  operator!=(const Vec2& op2) const { return (x != op2.x || y != op2.y); }
		inline Vec2  operator-() const { return { -x, -y }; }
		inline Vec2  operator+ (const Vec2& op2) const { return add(op2); }
		inline Vec2  operator- (const Vec2& op2) const { return sub(op2); }
		inline Vec2  operator+ (const f32& op2) const { return add(op2, op2); }
		inline Vec2  operator- (const f32& op2) const { return sub(op2, op2); }
		inline Vec2  operator* (const f32& op2) const { return mul(op2); }
		inline Vec2  operator/ (const f32& op2) const { return div(op2); }
		inline Vec2& operator= (const Vec2& val) { return set(val); }
		inline Vec2& operator= (const f32& val) { return set(val, val); }
		inline Vec2& operator+=(const Vec2& op2) { return addm(op2); }
		inline Vec2& operator-=(const Vec2& op2) { return subm(op2); }
		inline Vec2& operator+=(const f32& op2) { return addm(op2, op2); }
		inline Vec2& operator-=(const f32& op2) { return subm(op2, op2); }
		inline Vec2& operator*=(const f32& op2) { return mulm(op2); }
		inline Vec2& operator/=(const f32& op2) { return divm(op2); }
		//======================================================

	private:
		inline f32 _zp(f32 n1) const { return (n1 == 0 ? 1 : n1); }

	};

	template<class T>
	class Point : public __i_stringeable
	{
		public:
			T x;
			T y;

		public:
			inline Point(void)                                                : x(0), y(0) {  }
			inline Point(T xx, T yy)                                       : x(xx), y(yy) {}
			inline Point(const Vec2& vec)                                  { x = vec.x; y = vec.y; }
			inline Point<T>& set(const Point<T>& v2)                       { x = v2.x; y = v2.y; return *this; }
			inline Point<T>& set(f32 xx, f32 yy)                            { x = xx; y = yy; return *this; }

			//===================== Operators ======================
			inline bool  operator==(const Point<T>& op2 )         const    { return (x == op2.x && y == op2.y); }
			inline bool  operator!=(const Point<T>& op2 )         const    { return (x != op2.x || y != op2.y); }
			inline operator Vec2()                                const    { return { cast<f32>(x), cast<f32>(y) }; }
			inline Point<T>  operator+ (const Point<T>& op2 )     const    { return { x + op2.x, y + op2.y }; }
			inline Point<T>  operator- (const Point<T>& op2 )     const    { return { x - op2.x, y - op2.y }; }
			inline Point<T>  operator+ (const T& op2)             const    { return { x + op2, y + op2 }; }
			inline Point<T>  operator- (const T& op2)             const    { return { x + op2, y + op2 }; }
			inline Point<T>  operator* (const T& op2)             const    { return { x * op2, y * op2 }; }
			inline Point<T>  operator/ (const T& op2)             const    { return { x / op2, y / op2 }; }
			inline Point<T>& operator= (const Point<T>& val )              { return set(val); }
			inline Point<T>& operator= (const T& val)                      { return set(val, val); }
			inline Point<T>& operator+=(const Point<T>& op2 )              { x += op2.x; y += op2.y; return *this; }
			inline Point<T>& operator-=(const Point<T>& op2 )              { x -= op2.x; y -= op2.y; return *this; }
			inline Point<T>& operator+=(const T& op2)                      { x += op2; y += op2; return *this;; }
			inline Point<T>& operator-=(const T& op2)                      { x -= op2; y -= op2; return *this; }
			inline Point<T>& operator*=(const T& op2)                      { return x *= op2; y *= op2; return *this; }
			inline Point<T>& operator/=(const T& op2)                      { return x /= op2; y /= op2; return *this; }
			//======================================================

			template <class T2> inline Point(Point<T2> copy)
			{
				x = (T2)(copy.x);
				y = (T2)(copy.y);
			}

			inline Vec2 asVec2(void) const { return { cast<f32>(x), cast<f32>(y) }; }

			inline String toString(void) const override { return String("{ ").add(x).add(", ").add(y).add(" }"); }
	};

	typedef Point<f32> FPoint;
	typedef Point<f64> DPoint;
	typedef Point<u32> UIPoint;
	typedef Point<u64> UI64Point;
	typedef Point<u16> UI16Point;
	typedef Point<u8> UI8Point;
	typedef Point<i32> IPoint;
	typedef Point<i64> I64Point;
	typedef Point<i16> I16Point;
	typedef Point<i8> I8Point;

	struct Vec3 : public __i_stringeable
	{
		inline Vec3(f32 xx = 0, f32 yy = 0, f32 zz = 0)
		{
			x = xx;
			y = yy;
			z = zz;
		}
		inline Vec3(const Vec2& xy, f32 _z = 0.0f) { x = xy.x; y = xy.y; z = _z; }
		inline Vec2 xy(void) const { return Vec2(x, y); }
		inline Vec2 yz(void) const { return Vec2(y, z); }
		inline Vec2 zx(void) const { return Vec2(z, x); }
		inline String toString(void) const override { return String("{ ").add(x).add(", ").add(y).add(", ").add(z).add(" }"); }
		inline bool  operator==(const Vec3& op2 )     const    { return (x == op2.x && y == op2.y && op2.z == z); }
		inline bool  operator!=(const Vec3& op2 )     const    { return (x != op2.x || y != op2.y || op2.z != z); }
		inline Vec3& operator+=(const Vec3& op2 )             { x += op2.x; y += op2.y; z += op2.z; return *this; }

		f32 x;
		f32 y;
		f32 z;
	};

	struct Vec4 : public __i_stringeable
	{
		inline Vec4(f32 xx = 0, f32 yy = 0, f32 zz = 0, f32 ww = 0)
		{
			x = xx;
			y = yy;
			z = zz;
			w = ww;
		}
		inline Vec4(const Vec2& xy, f32 _z = 0.0f, f32 _w = 0.0f) { x = xy.x; y = xy.y; z = _z; w = _w; }
		inline Vec4(const Vec3& xyz, f32 _w = 0.0f) { x = xyz.x; y = xyz.y; z = xyz.z; w = _w; }
		inline Vec4(const Vec2& xy, const Vec2& zw) { x = xy.x; y = xy.y; z = zw.x; w = zw.y; }
		inline Vec2 xy(void) const { return Vec2(x, y); }
		inline Vec2 yz(void) const { return Vec2(y, z); }
		inline Vec2 zw(void) const { return Vec2(z, w); }
		inline Vec2 wx(void) const { return Vec2(w, x); }
		inline String toString(void) const override { return String("{ ").add(x).add(", ").add(y).add(", ").add(z).add(", ").add(w).add(" }"); }

		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};

	struct Triangle : public __i_stringeable
	{
		Vec2 vA;
		Vec2 vB;
		Vec2 vC;
		inline Triangle(void) {}
		inline Triangle(Vec2 a, Vec2 b, Vec2 c)
		{
			vA = a;
			vB = b;
			vC = c;
		}
		inline Triangle(f32 ax, f32 ay, f32 bx, f32 by, f32 cx, f32 cy)
		{
			vA.x = ax;
			vA.y = ay;
			vB.x = bx;
			vB.y = by;
			vC.x = cx;
			vC.y = cy;
		}
		inline bool contains(Vec2 p)
		{
			f32 d1, d2, d3;
			bool has_neg, has_pos;

			d1 = __sign(p, vA, vB);
			d2 = __sign(p, vB, vC);
			d3 = __sign(p, vC, vA);

			has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

			return !(has_neg && has_pos);
		}
		inline String toString(void) const override { return String("{ A: ").add(vA.toString()).add(", B: ").add(vB.toString()).add(", C: ").add(vC.toString()).add(" }"); }

	private:
		inline f32 __sign(Vec2 p1, Vec2 p2, Vec2 p3)
		{
			return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
		}
	};

	class Rectangle : public __i_stringeable
	{
		public:
			inline Rectangle(void) : x(0), y(0), w(0), h(0) {}
			inline Rectangle(f32 xx, f32 yy, f32 ww, f32 hh) : x(xx), y(yy), w(ww), h(hh) {}
			inline Rectangle(f32 xw, f32 yh, bool position = true) { if (position) { x = xw; y = yh; } else { w = xw; h = yh; } }
			inline Rectangle(f32 xx, f32 yy, Vec2 size) { x = xx; y = yy; w = size.x; h = size.y; }
			inline Rectangle(Vec2 position, f32 ww, f32 hh) { x = position.x; y = position.y; w = ww; h = hh; }
			inline Rectangle(Vec2 position, Vec2 size) { x = position.x; y = position.y; w = size.x; h = size.y; }
			virtual ~Rectangle(void) = default;

			inline bool operator==(const Rectangle& op2) const { return x == op2.x && y == op2.y && w == op2.w && h == op2.h; }
			inline bool operator!=(const Rectangle& op2) const { return !(*this == op2); }
			inline Rectangle operator+(const Rectangle& op2) const { return { x + op2.x, y + op2.y, w + op2.w, h + op2.h };     }
			inline Rectangle operator-(const Rectangle& op2) const {  return { x - op2.x, y - op2.y, w - op2.w, h - op2.h }; }
			inline Rectangle operator+(f32 s) const { return { x + s, y + s, w + s, h + s }; }
			inline Rectangle operator-(f32 s) const { return { x - s, y - s, w - s, h - s }; }
			inline Rectangle operator*(f32 s) const { return { x * s, y * s, w * s, h * s }; }
			inline Rectangle operator/(f32 s) const { return { x / s, y / s, w / s, h / s }; }
			inline Rectangle& operator=(const Rectangle& r) { x = r.x; y = r.y; w = r.w; h = r.h; return *this; }
			inline Rectangle& operator=(f32 s) { x = s; y = s; w = s; h = s; return *this; }
			inline Rectangle& operator+=(const Rectangle& op2) { x += op2.x; y += op2.y; w += op2.w; h += op2.h; return *this; }
			inline Rectangle& operator-=(const Rectangle& op2) { x -= op2.x; y -= op2.y; w -= op2.w; h -= op2.h; return *this; }
			inline Rectangle& operator+=(f32 s) { x += s; y += s; w += s; h += s; return *this; }
			inline Rectangle& operator-=(f32 s) { x -= s; y -= s; w -= s; h -= s; return *this; }
			inline Rectangle& operator*=(f32 s) { x *= s; y *= s; w *= s; h *= s; return *this; }
			inline Rectangle& operator/=(f32 s) { x /= s; y /= s; w /= s; h /= s; return *this; }

			inline virtual f32 getx(void) const { return x; }
			inline virtual f32 gety(void) const { return y; }
			inline virtual f32 getw(void) const { return w; }
			inline virtual f32 geth(void) const { return h; }
			inline virtual f32 getCenterX(void) const { return getx() + getw() / 2; }
			inline virtual f32 getCenterY(void) const { return gety() + geth() / 2; }

			inline virtual void setx(f32 xx) { x = xx; }
			inline virtual void sety(f32 yy) { y = yy; }
			inline virtual void setw(f32 ww) { w = ww; }
			inline virtual void seth(f32 hh) { h = hh; }

			inline virtual Vec2 getPosition(void) const { return Vec2(getx(), gety()); }
			inline virtual Vec2 getSize(void) const { return Vec2(getw(), geth()); }
			inline virtual Vec2 getCenter(void) const { return Vec2(getx() + getw() / 2, gety() + geth() / 2); }

			inline virtual void setPosition(const Vec2& pos) { setx(pos.x); sety(pos.y); }
			inline virtual void setPosition(f32 xx, f32 yy) { setx(xx); sety(yy); }
			inline virtual void setSize(Vec2 size) { setw(size.x); seth(size.y); }
			inline virtual void setSize(f32 ww, f32 hh) { setw(ww); seth(hh); }
			inline virtual void setBounds(f32 xx, f32 yy, f32 ww, f32 hh) { setx(xx); sety(yy); setw(ww); seth(hh); }

			inline virtual f32 addx(f32 xx) { setx(getx() + xx); return getx(); }
			inline virtual f32 addy(f32 yy) { sety(gety() + yy); return gety(); }
			inline virtual Vec2 addPos(f32 xx, f32 yy) { return Vec2(addx(xx), addy(yy)); }
			inline virtual Vec2 addPos(Vec2 pos) { return addPos(pos.x, pos.y); }
			inline virtual f32 addw(f32 ww) { setw(getw() + ww); return getw(); }
			inline virtual f32 addh(f32 hh) { seth(geth() + hh); return geth(); }
			inline virtual Vec2 addSize(f32 ww, f32 hh) { return Vec2(addw(ww), addh(hh)); }
			inline virtual Vec2 addSize(Vec2 size) { return addPos(size.x, size.y); }

			inline virtual f32 subx(f32 xx) { setx(getx() - xx); return getx(); }
			inline virtual f32 suby(f32 yy) { sety(gety() - yy); return gety(); }
			inline virtual Vec2 subPos(f32 xx, f32 yy) { return Vec2(subx(xx), suby(yy)); }
			inline virtual Vec2 subPos(Vec2 pos) { return subPos(pos.x, pos.y); }
			inline virtual f32 subw(f32 ww) { setw(getw() - ww); return getw(); }
			inline virtual f32 subh(f32 hh) { seth(geth() - hh); return geth(); }
			inline virtual Vec2 subSize(f32 ww, f32 hh) { return Vec2(subw(ww), subh(hh)); }
			inline virtual Vec2 subSize(Vec2 size) { return subPos(size.x, size.y); }

			inline virtual f32 mulx(f32 xx) { setx(getx() * xx); return getx(); }
			inline virtual f32 muly(f32 yy) { sety(gety() * yy); return gety(); }
			inline virtual Vec2 mulPos(f32 xx, f32 yy) { return Vec2(mulx(xx), muly(yy)); }
			inline virtual Vec2 mulPos(Vec2 pos) { return mulPos(pos.x, pos.y); }
			inline virtual f32 mulw(f32 ww) { setw(getw() * ww); return getw(); }
			inline virtual f32 mulh(f32 hh) { seth(geth() * hh); return geth(); }
			inline virtual Vec2 mulSize(f32 ww, f32 hh) { return Vec2(mulw(ww), mulh(hh)); }
			inline virtual Vec2 mulSize(Vec2 size) { return mulPos(size.x, size.y); }

			inline virtual f32 divx(f32 xx) { setx(getx() / xx); return getx(); }
			inline virtual f32 divy(f32 yy) { sety(gety() / yy); return gety(); }
			inline virtual Vec2 divPos(f32 xx, f32 yy) { return Vec2(divx(xx), divy(yy)); }
			inline virtual Vec2 divPos(Vec2 pos) { return divPos(pos.x, pos.y); }
			inline virtual f32 divw(f32 ww) { setw(getw() / ww); return getw(); }
			inline virtual f32 divh(f32 hh) { seth(geth() / hh); return geth(); }
			inline virtual Vec2 divSize(f32 ww, f32 hh) { return Vec2(divw(ww), divh(hh)); }
			inline virtual Vec2 divSize(Vec2 size) { return divPos(size.x, size.y); }

			inline virtual Vec2 topLeft(void) const { return getPosition(); }
			inline virtual Vec2 topRight(void) const { return Vec2(getx() + getw(), gety()); }
			inline virtual Vec2 bottomLeft(void) const { return Vec2(getx(), gety() + geth()); }
			inline virtual Vec2 bottomRight(void) const { return Vec2(getx() + getw(), gety() + geth()); }
			inline virtual Rectangle edgeRect(void) const { return { getx(), gety(), getx() + getw(), gety() + getw() }; }

			inline virtual f32 left(void) const { return getx(); }
			inline virtual f32 right(void) const { return getw(); }
			inline virtual f32 top(void) const { return gety(); }
			inline virtual f32 bottom(void) const { return geth(); }

			inline virtual String toString(void) const override { return String("{ ").add(x).add(", ").add(y).add(", ").add(w).add(", ").add(h).add(" }"); }

			inline virtual bool intersects(Rectangle rect, bool includeBounds = true) const
			{
				if (includeBounds)
				{
					if (x + w <= rect.x || x >= rect.x + rect.w)
						return false;
					if (y + h <= rect.y || y >= rect.y + rect.h)
						return false;
				}
				else
				{
					if (x + w < rect.x || x > rect.x + rect.w)
						return false;
					if (y + h < rect.y || y > rect.y + rect.h)
						return false;
				}
				return true;
			}
			inline virtual Rectangle getIntersection(Rectangle rect, bool includeBounds = true) const
			{
				if (!intersects(rect, includeBounds))
					return Rectangle();
				f32 leftX   = std::max(x, rect.x);
				f32 rightX  = std::min(x + w, rect.x + rect.w);
				f32 topY    = std::max(y, rect.y);
				f32 bottomY = std::min(y + h, rect.y + rect.h);
				return { leftX, topY, rightX - leftX, bottomY - topY };
			}
			inline virtual bool contains(Vec2 p, bool includeBounds = false) const
			{
				if (includeBounds)
					return p.x >= x && p.y >= y && p.x <= x + w && p.y <= y + h;
				else
					return p.x > x && p.y > y && p.x < x + w && p.y < y + h;
			}
			inline bool contains(f32 xx, f32 yy, bool includeBounds = false) const { return contains({ xx, yy }, includeBounds); }

			inline virtual f32 getDistance(Vec2 p) const { return sqrt(fabs((p.x - getx()) * (p.x - getx()) + (p.y - gety()) * (p.y - gety()))); }

		public:
			f32 x;
			f32 y;
			f32 w;
			f32 h;
	};

	template<class T>
	class Rect : public __i_stringeable
	{
		public:
			Rect(void) = default;
			inline Rect(T pos_x, T pos_y, T size_x, T size_y) : position(pos_x, pos_y), size(size_x, size_y) {  }
			inline Rect(Point<T> pos, Point<T> _size) : position(pos), size(_size) {  }
			template <class T2> inline Rect(Rect<T2> copy) { position = { (T2)(copy.position.x), (T2)(copy.position.y) }; size = { (T2)(copy.size.x), (T2)(copy.size.y) }; }
			inline String toString(void) const override { return String("{ ").add(position.x).add(", ").add(position.y).add(", ").add(size.x).add(", ").add(size.y).add(" }"); }

		public:
			Point<T> position;
			Point<T> size;
	};

	typedef Rect<f32> FRect;
	typedef Rect<f64> DRect;
	typedef Rect<u32> UIRect;
	typedef Rect<u64> UI64Rect;
	typedef Rect<u16> UI16Rect;
	typedef Rect<u8> UI8Rect;
	typedef Rect<i32> IRect;
	typedef Rect<i64> I64Rect;
	typedef Rect<i16> I16Rect;
	typedef Rect<i8> I8Rect;

	template<class T>
	class Line : public __i_stringeable
	{
		public:
			Line(void) = default;
			inline Line(T x1, T y1, T x2, T y2) : p1(x1, y1), p2(x2, y2) {  }
			inline Line(Point<T> _p1, Point<T> _p2) : p1(_p1), p2(_p2) {  }
			template <class T2> inline Line(Line<T2> copy) { p1 = { (T2)(copy.p1.x), (T2)(copy.p1.y) }; p2 = { (T2)(copy.p2.x), (T2)(copy.p2.y) }; }
			inline String toString(void) const override { return String("{ P1: ").add(p1.toString()).add(", P2").add(p2.toString()).add(" }"); }

		public:
			Point<T> p1;
			Point<T> p2;
	};

	typedef Line<f32> FLine;
	typedef Line<f64> DLine;
	typedef Line<u32> UILine;
	typedef Line<u64> UI64Line;
	typedef Line<f32> UI16Line;
	typedef Line<u8> UI8Line;
	typedef Line<i32> ILine;
	typedef Line<i64> I64Line;
	typedef Line<i16> I16Line;
	typedef Line<i8> I8Line;
}

namespace ogfx
{
	using Vec2 = ostd::Vec2;
	using Vec3 = ostd::Vec3;
	using Vec4 = ostd::Vec4;
	using Triangle = ostd::Triangle;
	using Rectangle = ostd::Rectangle;

	using FPoint = ostd::FPoint;
	using DPoint = ostd::DPoint;
	using UIPoint = ostd::UIPoint;
	using UI64Point = ostd::UI64Point;
	using UI16Point = ostd::UI16Point;
	using UI8Point = ostd::UI8Point;
	using IPoint = ostd::IPoint;
	using I64Point = ostd::I64Point;
	using I16Point = ostd::I16Point;
	using I8Point = ostd::I8Point;

	using FRect = ostd::FRect;
	using DRect = ostd::DRect;
	using UIRect = ostd::UIRect;
	using UI64Rect = ostd::UI64Rect;
	using UI16Rect = ostd::UI16Rect;
	using UI8Rect = ostd::UI8Rect;
	using IRect = ostd::IRect;
	using I64Rect = ostd::I64Rect;
	using I16Rect = ostd::I16Rect;
	using I8Rect = ostd::I8Rect;

	using FLine = ostd::FLine;
	using DLine = ostd::DLine;
	using UILine = ostd::UILine;
	using UI64Line = ostd::UI64Line;
	using UI16Line = ostd::UI16Line;
	using UI8Line = ostd::UI8Line;
	using ILine = ostd::ILine;
	using I64Line = ostd::I64Line;
	using I16Line = ostd::I16Line;
	using I8Line = ostd::I8Line;
}
