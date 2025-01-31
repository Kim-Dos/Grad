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
	//���� Number�� ������ Damaged ���� �ϱ�
	return false;
}

void GameRoom::SetMulticast()
{
	//��Ƽĳ��Ʈ ����

	//1������ UDP �׷� ����
	boost::asio::ip::address multicast_address = FirstTCPSession->getSocket().local_endpoint().address();
	UDPSession->JoinGroup(multicast_address);

	//2�� ���� UDP �׷� ����
	multicast_address = SecondTCPSession->getSocket().local_endpoint().address();
	UDPSession->JoinGroup(multicast_address);

}
