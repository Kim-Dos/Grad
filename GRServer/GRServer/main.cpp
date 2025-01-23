#include "TCPGameServer.hpp"
#include "UDPGameServer.hpp"
#include "GametoLobby.hpp"
#include "Player.h"




void Init_Server()
{
	_wsetlocale(LC_ALL, L"korean");
}

void worker_thread(boost::asio::io_context& IoContext)
{
	IoContext.run();
}

int main() {

	
	std::vector<std::thread> worker_Threads;

	boost::asio::io_context IoContext;
	//GametoLobby gameconnectlobby(IoContext);
	GameTCP tcpAcceptor(IoContext, SERVERPORT);
	GameUDP udpAcceptor(IoContext, SERVERPORT);

	Init_Server();

	for (auto i = 0; i < 6; ++i) worker_Threads.emplace_back(worker_thread, IoContext);
	for (auto& th : worker_Threads) th.join();

}