#ifndef __GEOMETRY_HPP__
#define __GEOMETRY_HPP__

#include <cmath>
#include <algorithm>
#include <cstdint>
#include <ostd/String.hpp>

namespace ostd
{
	class OutputHandlerBase;
	template<class T>
	class Point
	{
		public:
			T x;
			T y;

		public:
			inline Point(void) : x(0), y(0) {}
			inline Point(T xx, T yy) : x(xx), y(yy) {}
				
			inline bool  operator==(const Point<T>& op2 ) 	const	{ return (x == op2.x && y == op2.y); }
			inline bool  operator!=(const Point<T>& op2 ) 	const	{ return (x != op2.x || y != op2.y); }

			template <class T2> inline Point(Point<T2> copy)
			{
				x = (T2)(copy.x);
				y = (T2)(copy.y);
			}
	};

	typedef Point<float> FPoint;
	typedef Point<double> DPoint;
	typedef Point<uint32_t> UIPoint;
	typedef Point<uint64_t> UI64Point;
	typedef Point<float> UI16Point;
	typedef Point<uint8_t> UI8Point;
	typedef Point<int32_t> IPoint;
	typedef Point<int64_t> I64Point;
	typedef Point<int16_t> I16Point;
	typedef Point<int8_t> I8Point;

	struct Vec2
	{
		//======================== Data ========================
		float x;
		float y;
		//======================================================


		//==================== Construction ====================
		inline Vec2(float xx = 0, float yy = 0) 			: x(xx), y(yy) {  }
		inline Vec2(const Vec2& v2) 						{ set(v2); }
		inline Vec2& set(const Vec2& v2) 					{ x = v2.x; y = v2.y; return *this; }
		inline Vec2& set(float xx, float yy) 				{ x = xx; y = yy; return *this; }
		//======================================================


		//================== Static Functions ==================
		inline static Vec2 fromAngle(float angle)			{ return Vec2(std::cos(angle), std::sin(angle)); }
		inline static float angleBetween(const Vec2& v1, const Vec2& v2) { return std::acos(v1.dot(v2) / (v1.mag() * v2.mag())); }
		//======================================================


		//===================== Conversion =====================
		inline Vec2 toIsometric(void) 			const		{ return Vec2(x - y, (x + y) / 2.0f); }
		inline Vec2 toCartesian(void) 			const 		{ return Vec2((2 * y + x) / 2.0f, (2 * y - x) / 2.0f); }
		String toString(void)					const;
		//======================================================


		//===================== Operations =====================
		inline float mag(void) 					const 		{ return std::sqrt((x * x) + (y * y)); 					}

		inline Vec2 add(Vec2 v2) 				const 		{ return {		x + v2.x, 			y + v2.y 		}; 	}
		inline Vec2 add(float x2, float y2) 	const 		{ return {		x + x2, 			y + y2 			}; 	}
		inline Vec2 sub(Vec2 v2) 				const 		{ return {		x - v2.x, 			y - v2.y 		}; 	}
		inline Vec2 sub(float x2, float y2) 	const 		{ return {		x - x2, 			y - y2 			}; 	}
		inline Vec2 mul(float scalar) 			const 		{ return {		x * scalar, 		y * scalar 		}; 	}
		inline Vec2 div(float scalar) 			const 		{ return {		x / scalar, 		y / scalar 		}; 	}

		inline Vec2 normalize(void)				const		{ float m = _zp(mag()); return { x / m, 	y / m 	};	}
		inline float dist(Vec2 v2)				const		{ return std::sqrt((float)((v2.x - x) * (v2.x - x)) + ((v2.y - y) * (v2.y - y))); }
		inline float heading(void)				const		{ return std::atan2(y, x); }
		inline Vec2 rotate(float angle)			const		{ return Vec2(*this).rotate(angle); }
		inline float dot(const Vec2& v2)		const		{ return (x * v2.x) + (y * v2.y); }
		inline float cross(const Vec2& v2)		const		{ return (x * v2.y) - (v2.x * y); }
		//======================================================


		//===================== Modifiers ======================
		inline Vec2& addm(const Vec2& v2) 			 		{ 		x += v2.x; 			y += v2.y; 	 	return *this;	}
		inline Vec2& addm(const float& x2, const float& y2) { 		x += x2; 			y += y2; 		return *this; 	}
		inline Vec2& subm(const Vec2& v2) 					{ 		x -= v2.x; 			y -= v2.y; 	 	return *this;	}
		inline Vec2& subm(const float& x2, const float& y2) { 		x -= x2; 			y -= y2; 		return *this; 	}
		inline Vec2& mulm(const float& scalar) 				{ 		x *= scalar; 		y *= scalar; 	return *this; 	}
		inline Vec2& divm(const float& scalar) 				{ 		x /= scalar; 		y /= scalar; 	return *this; 	}

