#include "GameRoom.hpp"


concurrent_flat_map<std::string, std::shared_ptr<GameRoom>> Rooms;

GameRoom::GameRoom(unsigned char stageNum) : stagenumber(stageNum)
{
	userIDS.reserve(4);
	userSessionz.reserve(4);
}

void GameRoom::setUserIDS(int& userNumber)
{
	userIDS.emplace_back(userNumber);
}

bool GameRoom::IsDamaged()

{
	//얻은 Number를 가지고 Damaged 판정 하기
	return false;
}
