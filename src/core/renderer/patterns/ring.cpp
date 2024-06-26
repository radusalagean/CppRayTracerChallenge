#include "ring.h"

namespace CppRayTracerChallenge::Core::Renderer::Patterns
{
	Ring::Ring(Graphics::Color a, Graphics::Color b) : m_colorA(a), m_colorB(b) {};

	Graphics::Color Ring::colorA() const
	{
		return m_colorA;
	}

	Graphics::Color Ring::colorB() const
	{
		return m_colorB;
	}

	Graphics::Color Ring::colorAt(RML::Point position) const
	{
		if (static_cast<int>(floor(sqrt(pow(position.x(), 2) + pow(position.z(), 2)))) % 2 == 0)
		{
			return m_colorA;
		}

		return m_colorB;
	}

	bool Ring::operator==(const Ring& other) const
	{
		return m_colorA == other.m_colorA && m_colorB == other.m_colorB;
	}
}
