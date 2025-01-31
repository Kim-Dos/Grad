#include "UDPGameServer.hpp"


extern concurrent_flat_map<std::string , std::shared_ptr<GameRoom>> Rooms;


GameUDP::GameUDP(boost::asio::io_context& IOContext, int port) noexcept
	: mUDPSocket(IOContext, udp::endpoint(udp::v4(),port))
{
	//Rooms.visit ===>> 여기서 룸 코드를 기반으로 움직여야함. 그러면 생성자에서 행동할 수 없음.
	//Rooms.visit("1", [this](auto& x) {
	//	x.second->setUDPSocket(std::move(mUDPSocket));
	//});

}

//void GameUDP::Set_remote_endpoint(std::string str, int port)
//{
//	[this](auto& x) {
//		x->remote_endpoint(boost::asio::ip::address::from_string(str), port)) };
//}

UDPGameSession::UDPGameSession()
{
	//RoomNumber = 0;
	ZeroMemory(UDPPacketData, MAXSIZE);
	ZeroMemory(UDPrecvBuffer, MAXSIZE);
	//userID = 0;
	curDataSize = 0;
	prevDataSize = 0;

	UDPSocket.open(udp::v4());
	UDPSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
}

UDPGameSession::UDPGameSession(udp::socket udpsock) : UDPSocket(std::move(udpsock))
{
	//RoomNumber = 0;
	ZeroMemory(UDPPacketData, MAXSIZE);
	ZeroMemory(UDPrecvBuffer, MAXSIZE);
	//userID = 0;
	curDataSize = 0;
	prevDataSize = 0;

	UDPSocket.open(udp::v4());
	UDPSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
}

UDPGameSession::~UDPGameSession()
{
}

void UDPGameSession::timeSend()
{
}

void UDPGameSession::Start()
{
	std::cout << "UDPStart\n";
	recv();
}

//void UDPGameSession::UDPPacketProcess()
//{
//
//
//	switch (UDPPacketData[1])
//	{
//
//	default:
//		break;
//	}
//}

//void UDPGameSession::moveCharacter()
//{
//	FXYZ position;
//	memcpy(&position, UDPPacketData + 2, sizeof(FXYZ));
//
//	//PacketSend(&packet);
//}
//
//void UDPGameSession::generalAttack()
//{
//}

void UDPGameSession::recv()
{
	
	UDPSocket.async_receive_from(boost::asio::buffer(UDPrecvBuffer), remote,
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
					//UDPPacketProcess();
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



void UDPGameSession::PacketSend(void* packet)
{
	int packetsize = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buffer = new unsigned char[packetsize];
	memcpy(buffer, packet, packetsize);
	//auto self(shared_from_this());

	UDPSocket.async_send_to(boost::asio::buffer(buffer, static_cast<size_t>(packetsize)), remote,
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

void UDPGameSession::JoinGroup(const boost::asio::ip::address& multiaddress)
{
	UDPSocket.set_option(boost::asio::ip::multicast::join_group(multiaddress));
}

