#include "serverClient.h"
#include <platform/platformInput.h>
#include <Phisics.h>
#include <packet.h>
#include <unordered_map>
#include "Ui.h"

phisics::MapData map;

ENetPeer *server = {};
int32_t cid = {};
bool joined = false;
ENetHost *client;

std::unordered_map<int32_t, phisics::Entity> players;

static std::vector<phisics::Bullet> bullets;
static std::vector<phisics::Bullet> ownBullets;
static std::vector<phisics::Item> items;

glm::ivec2 spawnPositions[] =
{
	{5,5},
	{2,46},
	{44,44},
	{45,4}
};

glm::ivec2 getSpawnPosition()
{
	return spawnPositions[rand() % (sizeof(spawnPositions) / sizeof(spawnPositions[0]))];
}

void resetClient()
{

	if (!map.load(RESOURCES_PATH "mapData2.bin"))
	{
		return ;
	}

	players.clear();
	bullets.clear();
	ownBullets.clear();
	items.clear();

	//todo add a struct here

	joined = false;
	client = nullptr;

	//todo
	//enet_host_destroy(server);
	server = {};
	cid = {};
}

void sendPlayerData(phisics::Entity &e, bool reliable)
{
	Packet p;
	p.cid = cid;
	p.header = headerUpdateConnection;
	sendPacket(server, p, (const char *)&e, sizeof(phisics::Entity), reliable, 0);
}

bool connectToServer(ENetHost *&client, ENetPeer *&server, int32_t &cid, std::string ip)
{
	ENetAddress adress;
	ENetEvent event;

	if (ip.empty())
	{
		enet_address_set_host(&adress, "127.0.0.1");
	}
	else
	{
		enet_address_set_host(&adress, ip.c_str());
	}
	//enet_address_set_host(&adress, "95.76.249.14");
	//enet_address_set_host(&adress, "192.168.1.11");
	adress.port = 7777;

	//client, adress, channels, data to send rightAway
	server = enet_host_connect(client, &adress, SERVER_CHANNELS, 0);

	if (server == nullptr)
	{
		return false;
	}

	//see if we got events by server
	//client, event, ms to wait(0 means that we don't wait)

	if (enet_host_service(client, &event, 5000) > 0
		&& event.type == ENET_EVENT_TYPE_CONNECT)
	{
		//std::cout << "connected\n";
	}
	else
	{
		enet_peer_reset(server);
		return false;
	}


	if (enet_host_service(client, &event, 5000) > 0
		&& event.type == ENET_EVENT_TYPE_RECEIVE)
	{
		Packet p = {};
		size_t size;
		auto data = parsePacket(event, p, size);

		if (p.header != headerReceiveCIDAndData)
		{
			enet_peer_reset(server);
			return false;
		}

		cid = p.cid;

		glm::vec3 color = *(glm::vec3 *)data;
		auto e = phisics::Entity();
		e.pos = getSpawnPosition();
		e.lastPos = e.pos;
		e.color = color;
		players[cid] = e;

		sendPlayerData(e, true);

		//std::cout << "received cid: " << cid << "\n";
		enet_packet_destroy(event.packet);
		return true;
	}
	else
	{
		enet_peer_reset(server);
		return 0;
	}

	//std::cout << "fully connected\n";

	//name
	//{
	//	sendPacket(server, {headerClientSendName, cid}, userName.c_str(), userName.size() + 1);
	//}

	return true;
}

