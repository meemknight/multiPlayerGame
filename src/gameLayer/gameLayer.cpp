#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "Phisics.h"

gl2d::Renderer2D renderer;

gl2d::Font font;
gl2d::Texture sprites;

struct GameData
{
	phisics::Entity player;

}gameData;

phisics::MapData map;

bool initGame()
{
	renderer.create();
	font.createFromFile(RESOURCES_PATH "roboto_black.ttf");
	sprites.loadFromFileWithPixelPadding(RESOURCES_PATH "jawbreaker_tiles.png", tiles::pixelSize, true, true);

	if (!map.load(RESOURCES_PATH "mapData.txt"))
	{
		return false;
	}

	if(!platform::readEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData)))
	{
		gameData = GameData();
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
	float speed = 10 * deltaTime;
	float posy = 0;
	float posx = 0;

	if(platform::isKeyHeld(platform::Button::Up) 
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Up].held
		)
	{
		posy -= speed;
	}
	if (platform::isKeyHeld(platform::Button::Down)
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Down].held
		)
	{
		posy += speed;
	}
	if (platform::isKeyHeld(platform::Button::Left)
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Left].held
		)
	{
		posx -= speed;
	}
	if (platform::isKeyHeld(platform::Button::Right)
		|| platform::getControllerButtons().buttons[platform::ControllerButtons::Right].held
		)
	{
		posx += speed;
	}

	if (platform::isKeyPressedOn(platform::Button::Enter))
	{
		platform::setFullScreen(!platform::isFullScreen());
	}


#pragma endregion

	map.render(renderer, sprites);


#pragma region player

	gameData.player.move({posx, posy});
	gameData.player.resolveConstrains(map);
	gameData.player.updateMove();

	renderer.currentCamera.follow(gameData.player.pos * worldMagnification, deltaTime * 100, 2, w, h);

	gl2d::Texture none;
	none.id = 0;

	gameData.player.draw(renderer, deltaTime, none);

#pragma endregion

#pragma region imgui

	ImGui::Begin("debug");

	ImGui::InputFloat2("player", &gameData.player.pos.x);


	ImGui::End();

		
#pragma endregion






	

#pragma region set finishing stuff
	renderer.flush();

	return true;
#pragma endregion

}

void closeGame()
{

	platform::writeEntireFile(RESOURCES_PATH "gameData.data", &gameData, sizeof(GameData));

}
