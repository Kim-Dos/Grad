#include <iostream>
#include <boost/asio.hpp>
#include <conio.h>
#include "UDPClient.hpp"
#include "../../../Grad/GRServer/GRServer\Protocol.h"

using boost::asio::ip::tcp;

class TCPC
	//: public std::enable_shared_from_this<TCPC>
{
public:
	TCPC(boost::asio::io_context& service);

	void keyTrace() {

		traceKey();
	}

private:
	tcp::socket msocket;
	unsigned char recvBuffer[1024];
	unsigned char PacketData[1024];
	int prevDataSize;
	int curDataSize;


	void recv();

	void Packetsend(void* packet);

	void ClientPacketProcess();
	void Character_Positioning();

	void traceKey();

};

