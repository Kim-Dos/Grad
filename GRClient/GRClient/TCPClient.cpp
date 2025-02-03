#include <iostream>
#include <boost/asio.hpp>
#include <conio.h>
#include "UDPClient.hpp"
#include "GameObject.h"
#include "../../../Grad/GRServer/GRServer\Protocol.h"


using boost::asio::ip::tcp;

const char* ip = "127.0.0.1";
int port = 4000;
boost::asio::ip::tcp::endpoint LobbyIP(boost::asio::ip::address::from_string(ip), port);
boost::asio::ip::tcp::endpoint TCPGameIP;
boost::asio::ip::udp::endpoint UDPGameIP(boost::asio::ip::address::from_string(ip), port);

class TCPC 
	//: public std::enable_shared_from_this<TCPC>
{
public:
	TCPC(boost::asio::io_context& service)
		: msocket(service)
	{
		prevDataSize = 0, curDataSize = 0;
		ZeroMemory(recvBuffer, MAXSIZE);
		ZeroMemory(PacketData, MAXSIZE);

		msocket.async_connect(LobbyIP, [](boost::system::error_code ec) {
			if (ec) { std::cout << ec.what() << std::endl;  exit(-1); }
		});


		recv();
	}

	void keyTrace() {

		traceKey();
	}
private:
	tcp::socket msocket;
	unsigned char recvBuffer[1024];
	unsigned char PacketData[1024];
	int prevDataSize;
	int curDataSize;
	GameObject character;



	void recv() {
		//auto self(shared_from_this());
		msocket.async_read_some(boost::asio::buffer(recvBuffer),
			[this](boost::system::error_code ec, std::size_t length)
			{
				if (ec) {
					std::cout << ec.what() << std::endl;
					exit(-1);
				}
				int BufferLoad = static_cast<int>(length);

				unsigned char* PacketPoint = recvBuffer;
				while (0 < BufferLoad) {
					if (curDataSize == 0) {
						curDataSize = PacketPoint[0];
						if (curDataSize > 200) { std::cout << "BufferErr\n" << std::endl; exit(-1); } //find a error packet
					}
					int build = curDataSize - prevDataSize; 


					if (build <= BufferLoad) {
						memcpy(PacketData + prevDataSize, PacketPoint, build);

						ClientPacketProcess();
						curDataSize = 0;
						prevDataSize = 0;
						BufferLoad -= build;
						PacketPoint += build;
					}
					else {
						memcpy(PacketData + prevDataSize, PacketPoint, build);
						prevDataSize += build;
						build = 0;
						PacketPoint += BufferLoad;
					}
				}
				recv();
			});

	}

	void Packetsend(void* packet) {
		std::cout << "sendpack\n";
		//auto self(shared_from_this());
		int packetsize = reinterpret_cast<unsigned char*>(packet)[0];
		unsigned char* buffer = new unsigned char[packetsize];
		memcpy(buffer, packet, packetsize);
		msocket.async_write_some(boost::asio::buffer(buffer, (size_t)packetsize),
			[this, buffer, packetsize](boost::system::error_code ec, std::size_t bytes_transferred)
			{
				if (!ec)
				{
					if (packetsize != bytes_transferred) {
						std::cout << "ERR - Bytes_transferred\n";
					}
					delete buffer;
				}
			});
	}

	void ClientPacketProcess() {
		switch (PacketData[1])
		{
		case LC_LOG_INFO:
			break;
		case LC_PUSH_MATCHING_Q:
			break;
		case LC_FIND_ROOM_CODE:
			break;
		case LC_ROOM_CREATE:
			break;
		default:
			break;
		}

	}

	void Character_Positioning() {
	
		FXYZ position;
		memcpy(&position, PacketData+2, sizeof(FXYZ));
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

		//Packetsend(&pack);

	}

};



void worker(boost::asio::io_context* service) {
	service->run();
}


int main()
{
	std::vector<std::thread> th;

	boost::asio::io_context iocont;
	TCPC tt(iocont);

	th.emplace_back(worker, &iocont);
	th.front().join();


}