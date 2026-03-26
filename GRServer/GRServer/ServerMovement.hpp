#pragma once
#include <vector>
#include <unordered_map>
#include <chrono>
#include <cmath>
#include <iostream>
#include "Protocol.h"
#include "CollisionSystem.hpp"

// ----------------------------------------------------------
// 서버가 관리하는 오브젝트의 이동 상태
// ----------------------------------------------------------
struct ServerMoveState {
    unsigned char objNumber;
    int ownerPlayerNumber;

    FXYZ currentPos;
    FXYZ destination;
    FXYZ direction;         // 정규화된 이동 방향
    float speed;            // 현재 이동 속도
    bool isMoving;

    // 핵 방지용 타임스탬프
    std::chrono::steady_clock::time_point lastUpdateTime;
    FXYZ lastValidatedPos;  // 마지막으로 검증된 위치

    ServerMoveState() : objNumber(0), ownerPlayerNumber(0),
        speed(0.0f), isMoving(false) {
        currentPos = destination = direction = lastValidatedPos = { 0, 0, 0 };
        lastUpdateTime = std::chrono::steady_clock::now();
    }
};

// ----------------------------------------------------------
// 플레이어별 핵 감지 상태
// ----------------------------------------------------------
struct PlayerHackState {
    int warningCount;
    int suspiciousCount;        // 연속 의심 횟수
    std::chrono::steady_clock::time_point lastWarningTime;

    PlayerHackState() : warningCount(0), suspiciousCount(0) {
        lastWarningTime = std::chrono::steady_clock::now();
    }
};

// ----------------------------------------------------------
// 핵 검증 결과
// ----------------------------------------------------------
enum class MoveValidation {
    VALID,              // 정상
    COLLISION,          // 충돌로 인한 보정
    SPEED_HACK,         // 속도핵 감지
    NO_AUTHORITY,       // 권한 없음 (남의 오브젝트)
    OUT_OF_BOUNDS,      // 맵 밖
};

// ----------------------------------------------------------
// ServerMovementSystem
// ----------------------------------------------------------
class ServerMovementSystem {
public:
    ServerMovementSystem() {}

    // ----------------------------------------------------------
    // 이동 요청 처리 (핵심 함수)
    // ----------------------------------------------------------
    // 반환: MoveValidation 결과
    // outPos: 서버가 확정한 목적지 (보정된 위치일 수 있음)
    MoveValidation ProcessMoveRequest(
        unsigned char playerNumber,
        unsigned char objNumber,
        const FXYZ& requestedDest,
        FXYZ& outValidDest)
    {
        // 1) 오브젝트 존재 확인
        auto it = moveStates.find(objNumber);
        if (it == moveStates.end()) {
            std::cout << "[Move] Unknown obj: " << (int)objNumber << std::endl;
            outValidDest = { 0, 0, 0 };
            return MoveValidation::NO_AUTHORITY;
        }

        ServerMoveState& state = it->second;

        // 2) 권한 검사 - 자기 오브젝트만 조작 가능
        if (state.ownerPlayerNumber != playerNumber) {
            std::cout << "[Hack?] Player " << (int)playerNumber
                << " tried to move obj owned by "
                << state.ownerPlayerNumber << std::endl;
            outValidDest = state.currentPos;
            AddSuspicion(playerNumber);
            return MoveValidation::NO_AUTHORITY;
        }

        // 3) 맵 경계 검사
        if (!IsInMapBounds(requestedDest)) {
            // 맵 안쪽으로 클램프
            outValidDest = ClampToMapBounds(requestedDest);
            std::cout << "[Move] Dest clamped to map bounds" << std::endl;
            // 이건 핵이 아니라 UI 문제일 수 있으므로 경고 안 줌
        }
        else {
            outValidDest = requestedDest;
        }

        // 4) 거리 기반 속도핵 검사
        //    "이전 확정 위치에서 요청 목적지까지 순간이동하려는 건 아닌지" 체크
        //    (클라이언트가 위치를 속여서 보내는 경우 방지)
        //    주의: 여기서는 목적지 자체가 아니라, 현재 위치 조작을 검사
        //    실제 이동은 서버가 시뮬레이션하므로 목적지는 자유롭게 설정 가능

        // 5) 충돌 검사 (기존 CollisionSystem 활용)
        ServerGameObject tempObj;
        tempObj.objNumber = objNumber;
        tempObj.position = state.currentPos;
        tempObj.boundingBox.extents = state.currentPos; // 기존 BB 사용
        tempObj.UpdateBoundingBox();

        FXYZ validPos;
        bool noCollision = collisionSystem.ValidateMovement(
            tempObj, outValidDest, validPos);

        if (!noCollision) {
            outValidDest = validPos;  // 충돌 시 보정된 위치 사용
            // 충돌은 정상적인 게임플레이이므로 핵 경고 아님
        }

        // 6) 이동 상태 업데이트
        state.destination = outValidDest;
        state.isMoving = true;
        state.speed = MAX_MOVE_SPEED;  // 서버가 속도를 결정

        // 방향 계산
        float dx = outValidDest.x - state.currentPos.x;
        float dy = outValidDest.y - state.currentPos.y;
        float dz = outValidDest.z - state.currentPos.z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (dist > 0.01f) {
            state.direction = { dx / dist, dy / dist, dz / dist };
        }

        state.lastUpdateTime = std::chrono::steady_clock::now();
        state.lastValidatedPos = state.currentPos;

        return noCollision ? MoveValidation::VALID : MoveValidation::COLLISION;
    }

