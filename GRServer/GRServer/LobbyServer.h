#pragma once
#include "TCPDevice.h"
#include <boost\unordered\concurrent_flat_map.hpp>

class LobbySession;

concurrent_flat_map<int, std::shared_ptr<session>> clients;


class LobbyTCP : public TCPDevice {

private:
	std::atomic_int NumOfClients = 0;

	void ServerAccept();

public:
	inline int GetNewClient() { return NumOfClients++; }
};


class LobbySession
	: std::enable_shared_from_this<LobbySession>
{
private:
	tcp::socket plSock;
	int prevDataSize, curDataSize;
	int userID;
	UCHAR recvBuffer[MAXSIZE]; // 
	UCHAR remainData[MAXSIZE];

	void recv();


public:
	LobbySession(tcp::socket sock, int numofclient) noexcept
		: plSock(std::move(sock)), userID(numofclient) {
		prevDataSize = 0; curDataSize = 0;
	}
	void Start();

};
