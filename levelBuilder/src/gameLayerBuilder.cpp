#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "tiles.h"
#include "Phisics.h"

gl2d::Renderer2D renderer;

gl2d::Font font;
gl2d::Texture sprites;

struct GameData
{
	float posx=100;
	float posy=100;

}gameData;


phisics::MapData map;

bool initGame()
{
	renderer.create();
	sprites.loadFromFileWithPixelPadding(RESOURCES_PATH "jawbreaker_tiles.png", tiles::pixelSize, true, true);

	if(!platform::readEntireFile(RESOURCES_PATH "builderData.data", &gameData, sizeof(GameData)))
	{
		gameData = GameData();
	}

	if (!map.load(RESOURCES_PATH "mapData.txt"))
	{
		char mapInfo[] =
			"bbbbbbbbbb"
			"baaaaaaaab"
			"baaaaaaaab"
			"baaaaaaaab"
			"baaaaaaaab"
			"baaaaaaaab"
			"baaaaaaaab"
			"baaaaaaaab"
			"baaaaaaaab"
			"bbbbbbbbbb";

		for (int i = 0; i < sizeof(mapInfo); i++)
		{
			mapInfo[i] -= 'a';
		}

		map.create(10, 10, mapInfo);
	}
	

	return true;
}

bool gameLogic(float deltaTime)
{
#pragma region init stuff
	int w = 0; int h = 0;
	w= platform::getWindowSizeX();
	h = platform::getWindowSizeY();
	
	renderer.updateWindowMetrics(w, h);
	renderer.clearScreen();
#pragma endregion


#pragma region input
	float speed = 400 * deltaTime;

	if(platform::isKeyHeld(platform::Button::Up) 
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Up].held
		)
	{
		gameData.posy -= speed;
	}
	if (platform::isKeyHeld(platform::Button::Down)
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Down].held
		)
	{
		gameData.posy += speed;
	}
	if (platform::isKeyHeld(platform::Button::Left)
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Left].held
		)
	{
		gameData.posx -= speed;
	}
	if (platform::isKeyHeld(platform::Button::Right)
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Right].held
		)
	{
		gameData.posx += speed;
	}

	renderer.currentCamera.position.x = gameData.posx;
	renderer.currentCamera.position.y = gameData.posy;

#pragma endregion

	
	
#pragma region mouse picking

	
	auto mousePos = platform::getRelMousePosition();
	mousePos += renderer.currentCamera.position;
	mousePos /= worldMagnification;

	auto &clickedBlock = map.get(mousePos.x, mousePos.y);

	static char currentBlock = 0;
	bool inImgui = 0;

	ImGui::Begin("Block picker");
	{
		bool collidable = true;
		bool nonCollidable = true;

		ImGui::Checkbox("Show Collidable Blocks", &collidable);
		ImGui::Checkbox("Show Non-Collidable Blocks", &nonCollidable);

		unsigned short mCount = 0;
		ImGui::BeginChild("Block Selector");
		bool inImgui = ImGui::IsWindowHovered();

		if (collidable && nonCollidable)
		{
			unsigned short localCount = 0;
			while (mCount < tiles::tilesCount)
			{
				auto uv = tiles::getTileUV(mCount);

				ImGui::PushID(mCount);
				if (ImGui::ImageButton((void *)(intptr_t)sprites.id,
					{35,35}, {uv.x, uv.y}, {uv.z, uv.w}))
				{
					currentBlock = mCount;
				}
				ImGui::PopID();

				if (localCount % 10 != 0)
				{
					ImGui::SameLine();
				}
				localCount++;

				mCount++;
			}
		}
		else
		{
			if (collidable && !nonCollidable)
			{
				unsigned short localCount = 0;
				while (mCount < tiles::tilesCount)
				{
					if (tiles::isSolid(mCount))
					{
						auto uv = tiles::getTileUV(mCount);

						ImGui::PushID(mCount);
						if (ImGui::ImageButton((void *)(intptr_t)sprites.id,
							{35,35}, {uv.x, uv.y}, {uv.z, uv.w}));
						{
							currentBlock = mCount;
						}
						ImGui::PopID();

						if (localCount % 10 != 0)
						{
							ImGui::SameLine();
						}
						localCount++;

					}
					mCount++;
				}
			}
			else if (!collidable && nonCollidable)
			{
				unsigned short localCount = 0;
				while (mCount < tiles::tilesCount)
				{
					if (!tiles::isSolid(mCount))
					{
						auto uv = tiles::getTileUV(mCount);

						ImGui::PushID(mCount);
						if (ImGui::ImageButton((void *)(intptr_t)sprites.id,
							{35,35}, {uv.x, uv.y}, {uv.z, uv.w}));
						{
							currentBlock = mCount;
						}
						ImGui::PopID();

						if (localCount % 10 != 0)
						{
							ImGui::SameLine();
						}
						localCount++;

					}
					mCount++;
				}
			}
		}
		ImGui::EndChild();



	}
	ImGui::End();

	if (!inImgui && platform::isLMouseHeld())
	{
		clickedBlock.type = currentBlock;
	}


#pragma endregion





	
	map.render(renderer, sprites);


#pragma region set finishing stuff
	renderer.flush();

	return true;
#pragma endregion

}

void closeGame()
{

	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

	map.save(RESOURCES_PATH "mapData.txt");

}
