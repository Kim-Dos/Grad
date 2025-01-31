#include <boost/asio.hpp>
#include <atomic>
#include <map>
#include <iostream>
#include "TCPGameServer.hpp"
#include "Protocol.h"
#pragma once


using boost::asio::ip::udp;

class GameUDP { //UDP 소켓을 생성하고, 룸 코드를 기반으로 소켓을 할당하는 역할을 수행.
private:
	udp::socket mUDPSocket;
	udp::endpoint remote_endpoint;

public:

	GameUDP(boost::asio::io_context& IOContext, int port) noexcept;

	//inline int GetRoomNumber() { return roomNumber++; }
};

class UDPGameSession //오직 송수신 처리만 담당하는 용도로 사용하고싶음.
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

	void timeSend(); //일정 시간마다 데이터 요청.

	void Start();

	void PacketSend(void* packet);

	void JoinGroup(const boost::asio::ip::address& multiaddress);
};