		inline Vec2& normalizem(void)						{ float m = _zp(mag()); x /= m; y /= m; 	return *this;	}
		inline Vec2& setMag(const float& mag)				{ return normalizem().mulm(mag); }
		inline Vec2& setHeading(const float& angle) 		{ float m = mag(); set(m * std::cos(angle), m * std::sin(angle)); return *this; }
		inline Vec2& rotate(const float& angle)				{ return setHeading(heading() + angle); }
		inline Vec2& limit(const float& max)				{ float msq = mag(); msq *= msq; if (msq > (max * max)) divm(std::sqrt(msq)).mulm(max); return *this; }
		//======================================================


		//===================== Operators ======================
		inline bool  operator==(const Vec2& op2 ) 	const	{ return (x == op2.x && y == op2.y); }
		inline bool  operator!=(const Vec2& op2 ) 	const	{ return (x != op2.x || y != op2.y); }
		inline Vec2  operator+ (const Vec2& op2 ) 	const	{ return add(op2); }
		inline Vec2  operator- (const Vec2& op2 ) 	const	{ return sub(op2); }
		inline Vec2  operator+ (const float& op2) 	const	{ return add(op2, op2); }
		inline Vec2  operator- (const float& op2) 	const	{ return sub(op2, op2); }
		inline Vec2  operator* (const float& op2) 	const	{ return mul(op2); }
		inline Vec2  operator/ (const float& op2) 	const	{ return div(op2); }
		inline Vec2& operator= (const Vec2& val ) 			{ return set(val); }
		inline Vec2& operator= (const float& val) 			{ return set(val, val); }
		inline Vec2& operator+=(const Vec2& op2 ) 			{ return addm(op2); }
		inline Vec2& operator-=(const Vec2& op2 ) 			{ return subm(op2); }
		inline Vec2& operator+=(const float& op2) 			{ return addm(op2, op2); }
		inline Vec2& operator-=(const float& op2) 			{ return subm(op2, op2); }
		inline Vec2& operator*=(const float& op2) 			{ return mulm(op2); }
		inline Vec2& operator/=(const float& op2) 			{ return divm(op2); }

		friend std::ostream& operator<<(std::ostream& out, const Vec2& val);
		//======================================================

	private:
		inline float _zp(float n1) const { return (n1 == 0 ? 1 : n1); }	

	};

	inline std::ostream& operator<<(std::ostream& out, const Vec2& val)
	{
		out << val.toString();
		return out;
	}

	struct Vec3
	{
		inline Vec3(float xx = 0, float yy = 0, float zz = 0)
		{
			x = xx;
			y = yy;
			z = zz;
		}
		inline Vec3(const Vec2& xy, float _z = 0.0f) { x = xy.x; y = xy.y; z = _z; }
		inline Vec2 xy(void) const { return Vec2(x, y); }
		inline Vec2 yz(void) const { return Vec2(y, z); }
		inline Vec2 zx(void) const { return Vec2(z, x); }
		String toString(void) const;
		inline bool  operator==(const Vec3& op2 ) 	const	{ return (x == op2.x && y == op2.y && op2.z == z); }
		inline bool  operator!=(const Vec3& op2 ) 	const	{ return (x != op2.x || y != op2.y || op2.z != z); }
		inline Vec3& operator+=(const Vec3& op2 ) 			{ x += op2.x; y += op2.y; z += op2.z; return *this; }

		friend std::ostream& operator<<(std::ostream& out, const Vec2& val);

		float x;
		float y;
		float z;
	};

	inline std::ostream& operator<<(std::ostream& out, const Vec3& val)
	{
		out << val.toString();
		return out;
	}

	struct Vec4
	{
		inline Vec4(float xx = 0, float yy = 0, float zz = 0, float ww = 0)
		{
			x = xx;
			y = yy;
			z = zz;
			w = ww;
		}
		inline Vec4(const Vec2& xy, float _z = 0.0f, float _w = 0.0f) { x = xy.x; y = xy.y; z = _z; w = _w; }
		inline Vec4(const Vec3& xyz, float _w = 0.0f) { x = xyz.x; y = xyz.y; z = xyz.z; w = _w; }
		inline Vec4(const Vec2& xy, const Vec2& zw) { x = xy.x; y = xy.y; z = zw.x; w = zw.y; }
		inline Vec2 xy(void) const { return Vec2(x, y); }
		inline Vec2 yz(void) const { return Vec2(y, z); }
		inline Vec2 zw(void) const { return Vec2(z, w); }
		inline Vec2 wx(void) const { return Vec2(w, x); }


