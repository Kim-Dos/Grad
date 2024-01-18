#pragma once


constexpr unsigned int SERVERPORT = 4000;
const int MAXSIZE = 1024;

class session;


//how to receive data? get a type? or size?

enum actiontype {
	MainSkill,
	UltimitSkill,
	GeneralAttack
};

enum charactertype {
	Attacker,
	Sniper,
	Assister
};





struct FXYZ {
	float x, y, z;
};


#pragma pack(push, 1)

// Lobby
struct CSplayer 
{
	char type;
	char name[20];
};

//bind max 4 socket
struct LobbyToGame 
{
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