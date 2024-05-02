// ---------------------
// Game Server
// ---------------------
#include "GameServer.hpp"

concurrent_flat_map<int, std::shared_ptr<GameSession>> clients;

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
		clients.emplace(roomsession, std::make_shared<GameSession>(std::move(mTCPSocket), roomsession));
		clients.visit(roomsession, [](auto& x) {
			x.second->Start();
		});

		ServerAccept();
	});
}


void GameSession::recv() {
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

GameSession::GameSession(tcp::socket tcpsock, int roomnumber) noexcept
	: TCPSocket(std::move(tcpsock)), RoomNumber(roomnumber)
{
	prevDataSize = 0, curDataSize = 0;
	userID = 0;
	ZeroMemory(TCPrecvBuffer, MAXSIZE);
	ZeroMemory(TCPPacketData, MAXSIZE);
	std::cout << "createGameSession\n";
}

GameSession::~GameSession()
{
}

void GameSession::Start()
{
	recv();
	std::cout << "START\n";
}

void GameSession::GamePacketProcess()
{
	std::cout << TCPrecvBuffer << std::endl;
}

void GameUDP::StartReceive()
{
}

GameUDP::GameUDP(boost::asio::io_context& IOContext) : mUDPSocket(IOContext, udp::endpoint(udp::v4(), SERVERPORT))
{

}
