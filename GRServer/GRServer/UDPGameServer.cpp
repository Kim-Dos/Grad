#include "UDPGameServer.hpp"


extern concurrent_flat_map<int, std::shared_ptr<GameRoom>> Rooms;
thread_local concurrent_flat_map<int, std::shared_ptr<UDPGameSession>> udp_clients;

GameUDP::GameUDP(boost::asio::io_context& IOContext, int port) noexcept
	: mUDPSocket(IOContext, udp::endpoint(udp::v4(),port))
{
	int roomsession = GetRoomNumber();
	udp_clients.emplace(roomsession, std::make_shared<UDPGameSession>(std::move(mUDPSocket)));
	udp_clients.visit(roomsession, [](auto& x) {
		x.second->Start();
	});
}

//void GameUDP::Set_remote_endpoint(std::string str, int port)
//{
//	[this](auto& x) {
//		x->remote_endpoint(boost::asio::ip::address::from_string(str), port)) };
//}



UDPGameSession::UDPGameSession(udp::socket udpsock) noexcept
	: UDPSocket(std::move(udpsock))
{
	RoomNumber = 0;
	ZeroMemory(UDPPacketData, MAXSIZE);
	ZeroMemory(UDPrecvBuffer, MAXSIZE);
	userID = 0;
	curDataSize = 0;
	prevDataSize = 0;
	player.SetDefault();
	player.SetPosition(1, 1, 1);
	std::cout << player.GetPosX() << player.GetPosY() << player.GetPosZ() << std::endl;

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

void UDPGameSession::UDPPacketProcess()
{


	switch (UDPPacketData[1])
	{
	case CS_MOVEMENT:
		moveCharacter();
		break;
	case CS_ATTACK:
		generalAttack();
	default:
		break;
	}
}

void UDPGameSession::moveCharacter()
{
	FXYZ position;
	memcpy(&position, UDPPacketData + 2, sizeof(FXYZ));
	player.SetPosition(position);  

	std::cout << player.GetPosX() << player.GetPosY() << player.GetPosZ() << std::endl;

	clients.cvisit_all([](auto x) { if (x->second.getRemoteAdress() != remote.adress()) {


	}
		});

	//PacketSend(&packet);
}

void UDPGameSession::generalAttack()
{
}

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