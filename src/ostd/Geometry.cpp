#include "Geometry.hpp"

#include <iostream>
#include "String.hpp"

namespace ostd
{
	String Vec2::toString(void) const
	{
		return String("{ ").add(x).add(", ").add(y).add(" }");
	}
	String Vec3::toString(void) const
	{
		return String("{ ").add(x).add(", ").add(y).add(", ").add(z).add(" }");
	}
} 
