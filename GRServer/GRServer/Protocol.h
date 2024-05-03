#pragma once

typedef unsigned char BYTE;

constexpr unsigned int SERVERPORT = 4000;

const int MAXSIZE = 1024;
const int MAXUSER = 500;
const int RoomCodeLen = 20;

const int NumOfGameServer = 2;


//how to receive data? get a type? or size?

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

enum LobbyClick {
	LogIn,
	ClickMatching,
	CreateRoom,
	EnterRoomcode,
	GameStart,
	LogOut
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

/*
//bind max 4 socket
struct LobbyToGame 
{
	char stage;
	
};
*/

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
	char RoomCode[10];
};

struct CSCreateRoom
{
	BYTE size;
	BYTE type;
	BYTE stageNumber;
};



// Game

// charcater move
struct CSmove {
	BYTE size;
	BYTE type;
	FXYZ position;
	int roomnumber;
	int usernumber;
};


struct CSattack {
	BYTE size;
	BYTE type;
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