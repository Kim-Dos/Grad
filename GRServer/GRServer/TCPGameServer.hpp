#include <boost\unordered\concurrent_flat_map.hpp>
#include <atomic>
#include <boost\asio.hpp>
#include <iostream>
#include "Protocol.h"
#pragma once

using boost::concurrent_flat_map;
using boost::asio::ip::tcp;

class GameRoom;

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

	char PartyRoomCode[RoomCodeLen];
	unsigned char TCPrecvBuffer[MAXSIZE]; // 수신버퍼에서 꺼어오는 버퍼
	unsigned char TCPPacketData[MAXSIZE]; // 프로세스에 사용될 패킷 데이터

	void recv();

public:

	TCPGameSession(tcp::socket tcpsock) noexcept;

	~TCPGameSession();

	void Start();

	void GamePacketProcess();

	void PacketSend(void* packet);

	inline const tcp::socket& getSocket() { return TCPSocket; }

};