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
	unsigned char packetType = TCPPacketData[1]; // [0]=size, [1]=type

	switch (packetType)
	{
	case CS_MOVE_OBJ_REQUEST:
	{
		auto room = room_.lock();
		if (!room) break;

		CSMoveObjRequest req;
		memcpy(&req, TCPPacketData, sizeof(CSMoveObjRequest)); // 값 복사

		room->HandleMoveRequest(playerNumber, req);
		break;
	}

	case CS_MOVE_MULTI_REQUEST:
	{
		auto room = room_.lock();
		if (!room) break;

		CSMoveMultiRequest req;
		memcpy(&req, TCPPacketData, sizeof(CSMoveMultiRequest));

		room->HandleMultiMove(playerNumber, req);
		break;
	}

	case CS_STOP_OBJ_REQUEST:
	{
		auto room = room_.lock();
		if (!room) break;

		CSStopObjRequest req;
		memcpy(&req, TCPPacketData, sizeof(CSStopObjRequest));

		room->HandleStopRequest(playerNumber, req);
		break;
	}

	// 기존 Packet_Type 계열 (입장 등)
	case CG_LINKGAMESERVER:
	{
		// TODO: CGLinkInfo의 RoomCode 확인 → RoomManager::JoinRoom
		//       현재는 accept 시 DEMO 방에 바로 넣는 구조라면 생략 가능
		break;
	}

	default:
		std::cout << "[Session " << playerNumber
			<< "] unknown packet type: " << (int)packetType << std::endl;
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