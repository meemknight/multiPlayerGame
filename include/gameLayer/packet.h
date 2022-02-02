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
	headerReceiveCIDAndData,
	headerAnounceConnection,
	headerUpdateConnection,
	headerAnounceDisconnect,
	headerSendBullet,
	headerRegisterHit,			//contains pid of hit player, clients will recieve cid directly, no data associated
	headerSpawnItem,			//contains itemData
	headerPickupItem,			//contains itemId
};

constexpr int SERVER_CHANNELS = 2;

void sendPacket(ENetPeer *to, Packet p, const char *data, size_t size, bool reliable, int channel);
char *parsePacket(ENetEvent &event, Packet &p, size_t &dataSize);