#include "packet.h"
#include <vector>
#undef max
#undef min

//char *dataPool;
//size_t dataPoolSize;
  //todo
//void resize(size_t newSize)
//{
//	if (newSize > dataPoolSize)
//	{
//		delete[] dataPool;
//		dataPool = new char[newSize];
//		dataPoolSize = newSize;
//	}
//}

void sendPacket(ENetPeer *to, Packet p, const char *data, size_t size)
{

	//resize(size + sizeof(Packet));

	char *dataPool = new char[size + sizeof(Packet)];
	//size_t dataPoolSize;


	memcpy(dataPool, &p, sizeof(Packet));

	if (data && size)
	{
		memcpy(dataPool + sizeof(Packet), data, size);
	}

	ENetPacket *packet = enet_packet_create(dataPool, size + sizeof(Packet), ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
	enet_peer_send(to, 0, packet);
	//channel 0
	//enet_packet_destroy(packet);

	delete[] dataPool;

}

char *parsePacket(ENetEvent &event, Packet &p, size_t &dataSize)
{
	size_t size = event.packet->dataLength;
	void *data = event.packet->data;
	dataSize = std::max(size_t(0), size - sizeof(Packet));

	memcpy(&p, data, sizeof(Packet));

	if (size <= sizeof(Packet))
	{
		return nullptr;
	}
	else
	{
		return (char *)data + sizeof(Packet);
	}

}
