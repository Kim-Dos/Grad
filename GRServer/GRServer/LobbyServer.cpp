
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
			while()
			int BufferLoad = static_cast<int>(length);
			if (curDataSize == 0) {
				curDataSize = recvBuffer[0];
				if (curDataSize > 200) { std::cout << "BufferErr\n" << std::endl; exit(-1); } //find a error packet
			}
			int build = curDataSize - prevDataSize;
			if (build <= BufferLoad) {
				// 패킷 조립
				memcpy(PacketData + prevDataSize, recvBuffer, build);
				LobbyPacketProcess();
				curDataSize = 0;
				prevDataSize = 0;
				BufferLoad -= build;
				recvBuffer += build;
			}
			else {
				// 훗날을 기약
				memcpy(PacketData + prevDataSize, recvBuffer, build);
				prevDataSize += build;
				build = 0;
				buf += data_to_process;
			}






			recv();
		});
}

void LobbySession::Start() {
	recv();
	std::cout << "Start" << std::endl;
}


void LobbyTCP::ServerAccept() {
	mTCPAcceptor.async_accept(mTCPSocket, [this](boost::system::error_code ec) {
		if (ec) {
			std::cout << " AcceptError" << std::endl;
			exit(-1);
		}
		int newsession = GetNewClient();
		clients.emplace(newsession, std::make_shared<session>(std::move(mTCPSocket), newsession));
		clients.visit(newsession, [](auto& x) {
			x.second->Start();
			});

		ServerAccept();
		});
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