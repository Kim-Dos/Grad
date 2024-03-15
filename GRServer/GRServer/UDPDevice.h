#pragma once
#include <boost\asio.hpp>
#include <iostream>
#include "Protocol.h"

using boost::asio::ip::udp;



class UDPDevice {
private:
	udp::socket mUDPSocket;


public:
	UDPDevice(boost::asio::io_context& IOContext, int ServerPort)
		: mUDPSocket(IOContext, udp::endpoint(udp::v4(),ServerPort))
	{
	
	}

	virtual void StartSession() {}
};
