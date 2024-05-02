// ---------------------
// Game Server
// ---------------------
#include "TCPGameServer.hpp"

thread_local concurrent_flat_map<int, std::shared_ptr<TCPGameSession>> clients;

GameTCP::GameTCP(boost::asio::io_context& IOContext, int ServerPort) : mTCPAcceptor(IOContext, tcp::endpoint(tcp::v4(), ServerPort)),
	mTCPSocket(IOContext) {
	ServerAccept();
}


void GameTCP::ServerAccept()
{
	mTCPAcceptor.async_accept(mTCPSocket, [this](boost::system::error_code ec) {
		if (ec) {
			std::cout << " AcceptError" << std::endl;
			exit(-1);
		}
		int roomsession = GetRoomNumber();
		clients.emplace(roomsession, std::make_shared<TCPGameSession>(std::move(mTCPSocket), roomsession));
		clients.visit(roomsession, [](auto& x) {
			x.second->Start();
		});

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

TCPGameSession::TCPGameSession(tcp::socket tcpsock, int roomnumber) noexcept
	: TCPSocket(std::move(tcpsock)), RoomNumber(roomnumber)
{
	prevDataSize = 0, curDataSize = 0;
	userID = 0;
	ZeroMemory(TCPrecvBuffer, MAXSIZE);
	ZeroMemory(TCPPacketData, MAXSIZE);
	std::cout << "createGameSession\n";
	//player = 
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

	switch (TCPrecvBuffer[1])
	{
	case CSMOVE:
		moveCharacter();
	default:
		break;
	}
}

void TCPGameSession::moveCharacter()
{

}


