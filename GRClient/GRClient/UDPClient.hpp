#pragma once

#include <iostream>
#include <boost\asio.hpp>
#include "..\..\..\Grad\GRServer\GRServer\Protocol.h"

using boost::asio::ip::udp;



class UDPC {
public:

	UDPC(boost::asio::io_context& service) noexcept;

	~UDPC() { std::cout << "UDPC out\n"; }
	void SendPacket(void* Packet);

	void recv(unsigned char (&arr) [1024]);

private:
	udp::socket mUDPSocket;
	unsigned char recvBuffer[1024];
	unsigned char PacketData[1024];
	int prevDataSize;
	int curDataSize;

	void Packetsend(void* packet, size_t length);

};