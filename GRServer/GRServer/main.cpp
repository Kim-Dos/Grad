#include "GameServer.hpp"
#include "Player.h"



void Init_Server()
{
	_wsetlocale(LC_ALL, L"korean");
}

void worker_thread(boost::asio::io_context* service)
{
	service->run();
}

int main() {

	boost::asio::io_context IoContext;
	
	std::vector<std::thread> worker_Threads;


	GameTCP tcpAcceptor(IoContext, SERVERPORT);
	GameUDP udpAcceptor(IoContext);


	Init_Server();

	for (auto i = 0; i < 6; ++i) worker_Threads.emplace_back(worker_thread, &IoContext);
	for (auto& th : worker_Threads) th.join();

}