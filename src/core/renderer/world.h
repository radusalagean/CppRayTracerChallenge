#pragma once
#include "point_light.h"
#include "shape.h"
#include "csg.h"
#include "computed_values.h"
#include "../math/ray.h"
#include "../math/intersections.h"
#include "../renderer/group.h"

namespace CppRayTracerChallenge::Core::Renderer
{
	/// <summary>
	/// Represents a world composed of multiple objects and light sources that can be rendererd to a canvas
	/// </summary>
	class World
	{
	public:
		/// <summary>
		/// Constructs a World that is completely empty
		/// </summary>
		World();

		/// <summary>
		/// Constructs a World that has a single light source and two spheres
		/// </summary>
		static World defaultWorld();

		/// <summary>
		/// Adds a light to the world
		/// </summary>
		/// <param name="light">The light to add</param>
		/// <returns>The world, after the light being added to it</returns>
		World addLight(PointLight light);

		/// <summary>
		/// Returns a const ref to the light at the index
		/// </summary>
		/// <param name="index">Index of the light</param>
		/// <returns>Const reference to light</returns>
		const PointLight& lightAt(int index) const;

		/// <summary>
		/// Clears all lights from the world
		/// </summary>
		/// <returns>The world, after all lights being destroyed from it</returns>
		World clearLights();

		/// <summary>
		/// Adds a object to the world
		/// </summary>
		/// <param name="obj">The object to add</param>
		/// <returns>The world, after the object being added to it</returns>
		World addObject(Shape obj);
		
		/// <summary>
		/// Adds a object to the world
		/// </summary>
		/// <param name="obj">Shared pointer to the object to add</param>
		/// <returns>The world, after the object being added to it</returns>
		World addObject(std::shared_ptr<Shape> obj);

		/// <summary>
		/// Adds a group to the world
		/// </summary>
		/// <param name="group">The group to add</param>
		/// <returns>The world, after the group being added to it</returns>
		World addGroup(Group group);

		/// <summary>
		/// Adds a CSG object to the world
		/// </summary>
		/// <param name="group">The csg to add</param>
		/// <returns>The world, after the csg being added to it</returns>
		World World::addCSG(CSG csg);

		/// <summary>
		/// Returns a const ref to the object at the index
		/// </summary>
		/// <param name="index">Index of the object</param>
		/// <returns>Const reference to object</returns>
		const Shape& objectAt(int index) const;

		/// <summary>
		/// Replaces the shape at the index with the new shape
		/// </summary>
		/// <param name="index">Index of the object to replace</param>
		/// <param name="shape">The shape to replace it with</param>
		void objectAt(int index, Shape shape);

		/// <summary>
		/// Returns how many objects are in the world
		/// </summary>
		/// <returns>How many objects are in the world</returns>
		int objectCount() const;

		/// <summary>
		/// Returns how many lights are in the world
		/// </summary>
		/// <returns>How many lights are in the world</returns>
		int lightCount() const;

		/// <summary>
		/// Returns the intersections between objects in the world and the ray
		/// </summary>
		/// <param name="ray">The ray to cast in the world</param>
		/// <returns>Intersections</returns>
		Math::Intersections intersectRay(const Math::Ray ray) const;

		/// <summary>
		/// Calculates the color in the world from precomputed values
		/// </summary>
		/// <param name="cv">The precomputed values</param>
		/// <returns>The color</returns>
		Graphics::Color shadeHit(const ComputedValues& cv) const;

		/// <summary>
		/// Calculates the color in the world from precomputed values
		/// </summary>
		/// <param name="cv">The precomputed values</param>
		/// <param name="remainingCalls">The remaining calls when this method is used recursively</param>
		/// <returns>The color</returns>
		Graphics::Color shadeHit(const ComputedValues& cv, int remainingCalls) const;

		/// <summary>
		/// Calculates the reflected color in the world from precomputed values
		/// </summary>
		/// <param name="cv">The precomputed values</param>
		/// <returns>The reflected color</returns>
		Graphics::Color reflectedColor(const ComputedValues& cv) const;

		/// <summary>
		/// Calculates the reflected color in the world from precomputed values
		/// </summary>
		/// <param name="cv">The precomputed values</param>
		/// <param name="remainingCalls">The remaining calls when this method is used recursively</param>
		/// <returns>The reflected color</returns>
		Graphics::Color reflectedColor(const ComputedValues& cv, int remainingCalls) const;

		/// <summary>
		/// Calculates the refracted color in the world from precomputed values
		/// </summary>
		/// <param name="cv">The precomputed values</param>
		/// <returns>The refracted color</returns>
		Graphics::Color refractedColor(const ComputedValues& cv) const;

		/// <summary>
		/// Calculates the refracted color in the world from precomputed values
		/// </summary>
		/// <param name="cv">The precomputed values</param>
		/// <param name="remainingCalls">The remaining calls when this method is used recursively</param>
		/// <returns>The refracted color</returns>
		Graphics::Color refractedColor(const ComputedValues& cv, int remainingCalls) const;

		/// <summary>
		/// Calculates if a position in world space is lit by a specific light
		/// </summary>
		/// <param name="position">The position to check if it is shaded</param>
		/// <param name="light">The light that is lighting the position</param>
		/// <returns>True if in shade, False otherwise</returns>
		bool isShadowed(const RML::Point& position, const PointLight& light) const;

		/// <summary>
		/// Calculates the color from the point of view of the passed in ray
		/// </summary>
		/// <param name="ray">Ray to trace along the world</param>
		/// <returns>The color from the traced ray</returns>
		Graphics::Color colorAt(const Math::Ray& ray) const;

		/// <summary>
		/// Calculates the color from the point of view of the passed in ray
		/// </summary>
		/// <param name="ray">Ray to trace along the world</param>
		/// <param name="remainingCalls">The remaining calls when this method is used recursively</param>
		/// <returns>The color from the traced ray</returns>
		Graphics::Color colorAt(const Math::Ray& ray, int remainingCalls) const;
		
		int defaultRemainingCalls = 16;
	private:
		std::vector<PointLight> m_lights;
		std::vector<Shape> m_objects;
		std::vector<Group> m_groups;
		std::vector<CSG> m_csgs;
	};
}
