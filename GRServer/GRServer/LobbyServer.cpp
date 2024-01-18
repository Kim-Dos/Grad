
#include <boost/asio.hpp>
#include <boost\unordered\concurrent_flat_map.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <thread>
//#include <boost/timer.hpp>
#include <memory>
#include <atomic>

#include "Protocol.h"

int NumOfClient = 0;


using boost::asio::ip::tcp;
using boost::unordered::concurrent_flat_map;
//using boost::asio::awaitable;

class session;

concurrent_flat_map<int, std::shared_ptr<session>> clients;

// ---------------------
// Lobby Server
// ---------------------


// 게임 서버 세션일때
//How? , Session - 작업최소단위 , Coroutine 사용, shared_ptr 형태, atomic계산?
// 버퍼처리를 여기서? 작업 크기는 어느정도? Coroutine을 사용한다면 어느정도 크기가 있어야 하나?
// 각 session은 플레이어캐릭터 단위로한다면? -> 각 플레이어 마다 가지는 상위클래스 필요 -> 방 하나를 상속받아서 보내주는 느낌
// 각 행동을 지연가능객체로 지정하자 
// awaitable<> 객체로 행동 패턴을 설정 -> 비동기 형태로 작성해야함-> queue? 
// -----------------------------------------------------------------------
// 로비서버의 세션은 플레이어 단위로 지정하자
class session 
	: public std::enable_shared_from_this<session>
{
private:
	tcp::socket plSock;
	int prevDataSize,curDataSize;
	int userID;
	UCHAR recvBuffer[MAXSIZE];

	void recv() {
		auto self(shared_from_this());
		plSock.async_read_some(boost::asio::buffer(recvBuffer),
			[this, self](boost::system::error_code ec, std::size_t length)
			{
				if (ec) {
					std::cout << "READ ER" << std::endl;
					exit(-1);
				}
				int BufferLoad = static_cast<int>(length);
				if (curDataSize == 0) {

				}
			});

	
	}
	

public:
	session(tcp::socket sock, int numofclient) noexcept
		: plSock(std::move(sock)), userID(numofclient)  {
		prevDataSize = 0; curDataSize = 0;
	}
	void Start() {

	}
};



//서버 연결 및 통신 시작지점
class Server {
private:
	tcp::socket mServerSocket;
	tcp::acceptor mServerAcceptor;

	void ServerAccept() {
		mServerAcceptor.async_accept(mServerSocket, [this](boost::system::error_code ec) {
			if (ec) {
				std::cout << " AcceptError" << std::endl;
				exit(-1);
			}
			++NumOfClient;
			//clients.emplace(std::make_shared<session>(std::move(mServerSocket), NumOfClient));
			ServerAccept();
		});
	}
public:
	Server(boost::asio::io_context& IOContext, int ServerPort)
		: mServerAcceptor(IOContext, tcp::endpoint(tcp::v4(),ServerPort)),
		mServerSocket(IOContext)
	{
		ServerAccept();
	}
};

void worker_thread(boost::asio::io_context* service)
{
	service->run();
}

int main()
{
	boost::asio::io_context IOContext;

	//Server s(IOContext, SERVERPORT);

	std::vector<std::jthread> WorkerThreads;

	WorkerThreads.emplace_back(worker_thread, &IOContext);

}