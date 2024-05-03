#include <boost/asio.hpp>
#include <atomic>
#include <iostream>
#include "TCPGameServer.hpp"
#include "Protocol.h"
#pragma once


using boost::asio::ip::udp;

class GameUDP {
	udp::socket mUDPSocket;
	udp::endpoint remote_endpoint;
	


public:

	GameUDP(boost::asio::io_context& IOContext, int port) noexcept;
};

class UDPGameSession
{
private:

	udp::socket UDPSocket;
	int prevDataSize, curDataSize;
	int userID, RoomNumber;
	unsigned char UDPrecvBuffer[MAXSIZE];
	unsigned char UDPPacketData[MAXSIZE];


	void recv();

public:


	UDPGameSession(udp::socket udpsock) noexcept;

	~UDPGameSession();

	void Start();

	void UDPPacketProcess();

};