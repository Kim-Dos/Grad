#include "GameRoom.hpp"
#include "TCPGameServer.hpp"


extern concurrent_flat_map<std::string, std::shared_ptr<GameRoom>> Rooms;

constexpr int   START_UNITS_PER_PLAYER = 4;
constexpr float SPAWN_X[2] = { -60.0f, 60.0f };  // P1 왼쪽, P2 오른쪽
constexpr float SPAWN_Z_GAP = 20.0f;

void GameRoom::SpawnStartingUnits(int playerNumber)
{
	std::vector<ServerGameObject> units;
	units.reserve(START_UNITS_PER_PLAYER);

	for (int i = 0; i < START_UNITS_PER_PLAYER; ++i) {
		ServerGameObject obj;
		obj.objNumber = (unsigned char)i;
		obj.ownerPlayerNumber = playerNumber;
		obj.position = {
			SPAWN_X[playerNumber - 1],
			0.0f,
			(i - (START_UNITS_PER_PLAYER - 1) * 0.5f) * SPAWN_Z_GAP
		};
		obj.destination = obj.position;
		obj.UpdateBoundingBox();
		units.push_back(obj);
	}

	RegisterPlayerObjects(playerNumber, units);
}

void GameRoom::TryStartGame()
{
	if (gameStarted_) return;
	if (!sessions_[0] || !sessions_[1]) return;

	gameStarted_ = true;

	SCGameStart start;
	const auto& objs = collisionSystem->GetDynamicObjects();
	for (const auto& obj : objs) {
		if (start.unitCount >= MAX_TOTAL_START_UNITS) break;
		auto& u = start.units[start.unitCount++];
		u.objNumber = obj.objNumber;
		u.ownerPlayer = (unsigned char)obj.ownerPlayerNumber;
		u.objType = (unsigned char)ObjType::Knight;   // TODO: 유닛 타입 관리
		u.position = obj.position;
	}

	BroadcastToAll(Pack(start));
	DoPositionSync();   // 이미 strand 위 → 직접 호출로 동기화 루프 시작

	std::cout << "[Room " << roomCode << "] GAME START ("
		<< (int)start.unitCount << " units)\n";
}

GameRoom::GameRoom(boost::asio::io_context& ioc, const std::string& roomCode, const std::string& MapName) : strand_(boost::asio::make_strand(ioc)), roomCode(roomCode), mapName(MapName)
{
	collisionSystem = std::make_unique<ServerCollisionSystem>();
}

void GameRoom::AddSession(std::shared_ptr<TCPGameSession> session)
{
	boost::asio::post(strand_, [this, self = shared_from_this(), session]() {
		for (int i = 0; i < 2; ++i) {
			if (!sessions_[i]) {
				int playerNumber = i + 1;

				sessions_[i] = session;
				session->SetRoomCode(roomCode);
				session->SetPlayerNumber(playerNumber);
				session->SetRoom(self);

				SpawnStartingUnits(playerNumber);

				// 입장 결과 회신
				SCLinkResult res;
				res.playerNumber = (unsigned char)playerNumber;
				session->QueueSend(Pack(res));

				std::cout << "[Room " << roomCode << "] player "
					<< playerNumber << " linked\n";

				TryStartGame();
				return;
			}
		}

		// 방 꽉참 → 실패 회신 (playerNumber = 0)
		SCLinkResult fail;
		session->QueueSend(Pack(fail));
		});
}

std::shared_ptr<TCPGameSession> GameRoom::GetSession(int playerNumber) const
{
	return sessions_[playerNumber];
}

