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
	//���� Number�� ������ Damaged ���� �ϱ�
	return false;
}
