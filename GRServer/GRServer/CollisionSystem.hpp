#pragma once
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include "Protocol.h"

// 서버 측 충돌 감지를 위한 간단한 구조체들
struct ServerBoundingBox {
    FXYZ center;
    FXYZ extents;

    ServerBoundingBox() {
        center.x = center.y = center.z = 0;
        extents.x = extents.y = extents.z = 0.5f;
    }

    ServerBoundingBox(FXYZ c, FXYZ e) : center(c), extents(e) {}

    bool Intersects(const ServerBoundingBox& other) const {
        return (std::abs(center.x - other.center.x) <= (extents.x + other.extents.x)) &&
            (std::abs(center.y - other.center.y) <= (extents.y + other.extents.y)) &&
            (std::abs(center.z - other.center.z) <= (extents.z + other.extents.z));
    }
};

// 서버에서 관리하는 게임 오브젝트 정보
struct ServerGameObject {
    unsigned char objNumber;
    FXYZ position;
    FXYZ destination;
    ServerBoundingBox boundingBox;
    int ownerPlayerNumber;
    bool isStatic; // 정적 오브젝트 (벽, 건물 등)

    ServerGameObject() : objNumber(0), ownerPlayerNumber(0), isStatic(false) {
        position.x = position.y = position.z = 0;
        destination.x = destination.y = destination.z = 0;
    }

    void UpdateBoundingBox() {
        boundingBox.center = position;
    }
};

// 맵의 정적 충돌체 (벽, 장애물 등)
struct StaticCollider {
    FXYZ center;
    FXYZ extents;

    StaticCollider(FXYZ c, FXYZ e) : center(c), extents(e) {}

    ServerBoundingBox GetBoundingBox() const {
        return ServerBoundingBox(center, extents);
    }
};

class ServerCollisionSystem {
public:
    ServerCollisionSystem() {
        InitializeMapColliders();
    }

    // 이동 요청 검증 - 충돌 체크 후 유효한 위치 반환
    bool ValidateMovement(ServerGameObject& obj, const FXYZ& targetPos, FXYZ& validPos) {
        // 임시로 위치 업데이트
        FXYZ originalPos = obj.position;
        obj.position = targetPos;
        obj.UpdateBoundingBox();

        // 정적 충돌체와 충돌 검사
        for (const auto& collider : staticColliders) {
            if (obj.boundingBox.Intersects(collider.GetBoundingBox())) {
                // 충돌 발생 - 가장 가까운 유효한 위치 계산
                validPos = GetNearestValidPosition(originalPos, targetPos, collider);
                obj.position = originalPos;
                obj.UpdateBoundingBox();
                return false;
            }
        }

        // 다른 동적 오브젝트와 충돌 검사
        for (const auto& other : dynamicObjects) {
            if (other.objNumber != obj.objNumber &&
                other.ownerPlayerNumber != obj.ownerPlayerNumber) {

                if (obj.boundingBox.Intersects(other.boundingBox)) {
                    // 충돌 발생
                    validPos = GetNearestValidPosition(originalPos, targetPos, other);
                    obj.position = originalPos;
                    obj.UpdateBoundingBox();
                    return false;
                }
            }
        }

        // 충돌 없음 - 요청한 위치가 유효함
        validPos = targetPos;
        return true;
    }

    // 여러 오브젝트의 이동 검증
    std::vector<bool> ValidateMultipleMovements(
        std::vector<ServerGameObject>& objects,
        const std::vector<FXYZ>& targetPositions,
        std::vector<FXYZ>& validPositions) {

        std::vector<bool> results;
        validPositions.resize(targetPositions.size());

        for (size_t i = 0; i < objects.size(); ++i) {
            FXYZ validPos;
            bool isValid = ValidateMovement(objects[i], targetPositions[i], validPos);
            validPositions[i] = validPos;
            results.push_back(isValid);
        }

        return results;
    }

    // 동적 오브젝트 등록
    void RegisterDynamicObject(const ServerGameObject& obj) {
        dynamicObjects.push_back(obj);
    }

    // 동적 오브젝트 업데이트
    void UpdateDynamicObject(unsigned char objNumber, int playerNumber, const FXYZ& newPos) {
        for (auto& obj : dynamicObjects) {
            if (obj.objNumber == objNumber && obj.ownerPlayerNumber == playerNumber) {
                obj.position = newPos;
                obj.UpdateBoundingBox();
                break;
            }
        }
    }

    // 동적 오브젝트 제거
    void RemoveDynamicObject(unsigned char objNumber, int playerNumber) {
        dynamicObjects.erase(
            std::remove_if(dynamicObjects.begin(), dynamicObjects.end(),
                [objNumber, playerNumber](const ServerGameObject& obj) {
                    return obj.objNumber == objNumber && obj.ownerPlayerNumber == playerNumber;
                }),
            dynamicObjects.end());
    }

    // 모든 동적 오브젝트 가져오기
    const std::vector<ServerGameObject>& GetDynamicObjects() const {
        return dynamicObjects;
    }

private:
    std::vector<StaticCollider> staticColliders;
    std::vector<ServerGameObject> dynamicObjects;

    void InitializeMapColliders() {
        // 맵의 경계
        staticColliders.push_back(StaticCollider({ -100, 0, 0 }, { 1, 50, 100 })); // 왼쪽 벽
        staticColliders.push_back(StaticCollider({ 100, 0, 0 }, { 1, 50, 100 }));  // 오른쪽 벽
        staticColliders.push_back(StaticCollider({ 0, 0, -100 }, { 100, 50, 1 })); // 앞쪽 벽
        staticColliders.push_back(StaticCollider({ 0, 0, 100 }, { 100, 50, 1 }));  // 뒤쪽 벽

        // 예시: 맵 중앙의 장애물
        staticColliders.push_back(StaticCollider({ 0, 0, 0 }, { 5, 10, 5 }));

        // TODO: 실제 맵 데이터에서 로드
    }

    FXYZ GetNearestValidPosition(const FXYZ& from, const FXYZ& to,
        const StaticCollider& collider) {
        // 충돌체를 피한 가장 가까운 위치 계산
        FXYZ direction = {
            to.x - from.x,
            to.y - from.y,
            to.z - from.z
        };

        // 정규화
        float length = std::sqrt(direction.x * direction.x +
            direction.y * direction.y +
            direction.z * direction.z);

        if (length > 0.001f) {
            direction.x /= length;
            direction.y /= length;
            direction.z /= length;
        }

        // 충돌체 경계에서 약간 떨어진 위치 반환
        FXYZ result = from;
        float step = 0.1f;
        float maxDistance = length;

        for (float dist = 0; dist < maxDistance; dist += step) {
            FXYZ testPos = {
                from.x + direction.x * dist,
                from.y + direction.y * dist,
                from.z + direction.z * dist
            };

            ServerBoundingBox testBox(testPos, { 0.5f, 0.5f, 0.5f });
            if (!testBox.Intersects(collider.GetBoundingBox())) {
                result = testPos;
            }
            else {
                break;
            }
        }

        return result;
    }

    FXYZ GetNearestValidPosition(const FXYZ& from, const FXYZ& to,
        const ServerGameObject& obj) {
        // 동적 오브젝트 충돌 시 이전 위치로 복귀
        return from;
    }
};