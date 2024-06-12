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
		CGLinkInfo p;
		mTCPSocket.async_read_some(p, [this,p](boost::system::error_code ec, size_t length) {
			if (ec) { std::cout << " RoomCodeErr" << std::endl; exit(-1); }
			std::string code;
			memcpy(&code, p.RoomCode, sizeof(p.RoomCode));
			Rooms.contains(code);
			Rooms.visit(code, [this,p](auto& x) {
				for (auto usernumber : x.second->userIDS) {
					if (usernumber == p.userID) {
						x.seconde->userSessionS.emplace_back()
					}
				}
			});
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

TCPGameSession::TCPGameSession(tcp::socket tcpsock, int roomnumber) noexcept
	: TCPSocket(std::move(tcpsock)), PartyNumber(PartyNumber)
{
	prevDataSize = 0, curDataSize = 0;
	ZeroMemory(TCPrecvBuffer, MAXSIZE);
	ZeroMemory(TCPPacketData, MAXSIZE);
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

