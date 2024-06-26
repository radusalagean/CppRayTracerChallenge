#include <gtest/gtest.h>
#include <RML.h>
#include <math/smooth_triangle.h>
#include <math/comparison.h>

namespace CppRayTracerChallenge::Core::Math
{
	struct SmoothTriangleFixture
	{
		SmoothTriangleFixture()
			: p1(RML::Point(0, 1, 0)),
			p2(RML::Point(-1, 0, 0)),
			p3(RML::Point(1, 0, 0)),
			n1(RML::Vector(0, 1, 0)),
			n2(RML::Vector(-1, 0, 0)),
			n3(RML::Vector(1, 0, 0)) {};
		Point p1, p2, p3;
		Vector n1, n2, n3;
		SmoothTriangle tri = SmoothTriangle(p1, p2, p3, n1, n2, n3);
	};

	TEST(CppRayTracerChallenge_Core_Math_SmoothTriangle, constructing_smooth_triangle)
	{
		SmoothTriangleFixture f;

		EXPECT_EQ(f.tri.p1(), f.p1);
		EXPECT_EQ(f.tri.p2(), f.p2);
		EXPECT_EQ(f.tri.p3(), f.p3);
		EXPECT_EQ(f.tri.n1(), f.n1);
		EXPECT_EQ(f.tri.n2(), f.n2);
		EXPECT_EQ(f.tri.n3(), f.n3);
	}

	TEST(CppRayTracerChallenge_Core_Math_SmoothTriangle, intersection_with_smooth_triangle_stores_uv)
	{
		SmoothTriangleFixture f;

		Ray ray = Ray({ -0.2, 0.3, -2 }, { 0, 0, 1 });
		Intersections intersections = f.tri.intersectLocal(ray);

		EXPECT_TRUE(Comparison::equal(intersections.at(0).u(), 0.45));
		EXPECT_TRUE(Comparison::equal(intersections.at(0).v(), 0.25));
	}

	TEST(CppRayTracerChallenge_Core_Math_SmoothTriangle, smooth_triangle_uses_uv_to_calculate_normal)
	{
		SmoothTriangleFixture f;

		Vector normal = f.tri.normal(0.45, 0.25);

		Vector expectedResult = Vector(-0.5547, 0.83205, 0);

		EXPECT_EQ(normal, expectedResult);
	}
}
