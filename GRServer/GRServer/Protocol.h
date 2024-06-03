#pragma once

typedef unsigned char BYTE;

constexpr unsigned int SERVERPORT = 4000;

const char* Lobby_IP = "127.0.0.1";

const int MAXSIZE = 1024;
const int MAXUSER = 500;
const int RoomCodeLen = 20;

const int NumOfGameServer = 2;


//how to receive data? get a type? or size?
enum All_Packet_Type {
	// Client -> Lobby 
	CS_LOGIN,
	CS_LOGOUT,
	CS_QUICK_MATCHING,
	CS_ENTER_ROOM_CODE,
	CS_CREATE_ROOM,
	CS_START_GAME,
	// Lobby -> Client
	SC_LOG_INFO, // �α��ΰ� ���
	SC_PUSH_MATCHING_Q, // ����Ī�� ���
	SC_FIND_ROOM_CODE, // ���ڵ������ ���
	SC_ROOM_CREATE, // �� ������ ���
	// Lobby -> Game
	LG_ROOMINFO,		//���� �����ϰ�, �Էµ� ������ ���� Ŭ���̾�Ʈ �ο����� �޾ƶ�
	LG_REFAIRROOM,		// (�����ϳ� �ٿ��) �ٸ� ���������ִ� ������ �о�ͼ� �ʰ� ���������� �̾��
	// Game -> Lobby
	GL_SERVERAMOUNT, // ������ ��ŭ ���ִ���
	GL_ENDGAME,		//�������� ������ ������. ( == Ŭ���̾�Ʈ �������� ��������)

	// Client -> Game
	CS_MOVEMENT,
	CS_ATTACK,
	// Game -> Client
	SC_MOVEMENT,
	SC_CHECKATTACK,
};

enum Skilltype {
	NormalSkill,
	UltimitSkill,
};

enum charactertype {
	Attacker,
	Sniper,
	Assister,

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

// Lobby


//---------------Lobby -> GameServer   (Receive Client)----------
//bind max 4 socket
struct LGLobbyToGame 
{
	int stage;
	int L2Gport;
	char L2Gip[20];
};


//--------------Lobby - Client---------------
struct CSLobbyLogin
{
	BYTE size;
	BYTE type;
	char name[20];
};

struct CSLobbyLogOut
{
	BYTE size;
	BYTE type;
	char name[20];
};

struct CSClickMatching
{
	BYTE size;
	BYTE type;
	BYTE stageNumber;
};

struct CSEnterRoomCode
{
	BYTE size;
	BYTE type;
	char RoomCode[RoomCodeLen];
};

struct CSCreateRoom
{
	BYTE size;	
	BYTE type;
	BYTE stageNumber;
};

struct CSStartGame {
	BYTE size;
	BYTE type;
};

struct SCLogInfo {
	BYTE size;
	BYTE type;
	bool exist;
};

struct SCPuahMatchingQ
{
	BYTE size;
	BYTE type;
};

struct SCFindRoomCode {

	BYTE size;
	BYTE type;
	bool exist;
};

struct SCRoomCreate
{
	BYTE size;
	BYTE type;
	char RoomCode[RoomCodeLen];
};

struct GLServerAmount {
	BYTE size;
	BYTE type;
	size_t amount;
};

// Game

// charcater move
struct CSMove {
	BYTE size;
	BYTE type;
	MoveData movedata;
	int play_timer;
	int roomnumber;
	int usernumber;
};

struct SCPlayerMove {
	BYTE size;
	BYTE type;
	int usernumber;
	MoveData movedtata;
};


struct CSAttack {
	BYTE size;
	BYTE type;
	char act_type;
	FXYZ LookVector;
	FXYZ position;
};

struct SCPosition {
	BYTE size;
	BYTE type;
	FXYZ position;
};


struct SCMonster {
	BYTE size;
	BYTE type;
	FXYZ pos;
	char act;

	//send changing model data?
};


#pragma pack(pop)