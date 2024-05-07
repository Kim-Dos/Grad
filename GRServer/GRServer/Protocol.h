#pragma once


constexpr unsigned int SERVERPORT = 4000;

const int MAXSIZE = 1024;
const int MAXUSER = 500;
const int RoomCodeLen = 20;

const int NumOfGameServer = 2;


//how to receive data? get a type? or size?
enum All_Packet_Type {
	// Lobby - Client to Server 
	CS_LOGIN,
	CS_LOGOUT,
	CS_QUICK_MATCHING,
	CS_ENTER_ROOM_CODE,
	CS_CREATE_ROOM,
	CS_START_GAME,
	// Lobby - Server to Client
	// Game - Client to Server
	CS_MOVEMENT,
	CS_ATTACK,
};

enum actiontype {
	NormalSkill,
	UltimitSkill,
	Attack
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


enum packettype {
	SC_POSITION,
	CS_MOVE
};

struct FXYZ {
	float x, y, z;
};


#pragma pack(push, 1)

// Lobby


//bind max 4 socket
struct LGLobbyToGame 
{
	INT stage;
	INT port;
	CHAR ip[20];
};


struct CSLobbyLogin
{
	BYTE size;
	BYTE type = CS_LOGIN;
	char name[20];
};

struct CSLobbyLogOut
{
	BYTE size;
	BYTE type = CS_LOGOUT;
	char name[20];
};

struct CSClickMatching
{
	BYTE size;
	BYTE type = CS_QUICK_MATCHING;
	BYTE stageNumber;
};

struct CSEnterRoomCode
{
	BYTE size;
	BYTE type = CS_ENTER_ROOM_CODE;
	char RoomCode[10];
};

struct CSCreateRoom
{
	BYTE size;
	BYTE type = CS_CREATE_ROOM;
	BYTE stageNumber;
};



// Game

// charcater move
struct CSmove {
	BYTE size;
	BYTE type = CS_MOVEMENT;
	FXYZ position;
	int plyatimer;
	int roomnumber;
	int usernumber;
};


struct CSattack {
	BYTE size;
	BYTE type = CS_ATTACK;
	char act_type;
	FXYZ LookVector;
	FXYZ position;
};

struct SCposition {
	BYTE size;
	BYTE type;
	FXYZ position;
};


struct SCmonster {
	BYTE size;
	BYTE type;
	FXYZ pos;
	char act;



	//send changing model data
};


#pragma pack(pop)