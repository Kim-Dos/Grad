#include <boost\unordered\concurrent_flat_map.hpp>
#include <atomic>
#include <boost\asio.hpp>
#include <iostream>
#include "Player.h"
#include "Protocol.h"
#pragma once

using boost::concurrent_flat_map;
using boost::asio::ip::tcp;

//TCP Acceptor
class GameTCP {

public:

	GameTCP(boost::asio::io_context& IOContext, int ServerPort);
	

private:
	tcp::socket mTCPSocket;
	tcp::acceptor mTCPAcceptor;
	void ServerAccept();
};



//Player Session-------------------------------------------------------------

class TCPGameSession
	: public std::enable_shared_from_this<TCPGameSession>
{
private:

	tcp::socket TCPSocket;
	int prevDataSize, curDataSize;
	int PartyNumber;
	unsigned char TCPrecvBuffer[MAXSIZE]; 
	unsigned char TCPPacketData[MAXSIZE];

	Player player;

	void recv();

public:

	TCPGameSession(tcp::socket tcpsock, int roomnumber) noexcept;
	
	~TCPGameSession();

	boost::asio::ip::address_v4 getRemoteAdress();

	void Start();

	void GamePacketProcess();

	void PacketSend(void* packet);

	inline Player& getPlayer() { return player; }

};


class GameRoom 
	: public std::enable_shared_from_this<GameRoom>
{
private:
	unsigned char stagenumber;

	std::vector<TCPGameSession> userIPS;
	std::vector<GameObject> otherObj;


public:
	GameRoom(unsigned char stageNum);

	bool IsDamaged();

};