    // ----------------------------------------------------------
    // 서버 틱 업데이트 (매 프레임 호출)
    // ----------------------------------------------------------
    // 서버가 직접 오브젝트를 이동시킨다
    void UpdateMovements(float deltaTime)
    {
        for (auto& [objNum, state] : moveStates) {
            if (!state.isMoving) continue;

            float dx = state.destination.x - state.currentPos.x;
            float dy = state.destination.y - state.currentPos.y;
            float dz = state.destination.z - state.currentPos.z;
            float distRemaining = std::sqrt(dx * dx + dy * dy + dz * dz);

            float moveAmount = state.speed * deltaTime;

            if (distRemaining <= moveAmount) {
                // 목적지 도착
                state.currentPos = state.destination;
                state.isMoving = false;
                state.speed = 0.0f;
                state.direction = { 0, 0, 0 };
            }
            else {
                // 이동 중 - 서버가 위치를 갱신
                state.currentPos.x += state.direction.x * moveAmount;
                state.currentPos.y += state.direction.y * moveAmount;
                state.currentPos.z += state.direction.z * moveAmount;
            }

            // 이동 중 충돌 재검사 (옵션: 성능과 정확도 트레이드오프)
            ServerGameObject tempObj;
            tempObj.objNumber = objNum;
            tempObj.position = state.currentPos;
            tempObj.UpdateBoundingBox();

            FXYZ validPos;
            if (!collisionSystem.ValidateMovement(tempObj, state.currentPos, validPos)) {
                state.currentPos = validPos;
                state.isMoving = false;  // 충돌하면 정지
            }

            state.lastUpdateTime = std::chrono::steady_clock::now();
        }
    }

    // ----------------------------------------------------------
    // 위치 동기화 패킷 생성 (주기적으로 호출)
    // ----------------------------------------------------------
    SCPositionSync BuildSyncPacket() const
    {
        SCPositionSync pkt;
        pkt.objCount = 0;

        for (const auto& [objNum, state] : moveStates) {
            if (pkt.objCount >= MAX_SYNC_OBJECTS) break;

            auto& entry = pkt.entries[pkt.objCount];
            entry.objNumber = state.objNumber;
            entry.ownerPlayer = static_cast<unsigned char>(state.ownerPlayerNumber);
            entry.position = state.currentPos;
            entry.direction = state.direction;
            pkt.objCount++;
        }

        return pkt;
    }

    // ----------------------------------------------------------
    // 오브젝트 등록/제거
    // ----------------------------------------------------------
    void RegisterObject(unsigned char objNumber, int ownerPlayer,
        const FXYZ& initialPos, const FXYZ& bbExtents)
    {
        ServerMoveState state;
        state.objNumber = objNumber;
        state.ownerPlayerNumber = ownerPlayer;
        state.currentPos = initialPos;
        state.lastValidatedPos = initialPos;
        state.destination = initialPos;
        moveStates[objNumber] = state;

        // CollisionSystem에도 등록
        ServerGameObject obj;
        obj.objNumber = objNumber;
        obj.ownerPlayerNumber = ownerPlayer;
        obj.position = initialPos;
        obj.boundingBox = ServerBoundingBox(initialPos, bbExtents);
        collisionSystem.RegisterDynamicObject(obj);
    }

