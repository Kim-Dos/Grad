#include "TCPGameServer.hpp"
#include "UDPGameServer.hpp"
#include "Player.h"




void Init_Server()
{
	_wsetlocale(LC_ALL, L"korean");
}

void worker_thread(const int& i)
{
	boost::asio::io_context IoContext;

	GameTCP tcpAcceptor(IoContext, SERVERPORT+i);
	GameUDP udpAcceptor(IoContext, SERVERPORT+i);

	IoContext.run();
}

int main() {

	
	std::vector<std::thread> worker_Threads;



	Init_Server();

	for (auto i = 0; i < 6; ++i) worker_Threads.emplace_back(worker_thread, &i);
	for (auto& th : worker_Threads) th.join();

}