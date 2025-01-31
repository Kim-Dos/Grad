#include <boost\unordered\concurrent_flat_map.hpp>
#include <atomic>
#include <boost\asio.hpp>
#include <iostream>
#include "GameRoom.hpp"
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

	char PartyRoomCode[RoomCodeLen];
	unsigned char TCPrecvBuffer[MAXSIZE]; // ���Ź��ۿ��� ������� ����
	unsigned char TCPPacketData[MAXSIZE]; // ���μ����� ���� ��Ŷ ������
	
	void recv();

public:

	TCPGameSession(tcp::socket tcpsock) noexcept;
	
	~TCPGameSession();

	void Start();

	void GamePacketProcess();

	void PacketSend(void* packet);

	inline const tcp::socket& getSocket() { return TCPSocket; }

	//inline Player& getPlayer() { return player; }

};



