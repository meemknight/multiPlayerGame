#include "serverClient.h"
#include <platform/platformInput.h>
#include <Phisics.h>
#include <packet.h>
#include <unordered_map>

phisics::MapData map;

ENetPeer *server = {};
int32_t cid = {};
bool joined = false;
ENetHost *client;

std::unordered_map<int32_t, phisics::Entity> players;

void resetClient()
{

	if (!map.load(RESOURCES_PATH "mapData.txt"))
	{
		return ;
	}

	players.clear();

	//todo add a struct here

	joined = false;
	client = nullptr;

	//todo
	//enet_host_destroy(server);
	server = {};
	cid = {};
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
	server = enet_host_connect(client, &adress, 1, 0);

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
		parsePacket(event, p, size);

		if (p.header != headerReceiveCID)
		{
			enet_peer_reset(server);
			return false;
		}

		cid = p.cid;

		//std::cout << "received cid: " << cid << "\n";
		enet_packet_destroy(event.packet);
	}
	else
	{
		enet_peer_reset(server);
		return 0;
	}

	//std::cout << "fully connected\n";

	players[cid] = phisics::Entity();

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


void clientFunction(float deltaTime, gl2d::Renderer2D &renderer, gl2d::Texture sprites, std::string ip)
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


	#pragma region input
		float speed = 6 * deltaTime;
		float posy = 0;
		float posx = 0;

		if (platform::isKeyHeld(platform::Button::Up)
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



	#pragma region player

		auto &player = players[cid];
		player.move({posx, posy});


		for (auto &i : players)
		{
			i.second.resolveConstrains(map);
			i.second.updateMove();

		}

		renderer.currentCamera.follow(player.pos * worldMagnification, deltaTime * 100, 2, renderer.windowW, renderer.windowH);

		map.render(renderer, sprites);

		for (auto &i : players)
		{
			i.second.draw(renderer, deltaTime, {});
		}


	#pragma endregion

		Packet p;
		p.cid = cid;
		p.header = headerUpdateConnection;
		sendPacket(server, p, (const char *)&player, sizeof(phisics::Entity), true);

	}

	

}