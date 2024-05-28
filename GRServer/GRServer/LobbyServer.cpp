
#include "Protocol.h"
#include "LobbyServer.hpp"
std::atomic_int NumOfClient = 0;


using boost::asio::ip::tcp;
using boost::concurrent_flat_map;
//using boost::asio::awaitable;


concurrent_flat_map<int, std::shared_ptr<LobbyClientSession>> clients;
concurrent_flat_map<int, std::shared_ptr<LobbytoGameSession>> servers;

// --------------------------------------------------------------------------------------------------------------------------------
// Lobby Server -- Clients
// --------------------------------------------------------------------------------------------------------------------------------



void LobbyClientSession::recv() {
	auto self(shared_from_this());
	plSock.async_read_some(boost::asio::buffer(recvBuffer),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (ec) {
				std::cout << "READ ERR" << std::endl;
				exit(-1);
			}
			int BufferLoad = static_cast<int>(length);
			unsigned char* PacketPoint = recvBuffer;
			while (0 < BufferLoad) {
				if (curDataSize == 0) {
					curDataSize = PacketPoint[0];
					if (curDataSize > 200) { std::cout << "BufferErr\n" << std::endl; exit(-1); } //find a error packet
				}
				int build = curDataSize - prevDataSize;

				if (build <= BufferLoad) {
					memcpy(PacketData + prevDataSize, PacketPoint, build);
					LobbyPacketProcess();
					curDataSize = 0;
					prevDataSize = 0;
					BufferLoad -= build;
					PacketPoint += build;
				}
				else {
					memcpy(PacketData + prevDataSize, PacketPoint, build);
					prevDataSize += build;
					build = 0;
					PacketPoint += BufferLoad;
				}
			}
			recv();
		});
}

void LobbyClientSession::Start() {
	recv();
	std::cout << "Start - " << userID << std::endl;
}

void LobbyClientSession::SetAutoMatching()
{
}

void LobbyClientSession::MakeRoom()
{
}

void LobbyClientSession::EnterLobbyRoom()
{
}

void LobbyClientSession::IntoGameServer() 
{
}
void LobbyClientSession::LobbyPacketProcess() {

	switch (PacketData[1])
	{
	case CS_QUICK_MATCHING:
		SetAutoMatching();
		break;
	case CS_CREATE_ROOM:
		MakeRoom();
		break;
	case CS_ENTER_ROOM_CODE:
		EnterLobbyRoom();
		break;
	case CS_START_GAME:
		IntoGameServer();
		break;
	default:
		break;
	}


}




void LobbyTCP::ServerAccept() {
	mTCPAcceptor.async_accept(mTCPSocket, [this](boost::system::error_code ec) {
		if (ec) {
			std::cout << " AcceptError" << std::endl;
			exit(-1);
		}

		if (isGameServer()) {
			// Connect Game Server
			int newsession = GetNewClient();
			servers.emplace(newsession, std::make_shared<LobbytoGameSession>(std::move(mTCPSocket), newsession));
			servers.visit(newsession, [](auto& x) {
				x.second->Start();
				});
		}
		else {
			int newsession = GetNewClient();
			clients.emplace(newsession, std::make_shared<LobbyClientSession>(std::move(mTCPSocket), newsession));
			clients.visit(newsession, [](auto& x) {
				x.second->Start();
				});
		}
		ServerAccept();
		});

}

bool LobbyTCP::isGameServer() {
	for (int i = 0; i < NumOfGameServer; ++i) {
		//if (GameServers[i] == mTCPSocket.remote_endpoint().address()) return true;
	}
	return false;
}



// --------------------------------------------------------------------------------------------------------------------------------
// Lobby Server -- Game Server
// --------------------------------------------------------------------------------------------------------------------------------


void LobbytoGameSession::recv()
{
	auto self(shared_from_this());
	GameSerSock.async_read_some(boost::asio::buffer(recvBuffer),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (ec) {
				std::cout << "READ ERR" << std::endl;
				exit(-1);
			}
			int BufferLoad = static_cast<int>(length);
			unsigned char* PacketPoint = recvBuffer;
			while (0 < BufferLoad) {
				if (curDataSize == 0) {
					curDataSize = PacketPoint[0];
					if (curDataSize > 200) { std::cout << "BufferErr\n" << std::endl; exit(-1); } //find a error packet
				}
				int build = curDataSize - prevDataSize;

				if (build <= BufferLoad) {
					memcpy(PacketData + prevDataSize, PacketPoint, build);
					LobbytoGamePacketProcess();
					curDataSize = 0;
					prevDataSize = 0;
					BufferLoad -= build;
					PacketPoint += build;
				}
				else {
					memcpy(PacketData + prevDataSize, PacketPoint, build);
					prevDataSize += build;
					build = 0;
					PacketPoint += BufferLoad;
				}
			}
			recv();
		});
}

void LobbytoGameSession::LobbytoGamePacketProcess()
{
}
