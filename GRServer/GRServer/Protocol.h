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
	char type;
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
	char type;
	int keydown;

};


struct CSattack {
	char type;
	char act_type;
};



struct SCmonster {
	char type;
	FXYZ pos;
	char act;
	//send changing model data
};


#pragma pack(pop)