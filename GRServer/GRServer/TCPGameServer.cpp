// ---------------------
// Game Server
// ---------------------
#include "TCPGameServer.hpp"
#include "GameRoom.hpp"

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

		// 데모: 바로 DEMO 룸에 연결
		std::string demoRoomCode = "DEMO";
		if (Rooms.contains(demoRoomCode)) {
			Rooms.visit(demoRoomCode, [&newSession](auto&& pair) {
				auto& room = pair.second;

				if (!room->FirstTCPSession) {
					room->FirstTCPSession = newSession;
					newSession->SetRoomCode("DEMO");
					newSession->SetPlayerNumber(1);
					std::cout << "Player 1 connected to DEMO room" << std::endl;
				}
				else if (!room->SecondTCPSession) {
					room->SecondTCPSession = newSession;
					newSession->SetRoomCode("DEMO");
					newSession->SetPlayerNumber(2);
					std::cout << "Player 2 connected to DEMO room" << std::endl;
				}
				else {
					std::cout << "DEMO room is full" << std::endl;
				}
				});
		}

		// 세션 시작
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
				std::cout << "READ ERR: " << ec.message() << std::endl;
				return; // exit 대신 return으로 변경
			}

			int BufferLoad = static_cast<int>(length);
			unsigned char* PacketPoint = TCPrecvBuffer;
			while (0 < BufferLoad) {
				if (curDataSize == 0) {
					curDataSize = PacketPoint[0];
					if (curDataSize > 200) {
						std::cout << "BufferErr\n" << std::endl;
						return;
					}
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
	: TCPSocket(std::move(tcpsock)), playerNumber(0)
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
	case CG_MOVEMENT: // 피킹된 객체 이동
	{
		CGPickingMove packet;
		memcpy(&packet, TCPPacketData, sizeof(CGPickingMove));

		std::cout << "[Player " << playerNumber << "] Movement - PickingSize: "
			<< (int)packet.pickingsize << std::endl;

		// 다른 플레이어에게 GC_OTHER_MOVEMENT로 변환하여 전송
		GCPickingMove gcPacket;
		gcPacket.size = sizeof(GCPickingMove);
		gcPacket.type = GC_OTHER_MOVEMENT;
		gcPacket.pickingsize = packet.pickingsize;
		gcPacket.playerNumber = playerNumber; // 누가 보낸 것인지 표시
		gcPacket.act_command = packet.act_command;
		memcpy(gcPacket.move_data, packet.move_data, sizeof(packet.move_data));

		BroadcastToOthers(reinterpret_cast<unsigned char*>(&gcPacket));
		break;
	}

	case CG_ATTACK: // 공격
	{
		std::cout << "[Player " << playerNumber << "] Attack" << std::endl;

		// 다른 플레이어에게 전송
		BroadcastToOthers(TCPPacketData);
		break;
	}

	default:
		std::cout << "Unknown packet type: " << (int)type << std::endl;
		break;
	}
}

void TCPGameSession::BroadcastToOthers(unsigned char* packetData)
{
	std::string roomCode(PartyRoomCode, RoomCodeLen);

	if (Rooms.contains(roomCode)) {
		Rooms.visit(roomCode, [this, packetData](auto&& pair) {
			auto& room = pair.second;

			// 자신을 제외한 다른 플레이어에게 전송
			if (room->FirstTCPSession && room->FirstTCPSession.get() != this) {
				room->FirstTCPSession->PacketSend(packetData);
				std::cout << "  -> Broadcasted to Player 1" << std::endl;
			}
			if (room->SecondTCPSession && room->SecondTCPSession.get() != this) {
				room->SecondTCPSession->PacketSend(packetData);
				std::cout << "  -> Broadcasted to Player 2" << std::endl;
			}
			});
	}
}

void TCPGameSession::PacketSend(void* packet)
{
	int packetsize = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buffer = new unsigned char[packetsize];
	memcpy(buffer, packet, packetsize);

	auto self(shared_from_this());
	TCPSocket.async_write_some(boost::asio::buffer(buffer, static_cast<size_t>(packetsize)),
		[this, self, buffer, packetsize](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)
			{
				if (packetsize != bytes_transferred) {
					std::cout << "ERR - Bytes_transferred\n";
				}
				delete[] buffer;
			}
			else {
				std::cout << "Send ERR: " << ec.message() << std::endl;
				delete[] buffer;
			}
		});
}

void TCPGameSession::SetRoomCode(const std::string& code)
{
	memcpy(PartyRoomCode, code.c_str(), std::min(code.length(), (size_t)RoomCodeLen));
}

void TCPGameSession::SetPlayerNumber(int num)
{
	playerNumber = num;
}