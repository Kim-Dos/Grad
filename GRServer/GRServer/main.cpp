#include "TCPGameServer.hpp"
//#include "UDPGameServer.hpp"
#include "RoomManager.hpp"


void Init_Server()
{
    _wsetlocale(LC_ALL, L"korean");
}

void worker_thread(boost::asio::io_context& IoContext)
{
    IoContext.run();
}

int main() {
    boost::asio::io_context IoContext;
    auto work = boost::asio::make_work_guard(IoContext);

    RoomManager roomManager(std::ref(IoContext));
    roomManager.CreateRoom("DEMO", "DemoMap");

    GameTCP tcpAcceptor(std::ref(IoContext), SERVERPORT, roomManager);

    Init_Server();

    std::vector<std::thread> worker_Threads;
    for (int i = 0; i < 6; ++i)
        worker_Threads.emplace_back([&IoContext]() { IoContext.run(); });
    for (auto& th : worker_Threads) th.join();
}