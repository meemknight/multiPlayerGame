#include "serverClient.h"
#include <unordered_map>
#include "packet.h"
#include "phisics.h"
#include <atomic>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>

struct Client
{
	ENetPeer *peer = {};
	phisics::Entity entityData = {};
	bool changed = 1;
	//char clientName[56] = {};
};


std::vector<glm::ivec2> itemSpawnPosition =
{
	{22,12},
	{44,17},
	{31,32},
	{16,45},
	{39,28},
	{11,23},
	{25,5},
	{27,46},
	{22,27},
};

std::vector<phisics::Item> items;
constexpr int maxItems = 3;
std::unordered_map<int32_t, Client> connections;

void broadCast(Packet p, void *data, size_t size, ENetPeer *peerToIgnore, bool reliable, int channel)
{
	for (auto it = connections.begin(); it != connections.end(); it++)
	{
		if (!peerToIgnore || (it->second.peer != peerToIgnore))
		{
			sendPacket(it->second.peer, p, (const char *)data, size, true, channel);
		}
	}
}


void spawnItem()
{
	static int itemsIds = 1;
	int i = rand() % itemSpawnPosition.size();
	auto pos = itemSpawnPosition[i];
	itemSpawnPosition.erase(itemSpawnPosition.begin() + i);

	auto item = phisics::Item(pos, itemsIds++, rand()%phisics::itemsCount + 1);

	items.push_back(item);

	Packet p;
	p.cid = 0;
	p.header = headerSpawnItem;

	broadCast(p, &item, sizeof(item), nullptr, true, 1);
}

bool pickupItem(uint32_t itemId, phisics::Item &item)
{
	auto findPos = std::find_if(items.begin(), items.end(), [itemId](phisics::Item &i) { return i.itemId == itemId; });
	item = {};

	if (findPos == items.end())
	{
		return false;
	}

	item = *findPos;

	auto pos = findPos->pos;
	itemSpawnPosition.push_back(pos);
	items.erase(findPos);
	return true;
}

int pids = 1;
bool changedData = 0;

glm::vec3 getRandomColor()
{
	glm::vec3 colors[] = 
	{
		Colors_Blue
		,Colors_Yellow
		,Colors_Magenta
		,Colors_Turqoise
		,Colors_Orange
		,Colors_Purple
		,Colors_Gray
	};

	int index = rand() % (sizeof(colors) / sizeof(colors[0]));

	return colors[index];
}

void addConnection(ENetHost *server, ENetEvent &event)
{
	changedData = true;
	phisics::Entity entity = {};
	glm::vec3 color = getRandomColor();
	entity.color = color;

	connections.insert({pids, Client{event.peer, entity}});

	Packet p;
	p.header = headerReceiveCIDAndData;
	p.cid = pids;

	pids++;
	//send own cid
	sendPacket(event.peer, p, (const char*)&color, sizeof(color), true, 0);

	//send other players
	for (auto it = connections.begin(); it != connections.end(); it++)
	{
		if (it->second.peer != event.peer)
		{
			Packet sPacket;
			sPacket.header = headerUpdateConnection;
			sPacket.cid = it->first;
			sendPacket(event.peer, sPacket, (const char *)&it->second.entityData, sizeof(phisics::Entity), true, 0);
		}
	}

	//send other items
	for (auto &it : items)
	{
		Packet p;
		p.cid = 0;
		p.header = headerSpawnItem;
		sendPacket(event.peer, p, (const char *)&it, sizeof(phisics::Item), true, 1);
	}

	//broadcast data of new connection
	Packet sPacket;
	sPacket.header = headerAnounceConnection;
	sPacket.cid = p.cid;

	broadCast(sPacket, &entity, sizeof(entity), event.peer, true, 0);

}

void removeConnection(ENetHost *server, ENetEvent &event)
{

	for (auto it = connections.begin(); it != connections.end(); it++)
	{
		if (it->second.peer == event.peer)
		{

			//broadcast disconnect
			Packet sPacket;
			sPacket.header = headerAnounceDisconnect;
			sPacket.cid = it->first;

			broadCast(sPacket, nullptr, 0, event.peer, true, 0);

			enet_peer_disconnect(event.peer, 0);
			connections.erase(it);
			break;
		}
	}
}

