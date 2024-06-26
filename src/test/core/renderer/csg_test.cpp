#include <gtest/gtest.h>
#include <array>
#include <renderer/csg.h>
#include <math/intersections.h>
#include <math/sphere.h>
#include <math/cube.h>
#include <math/ray.h>
#include <RML.h>

namespace CppRayTracerChallenge::Core::Renderer
{
	using Math::Sphere;
	using Math::Cube;
	using Math::Intersections;
	using Math::Ray;
	using RML::Point;

	constexpr CSG::Operation UNION = CSG::Operation::UNION;
	constexpr CSG::Operation INTERSECT = CSG::Operation::INTERSECT;
	constexpr CSG::Operation DIFFERENCE = CSG::Operation::DIFFERENCE;

	struct CSGRuleEval
	{
		CSGRuleEval(CSG::Operation op, bool lhit, bool inl, bool inr, bool result)
			: op(op), lhit(lhit), inl(inl), inr(inr), result(result) {};

		CSG::Operation op;
		bool lhit;
		bool inl;
		bool inr;
		bool result;
	};

	TEST(CppRayTracerChallenge_Core_Renderer_CSG, csg_is_created_with_an_operation_and_two_shapes)
	{
		auto sphere = std::make_shared<Sphere>();
		auto cube = std::make_shared<Cube>();

		auto s1 = std::make_shared<Shape>(sphere);
		auto s2 = std::make_shared<Shape>(cube);

		auto csg = CSG::build(UNION, s1, s2);

		EXPECT_EQ(csg->operation(), UNION);
		EXPECT_EQ(csg->left(), s1);
		EXPECT_EQ(csg->right(), s2);
		EXPECT_EQ(std::dynamic_pointer_cast<CSG>(s1->parent().lock()), csg);
		EXPECT_EQ(std::dynamic_pointer_cast<CSG>(s2->parent().lock()), csg);
	}

	TEST(CppRayTracerChallenge_Core_Renderer_CSG, evaluation_for_csg_union_rule)
	{
		std::array<CSGRuleEval, 8> ruleTable{ {
			{ UNION, true, true, true, false },
			{ UNION, true, true, false, true },
			{ UNION, true, false, true, false },
			{ UNION, true, false, false, true },
			{ UNION, false, true, true, false },
			{ UNION, false, true, false, false },
			{ UNION, false, false, true, true },
			{ UNION, false, false, false, true }
		} };

		for (auto& eval : ruleTable)
		{
			bool result = CSG::intersectionAllowed(eval.op, eval.lhit, eval.inl, eval.inr);
			EXPECT_EQ(result, eval.result);
		}
	}

	TEST(CppRayTracerChallenge_Core_Renderer_CSG, evaluation_for_csg_intersect_rule)
	{
		std::array<CSGRuleEval, 8> ruleTable{ {
			{ INTERSECT, true, true, true, true },
			{ INTERSECT, true, true, false, false },
			{ INTERSECT, true, false, true, true },
			{ INTERSECT, true, false, false, false },
			{ INTERSECT, false, true, true, true },
			{ INTERSECT, false, true, false, true },
			{ INTERSECT, false, false, true, false },
			{ INTERSECT, false, false, false, false }
		} };

		for (auto& eval : ruleTable)
		{
			bool result = CSG::intersectionAllowed(eval.op, eval.lhit, eval.inl, eval.inr);
			EXPECT_EQ(result, eval.result);
		}
	}

	TEST(CppRayTracerChallenge_Core_Renderer_CSG, evaluation_for_csg_difference_rule)
	{
		std::array<CSGRuleEval, 8> ruleTable{ {
			{ DIFFERENCE, true, true, true, false },
			{ DIFFERENCE, true, true, false, true },
			{ DIFFERENCE, true, false, true, false },
			{ DIFFERENCE, true, false, false, true },
			{ DIFFERENCE, false, true, true, true },
			{ DIFFERENCE, false, true, false, true },
			{ DIFFERENCE, false, false, true, false },
			{ DIFFERENCE, false, false, false, false }
		} };

		for (auto& eval : ruleTable)
		{
			bool result = CSG::intersectionAllowed(eval.op, eval.lhit, eval.inl, eval.inr);
			EXPECT_EQ(result, eval.result);
		}
	}

	TEST(CppRayTracerChallenge_Core_Renderer_CSG, filters_list_of_intersections)
	{
		struct Param
		{
			Param(CSG::Operation op, int x0, int x1)
				: op(op), x0(x0), x1(x1) {};

			CSG::Operation op;
			int x0;
			int x1;
		};

		std::array<Param, 3> params{ {
			{ UNION, 0, 3 },
			{ INTERSECT, 1, 2 },
			{ DIFFERENCE, 0, 1 }
		} };

		auto sphere = std::make_shared<Sphere>();
		auto cube = std::make_shared<Cube>();

		auto s1 = std::make_shared<Shape>(sphere);
		auto s2 = std::make_shared<Shape>(cube);

		for (auto& param : params)
		{
			auto csg = CSG::build(param.op, s1, s2);
			auto xs = Intersections({ {1, *s1}, {2, *s2}, {3, *s1}, {4, *s2} });

			auto result = csg->filterIntersections(xs);

			EXPECT_EQ(result.count(), 2);
			EXPECT_EQ(result.at(0), xs.at(param.x0));
			EXPECT_EQ(result.at(1), xs.at(param.x1));
		}
	}

	TEST(CppRayTracerChallenge_Core_Renderer_CSG, ray_misses_csg)
	{
		auto sphere = std::make_shared<Sphere>();
		auto cube = std::make_shared<Cube>();

		auto s1 = std::make_shared<Shape>(sphere);
		auto s2 = std::make_shared<Shape>(cube);

		auto csg = CSG::build(UNION, s1, s2);

		auto ray = Ray({ 0, 2, -5 }, { 0, 0, 1 });

		auto xs = csg->intersectLocal(ray);

		EXPECT_EQ(xs.size(), 0);
	}

	TEST(CppRayTracerChallenge_Core_Renderer_CSG, ray_hits_csg)
	{
		auto sphere1 = std::make_shared<Sphere>();
		auto sphere2 = std::make_shared<Sphere>();

		auto s1 = std::make_shared<Shape>(sphere1);
		auto s2 = std::make_shared<Shape>(sphere2);

		s2->transform(Transform().
			translate(0, 0, 0.5));

		auto csg = CSG::build(UNION, s1, s2);

		auto ray = Ray({ 0, 0, -5 }, { 0, 0, 1 });

		auto xs = csg->intersectLocal(ray);

		EXPECT_EQ(xs.count(), 2);
		EXPECT_EQ(&xs.at(0).shape(), s1.get());
		EXPECT_EQ(xs.at(0).t(), 4);
		EXPECT_EQ(&xs.at(1).shape(), s2.get());
		EXPECT_EQ(xs.at(1).t(), 6.5);
	}

	TEST(CppRayTracerChallenge_Core_Renderer_CSG, csg_has_bounding_box_that_contains_children)
	{
		auto sphere1 = std::make_shared<Sphere>();
		auto sphere2 = std::make_shared<Sphere>();

		auto left = std::make_shared<Shape>(sphere1);
		auto right = std::make_shared<Shape>(sphere2);

		right->transform(Transform().translate(2, 3, 4));

		auto shape = CSG::build(DIFFERENCE, left, right);

		auto box = shape->bounds();

		EXPECT_EQ(box.min(), Point(-1, -1, -1));
		EXPECT_EQ(box.max(), Point(3, 4, 5));
	}
}
