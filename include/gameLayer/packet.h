#pragma once
#include<cstdint>
#include<enet/enet.h>

struct Packet
{
	int32_t header = 0;
	int32_t cid = 0;
	char *getData()
	{
		return (char *)((&cid) + 1);
	}
};

enum
{
	headerNone = 0,
	headerData,
	headerReceiveCID,
	headerAnounceConnection,
	headerUpdateConnection,
};

void sendPacket(ENetPeer *to, Packet p, const char *data, size_t size);
char *parsePacket(ENetEvent &event, Packet &p, size_t &dataSize);