		float x;
		float y;
		float z;
		float w;
	};

	struct Triangle
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
		inline Triangle(float ax, float ay, float bx, float by, float cx, float cy)
		{
			vA.x = ax;
			vA.y = ay;
			vB.x = bx;
			vB.y = by;
			vC.x = cx;
			vC.y = cy;
		}
		bool contains(Vec2 p)
		{
			float d1, d2, d3;
			bool has_neg, has_pos;

			d1 = __sign(p, vA, vB);
			d2 = __sign(p, vB, vC);
			d3 = __sign(p, vC, vA);

			has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

			return !(has_neg && has_pos);
		}

	private:
		inline float __sign(Vec2 p1, Vec2 p2, Vec2 p3)
		{
			return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
		}
	};

	class Rectangle
	{
		public:
			inline Rectangle(void) : x(0), y(0), w(0), h(0) {}
			inline Rectangle(float xx, float yy, float ww, float hh) : x(xx), y(yy), w(ww), h(hh) {}
			inline Rectangle(float xw, float yh, bool position = true) { if (position) { x = xw; y = yh; } else { w = xw; h = yh; } }
			inline Rectangle(float xx, float yy, Vec2 size) { x = xx; y = yy; w = size.x; h = size.y; }
			inline Rectangle(Vec2 position, float ww, float hh) { x = position.x; y = position.y; w = ww; h = hh; }
			inline Rectangle(Vec2 position, Vec2 size) { x = position.x; y = position.y; w = size.x; h = size.y; }
			virtual ~Rectangle(void) = default;

			inline virtual float getx(void) const { return x; }
			inline virtual float gety(void) const { return y; }
			inline virtual float getw(void) const { return w; }
			inline virtual float geth(void) const { return h; }
			inline virtual float getCenterX(void) const { return getx() + getw() / 2; }
			inline virtual float getCenterY(void) const { return gety() + geth() / 2; }

			inline virtual void setx(float xx) { x = xx; }
			inline virtual void sety(float yy) { y = yy; }
			inline virtual void setw(float ww) { w = ww; }
			inline virtual void seth(float hh) { h = hh; }

			inline virtual Vec2 getPosition(void) const { return Vec2(getx(), gety()); }
			inline virtual Vec2 getSize(void) const { return Vec2(getw(), geth()); }
			inline virtual Vec2 getCenter(void) const { return Vec2(getx() + getw() / 2, gety() + geth() / 2); }

			inline virtual void setPosition(Vec2 pos) { setx(pos.x); sety(pos.y); }
			inline virtual void setPosition(float xx, float yy) { setx(xx); sety(yy); }
			inline virtual void setSize(Vec2 size) { setw(size.x); seth(size.y); }
			inline virtual void setSize(float ww, float hh) { setw(ww); seth(hh); }
			inline virtual void setBounds(float xx, float yy, float ww, float hh) { setx(xx); sety(yy); setw(ww); seth(hh); }

			inline virtual float addx(float xx) { setx(getx() + xx); return getx(); }
			inline virtual float addy(float yy) { sety(gety() + yy); return gety(); }
			inline virtual Vec2 addPos(float xx, float yy) { return Vec2(addx(xx), addy(yy)); }
			inline virtual Vec2 addPos(Vec2 pos) { return addPos(pos.x, pos.y); }
			inline virtual float addw(float ww) { setw(getw() + ww); return getw(); }
			inline virtual float addh(float hh) { seth(geth() + hh); return geth(); }
			inline virtual Vec2 addSize(float ww, float hh) { return Vec2(addw(ww), addh(hh)); }
			inline virtual Vec2 addSize(Vec2 size) { return addPos(size.x, size.y); }

			inline virtual float subx(float xx) { setx(getx() - xx); return getx(); }
			inline virtual float suby(float yy) { sety(gety() - yy); return gety(); }
			inline virtual Vec2 subPos(float xx, float yy) { return Vec2(subx(xx), suby(yy)); }
			inline virtual Vec2 subPos(Vec2 pos) { return subPos(pos.x, pos.y); }
			inline virtual float subw(float ww) { setw(getw() - ww); return getw(); }
			inline virtual float subh(float hh) { seth(geth() - hh); return geth(); }
			inline virtual Vec2 subSize(float ww, float hh) { return Vec2(subw(ww), subh(hh)); }
			inline virtual Vec2 subSize(Vec2 size) { return subPos(size.x, size.y); }

			inline virtual float mulx(float xx) { setx(getx() * xx); return getx(); }
			inline virtual float muly(float yy) { sety(gety() * yy); return gety(); }
			inline virtual Vec2 mulPos(float xx, float yy) { return Vec2(mulx(xx), muly(yy)); }
			inline virtual Vec2 mulPos(Vec2 pos) { return mulPos(pos.x, pos.y); }
			inline virtual float mulw(float ww) { setw(getw() * ww); return getw(); }
			inline virtual float mulh(float hh) { seth(geth() * hh); return geth(); }
			inline virtual Vec2 mulSize(float ww, float hh) { return Vec2(mulw(ww), mulh(hh)); }
			inline virtual Vec2 mulSize(Vec2 size) { return mulPos(size.x, size.y); }

			inline virtual float divx(float xx) { setx(getx() / xx); return getx(); }
			inline virtual float divy(float yy) { sety(gety() / yy); return gety(); }
			inline virtual Vec2 divPos(float xx, float yy) { return Vec2(divx(xx), divy(yy)); }
			inline virtual Vec2 divPos(Vec2 pos) { return divPos(pos.x, pos.y); }
			inline virtual float divw(float ww) { setw(getw() / ww); return getw(); }
			inline virtual float divh(float hh) { seth(geth() / hh); return geth(); }
			inline virtual Vec2 divSize(float ww, float hh) { return Vec2(divw(ww), divh(hh)); }
			inline virtual Vec2 divSize(Vec2 size) { return divPos(size.x, size.y); }

			inline virtual Vec2 topLeft(void) const { return getPosition(); }
			inline virtual Vec2 topRight(void) const { return Vec2(getx() + getw(), gety()); }
			inline virtual Vec2 bottomLeft(void) const { return Vec2(getx(), gety() + geth()); }
			inline virtual Vec2 bottomRight(void) const { return Vec2(getx() + getw(), gety() + geth()); }

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
				float leftX   = std::max(x, rect.x);
				float rightX  = std::min(x + w, rect.x + rect.w);
				float topY    = std::max(y, rect.y);
				float bottomY = std::min(y + h, rect.y + rect.h);
				return { leftX, topY, rightX - leftX, bottomY - topY };
			}
			inline virtual bool contains(Vec2 p, bool includeBounds = false) const
			{
				if (includeBounds)
					return p.x >= x && p.y >= y & p.x <= x + w && p.y <= y + h;
				else
					return p.x > x && p.y > y & p.x < x + w && p.y < y + h;
			}
			inline virtual bool contains(float xx, float yy, bool includeBounds = false) const { return contains({ xx, yy }); }

			inline virtual float getDistance(Vec2 p) const { return sqrt(fabs((p.x - getx()) * (p.x - getx()) + (p.y - gety()) * (p.y - gety()))); }

		public:
			float x;
			float y;
			float w;
			float h;
	};

	template<class T>
	class Rect
	{
		public:
			Rect(void) = default;
			inline Rect(T pos_x, T pos_y, T size_x, T size_y) : position(pos_x, pos_y), size(size_x, size_y) {  }
			inline Rect(Point<T> pos, Point<T> _size) : position(pos), size(_size) {  }
			template <class T2> inline Rect(Rect<T2> copy) { position = { (T2)(copy.position.x), (T2)(copy.position.y) }; size = { (T2)(copy.size.x), (T2)(copy.size.y) }; }

		public:
			Point<T> position;
			Point<T> size;
	};

	typedef Rect<float> FRect;
	typedef Rect<double> DRect;
	typedef Rect<uint32_t> UIRect;
	typedef Rect<uint64_t> UI64Rect;
	typedef Rect<float> UI16Rect;
	typedef Rect<uint8_t> UI8Rect;
	typedef Rect<int32_t> IRect;
	typedef Rect<int64_t> I64Rect;
	typedef Rect<int16_t> I16Rect;
	typedef Rect<int8_t> I8Rect;

	template<class T>
	class Line
	{
		public:
			Line(void) = default;
			inline Line(T x1, T y1, T x2, T y2) : p1(x1, y1), p2(x2, y2) {  }
			inline Line(Point<T> _p1, Point<T> _p2) : p1(_p1), p2(_p2) {  }
			template <class T2> inline Line(Line<T2> copy) { p1 = { (T2)(copy.p1.x), (T2)(copy.p1.y) }; p2 = { (T2)(copy.p2.x), (T2)(copy.p2.y) }; }

		public:
			Point<T> p1;
			Point<T> p2;
	};

	typedef Line<float> FLine;
	typedef Line<double> DLine;
	typedef Line<uint32_t> UILine;
	typedef Line<uint64_t> UI64Line;
	typedef Line<float> UI16Line;
	typedef Line<uint8_t> UI8Line;
	typedef Line<int32_t> ILine;
	typedef Line<int64_t> I64Line;
	typedef Line<int16_t> I16Line;
	typedef Line<int8_t> I8Line;
} 

 #endif
