#include "Protocol.h"
#include "LobbyServer.hpp"

using boost::asio::ip::tcp;
using boost::concurrent_flat_map;
//using boost::asio::awaitable;


concurrent_flat_map<int, std::shared_ptr<LobbyClientSession>> clients;
concurrent_flat_map<int, std::shared_ptr<LobbytoGameSession>> servers;
concurrent_flat_map<std::string, std::shared_ptr<LobbyRoom>> MatchingRoom;



// --------------------------------------------------------------------------------------------------------------------------------
// LobbyTCP
// --------------------------------------------------------------------------------------------------------------------------------

void LobbyTCP::ServerAccept() {
	mTCPAcceptor.async_accept(mTCPSocket, [this](boost::system::error_code ec) {
		if (ec) {
			std::cout << " AcceptError" << std::endl;
			exit(-1);
		}

		int newsession = GetNewClient();
		if (isGameServer() or newsession == 0 ) {
			// Connect Game Server
			servers.emplace(newsession, std::make_shared<LobbytoGameSession>(std::move(mTCPSocket), newsession));
			servers.visit(newsession, [](auto& x) {
				x.second->Start();
				});
		}
		else {
			clients.emplace(newsession, std::make_shared<LobbyClientSession>(std::move(mTCPSocket), newsession));
			clients.visit(newsession, [](auto& x) {
				x.second->Start();
				});
		}
		ServerAccept();
		});

}


LobbyTCP::LobbyTCP(boost::asio::io_context& IOContext, int ServerPort) :mTCPSocket(IOContext), mTCPAcceptor(IOContext, tcp::endpoint(tcp::v4(),ServerPort))
{
	ServerAccept();
}

bool LobbyTCP::isGameServer() {

	for (auto  x : IPS) {
		if (mTCPSocket.remote_endpoint().address().to_v4().to_string() == x) return true;
	}
	return false;
}



// --------------------------------------------------------------------------------------------------------------------------------
// Lobby Server <--> Clients
// --------------------------------------------------------------------------------------------------------------------------------
void LobbyClientSession::recv() {
	auto self(shared_from_this());
	plSock.async_read_some(boost::asio::buffer(recvBuffer),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (ec) {
				std::cout << "READ ERR" << std::endl;
				exit(-1);
			}
			int BufferLoad = static_cast<int>(length);
			unsigned char* PacketPoint = recvBuffer;
			while (0 < BufferLoad) {
				if (curDataSize == 0) {
					curDataSize = PacketPoint[0];
					if (curDataSize > 200) { std::cout << "BufferErr\n" << std::endl; exit(-1); } //find a error packet
				}
				int build = curDataSize - prevDataSize;

				if (build <= BufferLoad) {
					memcpy(PacketData + prevDataSize, PacketPoint, build);
					LobbyPacketProcess();
					curDataSize = 0;
					prevDataSize = 0;
					BufferLoad -= build;
					PacketPoint += build;
				}
				else {
					memcpy(PacketData + prevDataSize, PacketPoint, build);
					prevDataSize += build;
					build = 0;
					PacketPoint += BufferLoad;
				}
			}
			recv();
		});
}

void LobbyClientSession::Start() {
	recv();
	std::cout << "Start - " << userID << std::endl;
}

void LobbyClientSession::SetAutoMatching()
{
	bool flag = MatchingRoom.visit_while([](auto& x) {
		if ( x.second->isAutoMatching()&& (!x.second->isFulled()) )
		{
			x.second->PushUser(userID);
			return false;
		}
		return true;
	});
	//모든 오토매칭 방 X, 매칭중 빈방X일때,
	if(flag) MakeRoom(true, PacketData[2]);
}

void LobbyClientSession::MakeRoom(bool flag, unsigned char stagenum)
{
	std::string RoomCode;
	for (int i = 0; i < RoomCodeLen; ++i) {
		int x = std::rand() % 62;
		if (x < RoomCodeLen) {
			RoomCode += std::to_string(x);
		}
		else if (x < RoomCodeLen+ 26) {
			RoomCode += static_cast<char>('A' + (x - RoomCodeLen));
		}
		else {
			RoomCode += static_cast<char>('a' + (x - RoomCodeLen - 26));
		}
	}
	//오토매칭 방 만들기, flag == True면 오토매칭, False면 그냥 방 생성, 첫번째 클라이언트에게 RoomCode 보냄
	MatchingRoom.emplace(RoomCode, std::make_shared<LobbyRoom>(userID, flag, stagenum));

	if (flag == false) {
		LCRoomCreate p;
		p.size = sizeof(LCRoomCreate);
		p.type = LC_ROOM_CREATE;
		memcpy(&p.RoomCode, &RoomCode, sizeof(RoomCodeLen));
		PacketSend(&p);
		
	}
}

void LobbyClientSession::EnterLobbyRoom()
{
	std::string code;
	memcpy(&code, PacketData + 2, sizeof(RoomCodeLen));

	LCFindRoomCode p;
	p.size = sizeof(LCFindRoomCode);
	p.type = LC_FIND_ROOM_CODE;
	p.exist = MatchingRoom.contains(code);
	if (p.exist) {
		MatchingRoom.visit(code, [p](auto& x) {
			if (!x.second->isFulled()) x.second->PushUser(userID);
			else p.Fulled = true;
		});
	}
	else {
		p.Fulled = false;
	}
	PacketSend(&p);
}

