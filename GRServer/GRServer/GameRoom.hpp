#include <boost\unordered\concurrent_flat_map.hpp>
#include <atomic>
#include <boost\asio.hpp>
#include <iostream>
#include "Player.h"
#include "Protocol.h"
#pragma once


using boost::concurrent_flat_map;
using boost::asio::ip::tcp;

class UDPGameSession;
class TCPGameSession;

class GameRoom
	: public std::enable_shared_from_this<GameRoom>
{
private:
	std::string roomCode;

	std::vector<int> userIDS;
	std::vector<TCPGameSession*> userSessionz;  //�ִ� 4��? 8��?
	std::vector<std::pair<int, UDPGameSession*>> userSessionz;  //�ִ� 4��? 8��?
	std::vector<GameObject*> otherObj; // ������Ʈ�� �δ� 200��, ����ִ� ������Ʈ �����ϸ� 1000�� ����


public:
	GameRoom(unsigned char stageNum);

	void setUserIDS(int& userNumber);

	bool IsDamaged();

};


