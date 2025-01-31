#include <boost/asio.hpp>
#include <atomic>
#include <map>
#include <iostream>
#include "TCPGameServer.hpp"
#include "Protocol.h"
#pragma once


using boost::asio::ip::udp;

class GameUDP { //UDP ������ �����ϰ�, �� �ڵ带 ������� ������ �Ҵ��ϴ� ������ ����.
private:
	udp::socket mUDPSocket;
	udp::endpoint remote_endpoint;

public:

	GameUDP(boost::asio::io_context& IOContext, int port) noexcept;

	//inline int GetRoomNumber() { return roomNumber++; }
};

class UDPGameSession //���� �ۼ��� ó���� ����ϴ� �뵵�� ����ϰ����.
{
private:

	udp::socket UDPSocket;
	int prevDataSize, curDataSize;
	std::string PartyRoomCode;
	unsigned char UDPrecvBuffer[MAXSIZE];
	unsigned char UDPPacketData[MAXSIZE];
	udp::endpoint remote;


	void recv();

public:
	
	UDPGameSession();

	UDPGameSession(udp::socket udpsock);

	~UDPGameSession();

	void timeSend(); //���� �ð����� ������ ��û.

	void Start();

	void PacketSend(void* packet);

	void JoinGroup(const boost::asio::ip::address& multiaddress);
};