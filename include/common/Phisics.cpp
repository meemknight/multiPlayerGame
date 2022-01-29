#include "Phisics.h"

constexpr float worldMagnification = 64;

namespace phisics
{

	bool aabb(glm::vec4 b1, glm::vec4 b2)
	{
		if (((b1.x - b2.x < b2.z)
			&& b2.x - b1.x < b1.z
			)
			&& ((b1.y - b2.y < b2.w)
				&& b2.y - b1.y < b1.w
				)
			)
		{
			return 1;
		}
		return 0;
	}

	void Entity::resolveConstrains(MapData& mapData)
	{
		bool upTouch = 0;
		bool downTouch = 0;
		bool leftTouch = 0;
		bool rightTouch = 0;

		float distance = glm::length(lastPos - pos);
		const float BLOCK_SIZE = mapData.BLOCK_SIZE;

		if (distance < BLOCK_SIZE)
		{
			checkCollisionBrute(pos,
				lastPos,
				mapData,
				upTouch,
				downTouch,
				leftTouch,
				rightTouch
			);
		}
		else
		{
			glm::vec2 newPos = lastPos;
			glm::vec2 delta = pos - lastPos;
			delta = glm::normalize(delta);
			delta *= 0.9 * BLOCK_SIZE;

			do
			{
				newPos += delta;
				glm::vec2 posTest = newPos;
				checkCollisionBrute(newPos,
					lastPos,
					mapData,
					upTouch,
					downTouch,
					leftTouch,
					rightTouch);

				if (newPos != posTest)
				{
					pos = newPos;
					goto end;
				}

			} while (glm::length((newPos + delta) - pos) > 1.0f * BLOCK_SIZE);

			checkCollisionBrute(pos,
				lastPos,
				mapData,
				upTouch,
				downTouch,
				leftTouch,
				rightTouch);
		}

	end:

		//clamp the box if needed
		//if (pos.x < 0) { pos.x = 0; }
		//if (pos.x + dimensions.x > (mapData.w) * BLOCK_SIZE) { pos.x = ((mapData.w) * BLOCK_SIZE) - dimensions.x; }
		void;

	}

	void Entity::move(glm::vec2 dir)
	{

		pos += dir;

	}

	void Entity::updateMove()
	{

		if (lastPos.x - pos.x < 0)
		{
			movingRight = -1;
		}
		else if (lastPos.x - pos.x > 0)
		{
			movingRight = 0;
		}

		lastPos = pos;

	}

	void Entity::draw(gl2d::Renderer2D& renderer, float deltaTime, gl2d::Texture characterSprite)
	{

		//renderer.renderRectangle({ pos, dimensions }, {}, 0, characterSprite);
		renderer.renderRectangle({ pos* worldMagnification, dimensions * worldMagnification}, Colors_Turqoise);

	}

	void Entity::checkCollisionBrute(glm::vec2& pos, glm::vec2 lastPos, MapData& mapData, bool& upTouch, bool& downTouch, bool& leftTouch, bool& rightTouch)
	{
		glm::vec2 delta = pos - lastPos;
		const float BLOCK_SIZE = mapData.BLOCK_SIZE;

		if (
			(pos.y < -dimensions.y)
			|| (pos.x < -dimensions.x)
			|| (pos.y > mapData.h * BLOCK_SIZE)
			|| (pos.x > mapData.w * BLOCK_SIZE)
			)
		{
			return;
		}


		glm::vec2 newPos = performCollision(mapData, { pos.x, lastPos.y }, { dimensions.x, dimensions.y }, { delta.x, 0 },
			upTouch, downTouch, leftTouch, rightTouch);
		pos = performCollision(mapData, { newPos.x, pos.y }, { dimensions.x, dimensions.y }, { 0, delta.y },
			upTouch, downTouch, leftTouch, rightTouch);

	}

	glm::vec2 Entity::performCollision(MapData& mapData, glm::vec2 pos, glm::vec2 size, glm::vec2 delta, bool& upTouch, bool& downTouch, bool& leftTouch, bool& rightTouch)
	{
		int minX = 0;
		int minY = 0;
		int maxX = mapData.w;
		int maxY = mapData.h;

		const float BLOCK_SIZE = mapData.BLOCK_SIZE;

		minX = (pos.x - abs(delta.x) -BLOCK_SIZE) / BLOCK_SIZE;
		maxX = ceil((pos.x + abs(delta.x) + BLOCK_SIZE + size.x) / BLOCK_SIZE);

		minY = (pos.y - abs(delta.y) - BLOCK_SIZE) / BLOCK_SIZE;
		maxY = ceil((pos.y + abs(delta.y) + BLOCK_SIZE + size.y) / BLOCK_SIZE);

		minX = std::max(0, minX);
		minY = std::max(0, minY);
		maxX = std::min(mapData.w, maxX);
		maxY = std::min(mapData.h, maxY);

		for (int y = minY; y < maxY; y++)
			for (int x = minX; x < maxX; x++)
			{
				if (mapData.get(x, y).isCollidable())
				{
					if (aabb({ pos,dimensions }, { x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE }))
					{
						if (delta.x != 0)
						{
							if (delta.x < 0) // moving left
							{
								leftTouch = 1;
								pos.x = x * BLOCK_SIZE + BLOCK_SIZE;
								goto end;
							}
							else
							{
								rightTouch = 1;
								pos.x = x * BLOCK_SIZE - dimensions.x;
								goto end;
							}
						}
						else
						{
							if (delta.y < 0) //moving up
							{
								upTouch = 1;
								pos.y = y * BLOCK_SIZE + BLOCK_SIZE;
								goto end;
							}
							else
							{
								downTouch = 1;
								pos.y = y * BLOCK_SIZE - dimensions.y;
								goto end;
							}
						}

					}
				}

			}

	end:
		return pos;

	}


	void MapData::create(int w, int h, const char* d)
	{
		cleanup();

		this->w = w;
		this->h = h;

		data = new BlockInfo[w * h];

		if (d)
		{
			for (int i = 0; i < w * h; i++)
			{
				data[i].type = d[i];

			}
		}

	}

	BlockInfo& MapData::get(int x, int y)
	{
		if(x < 0 || y < 0 || x >= w || y >= h)
		{
			nullBlock = {};
			return nullBlock;
		}

		return data[x + this->w * y];

	}

	void MapData::render(gl2d::Renderer2D &renderer, gl2d::Texture texture)
	{
		for (int y = 0; y < h; y++)
		{
			for (int x = 0; x < w; x++)
			{
				renderer.renderRectangle(glm::vec4{x,y,1,1}*worldMagnification, {}, 0.f, texture, tiles::getTileUV(get(x, y).type));
			}
		}

	}

	void MapData::cleanup()
	{
		if (data)
		{
			delete[] data;
			data = nullptr;
		}

		w = 0;
		h = 0;
	}

	bool BlockInfo::isCollidable()
	{
		return tiles::isSolid(type);
	}

};