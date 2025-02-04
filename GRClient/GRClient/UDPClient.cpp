#include "UDPClient.hpp"
#include <iostream>

boost::asio::ip::udp::endpoint UDPGameIP(boost::asio::ip::address::from_string("127.0.0.1"), SERVERPORT);

UDPC::UDPC(boost::asio::io_context& service) noexcept
	: mUDPSocket(service)
{
	prevDataSize = 0, curDataSize = 0;
	ZeroMemory(recvBuffer, MAXSIZE);
	ZeroMemory(PacketData, MAXSIZE);


	mUDPSocket.async_connect(UDPGameIP, [this](boost::system::error_code ec) {
		if (ec) { std::cout << ec.what() << std::endl; exit(-1); }
	});

}
void UDPC::SendPacket(void* Packet)
{
	size_t length = reinterpret_cast<unsigned char*>(Packet)[0];
	Packetsend(Packet, length);
}

class Player;


void UDPC::Packetsend(void* packet, size_t length) {
	std::cout << "sendpack\n";
	
	mUDPSocket.async_send(boost::asio::buffer(packet, (size_t)length),
		[this, packet, length](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)
			{
				if (length != bytes_transferred) {
					std::cout << "ERR - Bytes_transferred\n";
				}
			}
		});
}

void UDPC::recv(unsigned char (&arr)[1024])
{
	mUDPSocket.async_receive(boost::asio::buffer(arr),
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
			}
		});
}