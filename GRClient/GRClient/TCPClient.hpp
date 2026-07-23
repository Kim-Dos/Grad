#pragma once

#include <iostream>
#include <deque>
#include <vector>
#include <memory>
#include <atomic>
#include <boost/asio.hpp>
#include "..\..\..\Grad\GRServer\GRServer\Protocol.h"
#include "..\..\..\ppo\NetworkBridge.h"   // 경로는 실제 위치에 맞게 조정

using boost::asio::ip::tcp;

//-----------------------------------------------------------------
// TCPC : 게임서버용 TCP 클라이언트
//
//  - 서버의 TCPGameSession과 대칭 구조 (strand + writeQueue)
//  - 첫 바이트 = 패킷 전체 길이(size 필드 포함) 프레이밍
//  - 수신: 소켓 → 패킷 재조립 → bridge.EnqueueRecv()
//  - 송신: 메인스레드가 bridge.EnqueueSend() → IO스레드의
//          펌프 타이머가 DequeueSendAll() → async_write
//
//  spsc_queue 제약 준수:
//    RecvQ : 생산 = IO스레드(recv핸들러), 소비 = 메인스레드
//    SendQ : 생산 = 메인스레드,           소비 = IO스레드(펌프)
//-----------------------------------------------------------------
class TCPC {
public:
	TCPC(boost::asio::io_context& ioc, NetworkBridge& bridge) noexcept;
	~TCPC() { std::cout << "TCPC out\n"; }

	// 접속 시도(비동기). 성공 시 recv 루프 + 송신 펌프 자동 시작.
	void Connect(const char* ip, unsigned short port);

	bool IsConnected() const { return connected_.load(); }

	// 브리지를 거치지 않는 즉시 전송 (초기 CGLinkInfo 등 특수 용도)
	// 어느 스레드에서든 호출 가능. packet[0] == 전체 길이 규약.
	void SendPacketDirect(const void* packet);

private:
	void recv();
	void pumpSendQueue();   // SendQ 폴링 (IO스레드, 펌프 타이머)
	void doWrite();

	tcp::socket mTCPSocket;
	boost::asio::strand<boost::asio::io_context::executor_type> strand_;
	boost::asio::steady_timer sendTimer_;
	NetworkBridge& bridge_;

	std::deque<std::shared_ptr<std::vector<unsigned char>>> writeQueue_;
	bool writing_ = false;

	// 패킷 재조립 상태 (서버 세션과 동일한 방식)
	unsigned char recvBuffer[MAXSIZE];   // 소켓에서 읽은 원본
	unsigned char PacketData[MAXSIZE];   // 조립 중인 패킷
	int prevDataSize = 0;                // 지금까지 조립된 바이트 수
	int curDataSize = 0;                 // 조립 중인 패킷의 전체 길이

	std::atomic<bool> connected_{ false };

	static constexpr int SEND_PUMP_INTERVAL_MS = 5;
};