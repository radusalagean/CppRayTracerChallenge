#include <gtest/gtest.h>
#include <RML.h>
#include "renderer/shape.h"
#include "renderer/group.h"
#include "renderer/patterns/stripe.h"
#include "renderer/patterns/solid_color.h"
#include "renderer/patterns/test_pattern.h"
#include "math/sphere.h"
#include "math/bounding_box.h"
#include "math/comparison.h"

using namespace CppRayTracerChallenge::Core;
using namespace RML::Trig;
using Renderer::Shape;
using Renderer::Group;
using Renderer::Material;
using Renderer::Patterns::SolidColor;
using Renderer::Patterns::Stripe;
using Renderer::Patterns::TestPattern;
using Math::Sphere;
using Math::Transform;
using RML::Point;
using RML::Vector;
using Math::BoundingBox;
using Graphics::Color;

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_wraps_transform)
{
	auto sphere = std::make_shared<Sphere>(Sphere());
	Transform transform = Transform()
		.translate(2, 5, 20)
		.rotate(12, 30, 98);

	sphere->transform(transform);
	Shape shape = Shape(sphere);

	EXPECT_EQ(shape.transform(), sphere->transform());
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_wraps_normal)
{
	auto sphere = std::make_shared<Sphere>(Sphere());
	Shape shape = Shape(sphere);

	Point p = Point(2, 3, 4);

	EXPECT_EQ(sphere->normal(p), shape.normal(p));
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_wraps_normal_local)
{
	auto sphere = std::make_shared<Sphere>(Sphere());
	Shape shape = Shape(sphere);

	Point p = Point(2, 3, 4);

	EXPECT_EQ(sphere->normalLocal(p), shape.normalLocal(p));
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_has_a_default_material)
{
	auto sphere = std::make_shared<Sphere>();
	Shape shape = Shape(sphere);

	EXPECT_EQ(shape.material(), Material());
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_can_be_assigned_a_material)
{
	auto sphere = std::make_shared<Sphere>();
	Shape shape = Shape(sphere);

	Material material;
	material.pattern = std::make_shared<SolidColor>(Color(0.5, 0.5, 0.5));

	shape.material(material);

	EXPECT_EQ(shape.material(), material);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_has_parent_attribute)
{
	auto sphere = std::make_shared<Sphere>();
	Shape shape = Shape(sphere);

	EXPECT_EQ(shape.parent().expired(), true);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_has_pattern_with_object_transform)
{
	auto sphere = std::make_shared<Sphere>();
	Shape shape = Shape(sphere);

	shape.transform(Transform().scale(2, 2, 2));

	Material material;
	material.pattern = std::make_shared<TestPattern>();
	shape.material(material);

	Color result = shape.colorAt({ 2, 3, 4 });
	Color expectedResult = Color(1, 1.5f, 2);

	EXPECT_EQ(result, expectedResult);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_has_pattern_with_pattern_transform)
{
	auto sphere = std::make_shared<Sphere>();
	Shape shape = Shape(sphere);

	Material material;
	material.pattern = std::make_shared<TestPattern>();
	material.pattern->transform(Transform().scale(2, 2, 2));
	shape.material(material);

	Color result = shape.colorAt({ 2, 3, 4 });
	Color expectedResult = Color(1, 1.5f, 2);

	EXPECT_EQ(result, expectedResult);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_has_pattern_with_object_transform_and_pattern_transform)
{
	auto sphere = std::make_shared<Sphere>();
	Shape shape = Shape(sphere);

	Material material;
	material.pattern = std::make_shared<TestPattern>();
	material.pattern->transform(Transform().translate(0.5, 1, 1.5));
	shape.material(material);
	shape.transform(Transform().scale(2, 2, 2));

	Color result = shape.colorAt({ 2.5, 3, 3.5 });
	Color expectedResult = Color(0.75f, 0.5f, 0.25f);

	EXPECT_EQ(result, expectedResult);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, convert_point_from_world_to_object_space)
{
	auto g1 = std::make_shared<Group>();
	g1->transform(Transform().rotate(0, radians_to_degrees(PI/2), 0));

	auto g2 = std::make_shared<Group>();
	g2->transform(Transform().scale(2, 2, 2));

	g1->addChild(g2);

	auto sphere = std::make_shared<Sphere>();
	auto shape = std::make_shared<Shape>(sphere);

	shape->transform(Transform().translate(5, 0, 0));

	g2->addChild(shape);

	auto result = shape->worldToObject({ -2, 0, -10 });

	Point expectedResult = Point(0, 0, -1);

	EXPECT_EQ(result, expectedResult);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, convert_normal_from_object_to_world_space)
{
	auto g1 = std::make_shared<Group>();
	g1->transform(Transform().rotate(0, radians_to_degrees(PI / 2), 0));

	auto g2 = std::make_shared<Group>();
	g2->transform(Transform().scale(1, 2, 3));

	g1->addChild(g2);

	auto sphere = std::make_shared<Sphere>();
	auto shape = std::make_shared<Shape>(sphere);

	shape->transform(Transform().translate(5, 0, 0));

	g2->addChild(shape);

	auto result = shape->normalToWorld({ sqrt(3) / 3, sqrt(3) / 3, sqrt(3) / 3 });

	auto expectedResult = Vector(0.285714, 0.428571, -0.857143);

	EXPECT_EQ(result, expectedResult);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, find_normal_on_child_object)
{
	auto g1 = std::make_shared<Group>();
	g1->transform(Transform().rotate(0, radians_to_degrees(PI / 2), 0));

	auto g2 = std::make_shared<Group>();
	g2->transform(Transform().scale(1, 2, 3));

	g1->addChild(g2);

	auto sphere = std::make_shared<Sphere>();
	auto shape = std::make_shared<Shape>(sphere);

	shape->transform(Transform().translate(5, 0, 0));

	g2->addChild(shape);

	auto result = shape->normal({ 1.7321, 1.1547, -5.5774 });

	auto expectedResult = Vector(0.285704, 0.428543, -0.857161);

	EXPECT_EQ(result, expectedResult);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_equality)
{
	auto sphere = std::make_shared<Sphere>();
	Shape shape1 = Shape(sphere);
	Shape shape2 = Shape(sphere);

	EXPECT_TRUE(shape1 == shape2);
	EXPECT_FALSE(shape1 != shape2);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_inequality_shape)
{
	auto sphere1 = std::make_shared<Sphere>();
	auto sphere2 = std::make_shared<Sphere>();

	Shape shape1 = Shape(sphere1);
	Shape shape2 = Shape(sphere2);

	EXPECT_FALSE(shape1 == shape2);
	EXPECT_TRUE(shape1 != shape2);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, shape_inequality_material)
{
	auto sphere = std::make_shared<Sphere>();

	Shape shape1 = Shape(sphere);
	Shape shape2 = Shape(sphere);

	auto mat = shape1.material();
	mat.shininess = 0.253f;
	shape1.material(mat);

	EXPECT_FALSE(shape1 == shape2);
	EXPECT_TRUE(shape1 != shape2);
}

TEST(CppRayTracerChallenge_Core_Renderer_Shape, querying_bounding_box_in_parents_space)
{
	auto sphere = std::make_shared<Sphere>();
	
	sphere->transform(Transform()
		.scale(0.5, 2, 4)
		.translate(1, -3, 5));

	BoundingBox box = sphere->parentSpaceBounds();

	Point expectedMin = Point(0.5, -5, 1);
	Point expectedMax = Point(1.5, -1, 9);

	EXPECT_EQ(box.min(), expectedMin);
	EXPECT_EQ(box.max(), expectedMax);
}
