#pragma once
#include <boost\asio.hpp>
#include <iostream>
#include "Protocol.h"

using boost::asio::ip::tcp;




class TCPDevice {
protected:
	tcp::socket mTCPSocket;
	tcp::acceptor mTCPAcceptor;


public:
	TCPDevice(boost::asio::io_context& IOContext, int ServerPort)
		: mTCPAcceptor(IOContext, tcp::endpoint(tcp::v4(), ServerPort)),
		mTCPSocket(IOContext) {}
};
