#pragma once
#include <enet/enet.h>
#include <gl2d/gl2d.h>
#include "phisics.h"


void serverFunction();

void clientFunction(float deltaTime, gl2d::Renderer2D &renderer, gl2d::Texture sprites, gl2d::Texture character, std::string ip);
void resetClient();
void closeFunction();
void closeServer();
