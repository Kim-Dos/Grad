#pragma once
#include <boost\asio.hpp>
#include <iostream>
#include "Protocol.h"

using boost::asio::ip::tcp;


class session;


class TCPDevice {
protected:
	tcp::socket mTCPSocket;
	tcp::acceptor mTCPAcceptor;

	std::atomic_int mNumofClients = 0;

	virtual void ServerAccept();


public:
	TCPDevice(boost::asio::io_context& IOContext, int ServerPort)
		: mTCPAcceptor(IOContext, tcp::endpoint(tcp::v4(), ServerPort)),
		mTCPSocket(IOContext)
	{

		ServerAccept();
	}

	virtual void StartSession();

	int GetNumOfClient() { return mNumofClients; }

};
