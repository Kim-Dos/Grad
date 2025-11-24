// ---------------------
// Game Server
// ---------------------
#include "TCPGameServer.hpp"

extern concurrent_flat_map<std::string, std::shared_ptr<GameRoom>> Rooms;

GameTCP::GameTCP(boost::asio::io_context& IOContext, int ServerPort) : mTCPAcceptor(IOContext, tcp::endpoint(tcp::v4(), ServerPort)), mTCPSocket(IOContext)
{
	ServerAccept();
}


void GameTCP::ServerAccept()
{
	mTCPAcceptor.async_accept(mTCPSocket, [this](boost::system::error_code ec) {
		if (ec) {
			std::cout << " AcceptError" << std::endl;
			exit(-1);
		}

		// TCPGameSession 생성
		auto newSession = std::make_shared<TCPGameSession>(std::move(mTCPSocket));

		// 세션 시작 (recv 시작)
		newSession->Start();

		// 다음 클라이언트 accept 대기
		ServerAccept();
		});
}


void TCPGameSession::recv() {

	auto self(shared_from_this());
	TCPSocket.async_read_some(boost::asio::buffer(TCPrecvBuffer),
		[this, self](boost::system::error_code ec, std::size_t length)
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
					GamePacketProcess();
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

TCPGameSession::TCPGameSession(tcp::socket tcpsock) noexcept
	: TCPSocket(std::move(tcpsock))
{
	prevDataSize = 0, curDataSize = 0;
	ZeroMemory(TCPrecvBuffer, MAXSIZE);
	ZeroMemory(TCPPacketData, MAXSIZE);
	ZeroMemory(PartyRoomCode, RoomCodeLen);
	std::cout << "createGameSession\n";

}

TCPGameSession::~TCPGameSession()
{
}

void TCPGameSession::Start()
{
	recv();
	std::cout << "START\n";
}

void TCPGameSession::GamePacketProcess()
{
	// 패킷 타입에 따라 처리
	Packet_Type type = static_cast<Packet_Type>(TCPPacketData[1]);

	switch (type) {
	case CG_LINKGAMESERVER: // 클라이언트가 게임 서버에 연결 요청
	{
		CGLinkInfo packet;
		memcpy(&packet, TCPPacketData, sizeof(CGLinkInfo));

		// 룸 코드 저장
		memcpy(PartyRoomCode, packet.RoomCode, RoomCodeLen);

		std::string roomCode(packet.RoomCode, RoomCodeLen);

		// 해당 룸이 존재하는지 확인
		if (Rooms.contains(roomCode)) {
			Rooms.visit(roomCode, [this, &packet](auto& pair) {
				auto& room = pair.second;

				// 첫 번째 플레이어인지 두 번째 플레이어인지 확인
				if (!room->FirstTCPSession) {
					room->FirstTCPSession = shared_from_this();
					std::cout << "Player 1 connected to room: " << roomCode << std::endl;
				}
				else if (!room->SecondTCPSession) {
					room->SecondTCPSession = shared_from_this();
					std::cout << "Player 2 connected to room: " << roomCode << std::endl;
				}
				else {
					std::cout << "Room is full: " << roomCode << std::endl;
				}
				});
		}
		else {
			std::cout << "Room not found: " << roomCode << std::endl;
		}
		break;
	}

	case CG_MOVEMENT:
	{
		// 이동 패킷 처리
		CGPickingMove packet;
		memcpy(&packet, TCPPacketData, sizeof(CGPickingMove));

		// TODO: 게임 룸의 오브젝트 업데이트

		break;
	}

	case CG_ATTACK:
	{
		// 공격 패킷 처리
		// TODO: 구현
		break;
	}

	default:
		std::cout << "Unknown packet type: " << (int)type << std::endl;
		break;
	}
}



void TCPGameSession::PacketSend(void* packet)
{
	int packetsize = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buffer = new unsigned char[packetsize];
	memcpy(buffer, packet, packetsize);
	//auto self(shared_from_this());
	TCPSocket.async_write_some(boost::asio::buffer(buffer, static_cast<size_t>(packetsize)),
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