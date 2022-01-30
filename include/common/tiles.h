#pragma once
#include <gl2d/gl2d.h>

namespace tiles
{

	constexpr int width = 8;
	constexpr int height = 10;
	constexpr int tilesCount = 80;
	constexpr int pixelSize = 8;

	bool isSolid(int id);

	glm::vec4 getTileUV(int id);
	

};