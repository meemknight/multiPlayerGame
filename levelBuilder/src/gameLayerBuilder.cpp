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

#pragma endregion

	renderer.currentCamera.position.x = gameData.posx;
	renderer.currentCamera.position.y = gameData.posy;


	//renderer.renderRectangle({ gameData.posx,gameData.posy, 100, 100 }, { 0,0 }, 0, sprites, tiles::getTileUV(9));
	
	map.render(renderer, sprites);


#pragma region set finishing stuff
	renderer.flush();

	return true;
#pragma endregion

}

void closeGame()
{

	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

}
