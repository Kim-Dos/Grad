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
	unsigned char recvBuffer[MAXSIZE]; // ���Ź��ۿ��� ������� ����
	unsigned char PacketData[MAXSIZE]; // ���μ����� ���� ��Ŷ ������
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
