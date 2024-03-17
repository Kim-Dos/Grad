// ---------------------
// Game Server
// ---------------------


#include "Protocol.h"
#include "GameServer.h"




void GameTCP::ServerAccept() 
{
	mTCPAcceptor.async_accept(mTCPSocket, [this](boost::system::error_code ec) {
		if (ec) {
			std::cout << " AcceptError" << std::endl;
			exit(-1);
		}
		int newsession = GetNumOfClient();
		clients.emplace(newsession, std::make_shared<session>(std::move(mTCPSocket), newsession));
		clients.visit(newsession, [](auto& x) {
			x.second->Start();
			});

		ServerAccept();
		});
}




