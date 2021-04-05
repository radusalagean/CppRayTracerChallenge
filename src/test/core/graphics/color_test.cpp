#include <gtest/gtest.h>
#include "graphics/color.h"

using namespace CppRayTracerChallenge::Core::Graphics;

TEST(CppRayTracerChallenge_Core_Graphics_Color, red)
{
	Color color(0.5f, 0.4f, 1.7f);

	float expectedResult = 0.5f;

	EXPECT_EQ(color.red(), expectedResult);
}

TEST(CppRayTracerChallenge_Core_Graphics_Color, green)
{
	Color color(0.5f, 0.4f, 1.7f);

	float expectedResult = 0.4f;

	EXPECT_EQ(color.green(), expectedResult);
}

TEST(CppRayTracerChallenge_Core_Graphics_Color, blue)
{
	Color color(0.5f, 0.4f, 1.7f);

	float expectedResult = 1.7f;

	EXPECT_EQ(color.blue(), expectedResult);
}

TEST(CppRayTracerChallenge_Core_Graphics_Color, adding_colors)
{
	Color colorA(0.9f, 0.6f, 0.75f);
	Color colorB(0.7f, 0.1f, 0.25f);

	Color expectedResult(1.6f, 0.7f, 1.0f);

	EXPECT_EQ(colorA + colorB, expectedResult);
}

TEST(CppRayTracerChallenge_Core_Graphics_Color, subtracting_colors)
{
	Color colorA(0.9f, 0.6f, 0.75f);
	Color colorB(0.7f, 0.1f, 0.25f);

	Color expectedResult(0.2f, 0.5f, 0.5f);

	EXPECT_EQ(colorA - colorB, expectedResult);
}

TEST(CppRayTracerChallenge_Core_Graphics_Color, multiply_color_by_scalar)
{
	Color colorA(0.2f, 0.3f, 0.4f);
	float scalar = 2.0f;

	Color expectedResult(0.4f, 0.6f, 0.8f);

	EXPECT_EQ(colorA * scalar, expectedResult);
}

TEST(CppRayTracerChallenge_Core_Graphics_Color, multiply_two_colors_together)
{
	Color colorA(1.0f, 0.2f, 0.4f);
	Color colorB(0.9f, 1.0f, 0.1f);

	Color expectedResult(0.9f, 0.2f, 0.04f);

	EXPECT_EQ(colorA * colorB, expectedResult);
}