    void RemoveObject(unsigned char objNumber)
    {
        moveStates.erase(objNumber);
    }

    // ----------------------------------------------------------
    // 핵 감지 관련
    // ----------------------------------------------------------

    // 클라이언트가 보고한 현재 위치와 서버 위치 비교 (위치 조작 감지)
    MoveValidation ValidateClientPosition(
        unsigned char playerNumber,
        unsigned char objNumber,
        const FXYZ& clientReportedPos)
    {
        auto it = moveStates.find(objNumber);
        if (it == moveStates.end()) return MoveValidation::NO_AUTHORITY;

        const ServerMoveState& state = it->second;

        // 서버 위치와 클라이언트 보고 위치의 차이
        float dx = state.currentPos.x - clientReportedPos.x;
        float dy = state.currentPos.y - clientReportedPos.y;
        float dz = state.currentPos.z - clientReportedPos.z;
        float discrepancy = std::sqrt(dx * dx + dy * dy + dz * dz);

        // 네트워크 지연 고려하여 어느 정도 차이는 허용
        // MAX_MOVE_SPEED * 왕복지연시간(예: 200ms) * 여유배수
        float allowedDiscrepancy = MAX_MOVE_SPEED * 0.2f * SPEED_HACK_TOLERANCE;

        if (discrepancy > allowedDiscrepancy) {
            std::cout << "[Hack?] Position discrepancy: " << discrepancy
                << " (allowed: " << allowedDiscrepancy << ")" << std::endl;
            AddSuspicion(playerNumber);
            return MoveValidation::SPEED_HACK;
        }

        return MoveValidation::VALID;
    }

    // 핵 경고 횟수 조회
    int GetWarningCount(unsigned char playerNumber) const
    {
        auto it = hackStates.find(playerNumber);
        if (it == hackStates.end()) return 0;
        return it->second.warningCount;
    }

    // 킥 필요 여부
    bool ShouldKickPlayer(unsigned char playerNumber) const
    {
        return GetWarningCount(playerNumber) >= MAX_HACK_WARNINGS;
    }

    // 특정 오브젝트의 현재 서버 위치 조회
    FXYZ GetServerPosition(unsigned char objNumber) const
    {
        auto it = moveStates.find(objNumber);
        if (it != moveStates.end()) return it->second.currentPos;
        return { 0, 0, 0 };
    }

    bool IsObjectMoving(unsigned char objNumber) const
    {
        auto it = moveStates.find(objNumber);
        if (it != moveStates.end()) return it->second.isMoving;
        return false;
    }

private:
    std::unordered_map<unsigned char, ServerMoveState> moveStates;
    std::unordered_map<unsigned char, PlayerHackState> hackStates;
    ServerCollisionSystem collisionSystem;

    bool IsInMapBounds(const FXYZ& pos) const {
        return pos.x >= -MAP_BOUNDARY && pos.x <= MAP_BOUNDARY &&
            pos.z >= -MAP_BOUNDARY && pos.z <= MAP_BOUNDARY;
    }

    FXYZ ClampToMapBounds(const FXYZ& pos) const {
        FXYZ clamped = pos;
        clamped.x = std::max(-MAP_BOUNDARY, std::min(MAP_BOUNDARY, clamped.x));
        clamped.z = std::max(-MAP_BOUNDARY, std::min(MAP_BOUNDARY, clamped.z));
        return clamped;
    }

    void AddSuspicion(unsigned char playerNumber) {
        auto& hs = hackStates[playerNumber];
        hs.suspiciousCount++;

        // 연속 3회 의심이면 경고 1회
        if (hs.suspiciousCount >= 3) {
            hs.warningCount++;
            hs.suspiciousCount = 0;
            hs.lastWarningTime = std::chrono::steady_clock::now();

            std::cout << "[AntiCheat] Player " << (int)playerNumber
                << " warning: " << hs.warningCount
                << "/" << MAX_HACK_WARNINGS << std::endl;
        }
    }
};