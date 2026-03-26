#pragma once

#include <boost\unordered\concurrent_flat_map.hpp>
#include <atomic>
#include <boost\asio.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
//#include "UDPGameServer.hpp"
#include "Player.h"
#include "Protocol.h"
#include "CollisionSystem.hpp"
#include "ServerMovement.hpp"




using boost::concurrent_flat_map;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class UDPGameSession;
class TCPGameSession;

class GameRoom : public std::enable_shared_from_this<GameRoom>
{
private:
    std::string roomCode;
    std::string mapName;

    boost::asio::strand<boost::asio::io_context::executor_type> strand_;

    // 세션을 private 배열로 관리
    std::array<std::shared_ptr<TCPGameSession>, 2> sessions_;
    //std::shared_ptr<UDPGameSession> UDPSession;

    concurrent_flat_map<int, std::shared_ptr<GameActor>> GameObjs;
    std::unique_ptr<ServerCollisionSystem> collisionSystem;
    ServerMovementSystem movementSystem;
    float syncTimer = 0.0f;

public:
    GameRoom(boost::asio::io_context& ioc, const std::string& roomCode, const std::string& MapName);

    // 세션 관리 인터페이스
    void AddSession(std::shared_ptr<TCPGameSession> session);
    void RemoveSession(int playerNumber);
    std::shared_ptr<TCPGameSession> GetSession(int playerNumber) const;
    bool IsFull() const;

    auto get_executor() { return strand_; }

    // Broadcast를 GameRoom이 담당
    void BroadcastToOthers(int senderNumber, std::shared_ptr<std::vector<unsigned char>> data);
    void BroadcastToAll(unsigned char* packetData);

    // 기존 기능
    void CheckCollisions();
    void RegisterPlayerObjects(int playerNumber, const std::vector<ServerGameObject>& objects);
    bool ValidateAndProcessMovement(int playerNumber,
        const std::vector<MoveData>& moveRequests,
        std::vector<MoveData>& validatedMoves);
    ServerCollisionSystem* GetCollisionSystem() { return collisionSystem.get(); }
    void SetMulticast();

    //inline void setUDPSocket(udp::socket udpsock) {
    //    UDPSession = std::make_shared<UDPGameSession>(std::move(udpsock));
    //}

   
};