void GameRoom::BroadcastToOthers(int senderNumber, std::shared_ptr<std::vector<unsigned char>> data)
{
	boost::asio::post(strand_, [this, self = shared_from_this(), senderNumber, data]() {
		for (auto& session : sessions_) {
			if (session && session->GetPlayerNumber() != senderNumber) {
				session->QueueSend(data);
			}
		}
	});
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
		obj.position = { move.pos_x, 0, move.pos_z };
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
		validMove.pos_z = validPositions[i].z;
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


//-----------------------------------------------------------------
// 브로드캐스트 헬퍼
//-----------------------------------------------------------------
void GameRoom::BroadcastToAll(std::shared_ptr<std::vector<unsigned char>> data)
{
	boost::asio::post(strand_, [this, self = shared_from_this(), data]() {
		for (auto& session : sessions_) {
			if (session) session->QueueSend(data);
		}
		});
}

void GameRoom::SendToPlayer(int playerNumber,
	std::shared_ptr<std::vector<unsigned char>> data)
{
	boost::asio::post(strand_, [this, self = shared_from_this(),
		playerNumber, data]() {
			for (auto& session : sessions_) {
				if (session && session->GetPlayerNumber() == playerNumber) {
					session->QueueSend(data);
					return;
				}
			}
		});
}

//-----------------------------------------------------------------
// 단일 오브젝트 이동 요청
//  - 클라는 목적지만 보냄. 현재 위치는 서버 상태에서 조회 (핵 방지)
//-----------------------------------------------------------------
void GameRoom::HandleMoveRequest(int playerNumber, CSMoveObjRequest req)
{
	// 1) 소유권 검증
	if (req.playerNumber != playerNumber) {
		SCMoveRejected rej;
		rej.objNumber = req.objNumber;
		rej.reason = 2; // no-auth
		SendToPlayer(playerNumber, Pack(rej));
		return;
	}

	// 2) 서버가 아는 현재 위치 조회
	FXYZ curPos{};
	if (!collisionSystem->TryGetObjectPosition(req.objNumber,
		playerNumber, curPos)) {
		// 미등록 오브젝트 → 거부
		SCMoveRejected rej;
		rej.objNumber = req.objNumber;
		rej.reason = 2;
		SendToPlayer(playerNumber, Pack(rej));
		return;
	}

	// 3) 맵 경계 검증
	if (std::abs(req.destination.x) > MAP_BOUNDARY ||
		std::abs(req.destination.z) > MAP_BOUNDARY) {
		SCMoveRejected rej;
		rej.objNumber = req.objNumber;
		rej.correctedPos = curPos;
		rej.reason = 3; // out-of-map
		SendToPlayer(playerNumber, Pack(rej));
		return;
	}

	// 4) 충돌 검증 (기존 ValidateAndProcessMovement 재사용)
	std::vector<MoveData> reqs(1), valid;
	reqs[0].objnumber = req.objNumber;
	reqs[0].pos_x = curPos.x;
	reqs[0].pos_z = curPos.z;
	reqs[0].dest_x = req.destination.x;
	reqs[0].dest_z = req.destination.z;

	ValidateAndProcessMovement(playerNumber, reqs, valid);

	// 5) 확정 결과 전원 브로드캐스트 (요청자 포함 — 클라는 이걸 받아야 출발)
	SCMoveObjResult res;
	res.ownerPlayer = (unsigned char)playerNumber;
	res.objNumber = req.objNumber;
	res.currentPos = curPos;
	res.destination = { valid[0].dest_x, 0.f, valid[0].dest_z };
	res.speed = MAX_MOVE_SPEED;

	BroadcastToAll(Pack(res));
}

//-----------------------------------------------------------------
// 다중 오브젝트 이동 요청 (드래그 선택)
//  - 공통 목적지 → 서버가 간단한 그리드 포메이션 오프셋 부여
//-----------------------------------------------------------------
void GameRoom::HandleMultiMove(int playerNumber, CSMoveMultiRequest req)
{
	if (req.playerNumber != playerNumber) return;

	int count = (req.objCount < MAX_MULTI_MOVE) ? req.objCount : MAX_MULTI_MOVE;
	if (count <= 0) return;

	std::vector<MoveData> reqs, valid;
	reqs.reserve(count);

	constexpr float FORMATION_SPACING = 60.0f; // 유닛 간격 (맵 스케일에 맞게)
	const int cols = (int)std::ceil(std::sqrt((float)count));

	for (int i = 0; i < count; ++i) {
		FXYZ curPos{};
		if (!collisionSystem->TryGetObjectPosition(req.objNumbers[i],
			playerNumber, curPos))
			continue;

		// 그리드 포메이션: 목적지 주변에 격자 배치
		int row = i / cols, col = i % cols;
		float offX = (col - (cols - 1) * 0.5f) * FORMATION_SPACING;
		float offZ = (row - (cols - 1) * 0.5f) * FORMATION_SPACING;

		MoveData m;
		m.objnumber = req.objNumbers[i];
		m.pos_x = curPos.x;
		m.pos_z = curPos.z;
		m.dest_x = req.destination.x + offX;
		m.dest_z = req.destination.z + offZ;
		reqs.push_back(m);
	}
	if (reqs.empty()) return;

	ValidateAndProcessMovement(playerNumber, reqs, valid);

	SCMoveMultiResult res;
	res.ownerPlayer = (unsigned char)playerNumber;
	res.objCount = (unsigned char)valid.size();
	for (size_t i = 0; i < valid.size(); ++i) {
		res.objNumbers[i] = valid[i].objnumber;
		res.destinations[i] = { valid[i].dest_x, 0.f, valid[i].dest_z };
	}
	res.speed = MAX_MOVE_SPEED;

	BroadcastToAll(Pack(res));
}

//-----------------------------------------------------------------
// 정지 요청 — 서버 상태의 목적지를 현재 위치로 덮어서 전원 통지
//-----------------------------------------------------------------
void GameRoom::HandleStopRequest(int playerNumber, CSStopObjRequest req)
{
	if (req.playerNumber != playerNumber) return;

	FXYZ curPos{};
	if (!collisionSystem->TryGetObjectPosition(req.objNumber,
		playerNumber, curPos))
		return;

	collisionSystem->UpdateDynamicObject(req.objNumber, playerNumber, curPos);

	SCMoveObjResult res;                 // "현재 위치로 이동" == 정지
	res.ownerPlayer = (unsigned char)playerNumber;
	res.objNumber = req.objNumber;
	res.currentPos = curPos;
	res.destination = curPos;
	res.speed = 0.0f;

	BroadcastToAll(Pack(res));
}

//-----------------------------------------------------------------
// 주기적 위치 동기화 (POSITION_SYNC_INTERVAL = 0.15s)
//  ※ MAX_SYNC_OBJECTS는 반드시 8로 축소돼 있어야 함 (size 1바이트 한계)
//  오브젝트가 8개 초과면 여러 패킷으로 나눠 보낸다.
//-----------------------------------------------------------------
void GameRoom::StartPositionSync()
{
	boost::asio::post(strand_, [this, self = shared_from_this()]() {
		DoPositionSync();
		});
}

void GameRoom::DoPositionSync()
{
	// strand 위에서 실행 중이라는 전제
	std::vector<SCSyncEntry> entries;
	collisionSystem->CollectAllPositions(entries);   // [3] 헬퍼 참고

	for (size_t base = 0; base < entries.size(); base += MAX_SYNC_OBJECTS) {
		SCPositionSync sync;
		size_t n = std::min<size_t>(MAX_SYNC_OBJECTS, entries.size() - base);
		sync.objCount = (unsigned char)n;
		for (size_t i = 0; i < n; ++i)
			sync.entries[i] = entries[base + i];

		BroadcastToAll(Pack(sync));
	}

	syncTimer_.expires_after(std::chrono::milliseconds(
		(int)(POSITION_SYNC_INTERVAL * 1000)));
	syncTimer_.async_wait(
		boost::asio::bind_executor(strand_,
			[this, self = shared_from_this()](boost::system::error_code ec)
			{
				if (!ec) DoPositionSync();
			}));
}