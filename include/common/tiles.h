#pragma once
#include <gl2d/gl2d.h>

namespace tiles
{

	constexpr int width = 8;
	constexpr int height = 9;
	constexpr int tilesCount = 72;
	constexpr int pixelSize = 8;

	bool isSolid(int id);

	glm::vec4 getTileUV(int id);
	

};