//게임서버로 전송하는거 아직 안함
void LobbyClientSession::IntoGameServer() 
{
	std::string code;
	memcpy(&code, PacketData + 2, sizeof(RoomCodeLen));

	MatchingRoom.visit(code, [](auto& x) {
		x.second->PushUser(userID);
		x.second->SetAllReadyTrue();
	});

	MatchingRoom.visit_all([](auto& x) {
		if (x.second->isAllReady()) {
			//서버중에서 제일 연결 적은 서버로 가서
			
			//게임서버로 패킷 전송
			LGLobbyToGame p;
			p.size = sizeof(LGLobbyToGame);
			p.type = LG_ROOMINFO;
			p.
		}
		});
}

void LobbyClientSession::PacketSend(void* packet)
{
	int packetsize = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buffer = new unsigned char[packetsize];
	memcpy(buffer, packet, packetsize);
	auto self(shared_from_this());
	plSock.async_write_some(boost::asio::buffer(buffer, static_cast<size_t>(packetsize)),
		[this, buffer, packetsize](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)
			{
				if (packetsize != bytes_transferred) {
					std::cout << "ERR - Bytes_transferred\n";
				}
				delete buffer;
			}
		});
}

void LobbyClientSession::LobbyPacketProcess() {

	switch (PacketData[1])
	{
	case CL_QUICK_MATCHING:
		SetAutoMatching();
		break;
	case CL_CREATE_ROOM:
		MakeRoom(false, PacketData[2]);
		break;
	case CL_ENTER_ROOM_CODE:
		EnterLobbyRoom();
		break;
	case CL_START_GAME:
		IntoGameServer();
		break;
	default:
		break;
	}
}





// --------------------------------------------------------------------------------------------------------------------------------
// Lobby Server <--> Game Server
// --------------------------------------------------------------------------------------------------------------------------------


void LobbytoGameSession::recv()
{
	auto self(shared_from_this());
	GameSerSock.async_read_some(boost::asio::buffer(recvBuffer),
		[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (ec) {
				std::cout << "READ ERR" << std::endl;
				exit(-1);
			}
			int BufferLoad = static_cast<int>(length);
			unsigned char* PacketPoint = recvBuffer;
			while (0 < BufferLoad) {
				if (curDataSize == 0) {
					curDataSize = PacketPoint[0];
					if (curDataSize > 200) { std::cout << "BufferErr\n" << std::endl; exit(-1); } //find a error packet
				}
				int build = curDataSize - prevDataSize;

				if (build <= BufferLoad) {
					memcpy(PacketData + prevDataSize, PacketPoint, build);
					LobbytoGamePacketProcess();
					curDataSize = 0;
					prevDataSize = 0;
					BufferLoad -= build;
					PacketPoint += build;
				}
				else {
					memcpy(PacketData + prevDataSize, PacketPoint, build);
					prevDataSize += build;
					build = 0;
					PacketPoint += BufferLoad;
				}
			}
			recv();
		});
}

void LobbytoGameSession::LobbytoGamePacketProcess()
{
	switch (PacketData[1])
	{
	case GL_SERVERAMOUNT:
		SetServerAmount();
		break;
	case GL_ENDGAME:
		AttachMatchingRoom();
	default:
		break;
	}
}

void LobbytoGameSession::SetServerAmount()
{
	memcpy(&ServerAmount, PacketData + 2, sizeof(unsigned long long));
}

void LobbytoGameSession::AttachMatchingRoom()
{
}

void LobbytoGameSession::SendPacket(void* packet)
{
	int packetsize = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buffer = new unsigned char[packetsize];
	memcpy(buffer, packet, packetsize);
	//auto self(shared_from_this());
	GameSerSock.async_write_some(boost::asio::buffer(buffer, static_cast<size_t>(packetsize)),
		[this, buffer, packetsize](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)
			{
				if (packetsize != bytes_transferred) {
					std::cout << "ERR - Bytes_transferred\n";
				}
				delete buffer;
			}
		});
}

void LobbytoGameSession::Start()
{
	recv();
	std::cout << "Connect Lobby and Server" << std::endl;
}


// --------------------------------------------------------------------------------------------------------------------------------
// LobbyRoom
// --------------------------------------------------------------------------------------------------------------------------------



LobbyRoom::LobbyRoom(int firstUser, bool automatch, unsigned char stagenum) noexcept
	: user1(firstUser), AutoMatching(automatch),stageNumber(stagenum)
{
	Allready = false;
	std::cout << "Make Room" << std::endl;
}

bool LobbyRoom::isFulled()
{
	if (user2 != NULL) return false;
	if (user3 != NULL) return false;
	if (user4 != NULL) return false;
	return true;
}


void LobbyRoom::SetAllReadyTrue()
{
	Allready = true;
}

void LobbyRoom::PushUser(int userid)
{
	if (user1 == NULL) user1 = userid;
	else if (user2 == NULL) user2 = userid;
	else if (user3 == NULL) user3 = userid;
	else if (user4 == NULL) { 
		user4 = userid;
		if (AutoMatching) { Allready = true; }
	}
	else return;
}
