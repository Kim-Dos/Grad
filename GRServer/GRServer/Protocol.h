#include <vector>
#pragma once

typedef unsigned char BYTE;

constexpr unsigned int SERVERPORT = 4000;

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

enum All_Packet_Type {
	// Client -> Lobby 
	CL_LOGIN,				//로그인
	CL_LOGOUT,				//로그아웃
	CL_QUICK_MATCHING,		//퀵매칭
	CL_ENTER_ROOM_CODE,		//룸코드입력
	CL_CREATE_ROOM,			//방생성(방번호, 룸코드, 선택한 맵)
	CL_START_GAME,

	// Client -> Game
	CG_MOVEMENT,		//피킹된 객체들의 움직임(현재 위치와 목적지)를 보내자.
	CG_ATTACK,			//공격(

	// Lobby -> Client
	LC_LOG_INFO, // 로그인 및 검색결과
	LC_PUSH_MATCHING_Q, // 퀵매칭하려고 방 생성 후 오토매칭
	LC_FIND_ROOM_CODE, // 룸코드보고 입장
	LC_ROOM_CREATE, // 방 생성
	LC_COMPLETE_SESSION, //게임서버로 이제 갈 준비해라 및 게임서버 IP보내줘야함

	// Lobby -> Game
	LG_ROOMINFO,		//방을 생성하고, 입력된 정보를 통해 클라이언트 인원들을 받아라
	LG_REFAIRROOM,		// (서버하나 다운시) 다른 서버에서있는 정보를 읽어와서 너가 기존게임을 이어가라
	
	// Game -> Lobby
	GL_SERVERAMOUNT, // 서버에 얼만큼 차있는지
	GL_ENDGAME,		//서버에서 게임이 끝났다. ( == 클라이언트 정보들을 가져가라)

	// Game -> Client 
	GC_OTHER_MOVEMENT,		//이 객체의 정보를 다른 플레이어들에게 보내야 함.
	GC_CHECKATTACK,		//
	GC_LINKGAMESERVER
};

enum Skilltype {
	NormalSkill,
	UltimitSkill
};


struct ButtonPack {
	void* packet;
	BYTE type;
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


struct CLClickQMatching
{
	BYTE type;
};


struct CLEnterRoomCode
{
	BYTE type;
	char RoomCode[RoomCodeLen];
};


struct CLCreateRoom
{
	BYTE type;


	BYTE selected_map;
};


struct CLStartGame {
	BYTE type;
	char RoomCode[RoomCodeLen];
};

//------------------Client -->> GameServer -----------------

// 오브젝트 생성 ( 캐릭터 or 건물)
struct CGMaking {
	BYTE type;
	ObjType Obj;
	FXYZ position;
};

// 공격 및 이동 ( 1인칭일 때만 활성화 )
struct CGFPerspectAction {
	BYTE type;
	char act_type;
	FXYZ LookVector;
	FXYZ position;
};

// 오브젝트 이동 (피킹된 객체들만)
struct CGPickingMove {
	BYTE type;
	BYTE pickingsize;
	int playerNumber;
	char act_command;
	MoveData move_data[12];
};


// 처음 입장, 룸 코드를 통해 게임서버로 연결
struct CGLinkInfo {
	BYTE size;
	BYTE type;
	char RoomCode[RoomCodeLen];
	int userID;
};



//-------------- LobbyServer  -->> Client ---------------

// 로그인 및 검색결과 반환
struct LCLogInfo {
	BYTE type;
	bool exist;
};

// 퀵매칭으로 진입함 (방 생성 후 오토매칭)
struct LCPushMatchingQ
{
	BYTE type;
	bool done;
};

// 룸코드를 보고 입장가능 확인 반환
struct LCFindRoomCode 
{
	BYTE type;
	bool Fulled;
	bool exist;
};

// 방 생성후 반환
struct LCRoomCreate
{
	BYTE type;
	char RoomCode[RoomCodeLen];
};

// 게임서버로 이제 갈 준비해라 및 게임서버 IP보내줘야함
struct LCStartGameServer {
	BYTE size;
	BYTE type;
	int userNumber;
	char GameServerIP[ipsize];
	int ServerPort;
};


//---------------LobbyServer -> GameServer  --------------
struct LGLobbyToGame
{
	BYTE size;
	BYTE type;
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
	BYTE type;
	BYTE size;
	size_t amount;
};


//---------------- GameServer -->> Clients -----------------

// 다른유저의 데이터, (다른 유저가 피킹한 객체들의 움직임 또는 생성한 건물들)
struct GCReadytoStart {
	BYTE type;
	bool MapReady;
	bool AllPlayerReady;
};

// 일정거리에 상대편 객체가 와서, 공격할때.
// 공격 command가 아닐때, 공격당해서 같이 공격 command로 바뀔때.
struct GCAttack {
	BYTE type;
	char objNumber;
	int enemy_playerNumber;
	char enemy_objNumber;
};

// 다른유저의 데이터, (다른 유저가 피킹한 객체들의 움직임 또는 생성한 건물들)
struct GCPickingMove {
	BYTE type;
	BYTE pickingsize;
	int playerNumber;
	char act_command;
	MoveData move_data[12];
};




#pragma pack(pop)