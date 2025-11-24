#include "TCPClient.hpp"

using boost::asio::ip::tcp;

const char* ip = "127.0.0.1";
int port = 4000;  // 게임 서버 포트

boost::asio::ip::tcp::endpoint GameServerIP(boost::asio::ip::address::from_string(ip), port);

TCPC::TCPC(boost::asio::io_context& service) noexcept
    : msocket(service)
{
    prevDataSize = 0, curDataSize = 0;
    ZeroMemory(recvBuffer, MAXSIZE);
    ZeroMemory(PacketData, MAXSIZE);

    // 게임 서버로 바로 연결
    msocket.async_connect(GameServerIP, [](boost::system::error_code ec) {
        if (ec) {
            std::cout << "Connection failed: " << ec.what() << std::endl;
            exit(-1);
        }
        std::cout << "Connected to game server!" << std::endl;
        });
}

void TCPC::recv(unsigned char(&arr)[1024]) {

    msocket.async_read_some(boost::asio::buffer(arr),
        [this, &arr](boost::system::error_code ec, std::size_t length)
        {
            if (ec) {
                std::cout << ec.what() << std::endl;
                exit(-1);
            }
            int BufferLoad = static_cast<int>(length);

            unsigned char* PacketPoint = recvBuffer;
            while (0 < BufferLoad) {
                if (curDataSize == 0) {
                    curDataSize = PacketPoint[0];
                    if (curDataSize > 200) {
                        std::cout << "BufferErr\n" << std::endl;
                        exit(-1);
                    }
                }
                int build = curDataSize - prevDataSize;

                if (build <= BufferLoad) {
                    memcpy(PacketData + prevDataSize, PacketPoint, build);
                    ClientPacketProcess();
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
            recv(arr);
        });
}

void TCPC::Packetsend(void* packet) {
    std::cout << "sendpack\n";
    int packetsize = reinterpret_cast<unsigned char*>(packet)[0];
    unsigned char* buffer = new unsigned char[packetsize];
    memcpy(buffer, packet, packetsize);
    msocket.async_write_some(boost::asio::buffer(buffer, (size_t)packetsize),
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

void TCPC::ClientPacketProcess() {
    Packet_Type type = static_cast<Packet_Type>(PacketData[1]);

    switch (type)
    {
    case GC_OTHER_MOVEMENT: // 다른 플레이어의 이동 데이터 (서버 검증됨)
    {
        GCPickingMove packet;
        memcpy(&packet, PacketData, sizeof(GCPickingMove));

        std::cout << "Received validated movement from Player " << packet.playerNumber << std::endl;
        std::cout << "  PickingSize: " << (int)packet.pickingsize << std::endl;

        // 여기서 다른 플레이어의 오브젝트 위치 업데이트
        for (int i = 0; i < packet.pickingsize; i++) {
            std::cout << "  Object " << (int)packet.move_data[i].objnumber
                << " -> Pos(" << packet.move_data[i].pos_x
                << ", " << packet.move_data[i].pos_y << ")" << std::endl;

            // TODO: 게임 오브젝트 업데이트
            // UpdateOtherPlayerObject(packet.playerNumber, packet.move_data[i]);
        }
        break;
    }

    case GC_POSITION_CORRECTION: // 서버에서 보낸 위치 보정 (NEW)
    {
        GCPositionCorrection packet;
        memcpy(&packet, PacketData, sizeof(GCPositionCorrection));

        std::cout << "[CLIENT] Position correction received from server!" << std::endl;
        std::cout << "  Correcting " << (int)packet.pickingsize << " objects" << std::endl;

        // 서버에서 검증된 올바른 위치로 강제 업데이트
        for (int i = 0; i < packet.pickingsize; i++) {
            std::cout << "  [CORRECTION] Object " << (int)packet.corrected_data[i].objnumber
                << " corrected to (" << packet.corrected_data[i].pos_x
                << ", " << packet.corrected_data[i].pos_y << ")" << std::endl;

            // TODO: 자신의 오브젝트 위치를 서버 값으로 강제 보정
            // CorrectMyObjectPosition(packet.corrected_data[i]);
        }
        break;
    }

    case GC_CHECKATTACK: // 다른 플레이어의 공격
    {
        GCAttack packet;
        memcpy(&packet, PacketData, sizeof(GCAttack));

        std::cout << "Player " << packet.enemy_playerNumber << " attacked!" << std::endl;

        // TODO: 공격 이펙트 표시
        break;
    }

    case LC_LOG_INFO:
        break;
    case LC_PUSH_MATCHING_Q:
        break;
    case LC_FIND_ROOM_CODE:
        break;
    case LC_ROOM_CREATE:
        break;
    default:
        std::cout << "Unknown packet: " << (int)type << std::endl;
        break;
    }
}

void TCPC::Character_Positioning() {
    FXYZ position;
    memcpy(&position, PacketData + 2, sizeof(FXYZ));
    traceKey();
}

void TCPC::traceKey() {
    // 키보드 입력 처리
}

// 클라이언트에서 이동 데이터 전송 예시
void TCPC::SendMovement(const std::vector<MoveData>& moveData) {
    CGPickingMove packet;
    packet.size = sizeof(CGPickingMove);
    packet.type = CG_MOVEMENT;
    packet.pickingsize = static_cast<BYTE>(moveData.size());
    packet.playerNumber = myPlayerNumber; // 자신의 플레이어 번호
    packet.act_command = 'M'; // Move command

    for (size_t i = 0; i < moveData.size() && i < MAXPICKING; i++) {
        packet.move_data[i] = moveData[i];
    }

    Packetsend(&packet);
}
