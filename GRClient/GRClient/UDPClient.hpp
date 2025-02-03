
#include <boost\asio.hpp>
#include <iostream>
#include <conio.h>
#include "..\..\..\Grad\GRServer\GRServer\Protocol.h"
#include "../../../ppo/ppo/Player.h"

using boost::asio::ip::udp;



class UDPC {
public:

	UDPC(boost::asio::io_context& service) noexcept;

	void SendPacket(void* Packet);

	void recv(unsigned char* arr);

private:
	udp::socket mUDPSocket;
	unsigned char recvBuffer[1024];
	unsigned char PacketData[1024];
	int prevDataSize;
	int curDataSize;

	void recv(unsigned char* arr);

	void Packetsend(void* packet, size_t length);

};