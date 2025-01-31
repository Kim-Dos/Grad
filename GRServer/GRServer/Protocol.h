#include <vector>
#pragma once

typedef unsigned char BYTE;

constexpr unsigned int SERVERPORT = 4000;
constexpr unsigned int LOBBYPORT = 4100;

//const char* Lobby_IP = "127.0.0.1";

const int MAXSIZE = 1024;
const int MAXUSER = 500;
const int MAXPICKING = 12;

const int RoomCodeLen = 14;

const int ipsize = 16;
const int NumOfGameServer = 2;


struct FXYZ {
	float x, y, z;
};

enum ObjType : BYTE {
	Slave,
	Knight,
	Archer,
	Tower,
	Base
};

enum Packet_Type : BYTE {
	// Client -> Lobby 
	CL_LOGIN,				//�α���
	CL_LOGOUT,				//�α׾ƿ�
	CL_QUICK_MATCHING,		//����Ī
	CL_ENTER_ROOM_CODE,		//���ڵ��Է�
	CL_CREATE_ROOM,			//�����(���ȣ, ���ڵ�, ������ ��)
	CL_START_GAME,

	// Client -> Game
	CG_MOVEMENT,		//��ŷ�� ��ü���� ������(���� ��ġ�� ������)�� ������.
	CG_ATTACK,			//����(

	// Lobby -> Client
	LC_LOG_INFO, // �α��� �� �˻����
	LC_PUSH_MATCHING_Q, // ����Ī�Ϸ��� �� ���� �� �����Ī
	LC_FIND_ROOM_CODE, // ���ڵ庸�� ����
	LC_ROOM_CREATE, // �� ����
	LC_COMPLETE_SESSION, //���Ӽ����� ���� �� �غ��ض� �� ���Ӽ��� IP���������

	// Lobby -> Game
	LG_ROOMINFO,		//���� �����ϰ�, �Էµ� ������ ���� Ŭ���̾�Ʈ �ο����� �޾ƶ�
	LG_REFAIRROOM,		// (�����ϳ� �ٿ��) �ٸ� ���������ִ� ������ �о�ͼ� �ʰ� ���������� �̾��
	
	// Game -> Lobby
	GL_SERVERAMOUNT, // ������ ��ŭ ���ִ���
	GL_ENDGAME,		//�������� ������ ������. ( == Ŭ���̾�Ʈ �������� ��������)

	// Game -> Client 
	GC_OTHER_MOVEMENT,		//�� ��ü�� ������ �ٸ� �÷��̾�鿡�� ������ ��.
	GC_CHECKATTACK,		//
	GC_LINKGAMESERVER
};

enum Skilltype {
	NormalSkill,
	UltimitSkill
};


struct ButtonPack {
	void* packet;
	Packet_Type type;
};

struct MoveData {
	int objnumber;
	FXYZ position;
	FXYZ destination;
};



#pragma pack(push, 1)


//--------------Client  -->> LobbyServer ---------------
struct CLLobbyLogin
{
	Packet_Type type;
	BYTE size;
	char name[20];
};


struct CLLobbyLogOut
{
	Packet_Type type;
	BYTE size;
	char name[20];
};


struct CLClickQMatching
{
	Packet_Type type;
};


struct CLEnterRoomCode
{
	Packet_Type type; 
	char RoomCode[RoomCodeLen];
};


struct CLCreateRoom
{
	Packet_Type type; 
	BYTE selected_map;
};


struct CLStartGame {
	Packet_Type type;
	char RoomCode[RoomCodeLen];
};

//------------------Client -->> GameServer -----------------

// ������Ʈ ���� ( ĳ���� or �ǹ�)
struct CGMaking {
	Packet_Type type;
	ObjType Obj;
	FXYZ position;
};

// ���� �� �̵� ( 1��Ī�� ���� Ȱ��ȭ )
struct CGFPerspectAction {
	Packet_Type type;
	char act_type;
	FXYZ LookVector;
	FXYZ position;
};

// ������Ʈ �̵� (��ŷ�� ��ü�鸸)
struct CGPickingMove {
	Packet_Type type;
	BYTE pickingsize;
	int playerNumber;
	char act_command;
	MoveData move_data[12];
};


// ó�� ����, �� �ڵ带 ���� ���Ӽ����� ����
struct CGLinkInfo {
	Packet_Type type;
	BYTE size;
	char RoomCode[RoomCodeLen];
	int userID;
};



//-------------- LobbyServer  -->> Client ---------------

// �α��� �� �˻���� ��ȯ
struct LCLogInfo {
	Packet_Type type;
	bool exist;
};

// ����Ī���� ������ (�� ���� �� �����Ī)
struct LCPushMatchingQ
{
	Packet_Type type;
	bool done;
};

// ���ڵ带 ���� ���尡�� Ȯ�� ��ȯ
struct LCFindRoomCode 
{
	Packet_Type type;
	bool Fulled;
	bool exist;
};

// �� ������ ��ȯ
struct LCRoomCreate
{
	Packet_Type type;
	char RoomCode[RoomCodeLen];
};

// ���Ӽ����� ���� �� �غ��ض� �� ���Ӽ��� IP���������
struct LCStartGameServer {
	Packet_Type type;
	BYTE size;
	int userNumber;
	char GameServerIP[ipsize];
	int ServerPort;
};


//---------------LobbyServer -> GameServer  --------------
struct LGLobbyToGame
{
	Packet_Type type;
	BYTE size;
	char RoomCode[RoomCodeLen];
	int usernumber1;
	int usernumber2;
	int usernumber3;
	int usernumber4;
};

struct LGRefair {

};

//---------------- GameServer -->> LobbyServer -----------------

struct GLServerAmount {
	Packet_Type type;
	BYTE size;
	size_t amount;
};


//---------------- GameServer -->> Clients -----------------

// �ٸ������� ������, (�ٸ� ������ ��ŷ�� ��ü���� ������ �Ǵ� ������ �ǹ���)
struct GCReadytoStart {
	Packet_Type type;
	bool MapReady;
	bool AllPlayerReady;
};

// �����Ÿ��� ����� ��ü�� �ͼ�, �����Ҷ�.
// ���� command�� �ƴҶ�, ���ݴ��ؼ� ���� ���� command�� �ٲ�.
struct GCAttack {
	Packet_Type type;
	char objNumber;
	int enemy_playerNumber;
	char enemy_objNumber;
};

// �ٸ������� ������, (�ٸ� ������ ��ŷ�� ��ü���� ������ �Ǵ� ������ �ǹ���)
struct GCPickingMove {
	Packet_Type type;
	BYTE pickingsize;
	int playerNumber;
	char act_command;
	MoveData move_data[12];
};




#pragma pack(pop)