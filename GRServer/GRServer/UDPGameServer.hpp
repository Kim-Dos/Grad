#include <boost/asio.hpp>
#include <atomic>
#include <map>
#include <iostream>
#include "TCPGameServer.hpp"
#include "Protocol.h"
#pragma once


using boost::asio::ip::udp;

class GameUDP {
private:
	udp::socket mUDPSocket;
	udp::endpoint remote_endpoint;
	int roomNumber = 0;


public:

	GameUDP(boost::asio::io_context& IOContext, int port) noexcept;

	inline int GetRoomNumber() { return roomNumber++; }

	//udp::endpoint Get_remote_endpoint() inline { return remote_endpoint; }

	//void Set_remote_endpoint(std::string str, int port);
};

class UDPGameSession
{
private:

	udp::socket UDPSocket;
	int prevDataSize, curDataSize;
	int userID, RoomNumber;
	unsigned char UDPrecvBuffer[MAXSIZE];
	unsigned char UDPPacketData[MAXSIZE];
	udp::endpoint remote;
	Player player;


	void recv();

public:


	UDPGameSession(udp::socket udpsock) noexcept;

	~UDPGameSession();

	void Start();

	void UDPPacketProcess();

	void moveCharacter();

	void PacketSend(void* packet);
};