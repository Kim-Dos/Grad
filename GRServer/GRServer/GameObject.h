#pragma once
#include"stdafx.h"
#include "MathHelper.h"
#include "Protocol.h"

class Object 
{
public:

	Object();

	~Object() {}


	//-----------------------------------------------------------
	// Getter
	//-----------------------------------------------------------
	std::string GetName() { return mName; }

	FXYZ GetPos() const { return mPos; }

	inline float GetPosX() const { return mPos.x; }
	inline float GetPosY() const { return mPos.y; }
	inline float GetPosZ() const { return mPos.z; }

	//-----------------------------------------------------------
	// Setter
	//-----------------------------------------------------------
	void SetPosition(float x, float y, float z);
	void SetPosition(FXYZ position);


	void SetModelname(const std::string& modelname);

protected:
	std::string mName; // Model Name

	FXYZ mPos;
};


class GameActor : public Object
{
public:
	
	GameActor();

	~GameActor() {}


	//-----------------------------------------------------------
	// Getter
	//-----------------------------------------------------------
	
	inline FXYZ GetDir() const { return mDir; }
	inline FXYZ GetAcc() const { return mAcc; }
	inline FXYZ GetVel() const { return mVel; }

	//void MoveStrafe(float distance = 1.0f);
	//void MoveUp(float distance = 1.0f);
	//void MoveForward(float distance = 1.0f);
	//void MoveFront(float distance = 1.0f);

	void Rotate(float pitch, float yaw, float roll);

	//-----------------------------------------------------------
	// Setter
	//-----------------------------------------------------------

	void SetAcceleration(float x, float y, float z);
	void SetAcceleration(FXYZ Acceleration);
	void SetVelocity(float x, float y, float z);
	void SetVelocity(FXYZ velocity);
	void SetDirection( float x, float y, float z );
	void SetDirection(FXYZ dir);

	void SetDefault();

protected:
	int userNumber; // User Number 1 or 2 ( 0 is None )

	FXYZ mVel;
	FXYZ mAcc;
	FXYZ mDir;

};