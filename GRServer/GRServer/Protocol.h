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
	SC_LOG_INFO, // 로그인과 대비
	SC_PUSH_MATCHING_Q, // 퀵매칭과 대비
	SC_FIND_ROOM_CODE, // 룸코드입장과 대비
	SC_ROOM_CREATE, // 방 생성과 대비
	// Lobby -> Game
	LG_ROOMINFO,		//방을 생성하고, 입력된 정보를 통해 클라이언트 인원들을 받아라
	LG_REFAIRROOM,		// (서버하나 다운시) 다른 서버에서있는 정보를 읽어와서 너가 기존게임을 이어가라
	// Game -> Lobby
	GL_SERVERAMOUNT, // 서버에 얼만큼 차있는지
	GL_ENDGAME,		//서버에서 게임이 끝났다. ( == 클라이언트 정보들을 가져가라)

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