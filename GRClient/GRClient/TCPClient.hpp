#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <conio.h>
#include "UDPClient.hpp"
#include "../../../Grad/GRServer/GRServer/Protocol.h"

using boost::asio::ip::tcp;

class TCPC
{
public:
    TCPC(boost::asio::io_context& service) noexcept;
    ~TCPC() { std::cout << "TCPC out\n"; }

    void recv(unsigned char(&arr)[1024]);

    void SendMovement(const std::vector<MoveData>& moveData);

private:
    tcp::socket msocket;
    unsigned char recvBuffer[1024];
    unsigned char PacketData[1024];
    int prevDataSize;
    int curDataSize;

    int myPlayerNumber = 0;

    void Packetsend(void* packet);
    void ClientPacketProcess();
    void Character_Positioning();
    void traceKey();

    // 새로 추가된 함수들
    void UpdateOtherPlayerObject(int playerNumber, const MoveData& moveData);
    void CorrectMyObjectPosition(const MoveData& correctedData);
};
