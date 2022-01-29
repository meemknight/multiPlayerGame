#pragma once
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <algorithm>
#include <gl2d/gl2d.h>
#include "tiles.h"

#undef min
#undef max

constexpr float worldMagnification = 48;

namespace phisics
{
	
	struct BlockInfo
	{
		
		char type;
		
		bool isCollidable();
	};
	
	struct MapData
	{

		float BLOCK_SIZE = 1;
		
		BlockInfo* data;
		BlockInfo nullBlock = {};
	
		int w = 0;
		int h = 0;
	
		void create(int w, int h, const char* d);
		BlockInfo& get(int x, int y);
	
		void render(gl2d::Renderer2D &renderer, gl2d::Texture texture);

		void cleanup();
	
		bool load(const char *file);
		void save(const char *file);
	};
	
	struct Entity
	{
		glm::vec2 pos;
		glm::vec2 lastPos;
	
		glm::vec2 dimensions;
	
		void resolveConstrains(MapData& mapData);
	
	
		bool moving = 0;
	
	
		// 0 1  -> used for animations
		bool movingRight = 0;
	
	
		void move(glm::vec2 dir);
	
	
		//should be called only once per frame
		void updateMove();
	
		void draw(gl2d::Renderer2D& renderer, float deltaTime, gl2d::Texture characterSprite);
	
	
	private:
		void checkCollisionBrute(glm::vec2& pos, glm::vec2 lastPos, MapData& mapData,
			bool& upTouch, bool& downTouch, bool& leftTouch, bool& rightTouch);
		glm::vec2 performCollision(MapData& mapData, glm::vec2 pos, glm::vec2 size, glm::vec2 delta,
			bool& upTouch, bool& downTouch, bool& leftTouch, bool& rightTouch);
	};
	
	
	//pos and size on on every component
	bool aabb(glm::vec4 b1, glm::vec4 b2);


};
