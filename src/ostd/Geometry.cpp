#include "Geometry.hpp"

#include <iostream>
#include "Utils.hpp"

namespace ostd
{
	String Vec2::toString(void) const
	{
		return StringEditor("{ ").addf(x).add(", ").addf(y).add(" }").str();
	}
	String Vec3::toString(void) const
	{
		return StringEditor("{ ").addf(x).add(", ").addf(y).add(", ").addf(z).add(" }").str();
	}
} //namesoace ox
