#pragma once


constexpr unsigned int SERVERPORT = 4000;
const int MAXSIZE = 1024;
const int MAXUSER = 500;

//class session;


//how to receive data? get a type? or size?

enum actiontype {
	NormalSkill,
	UltimitSkill,
	Attack
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





struct FXYZ {
	float x, y, z;
};


#pragma pack(push, 1)

// Lobby
struct CSplayerInfo 
{
	BYTE size;
	BYTE type;
	char name[20];
};

//bind max 4 socket
struct LobbyToGame 
{
	char stage;
	
};



// Game

// charcater move
struct CSmove {
	BYTE size;
	BYTE type;
	FXYZ position;

};


struct CSattack {
	BYTE size;
	BYTE type;
	char act_type;
};



struct SCmonster {
	BYTE size;
	BYTE type;
	FXYZ pos;
	char act;



	//send changing model data
};


#pragma pack(pop)