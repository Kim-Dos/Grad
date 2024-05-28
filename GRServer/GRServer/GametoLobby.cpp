#include "GametoLobby.hpp"

void GametoLobby:: Connect_Handler(boost::system::error_code ec)
{
	if (ec) {
		std::cout << ec.what() << std::endl;
	}

	// ���Ŀ� ���� �ð����� Data ������ �� ����� �ο����� ������ �߰� ���� ����
}

void GametoLobby::LobbyConnect()
{
	lobbySock.async_connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(Lobby_IP), SERVERPORT), Connect_Handler);
}

GametoLobby::GametoLobby(boost::asio::io_context& context) noexcept
	:  lobbySock(context)
{
	ZeroMemory(TCPPacketData, MAXSIZE);
	ZeroMemory(TCPrecvBuffer, MAXSIZE);
	curDataSize = 0, prevDataSize = 0;

	LobbyConnect();
}
