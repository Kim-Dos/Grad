#include <boost\unordered\concurrent_flat_map.hpp>
#include <boost\unordered\concurrent_flat_set.hpp>
#include <atomic>
#include <vector>
#include <boost\asio.hpp>
#include <iostream>
#include "Player.h"
#include "Protocol.h"
#pragma once


class LobbyTCP  {

private:
	std::atomic_int NumOfClients = 0;
	tcp::socket mTCPSocket;
	tcp::acceptor mTCPAcceptor;
	std::vector<std::string> IPS;
	void ServerAccept();

public:

	LobbyTCP(boost::asio::io_context& IOContext, int ServerPort);
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

	void recv();

	void LobbyPacketProcess();

public:
	LobbyClientSession(tcp::socket sock, int numofclient) noexcept
		: plSock(std::move(sock)), userID(numofclient) {
		prevDataSize = 0; curDataSize = 0; 
	}
	void Start();

	void SetAutoMatching();
	void MakeRoom(bool flag, unsigned char stageNum);
	void EnterLobbyRoom();
	void IntoGameServer();


	void PacketSend(void* packet);

};

class LobbytoGameSession
	: std::enable_shared_from_this<LobbytoGameSession>
{
private:
	tcp::socket GameSerSock;
	int prevDataSize, curDataSize;
	unsigned char recvBuffer[MAXSIZE]; // 수신버퍼에서 끌어오는 버퍼
	unsigned char PacketData[MAXSIZE]; // 프로세스에 사용될 패킷 데이터
	unsigned long long ServerAmount;

	void recv();

	void LobbytoGamePacketProcess();
	void SetServerAmount();
	void AttachMatchingRoom();

	void SendPacket(void* packet);

public:
	LobbytoGameSession(tcp::socket sock) noexcept
		: GameSerSock(std::move(sock)) {
		prevDataSize = 0; curDataSize = 0; 
		ServerAmount = 0;
	}
	void Start();

};

class LobbyRoom 
	: std::enable_shared_from_this<LobbyRoom>
{
private:
	std::atomic_int user1, user2, user3, user4;
	unsigned char stageNumber;
	bool AutoMatching;
	bool Allready;

public:
	LobbyRoom(int firstUser, bool automatch, unsigned char stagenum) noexcept;

	bool isFulled();
	inline bool isAllReady() { return Allready; }
	inline bool isAutoMatching() { return AutoMatching; }

	inline unsigned char GetStageNumber() { return stageNumber; }

	void SetAllReadyTrue();

	void PushUser(int userid);

};