void msgLoop(ENetHost *client)
{
	
	ENetEvent event;
	if(enet_host_service(client, &event, 0) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_RECEIVE:
			{
				//std::cout << event.packet->dataLength << "\n";
				//std::cout << "recieved: " << event.packet->data << "\n";
				//std::cout << event.peer->data << "\n"; //recieved from
				//std::cout << event.peer->address.host << "\n"; //recieved from
				//std::cout << event.peer->address.port << "\n"; //recieved from
				//std::cout << event.channelID << "\n";
				Packet p = {};
				size_t size = {};
				auto data = parsePacket(event, p, size);

				if (p.header == headerAnounceConnection)
				{

					players[p.cid] = *(phisics::Entity*)data;

				}else if (p.header == headerUpdateConnection)
				{

					players[p.cid] = *(phisics::Entity *)data;

				}else if (p.header == headerAnounceDisconnect)
				{
					auto find = players.find(p.cid);
					players.erase(find);
				}else if (p.header == headerSendBullet)
				{
					bullets.push_back(*(phisics::Bullet *)data);
				}
				else if (p.header == headerRegisterHit)
				{
					auto find = players.find(p.cid);
					bool h = find->second.hit();

					if (h && find->first == cid)
					{
						find->second.life -= 1;

						if (find->second.life <= 0)
						{
							auto &p = find->second;
							p.pos = getSpawnPosition();
							p.lastPos = p.pos;
							p.life = p.maxLife;

							sendPlayerData(p, true);
						}

					}
				}
				else if (p.header == headerSpawnItem)
				{
					items.push_back(*(phisics::Item *)data);
				}
				else if (p.header == headerPickupItem)
				{
					uint32_t itemId = *(uint32_t *)data;
					auto f = std::find_if(items.begin(), items.end(), [itemId](phisics::Item &i) { return i.itemId == itemId; });

					if (f != items.end())
					{
						items.erase(f);
					}

					if (p.cid == cid)
					{
						auto find = players.find(p.cid);
						find->second.life = phisics::Entity::maxLife;
					}

				}

				enet_packet_destroy(event.packet);

				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				//std::cout << "disconect\n";
				exit(0);

				break;
			}
		}
	}

}

void closeFunction()
{
	if (!server) { return; }

	ENetEvent event;

	enet_peer_disconnect(server, 0);
	//wait for disconect
	while (enet_host_service(client, &event, 10) > 0)
	{
		switch (event.type)
		{
			case ENET_EVENT_TYPE_RECEIVE:
			{
				enet_packet_destroy(event.packet);
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				break;
			}
		}
	}


}


