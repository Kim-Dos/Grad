
#include <boost\asio.hpp>
#include <iostream>
#include <conio.h>
#include "..\..\..\Grad\GRServer\GRServer\Protocol.h"
#include "../../../ppo/ppo/GameObject.h"
#include "../../../ppo/ppo/Player.h"
using boost::asio::ip::udp;



class UDPC {
public:

	UDPC(boost::asio::io_context& service) noexcept;

	void SendPosition(Player& player);

private:
	udp::socket mUDPSocket;
	unsigned char recvBuffer[1024];
	unsigned char PacketData[1024];
	int prevDataSize;
	int curDataSize;

	void recv();

	void Packetsend(void* packet);


	

};