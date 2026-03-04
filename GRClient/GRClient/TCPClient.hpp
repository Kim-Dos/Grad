#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <conio.h>
#include "UDPClient.hpp"
#include "../../../Grad/GRServer/GRServer/Protocol.h"

class NetworkBridge;  // forward declaration

using boost::asio::ip::tcp;

class TCPC
{
public:
    // ★ bridge 포인터 추가
    TCPC(boost::asio::io_context& service, NetworkBridge* bridge) noexcept;
    ~TCPC() { std::cout << "TCPC out\n"; }

    void recv(unsigned char(&arr)[1024]);

    void SendMovement(const std::vector<MoveData>& moveData);

    void SendRaw(const void* packet, size_t length) {
        Packetsend(packet, length);
    }

private:
    boost::asio::basic_stream_socket<tcp, boost::asio::io_context::executor_type> msocket;
    unsigned char recvBuffer[1024];
    unsigned char PacketData[1024];
    int prevDataSize;
    int curDataSize;

    int myPlayerNumber = 0;

    NetworkBridge* mBridge = nullptr;  // ★ 추가

    void Packetsend(const void* packet, size_t size);
    void ClientPacketProcess();
    void Character_Positioning();
    void traceKey();

    void UpdateOtherPlayerObject(int playerNumber, const MoveData& moveData);
    void CorrectMyObjectPosition(const MoveData& correctedData);
};