void clientFunction(float deltaTime, gl2d::Renderer2D &renderer, Textures textures, std::string ip)
{

	if (!joined)
	{
		if (!client)
		{
			client = enet_host_create(nullptr, 1, 1, 0, 0);
		}

		if (connectToServer(client, server, cid, ip))
		{
			joined = true;
		}
	}
	else
	{

		msgLoop(client);

		auto &player = players[cid];

	#pragma region input
		float speed = 10 * deltaTime;
		float bulletSpeed = 16;
		float posy = 0;
		float posx = 0;

		if (platform::isKeyHeld(platform::Button::Up)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::Up].held
			)
		{
			posy = -1;
		}
		if (platform::isKeyHeld(platform::Button::Down)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::Down].held
			)
		{
			posy = 1;
		}
		if (platform::isKeyHeld(platform::Button::Left)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::Left].held
			)
		{
			posx = -1;
		}
		if (platform::isKeyHeld(platform::Button::Right)
			|| platform::getControllerButtons().buttons[platform::ControllerButtons::Right].held
			)
		{
			posx = 1;
		}

		if (platform::isKeyPressedOn(platform::Button::Enter))
		{
			platform::setFullScreen(!platform::isFullScreen());
		}

		if (platform::isLMouseReleased())
		{
			phisics::Bullet b;
			b.pos = player.pos + (player.dimensions/2.f);
			b.color = player.color;
			b.cid = cid;

			auto mousePos = platform::getRelMousePosition();
			auto screenCenter = glm::vec2(renderer.windowW, renderer.windowH) / 2.f;

			auto delta = glm::vec2(mousePos) - screenCenter;
			
			float magnitude = glm::length(delta);
			if (magnitude == 0)
			{
				b.direction = {1,0};
			}
			else
			{
				b.direction = delta / magnitude;
			}

			Packet p;
			p.cid = cid;
			p.header = headerSendBullet;
			sendPacket(server, p, (const char *)&b, sizeof(phisics::Bullet), true, 1);

			ownBullets.push_back(b);
		}

	#pragma endregion

	#pragma region items

		for (int i = 0; i < items.size(); i++)
		{
			//pickup item
			if (items[i].checkCollisionPlayer(player))
			{
				Packet p;
				p.cid = cid;
				p.header = headerPickupItem;
				uint32_t itemId = items[i].itemId;
				sendPacket(server, p, (const char *)&itemId, sizeof(itemId), true, 1);
				//sendPacket(server, p, (const char*)&itemId, sizeof(itemId), true, 1);

				items.erase(items.begin() + i);
				i--;
				continue;
			}

		}

	#pragma endregion

	
	#pragma region player
		{
			bool playerChaged = 0;

			if (posx || posy || player.input.x != posx || player.input.y != posy)
			{
				playerChaged = true;
			}

			player.input = {posx, posy};

			for (auto &i : players)
			{
				glm::vec2 dir = i.second.input;
				if (dir.x != 0 || dir.y != 0)
				{
					i.second.move(glm::normalize(dir) * speed);
				}
				i.second.resolveConstrains(map);
				i.second.updateMove(deltaTime);
			}

			renderer.currentCamera.follow(player.pos * worldMagnification, deltaTime * 100, 2, renderer.windowW, renderer.windowH);

			map.render(renderer, textures.sprites);

			for (auto &i : players)
			{
				i.second.draw(renderer, deltaTime, textures.character);
			}

			static float timer = 0;
			constexpr float updateTime = 1.f / 10;

			timer -= deltaTime;
			if (playerChaged || timer <= 0)
			{
				timer = updateTime;
				playerChaged = true;
			}

			if (playerChaged)
			{
				sendPlayerData(player, false);
			}
		}
	#pragma endregion

	#pragma region items

		for (int i = 0; i < items.size(); i++)
		{
			items[i].draw(renderer, textures.medKit);

		}

	#pragma endregion



	#pragma region bullets

		for (int i = 0; i < bullets.size(); i++)
		{
			bullets[i].updateMove(deltaTime * bulletSpeed);
			bullets[i].draw(renderer, textures.character);

			for (auto &e : players)
			{
				if (bullets[i].cid != e.first)
				{
					if (bullets[i].checkCollisionPlayer(e.second))
					{
						//hit player
						bullets.erase(bullets.begin() + i);
						i--;
						break;
					}
				}
			}
		}

		for (int i = 0; i < bullets.size(); i++)
		{
			if (bullets[i].checkCollisionMap(map))
			{
				bullets.erase(bullets.begin() + i);
				i--;
				continue;
			}
		}


		for (int i = 0; i < ownBullets.size(); i++)
		{
			ownBullets[i].updateMove(deltaTime * bulletSpeed);
			ownBullets[i].draw(renderer, textures.character);

			for (auto &e : players)
			{
				if (e.first != cid)
				{
					if (ownBullets[i].checkCollisionPlayer(e.second))
					{
						//hit player, register hit
						e.second.hit();

						Packet p;
						p.header = headerRegisterHit;
						p.cid = cid;
						sendPacket(server, p, (const char*)&e.first, sizeof(int32_t), true, 1);

						ownBullets.erase(ownBullets.begin() + i);
						i--;
						break;
					}
				}
			}


			
		}

		for (int i = 0; i < ownBullets.size(); i++)
		{
			if (ownBullets[i].checkCollisionMap(map))
			{
				ownBullets.erase(ownBullets.begin() + i);
				i--;
				continue;
			}
		}

	#pragma endregion


	#pragma region ui
		{
			Ui::Frame f({0,0, renderer.windowW, renderer.windowH});

			auto c = renderer.currentCamera; //todo push pop camera
			renderer.currentCamera.setDefault();

			float xLeft = 0.95;
			float xSize = 0.04;
			float xAdvance = xSize - 0.025;

			for (int i = 0; i < player.life; i++)
			{
				auto crossPos = Ui::Box().xLeftPerc(xLeft).yTopPerc(0.02).xDimensionPercentage(xSize).yAspectRatio(1.f);
				auto crossPosDown = Ui::Box().xLeftPerc(xLeft+0.003).yTopPerc(0.025).xDimensionPercentage(xSize).yAspectRatio(1.f);
				renderer.renderRectangle(crossPosDown, {0.f,0.f,0.f,1.f}, {}, 0.f, textures.cross);
				renderer.renderRectangle(crossPos, {1.f,1.f,1.f,1.f}, {}, 0.f, textures.cross);
				xLeft -= xAdvance;
			}


			renderer.currentCamera = c;

		

		}
	#pragma endregion




	}

	

}