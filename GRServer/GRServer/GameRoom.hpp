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
	std::vector<TCPGameSession*> userSessionz;  //최대 4명? 8명?
	std::vector<std::pair<int, UDPGameSession*>> userSessionz;  //최대 4명? 8명?
	std::vector<GameObject*> otherObj; // 오브젝트는 인당 200개, 깔려있는 오브젝트 포함하면 1000개 정도


public:
	GameRoom(unsigned char stageNum);

	void setUserIDS(int& userNumber);

	bool IsDamaged();

};


