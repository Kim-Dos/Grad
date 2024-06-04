#pragma once

typedef unsigned char BYTE;

constexpr unsigned int SERVERPORT = 4000;

const char* Lobby_IP = "127.0.0.1";

const int MAXSIZE = 1024;
const int MAXUSER = 500;
const int RoomCodeLen = 14;

const int ipsize = 16;
const int NumOfGameServer = 2;


//how to receive data? get a type? or size?
enum All_Packet_Type {
	// Client -> Lobby 
	CL_LOGIN,
	CL_LOGOUT,
	CL_QUICK_MATCHING,
	CL_ENTER_ROOM_CODE,
	CL_CREATE_ROOM,
	CL_START_GAME,
	// Client -> Game
	CG_MOVEMENT,
	CG_ATTACK,

	// Lobby -> Client
	LC_LOG_INFO, // �α��ΰ� ���
	LC_PUSH_MATCHING_Q, // ����Ī�� ���
	LC_FIND_ROOM_CODE, // ���ڵ������ ���
	LC_ROOM_CREATE, // �� ������ ���
	// Lobby -> Game
	LG_ROOMINFO,		//���� �����ϰ�, �Էµ� ������ ���� Ŭ���̾�Ʈ �ο����� �޾ƶ�
	LG_REFAIRROOM,		// (�����ϳ� �ٿ��) �ٸ� ���������ִ� ������ �о�ͼ� �ʰ� ���������� �̾��
	
	// Game -> Lobby
	GL_SERVERAMOUNT, // ������ ��ŭ ���ִ���
	GL_ENDGAME,		//�������� ������ ������. ( == Ŭ���̾�Ʈ �������� ��������)
	// Game -> Client
	GC_MOVEMENT,
	GC_CHECKATTACK
};

enum Skilltype {
	NormalSkill,
	UltimitSkill
};

enum charactertype {
	Attacker,
	Sniper,
	Assister
};

enum stagetype {
	first,
	second,
	third
};


struct ButtonPack {
	void* packet;
	BYTE type;
};

struct MoveData {
	FXYZ position;
	FXYZ velocity;
	FXYZ look;
	char act;
};

struct FXYZ {
	float x, y, z;
};


#pragma pack(push, 1)


//--------------Client  -->> LobbyServer ---------------
struct CLLobbyLogin
{
	BYTE size;
	BYTE type;
	char name[20];
};


struct CLLobbyLogOut
{
	BYTE size;
	BYTE type;
	char name[20];
};


struct CLClickMatching
{
	BYTE size;
	BYTE type;
	BYTE stageNumber;
};


struct CLEnterRoomCode
{
	BYTE size;
	BYTE type;
	char RoomCode[RoomCodeLen];
};


struct CLCreateRoom
{
	BYTE size;	
	BYTE type;
	BYTE stageNumber;
};


struct CLStartGame {
	BYTE size;
	BYTE type;
	char RoomCode[RoomCodeLen];
};



//------------------Client -->> GameServer -----------------
struct CGAttack {
	BYTE size;
	BYTE type;
	char act_type;
	FXYZ LookVector;
	FXYZ position;
};

struct CGMove {
	BYTE size;
	BYTE type;
	MoveData movedata;
	int play_timer;
	int roomnumber;
	int partynumber;
};




//-------------- LobbyServer  -->> Client ---------------

struct LCLogInfo {
	BYTE size;
	BYTE type;
	bool exist;
};


struct LCPuahMatchingQ
{
	BYTE size;
	BYTE type;
};


struct LCFindRoomCode {

	BYTE size;
	BYTE type;
	bool Fulled;
	bool exist;
};


struct LCRoomCreate
{
	BYTE size;
	BYTE type;
	char RoomCode[RoomCodeLen];
};



//---------------LobbyServer -> GameServer  --------------
struct LGLobbyToGame
{
	BYTE size;
	BYTE type;
	char RoomCode[RoomCodeLen];
	int L2Gport;
	char userSet1[ipsize];
	char userSet2[ipsize];
	char userSet3[ipsize];
	char userSet[ipsize];
};

struct LGRefair {

};

//---------------- GameServer -->> LobbyServer -----------------
struct GLServerAmount {
	BYTE size;
	BYTE type;
	size_t amount;
};


//---------------- GameServer -->> Clients -----------------

struct GCPlayerMove {
	BYTE size;
	BYTE type;
	int partynumber;
	MoveData movedtata;
};

struct GCMonster {
	BYTE size;
	BYTE type;
	FXYZ pos;
	char act;

	//send changing model data?
};



#pragma pack(pop)