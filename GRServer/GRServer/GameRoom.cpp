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
	//얻은 Number를 가지고 Damaged 판정 하기
	return false;
}

void GameRoom::SetMulticast()
{
	//멀티캐스트 설정

	//1번유저 UDP 그룹 조인
	boost::asio::ip::address multicast_address = FirstTCPSession->getSocket().local_endpoint().address();
	UDPSession->JoinGroup(multicast_address);

	//2번 유저 UDP 그룹 조인
	multicast_address = SecondTCPSession->getSocket().local_endpoint().address();
	UDPSession->JoinGroup(multicast_address);

}
