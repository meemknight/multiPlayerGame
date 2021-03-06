#include "gameLayer.h"
#include "gl2d/gl2d.h"
#include "platformInput.h"
#include "imgui.h"
#include <iostream>
#include <sstream>
#include "Phisics.h"
#include <enet/enet.h>
#include "glui/glui.h"
#include "serverClient.h"
#include <thread>
#include <ctime>

gl2d::Renderer2D renderer;

Textures textures;

bool initGame()
{
	renderer.create();
	textures.font.createFromFile(RESOURCES_PATH "font/ANDYB.TTF");
	textures.sprites.loadFromFileWithPixelPadding(RESOURCES_PATH "jawbreaker_tiles.png", tiles::pixelSize, true, true);
	textures.character.loadFromFile(RESOURCES_PATH "character.png", true, true);
	textures.medKit.loadFromFile(RESOURCES_PATH "medkit.png", true, true);
	textures.battery.loadFromFile(RESOURCES_PATH "battery.png", true, true);
	textures.cross.loadFromFile(RESOURCES_PATH "cross.png", true, true);

	glui::gluiInit();

	if (enet_initialize() != 0)
	{
		return false;
	}

	std::srand(std::time(0));

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


	//0 main menu
	//1 client
	//2 server
	static int state = 0;
	static char ip[17] = {};
	static char name[playerNameSize] = {};
	
	if (state == 0)
	{
		glui::Text("Multi player game", Colors_White);
		glui::Text("Enter your name:", Colors_White);

		glui::InputText("Enter name##1", name, sizeof(name));
		
		glui::BeginMenu("Host server", glm::vec4(0, 0, 0, 0), {});
			//glui::Text(currentIPString.c_str(), Colors_White);
			if (glui::Button("start", glm::vec4(0, 0, 0, 0)))
			{
				std::thread t(serverFunction);
				t.detach();
				resetClient();
				state = 2;
			}
		glui::EndMenu();
		glui::BeginMenu("Join server", glm::vec4(0, 0, 0, 0), {});
			glui::Text("enter ip: ", Colors_White);
			glui::InputText("input ip", ip, sizeof(ip));
			if (glui::Button("join", glm::vec4(0, 0, 0, 0)))
			{
				resetClient();
				state = 1;
			}
		glui::EndMenu();

		if (glui::Button("Exit", glm::vec4(0, 0, 0, 0)))
		{
			return 0;
		}

		glui::renderFrame(renderer, textures.font, platform::getRelMousePosition(),
			platform::isLMousePressed(), platform::isLMouseHeld(), platform::isLMouseReleased(),
			platform::isKeyReleased(platform::Button::Escape), platform::getTypedInput(), deltaTime);

	}
	else if (state == 1)
	{
		clientFunction(deltaTime, renderer, textures, ip, name);
	}
	else if (state == 2)
	{
		clientFunction(deltaTime, renderer, textures, ip, name);


	}

	
#pragma region set finishing stuff
	renderer.flush();

	return true;
#pragma endregion

}

void closeGame()
{

	closeServer();
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
	closeFunction();


}
