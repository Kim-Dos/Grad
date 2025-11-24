#include "GameRoom.hpp"
#include "TCPGameServer.hpp"
#include "UDPGameServer.hpp"


extern concurrent_flat_map<std::string, std::shared_ptr<GameRoom>> Rooms;

GameRoom::GameRoom(const std::string& roomCode, const std::string& MapName)
{
	this->roomCode = roomCode;
	this->mapName = MapName;

	// 충돌 시스템 초기화
	collisionSystem = std::make_unique<ServerCollisionSystem>();

	std::cout << "GameRoom created with collision system: " << roomCode << std::endl;
}


bool GameRoom::IsDamaged()
{
	// Damage Detection 
	return false;
}

void GameRoom::SetMulticast()
{
	// Set Multicast Groups for UDP Session

	// User 1 UDP JOIN
	boost::asio::ip::address multicast_address = FirstTCPSession->getSocket().local_endpoint().address();
	UDPSession->JoinGroup(multicast_address);

	// User 2 UDP JOIN
	multicast_address = SecondTCPSession->getSocket().local_endpoint().address();
	UDPSession->JoinGroup(multicast_address);

}

// 기존 간단한 충돌 검사 함수 (원형 충돌 기반)
void GameRoom::CheckCollisions()
{
	// 모든 오브젝트 쌍에 대해 충돌 검사
	std::vector<std::pair<int, std::shared_ptr<GameActor>>> objects;

	// concurrent_flat_map에서 데이터 복사
	GameObjs.visit_all([&objects](auto& pair) {
		objects.push_back({ pair.first, pair.second });
		});

	// 충돌 검사 (간단한 원형 충돌)
	for (size_t i = 0; i < objects.size(); i++) {
		for (size_t j = i + 1; j < objects.size(); j++) {
			auto& obj1 = objects[i].second;
			auto& obj2 = objects[j].second;

			FXYZ pos1 = obj1->GetPos();
			FXYZ pos2 = obj2->GetPos();

			// 거리 계산
			float dx = pos1.x - pos2.x;
			float dy = pos1.y - pos2.y;
			float dz = pos1.z - pos2.z;
			float distanceSquared = dx * dx + dy * dy + dz * dz;

			// 충돌 반경 (예시: 1.0f)
			const float collisionRadius = 1.0f;
			const float collisionRadiusSquared = collisionRadius * collisionRadius;

			if (distanceSquared < collisionRadiusSquared && distanceSquared > 0.001f) {
				// 충돌 발생!
				// 여기서 충돌 처리 로직 추가 가능
				// 예: 데미지, 밀어내기, 이벤트 발생 등

				std::cout << "Collision detected between object "
					<< objects[i].first << " and " << objects[j].first << std::endl;
			}
		}
	}
}

// 새로운 서버 권위 충돌 시스템 함수들
void GameRoom::RegisterPlayerObjects(int playerNumber, const std::vector<ServerGameObject>& objects)
{
	for (const auto& obj : objects) {
		collisionSystem->RegisterDynamicObject(obj);
	}
	std::cout << "Registered " << objects.size() << " objects for player " << playerNumber << std::endl;
}

bool GameRoom::ValidateAndProcessMovement(int playerNumber, const std::vector<MoveData>& moveRequests,
	std::vector<MoveData>& validatedMoves)
{
	std::vector<ServerGameObject> objects;
	std::vector<FXYZ> targetPositions;

	// 이동 요청을 ServerGameObject로 변환
	for (const auto& move : moveRequests) {
		ServerGameObject obj;
		obj.objNumber = move.objnumber;
		obj.ownerPlayerNumber = playerNumber;
		obj.position = { move.pos_x, 0, move.pos_y };
		obj.destination = { move.dest_x, 0, move.dest_z };
		obj.UpdateBoundingBox();

		objects.push_back(obj);
		targetPositions.push_back({ move.dest_x, 0, move.dest_z });
	}

	// 충돌 검증
	std::vector<FXYZ> validPositions;
	auto validationResults = collisionSystem->ValidateMultipleMovements(
		objects, targetPositions, validPositions);

	// 검증된 위치로 MoveData 생성
	validatedMoves.clear();
	bool allValid = true;

	for (size_t i = 0; i < moveRequests.size(); ++i) {
		MoveData validMove;
		validMove.objnumber = moveRequests[i].objnumber;
		validMove.pos_x = validPositions[i].x;
		validMove.pos_y = validPositions[i].z;
		validMove.dest_x = validPositions[i].x;
		validMove.dest_z = validPositions[i].z;

		validatedMoves.push_back(validMove);

		// 하나라도 보정되었다면 false
		if (!validationResults[i]) {
			allValid = false;
			std::cout << "  [COLLISION] Object " << (int)validMove.objnumber
				<< " corrected from (" << moveRequests[i].dest_x << ", " << moveRequests[i].dest_z << ")"
				<< " to (" << validMove.dest_x << ", " << validMove.dest_z << ")" << std::endl;
		}

		// 서버의 동적 오브젝트 위치 업데이트
		collisionSystem->UpdateDynamicObject(
			validMove.objnumber, playerNumber, validPositions[i]);
	}

	return allValid;
}
