#include "TCPGameServer.hpp"
#include "UDPGameServer.hpp"
#include "GameRoom.hpp"

concurrent_flat_map<std::string, std::shared_ptr<GameRoom>> Rooms;

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

    // 데모용 게임 룸 미리 생성
    auto demoRoom = std::make_shared<GameRoom>("DEMO", "DemoMap");
    Rooms.emplace("DEMO", demoRoom);
    std::cout << "Demo room created: DEMO" << std::endl;

    GameTCP tcpAcceptor(IoContext, SERVERPORT);
    //GameUDP udpAcceptor(IoContext, SERVERPORT);

    Init_Server();

    for (auto i = 0; i < 6; ++i) worker_Threads.emplace_back(worker_thread, std::ref(IoContext));
    for (auto& th : worker_Threads) th.join();
}