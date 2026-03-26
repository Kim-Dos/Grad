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

constexpr float MAX_MOVE_SPEED = 300.0f;        // 오브젝트 최대 이동 속도 (units/sec)
constexpr float SPEED_HACK_TOLERANCE = 1.5f;    // 속도핵 판정 여유 (1.5배까지 허용)
constexpr float MAP_BOUNDARY = 10000.0f;        // 맵 경계
constexpr float POSITION_SYNC_INTERVAL = 0.15f; // 위치 동기화 간격 (초)
constexpr int   MAX_HACK_WARNINGS = 5;

#pragma pack(push, 1)

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
	CL_LOGIN,				//로그인
	CL_LOGOUT,				//로그아웃
	CL_QUICK_MATCHING,		//퀵매칭
	CL_ENTER_ROOM_CODE,		//룸코드입력
	CL_CREATE_ROOM,			//방생성(방번호, 룸코드, 선택한 맵)
	CL_START_GAME,

	// Client -> Game
	CG_LINKGAMESERVER,	//처음 입장, 룸 코드를 통해 게임서버로 연결 ( 임시 연결)
	CG_MOVEMENT,		//피킹된 객체들의 움직임(현재 위치와 목적지)를 보내자.
	CG_ATTACK,			//공격(

	// Lobby -> Client
	LC_LOG_INFO, // 로그인 및 검색결과
	LC_PUSH_MATCHING_Q, // 퀵매칭완료고 방 생성 후 오토매칭
	LC_FIND_ROOM_CODE, // 룸코드보고 입장
	LC_ROOM_CREATE, // 방 생성
	LC_COMPLETE_SESSION, //게임서버로 이제 갈 준비하라 및 게임서버 IP보내줘야함

	// Lobby -> Game
	LG_ROOMINFO,		//방을 생성하고, 입력된 정보를 통해 클라이언트 인원들을 받아라
	LG_REFAIRROOM,		// (서버하나 다운시) 다른 서버에서있는 정보를 읽어와서 넘겨 기존게임을 이어라

	// Game -> Lobby
	GL_SERVERAMOUNT, // 서버에 얼만큼 차있는지
	GL_ENDGAME,		//서버에서 게임이 끝났다. ( == 클라이언트 정보들을 가져가라)

	// Game -> Client 
	GC_OTHER_MOVEMENT,		//이 객체의 정보를 다른 플레이어들에게 보내야 함.
	GC_POSITION_CORRECTION,	//서버에서 클라이언트 위치 보정 (NEW)
	GC_CHECKATTACK,		//
	GC_LINKGAMESERVER
};

// Client → Server (Game)
constexpr unsigned char CS_MOVE_OBJ_REQUEST = 50;  // 오브젝트 이동 요청
constexpr unsigned char CS_MOVE_MULTI_REQUEST = 51;  // 다중 오브젝트 이동 요청
constexpr unsigned char CS_STOP_OBJ_REQUEST = 52;  // 오브젝트 정지 요청

// Server → Client (Game)
constexpr unsigned char SC_MOVE_OBJ_RESULT = 60;  // 단일 오브젝트 이동 결과
constexpr unsigned char SC_MOVE_MULTI_RESULT = 61;  // 다중 오브젝트 이동 결과
constexpr unsigned char SC_OBJ_POSITION_SYNC = 62;  // 주기적 위치 동기화
constexpr unsigned char SC_MOVE_REJECTED = 63;  // 이동 거부 (핵 감지 등)
constexpr unsigned char SC_HACK_WARNING = 64;  // 핵 경고

struct CSMoveObjRequest {
	unsigned char size;
	unsigned char type;
	unsigned char playerNumber;     // 요청한 플레이어 번호
	unsigned char objNumber;        // 이동시킬 오브젝트 번호
	FXYZ destination;               // 마우스 피킹으로 계산한 목적지 (월드 좌표)

	CSMoveObjRequest() {
		size = sizeof(CSMoveObjRequest);
		type = CS_MOVE_OBJ_REQUEST;
		playerNumber = 0;
		objNumber = 0;
		destination = { 0, 0, 0 };
	}
};

// 다중 오브젝트 이동 요청 (드래그 선택 후 이동)
// 최대 8개 오브젝트 동시 이동
constexpr int MAX_MULTI_MOVE = 8;

