#include <gtest/gtest.h>
#include <math/cube.h>

using namespace CppRayTracerChallenge::Core::Math;

class RayIntersectsCubeParam
{
public:
	RayIntersectsCubeParam(Point origin, Vector direction, float t1, float t2) : origin(origin), direction(direction), t1(t1), t2(t2) {};

	Point origin;
	Vector direction;
	float t1;
	float t2;

	friend std::ostream& operator<<(std::ostream& os, const RayIntersectsCubeParam& param)
	{
		os << ": origin(" << param.origin << "), dir(" << param.direction << "), t1(" << param.t1 << "), t2(" << param.t2 << ")";
		return os;
	};
};

TEST(CppRayTracerChallenge_Core_Math_Cube, ray_intersects_cube)
{
	std::vector<RayIntersectsCubeParam> paramsList = {
		RayIntersectsCubeParam({ 5, 0.5, 0 }, { -1, 0, 0 }, 4, 6), // +x
		RayIntersectsCubeParam({ -5, 0.5, 0 }, { 1, 0, 0 }, 4, 6), // -x
		RayIntersectsCubeParam({ 0.5, 5, 0 }, { 0, -1, 0 }, 4, 6), // +y
		RayIntersectsCubeParam({ 0.5, -5, 0 }, { 0, 1, 0 }, 4, 6), // -y
		RayIntersectsCubeParam({ 0.5, 0, 5 }, { 0, 0, -1 }, 4, 6), // +z
		RayIntersectsCubeParam({ 0.5, 0, -5 }, { 0, 0, 1 }, 4, 6), // -z
		RayIntersectsCubeParam({ 0, 0.5, 0 }, { 0, 0, 1 }, -1, 1) // inside
	};

	for (int i = 0; i < paramsList.size(); ++i)
	{
		RayIntersectsCubeParam& params = paramsList[i];

		Cube cube = Cube();
		Ray ray = Ray(params.origin, params.direction);
		Intersections intersections = cube.intersectLocal(ray);

		EXPECT_EQ(intersections.count(), 2);
		EXPECT_EQ(intersections.at(0).t(), params.t1);
		EXPECT_EQ(intersections.at(1).t(), params.t2);
	}
}

class RayMissesCubeParam
{
public:
	RayMissesCubeParam(Point origin, Vector direction) : origin(origin), direction(direction) {};

	Point origin;
	Vector direction;

	friend std::ostream& operator<<(std::ostream& os, const RayMissesCubeParam& param)
	{
		os << ": origin(" << param.origin << "), dir(" << param.direction << ")";
		return os;
	};
};

TEST(CppRayTracerChallenge_Core_Math_Cube, ray_misses_cube)
{
	std::vector<RayMissesCubeParam> paramsList = {
		RayMissesCubeParam({ -2, 0, 0 }, { 0.2673, 0.5345, 0.8018 }),
		RayMissesCubeParam({ 0, -2, 0 }, { 0.8018, 0.2673, 0.5345 }),
		RayMissesCubeParam({ 0, 0, -2 }, { 0.5345, 0.8018, 0.2673 }),
		RayMissesCubeParam({ 2, 0, 2 }, { 0, 0, -1 }),
		RayMissesCubeParam({ 0, 2, 2 }, { 0, -1, 0 }),
		RayMissesCubeParam({ 2, 2, 0 }, { -1, 0, 0 })
	};

	for (int i = 0; i < paramsList.size(); ++i)
	{
		RayMissesCubeParam& params = paramsList[i];

		Cube cube = Cube();
		Ray ray = Ray(params.origin, params.direction);
		Intersections intersections = cube.intersectLocal(ray);

		EXPECT_EQ(intersections.count(), 0);
	}
}
