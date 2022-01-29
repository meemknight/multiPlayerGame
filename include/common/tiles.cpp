#include "tiles.h"

namespace tiles
{
	constexpr const char *collisionMap =
		"-X------"
		"-----X--"
		"-X-XXX--"
		"-X-XXX--"
		"-XXXXX-X"
		"XXXXXXXX"
		"------XX"
		"---X----"
		"--------"
		;

	gl2d::TextureAtlasPadding textureAtlas{width, height, width * pixelSize, height * pixelSize};

	bool isSolid(int id)
	{
		return(collisionMap[id] == 'X');
	}


	glm::vec4 getTileUV(int id)
	{
		int x = id % width;
		int y = id / width;
		return textureAtlas.get(x, y);
	}


};