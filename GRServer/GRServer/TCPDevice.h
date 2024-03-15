#pragma once
#include <boost\asio.hpp>
#include <iostream>
#include "Protocol.h"

using boost::asio::ip::tcp;




class TCPDevice {
protected:
	tcp::socket mTCPSocket;
	tcp::acceptor mTCPAcceptor;

	void ServerAccept() {
		mTCPAcceptor.async_accept(mTCPSocket, [this](boost::system::error_code ec) {
			if (ec) {
				std::cout << " AcceptError" << std::endl;
				exit(-1);
			}
			ServerAccept();
			});
	}


public:
	TCPDevice(boost::asio::io_context& IOContext, int ServerPort)
		: mTCPAcceptor(IOContext, tcp::endpoint(tcp::v4(), ServerPort)),
		mTCPSocket(IOContext)
	{

		ServerAccept();
	}

	virtual void StartSession(){}
};
