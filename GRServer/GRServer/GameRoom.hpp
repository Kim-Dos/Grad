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

    boost::asio::steady_timer syncTimer_{ strand_ };  // strand 위에서 동작
    void DoPositionSync();

    // POD 패킷 → 송신용 버퍼 복사 헬퍼
    template<typename PacketT>
    static std::shared_ptr<std::vector<unsigned char>> Pack(const PacketT& p)
    {
        auto src = reinterpret_cast<const unsigned char*>(&p);
        return std::make_shared<std::vector<unsigned char>>(src, src + p.size);
    }

    bool gameStarted_ = false;

    // 시작 유닛 등록 + SCGameStart 페이로드 생성 (strand 위에서 호출)
    void SpawnStartingUnits(int playerNumber);
    void TryStartGame();

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

    // CS_* 패킷 핸들러 (세션이 room strand로 post해서 호출)
    void HandleMoveRequest(int playerNumber, CSMoveObjRequest   req);
    void HandleMultiMove(int playerNumber, CSMoveMultiRequest req);
    void HandleStopRequest(int playerNumber, CSStopObjRequest   req);

    // 전원 브로드캐스트 (요청자 포함)
    void BroadcastToAll(std::shared_ptr<std::vector<unsigned char>> data);

    // 특정 플레이어에게만 전송 (거부/경고용)
    void SendToPlayer(int playerNumber,
        std::shared_ptr<std::vector<unsigned char>> data);

    // 주기적 위치 동기화 시작 (방 생성 or 게임 시작 시 1회 호출)
    void StartPositionSync();
   
};