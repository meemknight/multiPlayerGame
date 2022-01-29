#pragma once
#include <enet/enet.h>
#include <gl2d/gl2d.h>
#include "phisics.h"


void serverFunction();

void clientFunction(float deltaTime, gl2d::Renderer2D &renderer, gl2d::Texture sprites);
void resetClient();
