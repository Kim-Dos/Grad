#pragma once
#include "TCPDevice.h"
#include <boost\unordered\concurrent_flat_map.hpp>
#include <atomic>

using boost::concurrent_flat_map;

//TCP Connector
class GameTCP : public TCPDevice {

public:

	void StartSession();

	int GetRoomNumver() { return roomNumber++; }

	void PacketProcess() {}

private:
	std::atomic_int roomNumber = 0;
	concurrent_flat_map<int, std::shared_ptr<session>> clients;

	void ServerAccept();
};

//Player Session-------------------------------------------------------------

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