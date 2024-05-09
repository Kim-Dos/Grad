#pragma once
#include "TCPDevice.h"
#include <boost\unordered\concurrent_flat_map.hpp>

class LobbySession;

concurrent_flat_map<int, std::shared_ptr<LobbySession>> clients;


class LobbyTCP : public TCPDevice {

private:
	std::atomic_int NumOfClients = 0;

	void ServerAccept();

public:
	inline int GetNewClient() { return NumOfClients++; }
	bool isGameServer();
};


class LobbySession
	: std::enable_shared_from_this<LobbySession>
{
private:
	tcp::socket plSock;
	int prevDataSize, curDataSize;
	int userID;
	unsigned char recvBuffer[MAXSIZE]; // 수신버퍼에서 끌어오는 버퍼
	unsigned char PacketData[MAXSIZE]; // 프로세스에 사용될 패킷 데이터
	char RoomCode[RoomCodeLen];

	void recv();

	void LobbyPacketProcess();

public:
	LobbySession(tcp::socket sock, int numofclient) noexcept
		: plSock(std::move(sock)), userID(numofclient) {
		prevDataSize = 0; curDataSize = 0;
	}
	void Start();

	void SetAutoMatching();
	void MakeRoom();
	void EnterLobbyRoom();
	void IntoGameServer();

};
