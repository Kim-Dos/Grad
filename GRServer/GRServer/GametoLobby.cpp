#include "GametoLobby.hpp"


extern concurrent_flat_map<std::string, std::shared_ptr<GameRoom>> Rooms;

// Handler ----------------------------------------------------
void GametoLobby:: Connect_Handler(boost::system::error_code ec)
{
	if (ec) {
		std::cout << ec.what() << std::endl;
		exit(-1);
	}

	// 이후에 일정 시간마다 Data 보내기 및 허락된 인원과의 연결의 중간 지점 설정
}

void GametoLobby::GameLobbyProcess()
{
	switch (TCPPacketData[1])
	{
	case LG_ROOMINFO:
		SetGameRoom();
		break;
	case LG_REFAIRROOM:
		RefairOtherServer();
		break;
	default:
		break;
	}
}

void GametoLobby::SendPacket(void* packet)
{
	int packetsize = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buffer = new unsigned char[packetsize];
	memcpy(buffer, packet, packetsize);
	//auto self(shared_from_this());
	lobbySock.async_write_some(boost::asio::buffer(buffer, static_cast<size_t>(packetsize)),
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

void GametoLobby::LobbyConnect()
{
	lobbySock.async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(Lobby_IP), SERVERPORT), Connect_Handler);
}

void GametoLobby::SetGameRoom()
{
	std::string tmp;
	int add = 2;
	memcpy(&tmp, TCPPacketData + add, sizeof(RoomCodeLen));
	add += sizeof(RoomCodeLen);
	Rooms.emplace(tmp, std::make_shared<GameRoom>(TCPPacketData[add]));
	add += sizeof(unsigned char);
	Rooms.visit(tmp, [this](auto& x) {
		int userNumber;
		for (int i = 0; i < 4; ++i) {
			memcpy(&userNumber, TCPPacketData + add, sizeof(int));
			x.second->setUserIDS(&userNumber);
			add += sizeof(int);
		}
	});
}

// 아직 안만듬
void GametoLobby::RefairOtherServer()
{
	//아직 안만듬
}

GametoLobby::GametoLobby(boost::asio::io_context& context) noexcept
	:  lobbySock(context)
{
	ZeroMemory(TCPPacketData, MAXSIZE);
	ZeroMemory(TCPrecvBuffer, MAXSIZE);
	curDataSize = 0, prevDataSize = 0;

	LobbyConnect();
}

void GametoLobby::timeSend()
{
	GLServerAmount packet;
	packet.size = sizeof(GLServerAmount);
	packet.type = GL_SERVERAMOUNT;
	packet.amount = Rooms.size();
	SendPacket(&packet);
}	


void GametoLobby::recv() {

	lobbySock.async_read_some(boost::asio::buffer(TCPrecvBuffer),
		[this](boost::system::error_code ec, std::size_t length)
		{
			if (ec) {
				std::cout << "READ ERR" << std::endl;
				exit(-1);
			}
			//std::cout  << "recv length" << length << std::endl;
			int BufferLoad = static_cast<int>(length);
			unsigned char* PacketPoint = TCPrecvBuffer;
			while (0 < BufferLoad) {
				if (curDataSize == 0) {
					curDataSize = PacketPoint[0];
					if (curDataSize > 200) { std::cout << "BufferErr\n" << std::endl; exit(-1); } //find a error packet
				}
				int build = curDataSize - prevDataSize;

				if (build <= BufferLoad) {
					memcpy(TCPPacketData + prevDataSize, PacketPoint, build);
					GameLobbyProcess();
					curDataSize = 0;
					prevDataSize = 0;
					BufferLoad -= build;
					PacketPoint += build;
				}
				else {
					memcpy(TCPPacketData + prevDataSize, PacketPoint, build);
					prevDataSize += build;
					build = 0;
					PacketPoint += BufferLoad;
				}
			}
			recv();
		});
}