struct CSMoveMultiRequest {
	unsigned char size;
	unsigned char type;
	unsigned char playerNumber;
	unsigned char objCount;         // 이동시킬 오브젝트 수
	unsigned char objNumbers[MAX_MULTI_MOVE];
	FXYZ destination;               // 공통 목적지 (서버에서 포메이션 오프셋 계산)

	CSMoveMultiRequest() {
		size = sizeof(CSMoveMultiRequest);
		type = CS_MOVE_MULTI_REQUEST;
		playerNumber = 0;
		objCount = 0;
		memset(objNumbers, 0, sizeof(objNumbers));
		destination = { 0, 0, 0 };
	}
};
// 오브젝트 정지 요청
struct CSStopObjRequest {
	unsigned char size;
	unsigned char type;
	unsigned char playerNumber;
	unsigned char objNumber;

	CSStopObjRequest() {
		size = sizeof(CSStopObjRequest);
		type = CS_STOP_OBJ_REQUEST;
		playerNumber = 0;
		objNumber = 0;
	}
};

// ----------------------------------------------------------
// Server → Client 패킷 구조체
// ----------------------------------------------------------

// 단일 오브젝트 이동 결과 (서버가 검증 완료 후 브로드캐스트)
struct SCMoveObjResult {
	unsigned char size;
	unsigned char type;
	unsigned char ownerPlayer;      // 이 오브젝트의 소유자
	unsigned char objNumber;
	FXYZ currentPos;                // 서버가 확정한 현재 위치
	FXYZ destination;               // 서버가 확정한 목적지
	float speed;                    // 서버가 정한 이동 속도

	SCMoveObjResult() {
		size = sizeof(SCMoveObjResult);
		type = SC_MOVE_OBJ_RESULT;
		ownerPlayer = 0;
		objNumber = 0;
		currentPos = { 0, 0, 0 };
		destination = { 0, 0, 0 };
		speed = 0.0f;
	}
};

// 다중 오브젝트 이동 결과
struct SCMoveMultiResult {
	unsigned char size;
	unsigned char type;
	unsigned char ownerPlayer;
	unsigned char objCount;
	unsigned char objNumbers[MAX_MULTI_MOVE];
	FXYZ destinations[MAX_MULTI_MOVE];  // 각 오브젝트별 확정된 목적지
	float speed;

	SCMoveMultiResult() {
		size = sizeof(SCMoveMultiResult);
		type = SC_MOVE_MULTI_RESULT;
		ownerPlayer = 0;
		objCount = 0;
		memset(objNumbers, 0, sizeof(objNumbers));
		speed = 0.0f;
	}
};

// 주기적 위치 동기화 (서버 → 모든 클라이언트, 100~200ms 주기)
constexpr int MAX_SYNC_OBJECTS = 16;

struct SCSyncEntry {
	unsigned char objNumber;
	unsigned char ownerPlayer;
	FXYZ position;
	FXYZ direction;
};

struct SCPositionSync {
	unsigned char size;
	unsigned char type;
	unsigned char objCount;
	SCSyncEntry entries[MAX_SYNC_OBJECTS];

	SCPositionSync() {
		size = sizeof(SCPositionSync);
		type = SC_OBJ_POSITION_SYNC;
		objCount = 0;
	}
};

// 이동 거부 (클라이언트에게만 전송)
struct SCMoveRejected {
	unsigned char size;
	unsigned char type;
	unsigned char objNumber;
	FXYZ correctedPos;              // 서버가 판단한 올바른 위치
	unsigned char reason;           // 0=충돌, 1=속도초과, 2=권한없음, 3=맵밖

	SCMoveRejected() {
		size = sizeof(SCMoveRejected);
		type = SC_MOVE_REJECTED;
		objNumber = 0;
		correctedPos = { 0, 0, 0 };
		reason = 0;
	}
};

// 핵 경고 (누적 시 킥)
struct SCHackWarning {
	unsigned char size;
	unsigned char type;
	unsigned char warningCount;     // 현재까지 누적 경고 횟수
	unsigned char maxWarnings;      // 최대 허용 횟수

	SCHackWarning() {
		size = sizeof(SCHackWarning);
		type = SC_HACK_WARNING;
		warningCount = 0;
		maxWarnings = 5;
	}
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
	unsigned char objnumber;
	float pos_x, pos_z;
	float dest_x, dest_z;
};






