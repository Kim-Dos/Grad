#pragma once
#include "TCPDevice.h"
#include <boost\unordered\concurrent_flat_map.hpp>


class LobbyTCP : public TCPDevice {

private:
	std::atomic_int NumOfClients = 0;

	void ServerAccept();

public:
	inline int GetNewClient() { return NumOfClients++; }
	bool isGameServer();
};


class LobbyClientSession
	: std::enable_shared_from_this<LobbyClientSession>
{
private:
	tcp::socket plSock;
	int prevDataSize, curDataSize;
	int userID;
	unsigned char recvBuffer[MAXSIZE]; // 수신버퍼에서 끌어오는 버퍼
	unsigned char PacketData[MAXSIZE]; // 프로세스에 사용될 패킷 데이터
	bool GetReady;
	char RoomCode[RoomCodeLen];

	void recv();

	void LobbyPacketProcess();

public:
	LobbyClientSession(tcp::socket sock, int numofclient) noexcept
		: plSock(std::move(sock)), userID(numofclient) {
		prevDataSize = 0; curDataSize = 0; GetReady = false;
	}
	void Start();

	void SetAutoMatching();
	void MakeRoom();
	void EnterLobbyRoom();
	void IntoGameServer();

};

class LobbytoGameSession
	: std::enable_shared_from_this<LobbytoGameSession>
{
private:
	tcp::socket GameSerSock;
	int prevDataSize, curDataSize;
	unsigned char recvBuffer[MAXSIZE]; // 수신버퍼에서 끌어오는 버퍼
	unsigned char PacketData[MAXSIZE]; // 프로세스에 사용될 패킷 데이터

	void recv();

	void LobbytoGamePacketProcess();

public:
	LobbytoGameSession(tcp::socket sock) noexcept
		: GameSerSock(std::move(sock)) {
		prevDataSize = 0; curDataSize = 0; 
	}
	void Start();



};
