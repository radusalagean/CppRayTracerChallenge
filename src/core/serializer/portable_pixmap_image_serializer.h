#pragma once
#include "base_image_serializer.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>

namespace CppRayTracerChallenge::Core::Serializer
{
	/// <summary>
	/// A PortablePixmap serializer responsible for serializing an image to a buffer and vice versa
	/// http://netpbm.sourceforge.net/doc/ppm.html
	/// </summary>
	class PortablePixmapImageSerializer : public BaseImageSerializer
	{
	public:
		void serialize(Graphics::Image image) override;
		std::string fileExtension() const override;
	private:
		std::string buildHeader();
		std::string buildBody();
		std::string convertColorValue(float colorValue);
	};
}
