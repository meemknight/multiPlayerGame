#include "serverClient.h"
#include <unordered_map>
#include "packet.h"
#include "phisics.h"
#include <atomic>
#include <cstdlib>
#include <ctime>

struct Client
{
	ENetPeer *peer = {};
	phisics::Entity entityData = {};
	//char clientName[56] = {};
};


std::unordered_map<int32_t, Client> connections;
int pids = 1;

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
	phisics::Entity entity = {};
	glm::vec3 color = getRandomColor();
	entity.color = color;

	connections.insert({pids, Client{event.peer, entity}});

	Packet p;
	p.header = headerReceiveCIDAndData;
	p.cid = pids;

	pids++;
	//send own cid
	sendPacket(event.peer, p, (const char*)&color, sizeof(color));

	//send other players
	for (auto it = connections.begin(); it != connections.end(); it++)
	{
		if (it->second.peer != event.peer)
		{
			Packet sPacket;
			sPacket.header = headerUpdateConnection;
			sPacket.cid = it->first;
			sendPacket(event.peer, sPacket, (const char *)&it->second.entityData, sizeof(phisics::Entity));
		}
	}


	//broadcast data of new connection
	Packet sPacket;
	sPacket.header = headerAnounceConnection;
	sPacket.cid = p.cid;


	for (auto it = connections.begin(); it != connections.end(); it++)
	{
		if (it->second.peer != event.peer)
		{
			sendPacket(it->second.peer, sPacket, (const char*)&entity, sizeof(entity));
		}
	}
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
			for (auto it = connections.begin(); it != connections.end(); it++)
			{
				if (it->second.peer != event.peer)
				{
					sendPacket(it->second.peer, sPacket, nullptr, 0);
				}
			}

			enet_peer_disconnect(event.peer, 0);
			connections.erase(it);
			break;
		}
	}
}

void recieveData(ENetHost *server, ENetEvent &event)
{

	Packet p;
	size_t size = 0;
	auto data = parsePacket(event, p, size);

	//validate data
	if (connections[p.cid].peer != event.peer)
	{
		//std::cout << "invalid data!\n";
		return;
	}

	if (p.header == headerData)
	{
		if (data)
		{
			//std::cout << "recieved from cid: " << p.cid << " with name: "
			//	<< connections[p.cid].clientName << " -> " << data << "\n";
		}

		//broadcast data
		//Packet sPacket;
		//sPacket.header = headerData;
		//sPacket.cid = p.cid;
		//for (auto it = connections.begin(); it != connections.end(); it++)
		//{
		//	if (it->second.peer != event.peer)
		//	{
		//		sendPacket(it->second.peer, sPacket, data, size);
		//	}
		//}

	}
	else if (p.header == headerUpdateConnection)
	{
		connections[p.cid].entityData = *(phisics::Entity*)(data);
		Packet sPacket;
		sPacket.header = headerUpdateConnection;
		sPacket.cid = p.cid;

		for (auto it = connections.begin(); it != connections.end(); it++)
		{
			if (it->second.peer != event.peer)
			{
				sendPacket(it->second.peer, sPacket, (const char*)(data), sizeof(phisics::Entity), false);
			}
		}
	}
	//else if (p.header == headerClientSendName)
	//{
	//	if (data)
	//	{
	//		memcpy(connections[p.cid].clientName, data, std::min(sizeof(Client::clientName), size));
	//	}
	//}


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
	serverOpen = true;
	std::srand(std::time(0));

	ENetAddress adress;
	adress.host = ENET_HOST_ANY;
	adress.port = 7777;
	ENetEvent event;

	//first param adress, players limit, channels, bandwith limit
	ENetHost *server = enet_host_create(&adress, 32, 1, 0, 0);

	if (!server)
	{
		std::terminate();
	}


	while (serverOpen)
	{

		while (enet_host_service(server, &event, 200) > 0 && serverOpen)
		{

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

	}
	
	enet_host_destroy(server);


}