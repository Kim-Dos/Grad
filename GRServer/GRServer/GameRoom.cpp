#include "GameRoom.hpp"
#include "TCPGameServer.hpp"
#include "UDPGameServer.hpp"


extern concurrent_flat_map<std::string, std::shared_ptr<GameRoom>> Rooms;

GameRoom::GameRoom(const std::string& roomCode, const std::string& MapName)
{
	this->roomCode = roomCode;
	this->mapName = MapName;
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

// 간단한 충돌 검사 함수
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