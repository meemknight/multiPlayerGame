#include "Phisics.h"
#include <safeSave.h>
#include <chrono>

namespace phisics
{

	constexpr float pi = 3.141592653;

	bool aabb(glm::vec4 b1, glm::vec4 b2, float delta = 0)
	{
		b2.x += delta;
		b2.y += delta;
		b2.z -= delta*2;
		b2.w -= delta*2;

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
		if (distance == 0 || distance == INFINITY) { return; }

		const float BLOCK_SIZE = 1;;

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
		if (pos.x < 0) { pos.x = 0; }
		if (pos.y < 0) { pos.y = 0; }
		if (pos.x + dimensions.x > (mapData.w) * BLOCK_SIZE) { pos.x = ((mapData.w) * BLOCK_SIZE) - dimensions.x; }
		if (pos.y + dimensions.y > (mapData.h) * BLOCK_SIZE) { pos.y = ((mapData.h) * BLOCK_SIZE) - dimensions.y; }
		void;

	}

	void Entity::move(glm::vec2 dir)
	{

		pos += dir;

	}

	bool Entity::hit()
	{
		if (hitTime <= 0.f)
		{
			hitTime = invincibilityTime;
			return true;
		}
		else
		{
			return false;
		}
	}

	void Entity::updateMove(float deltaTime)
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
		
		hitTime -= deltaTime;
		if (hitTime <= 0.f)
		{
			hitTime = 0.f;
		}

	}

	void Entity::draw(gl2d::Renderer2D& renderer, float deltaTime, gl2d::Texture characterSprite)
	{

		glm::vec2 displacement = {};

		if (hitTime)
		{
			displacement.x = std::sin(hitTime * pi * 65) * (dimensions.x * 0.15);
			displacement.y = std::sin(pi + hitTime * pi * 25) * (dimensions.x * 0.03);
		}

		renderer.renderRectangle({ (pos+ displacement) * worldMagnification, dimensions * worldMagnification}, glm::vec4(color,1), {}, 0, characterSprite);
		//renderer.renderRectangle({ pos* worldMagnification, dimensions * worldMagnification}, Colors_Turqoise);

	}

	void Entity::checkCollisionBrute(glm::vec2& pos, glm::vec2 lastPos, MapData& mapData, bool& upTouch, bool& downTouch, bool& leftTouch, bool& rightTouch)
	{
		glm::vec2 delta = pos - lastPos;
		const float BLOCK_SIZE = 1;

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

		const float BLOCK_SIZE = 1;

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
					if (aabb({ pos,dimensions }, { x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE }, 0.0001f))
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
						else if(delta.y !=0)
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

	bool MapData::load(const char *file)
	{
		std::vector<char> data;
		auto err = sfs::readEntireFile(data, file);
		
		if (err != sfs::Errors::noError)
		{
			return 0;
		}
		
		char w = data[0];
		char h = data[1];
		
		create(w, h, &data[2]);
		return 1;
	}

	void MapData::save(const char *file)
	{
		char *buff = new char[w * h + 2];
		buff[0] = w;
		buff[1] = h;

		memcpy(buff + 2, data, w * h);

		sfs::writeEntireFile(buff, w * h + 2, file);

		delete[] buff;
	}

	bool BlockInfo::isCollidable()
	{
		return tiles::isSolid(type);
	}

	bool Bullet::checkCollisionMap(MapData &mapData)
	{
		//first check outsize walls
		if(pos.x - size/2.f <= 0
			|| pos.y - size / 2.f < 0.f
			|| pos.x + size / 2.f > mapData.w
			|| pos.y + size / 2.f > mapData.h
			)
		{
			return true;
		}

		glm::vec4 transform = getTransform();

		int minX = 0;
		int minY = 0;
		int maxX = mapData.w;
		int maxY = mapData.h;

		const float BLOCK_SIZE = 1;

		minX = (pos.x - BLOCK_SIZE) / BLOCK_SIZE;
		maxX = ceil((pos.x + BLOCK_SIZE + size) / BLOCK_SIZE);

		minY = (pos.y - BLOCK_SIZE) / BLOCK_SIZE;
		maxY = ceil((pos.y + BLOCK_SIZE + size) / BLOCK_SIZE);

		minX = std::max(0, minX);
		minY = std::max(0, minY);
		maxX = std::min(mapData.w, maxX);
		maxY = std::min(mapData.h, maxY);

		for (int y = minY; y < maxY; y++)
			for (int x = minX; x < maxX; x++)
			{
				if (mapData.get(x, y).isCollidable())
				{
					if (aabb(transform, {x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE}, 0.0001f))
					{
						return true;
					}
				}

			}


		return false;
	}

	bool Bullet::checkCollisionPlayer(Entity &e)
	{
		return aabb(getTransform(), glm::vec4(e.pos, e.dimensions), 0.f);
	}

	void Bullet::updateMove(float deltaTime)
	{
		pos += direction * deltaTime;
	}

	void Bullet::draw(gl2d::Renderer2D &renderer, gl2d::Texture bulletSprite)
	{
		renderer.renderRectangle({(pos - (glm::vec2(size,size) / 2.f)) * worldMagnification, glm::vec2(size,size) * worldMagnification},
			glm::vec4(color,1), {}, 0, bulletSprite);
		//renderer.renderRectangle({ (pos - (glm::vec2(size,size)/2.f)) * worldMagnification, glm::vec2(size,size) * worldMagnification}, Colors_Orange);
	}

	glm::vec4 Bullet::getTransform()
	{
		return glm::vec4(pos.x - size / 2.f, pos.y - size / 2.f, size, size);
	}

	bool Item::checkCollisionPlayer(Entity &e)
	{
		return aabb({pos, 1.f, 1.f}, {e.pos, e.dimensions}, 0.f);
	}

	void Item::draw(gl2d::Renderer2D &renderer, gl2d::Texture medkitTexture, gl2d::Texture bateryTexture)
	{
		auto duration = std::chrono::steady_clock::now().time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

		auto p = pos;
		p.y += 0.15f * std::sin(millis / 180.f);

		auto shadow = p + glm::vec2(0.1, 0.1);

		auto t = medkitTexture;
		if (itemType == itemTypeBatery)
		{
			t = bateryTexture;
		}

		renderer.renderRectangle({shadow * worldMagnification, glm::vec2(1,1) * worldMagnification},
			glm::vec4(0, 0, 0, 1), {}, 0, t);
		renderer.renderRectangle({p * worldMagnification, glm::vec2(1,1) * worldMagnification},
			glm::vec4(1,1,1, 1), {}, 0, t);
	}

};