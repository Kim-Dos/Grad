#include <boost\unordered\concurrent_flat_map.hpp>
#include <atomic>
#include <boost\asio.hpp>
#include <iostream>
#include "Player.h"
#include "Protocol.h"
#pragma once


using boost::concurrent_flat_map;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

class UDPGameSession;
class TCPGameSession;

class GameRoom	//게임 내부의 전반적인 처리 및 게임 오브젝트들을 관리
	: public std::enable_shared_from_this<GameRoom>
{
private:
	std::string roomCode;
	std::string mapName;
	
	std::shared_ptr<UDPGameSession> UDPSession; // 서버에서 실시간으로 처리되는 것들을 클라이언트로 송신 ( 멀티캐스트를 통해서 모든 클라이언트에게 송신 )
	std::shared_ptr<TCPGameSession> FirstTCPSession; // 클라이언트를 수신받아 서버에서 이벤트 처리			1번 플레이어
	std::shared_ptr<TCPGameSession> SecondTCPSession; // 클라이언트를 수신받아 서버에서 이벤트 처리			2번 플레이어

	concurrent_flat_map<int, std::shared_ptr<GameActor>> GameObjs; // 오브젝트는 인당 200개, 깔려있는 오브젝트 포함하면 1000개 정도

public:
	GameRoom(const std::string& roomCode,const std::string& MapName);

	bool IsDamaged();

	inline void setUDPSocket(udp::socket udpsock) { UDPSession = std::make_shared<UDPGameSession>(std::move(udpsock)); }

	void SetMulticast();
};


