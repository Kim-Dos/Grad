#include "RoomManager.hpp"
#include "Protocol.h"
#pragma once

using boost::concurrent_flat_map;
using boost::asio::ip::tcp;

class GameRoom;

//TCP Acceptor
class GameTCP {

public:

	GameTCP(boost::asio::io_context& IOContext, int ServerPort, RoomManager& roomMgr);

private:
	boost::asio::io_context& ioc_;
	tcp::socket mTCPSocket;
	tcp::acceptor mTCPAcceptor;
	RoomManager& roomManager_;
	void ServerAccept();
};



//Player Session-------------------------------------------------------------

class TCPGameSession
	: public std::enable_shared_from_this<TCPGameSession>
{
private:

	tcp::socket TCPSocket;
	boost::asio::strand<boost::asio::io_context::executor_type> strand_;

	std::deque<std::shared_ptr<std::vector<unsigned char>>> writeQueue_;
	bool writing_ = false;

	int playerNumber;
	char PartyRoomCode[RoomCodeLen];
	unsigned char TCPrecvBuffer[MAXSIZE];
	unsigned char TCPPacketData[MAXSIZE];
	int prevDataSize, curDataSize;

	void recv();

	void doWrite();

public:

	TCPGameSession(tcp::socket tcpsock, boost::asio::io_context& ioc);

	~TCPGameSession();

	void Start();


	void QueueSend(std::shared_ptr<std::vector<unsigned char>> data);

	void GamePacketProcess();

	void PacketSend(void* packet);

	inline const tcp::socket& getSocket() { return TCPSocket; }

	void SetRoomCode(const std::string& code);
	void SetPlayerNumber(int num);
	inline int GetPlayerNumber() const { return playerNumber; }
};