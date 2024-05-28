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
	unsigned char recvBuffer[MAXSIZE]; // ���Ź��ۿ��� ������� ����
	unsigned char PacketData[MAXSIZE]; // ���μ����� ���� ��Ŷ ������
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
	unsigned char recvBuffer[MAXSIZE]; // ���Ź��ۿ��� ������� ����
	unsigned char PacketData[MAXSIZE]; // ���μ����� ���� ��Ŷ ������

	void recv();

	void LobbytoGamePacketProcess();

public:
	LobbytoGameSession(tcp::socket sock) noexcept
		: GameSerSock(std::move(sock)) {
		prevDataSize = 0; curDataSize = 0; 
	}
	void Start();



};