void recieveData(ENetHost *server, ENetEvent &event)
{
	changedData = true;

	Packet p;
	size_t size = 0;
	auto data = parsePacket(event, p, size);

	//validate data
	if (connections[p.cid].peer != event.peer)
	{
		//std::cout << "invalid data!\n";
		return;
	}

	if (p.header == headerUpdateConnection)
	{
		connections[p.cid].entityData = *(phisics::Entity*)(data);
		connections[p.cid].changed = true;

		//Packet sPacket;
		//sPacket.header = headerUpdateConnection;
		//sPacket.cid = p.cid;
		//for (auto it = connections.begin(); it != connections.end(); it++)
		//{
		//	if (it->second.peer != event.peer)
		//	{
		//		sendPacket(it->second.peer, sPacket, (const char*)(data), sizeof(phisics::Entity), false);
		//	}
		//}
	}
	else if (p.header == headerSendBullet)
	{
		for (auto it = connections.begin(); it != connections.end(); it++)
		{
			if (it->second.peer != event.peer)
			{
				Packet sPacket;
				sPacket.header = headerSendBullet;
				sPacket.cid = p.cid;
				sendPacket(it->second.peer, sPacket, data, size, true, 1);
			}
		}
	}
	else if (p.header == headerRegisterHit)
	{
		Packet sPacket;
		sPacket.header = headerRegisterHit;
		sPacket.cid = *(int32_t *)data;
		broadCast(sPacket, nullptr, 0, event.peer, true, 1);
	}
	else if (p.header == headerPickupItem)
	{
		//std::cout << "pickup\n";
		Packet sPacket;
		sPacket.header = headerPickupItem;
		sPacket.cid = p.cid;
		phisics::Item item = {};

		if (pickupItem(*(uint32_t *)data, item))
		{

			broadCast(sPacket, &item, sizeof(item), nullptr, true, 1);
		}

	}


	//std::cout << event.packet->dataLength << "\n";
	//std::cout << event.packet->data << "\n";
	//std::cout << event.peer->data << "\n"; //recieved from
	//std::cout << event.peer->address.host << "\n"; //recieved from
	//std::cout << event.peer->address.port << "\n"; //recieved from
	//std::cout << event.channelID << "\n\n";

	enet_packet_destroy(event.packet);

}

std::atomic_bool serverOpen;

void closeServer()
{
	serverOpen = false;
}

void serverFunction()
{

	std::srand(std::time(0));
	serverOpen = true;

	ENetAddress adress;
	adress.host = ENET_HOST_ANY;
	adress.port = 7777;
	ENetEvent event;

	//first param adress, players limit, channels, bandwith limit
	ENetHost *server = enet_host_create(&adress, 32, SERVER_CHANNELS, 0, 0);

	if (!server)
	{
		std::terminate();
	}


	while (serverOpen)
	{
		int counter = 0;
		constexpr int maxCounter = 1;

		while (enet_host_service(server, &event, 0) > 0 && counter < maxCounter && serverOpen)
		{
			counter++;
			switch (event.type)
			{
				case ENET_EVENT_TYPE_CONNECT:
				{
					addConnection(server, event);

					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					recieveData(server, event);

					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					//std::cout << "disconnect: "
					//	<< event.peer->address.host << " "
					//	<< event.peer->address.port << "\n\n";
					removeConnection(server, event);
					break;
				}
			}

		}

		if (changedData)
		{
			for (auto p = connections.begin(); p != connections.end(); p++)
			{
				
				if (!p->second.changed)
				{
					continue;
				}
				
				p->second.changed = false;

				Packet sPacket;
				sPacket.header = headerUpdateConnection;
				sPacket.cid = p->first;
				broadCast(sPacket, &p->second.entityData, sizeof(phisics::Entity), p->second.peer, false, 0);
			}
			
		}

		changedData = false;
		
		float deltaTime = 0.f;
		{
			static auto stop = std::chrono::high_resolution_clock::now();
			auto start = std::chrono::high_resolution_clock::now();

			deltaTime = (std::chrono::duration_cast<std::chrono::microseconds>(start - stop)).count() / 1000000.0f;
			stop = std::chrono::high_resolution_clock::now();
		}

	#pragma region items
		{

			static float spawnTime = 5.f;
			
			if (items.size() < maxItems)
			{
				spawnTime -= deltaTime;

				if (spawnTime <= 0.f)
				{
					spawnTime = rand() % 5 + 5;

					spawnItem();
				}

			}



		}
	#pragma endregion



	}
	
	enet_host_destroy(server);


}