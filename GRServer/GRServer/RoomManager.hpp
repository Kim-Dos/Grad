#include "GameRoom.hpp"


using boost::concurrent_flat_map;
using boost::asio::ip::tcp;

class GameRoom;
class TCPGameSession;


#pragma once
class RoomManager {
    boost::asio::io_context& ioc_;
    concurrent_flat_map<std::string, std::shared_ptr<GameRoom>> rooms_;

public:
    RoomManager(boost::asio::io_context& ioc) : ioc_(ioc) {}

    std::shared_ptr<GameRoom> CreateRoom(const std::string& code, const std::string& map)
    {
        auto room = std::make_shared<GameRoom>(ioc_, code, map);
        rooms_.emplace(code, room);
        return room;
    }

    void JoinRoom(const std::string& code, std::shared_ptr<TCPGameSession> session)
    {
        rooms_.visit(code, [&session](auto&& pair) {
            pair.second->AddSession(session);
        });
        return;
    }

};