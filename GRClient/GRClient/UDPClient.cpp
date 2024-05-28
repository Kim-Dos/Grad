#include "UDPClient.hpp"

boost::asio::ip::udp::endpoint UDPGameIP(boost::asio::ip::address::from_string("127.0.0.1"), 4000);

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
class Player;

void UDPC::SendPosition(Player& pl)
{
	CSMove packet;
	ZeroMemory(&packet, sizeof(CSMove));
	packet.size = sizeof(CSMove);
	packet.type = SC_MOVEMENT;
	XMFLOAT3 temp = pl.GetVelocity();
	memcpy(&packet.velocity, &temp, sizeof(FXYZ));
	temp = pl.GetLook();
	memcpy(&packet.look, &temp, sizeof(FXYZ));
	temp = pl.GetPosition();
	memcpy(&packet.position, &temp, sizeof(FXYZ));

	Packetsend(&packet);
}


void UDPC::Packetsend(void* packet) {
	std::cout << "sendpack\n";
	int packetsize = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buffer = new unsigned char[packetsize];
	memcpy(buffer, packet, packetsize);
	mUDPSocket.async_send(boost::asio::buffer(buffer, (size_t)packetsize),
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

void UDPC::recv() 
{
	mUDPSocket.async_receive(boost::asio::buffer(recvBuffer),
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