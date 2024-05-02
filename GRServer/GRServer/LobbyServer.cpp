
#include "Protocol.h"
#include "LobbyServer.h"
std::atomic_int NumOfClient = 0;


using boost::asio::ip::tcp;
using boost::concurrent_flat_map;
//using boost::asio::awaitable;


// ---------------------
// Lobby Server
// ---------------------


void LobbySession::recv() {
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

void LobbySession::Start() {
	recv();
	std::cout << "Start - " << userID << std::endl;
}

void LobbySession::SetAutoMatching()
{
}

void LobbySession::MakeRoom()
{
}

void LobbySession::EnterLobbyRoom()
{
}

void LobbySession::IntoGameServer() {

}
void LobbySession::LobbyPacketProcess() {

	switch (PacketData[1])
	{
	case ClickMatching:
		SetAutoMatching();
		break;
	case CreateRoom:
		MakeRoom();
		break;
	case EnterRoomcode:
		EnterLobbyRoom();
		break;
	case GameStart:
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
		}
		else {
			int newsession = GetNewClient();
			clients.emplace(newsession, std::make_shared<session>(std::move(mTCPSocket), newsession));
			clients.visit(newsession, [](auto& x) {
				x.second->Start();
				});
		}
		ServerAccept();
		});
}

bool LobbyTCP::isGameServer() {
	for (int i = 0; i < NumOfGameServer; ++i) {
		if (GameServers[i] == mTCPSocket.remote_endpoint().address()) return true;
	}
	return false;
}

/*
void worker_thread(boost::asio::io_context* service)
{
	service->run();
}

int main()
{
	boost::asio::io_context IOContext;

	Server s(IOContext, SERVERPORT);

	std::vector<std::jthread> WorkerThreads;

	WorkerThreads.emplace_back(worker_thread, &IOContext);

}
*/