//--------------Client  -->> LobbyServer ---------------
struct CLLobbyLogin
{
	BYTE size;
	Packet_Type type;
	char name[20];
};


struct CLLobbyLogOut
{
	BYTE size;
	Packet_Type type;
	char name[20];
};


struct CLClickQMatching
{
	BYTE size;
	Packet_Type type;
};


struct CLEnterRoomCode
{
	BYTE size;
	Packet_Type type;
	char RoomCode[RoomCodeLen];
};


struct CLCreateRoom
{
	BYTE size;
	Packet_Type type;
	BYTE selected_map;
};


struct CLStartGame {
	BYTE size;
	Packet_Type type;
	char RoomCode[RoomCodeLen];
};

//------------------Client -->> GameServer -----------------

// 오브젝트 생성 ( 캐릭터 or 건물 )
struct CGMaking {
	BYTE size;
	Packet_Type type;
	ObjType Obj;
	float pos_x;
	float pos_y;
};

// 공격 및 이동 ( 1인칭일 때만 활성화 )
struct CGFPerspectAction {
	BYTE size;
	Packet_Type type;
	char act_type;
	FXYZ LookVector;
	FXYZ position;
};

// 오브젝트 이동 (피킹된 객체들만)
struct CGPickingMove {
	BYTE size;
	Packet_Type type;
	BYTE pickingsize;
	int playerNumber;
	char act_command;
	MoveData move_data[12];
};


// 처음 입장, 룸 코드를 통해 게임서버로 연결
struct CGLinkInfo {
	BYTE size;
	Packet_Type type;
	char RoomCode[RoomCodeLen];
	int userID;
};



//-------------- LobbyServer  -->> Client ---------------

// 로그인 및 검색결과 반환
struct LCLogInfo {
	BYTE size;
	Packet_Type type;
	bool exist;
};

// 퀵매칭으로 진입함 (방 생성 후 오토매칭)
struct LCPushMatchingQ
{
	BYTE size;
	Packet_Type type;
	bool done;
};

// 룸코드를 보고 입장가능 확인 반환
struct LCFindRoomCode
{
	BYTE size;
	Packet_Type type;
	bool Fulled;
	bool exist;
};

// 방 생성후 반환
struct LCRoomCreate
{
	BYTE size;
	Packet_Type type;
	char RoomCode[RoomCodeLen];
};

// 게임서버로 이제 갈 준비하라 및 게임서버 IP보내줘야함
struct LCStartGameServer {
	BYTE size;
	Packet_Type type;
	int userNumber;
	char GameServerIP[ipsize];
	int ServerPort;
};


//---------------LobbyServer -> GameServer  --------------
struct LGLobbyToGame
{
	BYTE size;
	Packet_Type type;
	char RoomCode[RoomCodeLen];
	int usernumber1;
	int usernumber2;
};

struct LGRefair {
	BYTE size;
};

//---------------- GameServer -->> LobbyServer -----------------

struct GLServerAmount {
	Packet_Type type;
	BYTE size;
	size_t amount;
};


//---------------- GameServer -->> Clients -----------------

// 다른유저의 데이터, (다른 유저가 피킹한 객체들의 움직임 또는 생성한 건물들)
struct GCReadytoStart {
	BYTE size;
	Packet_Type type;
	bool MapReady;
	bool AllPlayerReady;
};

// 일정거리에 상대편 객체가 와서, 공격할때.
// 공격 command가 아닐때, 공격대해서 같이 공격 command로 바뀔때.
struct GCAttack {
	BYTE size;
	Packet_Type type;
	char objNumber;
	int enemy_playerNumber;
	char enemy_objNumber;
};

// 다른유저의 데이터, (다른 유저가 피킹한 객체들의 움직임 또는 생성한 건물들)
struct GCPickingMove {
	BYTE size;
	Packet_Type type;
	BYTE pickingsize;
	int playerNumber;
	char act_command;
	MoveData move_data[12];
};

// 서버에서 클라이언트 위치 보정 패킷 (NEW)
struct GCPositionCorrection {
	BYTE size;
	Packet_Type type;
	BYTE pickingsize;
	int playerNumber;  // 보정 대상 플레이어
	MoveData corrected_data[12];  // 서버에서 검증된 올바른 위치
};

#pragma pack(pop)
