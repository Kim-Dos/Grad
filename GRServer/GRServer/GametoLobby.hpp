#include <atomic>
#include <boost\asio.hpp>
#include <iostream>
#include "GameRoom.hpp"
#include "TCPGameServer.hpp"
#include "Player.h"
#include "Protocol.h"
#pragma once


using boost::asio::ip::tcp;

class GametoLobby {

	tcp::socket lobbySock;

	int prevDataSize, curDataSize;
	unsigned char TCPrecvBuffer[MAXSIZE];
	unsigned char TCPPacketData[MAXSIZE];

	void Connect_Handler(boost::system::error_code ec);

	void GameLobbyProcess();

	void recv();

	void SendPacket(void* packet);

	void LobbyConnect();

	void SetGameRoom();

	void RefairOtherServer();

public:
	GametoLobby(boost::asio::io_context& context) noexcept;
	
	~GametoLobby() {}

	void timeSend();
};