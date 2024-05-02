#include "UDPGameServer.hpp"


extern thread_local concurrent_flat_map<int, std::shared_ptr<TCPGameSession>> clients;


GameUDP::GameUDP(boost::asio::io_context& IOContext, int port) noexcept
	: mUDPSocket(IOContext, udp::endpoint(udp::v4(),port))
{
}


UDPGameSession::UDPGameSession(udp::socket udpsock) noexcept
	: UDPSocket(std::move(udpsock))
{
	RoomNumber = 0;
	ZeroMemory(UDPPacketData, MAXSIZE);
	ZeroMemory(UDPrecvBuffer, MAXSIZE);
	userID = 0;
	curDataSize = 0;
	prevDataSize = 0;
}

UDPGameSession::~UDPGameSession()
{
}

void UDPGameSession::Start()
{
	std::cout << "UDPStart\n";
	recv();

}

void UDPGameSession::UDPPacketProcess()
{
}



void UDPGameSession::recv()
{
	UDPSocket.async_receive(boost::asio::buffer(UDPrecvBuffer),
		[this](boost::system::error_code ec, std::size_t length)
		{
			if (ec) {
				std::cout << "READ ERR" << std::endl;
				exit(-1);


			}
			int BufferLoad = static_cast<int>(length);
			unsigned char* PacketPoint = UDPrecvBuffer;
			while (0 < BufferLoad) {
				if (curDataSize == 0) {
					curDataSize = PacketPoint[0];
					if (curDataSize > 200) { std::cout << "BufferErr\n" << std::endl; exit(-1); } //find a error packet
				}
				int build = curDataSize - prevDataSize;

				if (build <= BufferLoad) {
					memcpy(UDPPacketData + prevDataSize, PacketPoint, build);
					UDPPacketProcess();
					curDataSize = 0;
					prevDataSize = 0;
					BufferLoad -= build;
					PacketPoint += build;
				}
				else {
					memcpy(UDPPacketData + prevDataSize, PacketPoint, build);
					prevDataSize += build;
					build = 0;
					PacketPoint += BufferLoad;
				}
			}
			recv();
		});

}