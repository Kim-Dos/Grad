#include <boost\asio.hpp>
#include <iostream>
#include <conio.h>
#include "GameObject.h"
#include "..\..\..\Grad\GRServer\GRServer\Protocol.h"

using boost::asio::ip::udp;

extern boost::asio::ip::udp::endpoint UDPGameIP;

class UDPC {
public:

	UDPC(boost::asio::io_context& service) noexcept;

	void keyTrace() {

		traceKey();
	}

private:
	udp::socket mUDPSocket;
	unsigned char recvBuffer[1024];
	unsigned char PacketData[1024];
	int prevDataSize;
	int curDataSize;
	GameObject character;
	

	void recv();

	void Packetsend(void* packet);


	void ClientPacketProcess() {
		switch (PacketData[1])
		{
		case SC_POSITION:
			Character_Positioning();
		default:
			break;
		}

	}

	void Character_Positioning() {

		FXYZ position;
		memcpy(&position, PacketData + 2, sizeof(FXYZ));
		character.SetPosition(position);

		std::cout << character.GetPosX() << character.GetPosY() << character.GetPosZ() << std::endl;
		traceKey();
	}

	void traceKey() {

		FXYZ temp = character.GetPos();

		char ch;
		ch = _getch();

		switch (ch)
		{
		case 'w':
			character.SetPosition(temp.x + 1.0, temp.y, temp.z);
			std::cout << "press w\n";
			break;
		case 'a':
			character.SetPosition(temp.x, temp.y - 1.0, temp.z);
			std::cout << "press a\n";
			break;
		case 's':
			character.SetPosition(temp.x - 1.0, temp.y, temp.z);
			std::cout << "press s\n";
			break;
		case 'd':
			character.SetPosition(temp.x, temp.y + 1.0, temp.z);
			std::cout << "press d\n";
			break;
		}

		CSmove pack;
		pack.size = sizeof(CSmove);
		pack.type = CS_MOVE;
		pack.position = character.GetPos();
		pack.roomnumber = NULL;
		pack.usernumber = NULL;

		Packetsend(&pack);

	}
};