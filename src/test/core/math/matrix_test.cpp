#include "gtest/gtest.h"
#include "math/matrix.h"

using namespace CppRayTracerChallenge::Core::Math;

TEST(CppRayTracerChallenge_Core_Math_Matrix, construct_blank_matrix_4x4)
{
	Matrix<float> matrix(4, 4);

	EXPECT_EQ(matrix(0, 0), 0);
	EXPECT_EQ(matrix(0, 1), 0);
	EXPECT_EQ(matrix(0, 2), 0);
	EXPECT_EQ(matrix(0, 3), 0);
	EXPECT_EQ(matrix(1, 0), 0);
	EXPECT_EQ(matrix(1, 1), 0);
	EXPECT_EQ(matrix(1, 2), 0);
	EXPECT_EQ(matrix(1, 3), 0);
	EXPECT_EQ(matrix(2, 0), 0);
	EXPECT_EQ(matrix(2, 1), 0);
	EXPECT_EQ(matrix(2, 2), 0);
	EXPECT_EQ(matrix(2, 3), 0);
	EXPECT_EQ(matrix(3, 0), 0);
	EXPECT_EQ(matrix(3, 1), 0);
	EXPECT_EQ(matrix(3, 2), 0);
	EXPECT_EQ(matrix(3, 3), 0);
}

TEST(CppRayTracerChallenge_Core_Math_Matric, construct_matrix_4x4)
{
	Matrix<float> matrix(4, 4, std::vector<float> {
		1, 2, 3, 4,
		5.5, 6.5, 7.5, 8.5,
		9, 10, 11, 12,
		13.5, 14.5, 15.5, 16.5
	});

	EXPECT_EQ(matrix(0, 0), 1);
	EXPECT_EQ(matrix(0, 1), 2);
	EXPECT_EQ(matrix(0, 2), 3);
	EXPECT_EQ(matrix(0, 3), 4);
	EXPECT_EQ(matrix(1, 0), 5.5f);
	EXPECT_EQ(matrix(1, 1), 6.5f);
	EXPECT_EQ(matrix(1, 2), 7.5f);
	EXPECT_EQ(matrix(1, 3), 8.5f);
	EXPECT_EQ(matrix(2, 0), 9);
	EXPECT_EQ(matrix(2, 1), 10);
	EXPECT_EQ(matrix(2, 2), 11);
	EXPECT_EQ(matrix(2, 3), 12);
	EXPECT_EQ(matrix(3, 0), 13.5f);
	EXPECT_EQ(matrix(3, 1), 14.5f);
	EXPECT_EQ(matrix(3, 2), 15.5f);
	EXPECT_EQ(matrix(3, 3), 16.5f);
}
