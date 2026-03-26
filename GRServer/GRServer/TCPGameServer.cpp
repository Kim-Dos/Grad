// ---------------------
// Game Server
// ---------------------
#include "TCPGameServer.hpp"


GameTCP::GameTCP(boost::asio::io_context& IOContext, int ServerPort, RoomManager& roomMgr)
	: ioc_(IOContext)
	, mTCPAcceptor(IOContext, tcp::endpoint(tcp::v4(), ServerPort))
	, mTCPSocket(IOContext)
	, roomManager_(roomMgr)
{
	ServerAccept();
}


void GameTCP::ServerAccept()
{
	mTCPAcceptor.async_accept(mTCPSocket, [this](boost::system::error_code ec) {
		if (ec) {
			std::cout << "AcceptError" << std::endl;
			return;
		}

		auto newSession = std::make_shared<TCPGameSession>(std::move(mTCPSocket), ioc_);

		roomManager_.JoinRoom("DEMO", newSession);
		newSession->Start();
		ServerAccept();
	});
}


void TCPGameSession::recv() {

	auto self(shared_from_this());
	TCPSocket.async_read_some(boost::asio::buffer(TCPrecvBuffer), boost::asio::bind_executor(strand_,
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
		}));
}

void TCPGameSession::doWrite()
{
	if (writeQueue_.empty()) {
		writing_ = false;
		return;
	}

	writing_ = true;
	auto& front = writeQueue_.front();

	boost::asio::async_write(TCPSocket,
		boost::asio::buffer(*front),
		boost::asio::bind_executor(strand_,
			[this, self = shared_from_this()](boost::system::error_code ec, std::size_t) {
				if (ec) {
					std::cout << "Write ERR: " << ec.message() << std::endl;
					writeQueue_.clear();
					writing_ = false;
					return;
				}
				writeQueue_.pop_front();
				doWrite(); // 큐에 남은 거 계속 처리
			}));
}

TCPGameSession::TCPGameSession(tcp::socket sock, boost::asio::io_context& ioc)
	: TCPSocket(std::move(sock))
	, strand_(boost::asio::make_strand(ioc))
	, playerNumber(0), prevDataSize(0), curDataSize(0)
{
	ZeroMemory(TCPrecvBuffer, MAXSIZE);
	ZeroMemory(TCPPacketData, MAXSIZE);
	ZeroMemory(PartyRoomCode, RoomCodeLen);
}

TCPGameSession::~TCPGameSession()
{
}

void TCPGameSession::Start()
{
	recv();
	std::cout << "START\n";
}

void TCPGameSession::QueueSend(std::shared_ptr<std::vector<unsigned char>> data)
{
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

		// GameRoom에 위임
		//std::string roomCode(PartyRoomCode, RoomCodeLen);
		//Rooms.visit(roomCode, [this, &gcPacket](auto&& pair) {
		//	pair.second->BroadcastToOthers(
		//		playerNumber, reinterpret_cast<unsigned char*>(&gcPacket));
		//	});
		break;
	}

	case CG_ATTACK: // 공격
	{
		std::cout << "[Player " << playerNumber << "] Attack" << std::endl;

		// 다른 플레이어에게 전송
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