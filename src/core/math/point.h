#ifndef _CPPRAYTRACERCHALLENGE_CORE_MATH_POINT
#define _CPPRAYTRACERCHALLENGE_CORE_MATH_POINT

#include "vector.h"
#include "tuple.h"

namespace CppRayTracerChallenge::Core::Math
{
	/// <summary>
	/// Point in Euclidean Space
	/// </summary>
	class Point : public Tuple
	{
	public:
		Point() = delete;

		/// <summary>
		/// Constructs a Point from a tuple, by copying x, y and z components.
		/// W component is set to 1.
		/// </summary>
		/// <param name="tuple">The tuple to convert to a Point</param>
		Point(const Tuple tuple) : Tuple(tuple.x(), tuple.y(), tuple.z(), 1) {};

		/// <summary>
		/// Constructs a Point with x, y and z components
		/// </summary>
		/// <param name="x">X component</param>
		/// <param name="y">Y component</param>
		/// <param name="z">Z component</param>
		Point(const double x, const double y, const double z) : Tuple(x, y, z, 1) {};
	};
}

#endif _CPPRAYTRACERCHALLENGE_CORE_MATH_POINT
