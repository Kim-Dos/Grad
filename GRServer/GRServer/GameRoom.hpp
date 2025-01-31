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

class GameRoom	//���� ������ �������� ó�� �� ���� ������Ʈ���� ����
	: public std::enable_shared_from_this<GameRoom>
{
private:
	std::string roomCode;
	std::string mapName;
	
	std::shared_ptr<UDPGameSession> UDPSession; // �������� �ǽð����� ó���Ǵ� �͵��� Ŭ���̾�Ʈ�� �۽� ( ��Ƽĳ��Ʈ�� ���ؼ� ��� Ŭ���̾�Ʈ���� �۽� )
	std::shared_ptr<TCPGameSession> FirstTCPSession; // Ŭ���̾�Ʈ�� ���Ź޾� �������� �̺�Ʈ ó��			1�� �÷��̾�
	std::shared_ptr<TCPGameSession> SecondTCPSession; // Ŭ���̾�Ʈ�� ���Ź޾� �������� �̺�Ʈ ó��			2�� �÷��̾�

	concurrent_flat_map<int, std::shared_ptr<GameActor>> GameObjs; // ������Ʈ�� �δ� 200��, ����ִ� ������Ʈ �����ϸ� 1000�� ����

public:
	GameRoom(const std::string& roomCode,const std::string& MapName);

	bool IsDamaged();

	inline void setUDPSocket(udp::socket udpsock) { UDPSession = std::make_shared<UDPGameSession>(std::move(udpsock)); }

	void SetMulticast();
};


