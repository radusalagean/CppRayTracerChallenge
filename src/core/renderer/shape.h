#ifndef _CPPRAYTRACERCHALLENGE_CORE_RENDERER_SHAPE
#define _CPPRAYTRACERCHALLENGE_CORE_RENDERER_SHAPE

#include "material.h"
#include "../math/transform.h"
#include "../math/vector.h"
#include "../math/i_shape.h"

namespace CppRayTracerChallenge::Core::Renderer
{
	using namespace CppRayTracerChallenge::Core;

	/// <summary>
	/// Represents a shape within the rendered world.
	/// This is a wrapper around a Math::IShape which adds materials
	/// </summary>
	class Shape : public Math::IShape
	{
	public:
		Shape() = delete;

		Shape(std::shared_ptr<Math::IShape> shape);

		Shape(std::shared_ptr<Math::IShape>, Material material);

		/// <summary>
		/// Returns the material assigned to the shape
		/// </summary>
		/// <returns>Material assigned to the shape</returns>
		const Material material() const;

		/// <summary>
		/// Assigns a material to the shape
		/// </summary>
		/// <param name="material">Material to assign</param>
		void material(const Material material);

		// Delegating methods

		void transform(Math::Transform transform) override;
		const Math::Transform transform() const override;
		const Math::Vector normal(const Math::Point position) const override;
	private:
		std::shared_ptr<IShape> m_shape;
		Material m_material;
	};
}

#endif _CPPRAYTRACERCHALLENGE_CORE_RENDERER_SHAPE