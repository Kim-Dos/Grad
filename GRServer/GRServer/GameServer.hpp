#include <boost\unordered\concurrent_flat_map.hpp>
#include <atomic>
//#include "TCPDevice.h"
//#include "UDPDevice.h"
//#include "Player.h"
#include <boost\asio.hpp>
#include <iostream>
#include "Protocol.h"
#pragma once

using boost::concurrent_flat_map;

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

//TCP Acceptor
class GameTCP {

public:

	GameTCP(boost::asio::io_context& IOContext, int ServerPort);
	
	//void StartSession();

	inline int GetRoomNumber() { return roomNumber++; }

	void PacketProcess() {}

private:
	std::atomic_int roomNumber = 0;
	//concurrent_flat_map<int, std::shared_ptr<GameSession>> clients;
	tcp::socket mTCPSocket;
	tcp::acceptor mTCPAcceptor;
	void ServerAccept();
};


class GameUDP {
	udp::socket mUDPSocket;
	udp::endpoint remote_endpoint;
	void StartReceive();
public:

	GameUDP(boost::asio::io_context& IOContext);
};

//Player Session-------------------------------------------------------------

class GameSession
	: public std::enable_shared_from_this<GameSession>
{
private:

	tcp::socket TCPSocket;
	//udp::socket UDPSocket;
	int prevDataSize, curDataSize;
	int userID, RoomNumber;
	UCHAR TCPrecvBuffer[MAXSIZE]; 
	UCHAR TCPPacketData[MAXSIZE];
	//UCHAR UDPrecvBuffer[MAXSIZE];
	//UCHAR UDPPacketData[MAXSIZE];


	void recv();

public:


	GameSession(tcp::socket tcpsock, int roomnumber) noexcept;
	
	~GameSession();

	void Start();

	void GamePacketProcess();

};


