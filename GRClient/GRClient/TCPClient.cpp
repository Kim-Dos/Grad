#include "TCPClient.hpp"

TCPC::TCPC(boost::asio::io_context& ioc, NetworkBridge& bridge) noexcept
	: mTCPSocket(ioc)
	, strand_(boost::asio::make_strand(ioc))
	, sendTimer_(ioc)
	, bridge_(bridge)
{
	ZeroMemory(recvBuffer, MAXSIZE);
	ZeroMemory(PacketData, MAXSIZE);
}

void TCPC::Connect(const char* ip, unsigned short port)
{
	tcp::endpoint serverEP(boost::asio::ip::make_address(ip), port);

	mTCPSocket.async_connect(serverEP,
		boost::asio::bind_executor(strand_,
			[this](boost::system::error_code ec)
			{
				if (ec) {
					std::cout << "[TCPC] connect fail: " << ec.message() << std::endl;
					connected_ = false;
					return;
				}
				std::cout << "[TCPC] connected\n";
				mTCPSocket.set_option(tcp::no_delay(true)); // 실시간성: Nagle off
				connected_ = true;

				recv();            // 수신 루프 시작
				pumpSendQueue();   // SendQ 펌프 시작
			}));
}

//-----------------------------------------------------------------
// 수신: async_read_some → 길이 프리픽스 기준 패킷 조립 →
//       완성될 때마다 bridge_.EnqueueRecv() → 다시 recv()
//-----------------------------------------------------------------
void TCPC::recv()
{
	mTCPSocket.async_read_some(boost::asio::buffer(recvBuffer, MAXSIZE),
		boost::asio::bind_executor(strand_,
			[this](boost::system::error_code ec, std::size_t length)
			{
				if (ec) {
					std::cout << "[TCPC] recv err: " << ec.message() << std::endl;
					connected_ = false;
					return; // TODO: 재접속 로직
				}

				int remain = static_cast<int>(length);
				unsigned char* p = recvBuffer;

				while (remain > 0) {
					// 새 패킷 시작: 첫 바이트가 전체 길이
					if (curDataSize == 0) {
						curDataSize = p[0];
						// 방어: 최소 size+type 2바이트,
						//       PacketBuffer::MAX_PACKET(256) 초과 금지
						if (curDataSize < 2 ||
							curDataSize >(int)PacketBuffer::MAX_PACKET) {
							std::cout << "[TCPC] bad packet size: "
								<< curDataSize << std::endl;
							connected_ = false;
							return;
						}
					}

					int need = curDataSize - prevDataSize; // 이 패킷에 더 필요한 양
					int copy = (need < remain) ? need : remain;

					memcpy(PacketData + prevDataSize, p, copy);
					prevDataSize += copy;
					p += copy;
					remain -= copy;

					// 패킷 하나 완성 → 메인스레드용 RecvQ로
					if (prevDataSize == curDataSize) {
						bridge_.EnqueueRecv(PacketData, curDataSize);
						prevDataSize = 0;
						curDataSize = 0;
					}
				}

				recv();
			}));
}

//-----------------------------------------------------------------
// 송신 펌프: 일정 주기로 SendQ를 비워 writeQueue로 옮긴다.
//  - spsc SendQ의 유일한 소비자 = 이 펌프(IO스레드, strand 위)
//-----------------------------------------------------------------
void TCPC::pumpSendQueue()
{
	if (!connected_) return;

	PacketBuffer pkt;
	while (bridge_.DequeueSend(pkt)) {
		if (pkt.length < 2) continue;

		auto data = std::make_shared<std::vector<unsigned char>>(
			pkt.data, pkt.data + pkt.length);

		writeQueue_.push_back(data);
	}

	if (!writing_ && !writeQueue_.empty()) {
		writing_ = true;
		doWrite();
	}

	// 다음 펌프 예약
	sendTimer_.expires_after(
		std::chrono::milliseconds(SEND_PUMP_INTERVAL_MS));
	sendTimer_.async_wait(
		boost::asio::bind_executor(strand_,
			[this](boost::system::error_code ec)
			{
				if (!ec) pumpSendQueue();
			}));
}

void TCPC::doWrite()
{
	auto& front = writeQueue_.front();

	boost::asio::async_write(mTCPSocket,
		boost::asio::buffer(*front),
		boost::asio::bind_executor(strand_,
			[this](boost::system::error_code ec, std::size_t /*bytes*/)
			{
				if (ec) {
					std::cout << "[TCPC] send err: " << ec.message() << std::endl;
					connected_ = false;
					writing_ = false;
					writeQueue_.clear();
					return;
				}

				writeQueue_.pop_front();
				if (!writeQueue_.empty())
					doWrite();
				else
					writing_ = false;
			}));
}

//-----------------------------------------------------------------
// 브리지를 거치지 않는 즉시 전송 (초기 연결 패킷 등)
//  - strand로 post하므로 어느 스레드에서 불러도 안전
//-----------------------------------------------------------------
void TCPC::SendPacketDirect(const void* packet)
{
	const unsigned char* src = reinterpret_cast<const unsigned char*>(packet);
	std::size_t len = src[0];
	if (len < 2 || len > PacketBuffer::MAX_PACKET) return;

	auto data = std::make_shared<std::vector<unsigned char>>(src, src + len);

	boost::asio::post(strand_, [this, data]() {
		writeQueue_.push_back(data);
		if (!writing_ && connected_) {
			writing_ = true;
			doWrite();
		}
		});
}