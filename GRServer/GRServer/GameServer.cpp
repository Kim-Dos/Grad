// ---------------------
// Game Server
// ---------------------

//���� ������ ���ư��� �� UDP,TCP�� ���� �����
// �÷��̾���� ��� UDP+TCP
// DataServer���� ��� TCP
// �������� ���� ������ + ��Ž����� -> 2������� 1���� (��������Ʈ)


#include <boost/asio.hpp>
#include <boost\unordered\concurrent_flat_map.hpp>
#include <iostream>

#include <vector>
#include <thread>
//#include <boost/timer.hpp>
#include <memory>
#include <atomic>

#include "Protocol.h"

std::atomic_int NumOfClient = 0;


using boost::asio::ip::tcp;
using boost::asio::ip::udp;




using boost::concurrent_flat_map;
//using boost::asio::awaitable;

class session;

concurrent_flat_map<int, std::shared_ptr<session>> clients;




// ---------------------
// Lobby Server
// ---------------------



int GetClientID()
{
	return NumOfClient++;
}


class session
	: public std::enable_shared_from_this<session>
{
private:
	tcp::socket plSock; 
	int prevDataSize, curDataSize; 
	int userID; 
	UCHAR recvBuffer[MAXSIZE]; // 

	void recv() {
		auto self(shared_from_this());
		plSock.async_read_some(boost::asio::buffer(recvBuffer),
			[this, self](boost::system::error_code ec, std::size_t length)
			{
				if (ec) {
					std::cout << "READ ERR" << std::endl;
					exit(-1);
				}
				int BufferLoad = static_cast<int>(length);
				if (curDataSize == 0) {

				}



				recv();
			});
	}

public:
	session(tcp::socket sock, int numofclient) noexcept
		: plSock(std::move(sock)), userID(numofclient) {
		prevDataSize = 0; curDataSize = 0;
	}
	~session() {}

	void Start() {

		recv();

		std::cout << "Start" << std::endl;
		


	}
};



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
			int newsession = GetClientID();
			clients.emplace(newsession, std::make_shared<session>(std::move(mServerSocket), newsession));
			clients.visit(newsession, [](auto& x) {
				x.second->Start();
				});

			ServerAccept();
			});
	}
public:
	Server(boost::asio::io_context& IOContext, int ServerPort)
		: mServerAcceptor(IOContext, tcp::endpoint(tcp::v4(), ServerPort)),
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

	Server s(IOContext, SERVERPORT);

	std::vector<std::jthread> WorkerThreads;

	WorkerThreads.emplace_back(worker_thread, &IOContext);

}
