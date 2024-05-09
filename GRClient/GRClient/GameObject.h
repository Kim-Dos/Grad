#pragma once
//#include"stdafx.h"
//#include "MathHelper.h"
#include "..\..\..\Grad\GRServer\GRServer\Protocol.h"

using namespace std;



class GameObject
{
public:
	
	GameObject() {}

	~GameObject() {}


	//-----------------------------------------------------------
	// Getter
	//-----------------------------------------------------------
	std::string GetName() { return mName; }

	FXYZ GetPos() const { return mPos; }

	inline float GetPosX() const { return mPos.x; }
	inline float GetPosY() const { return mPos.y; }
	inline float GetPosZ() const { return mPos.z; }
	
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
	void SetPosition(float x, float y, float z);
	void SetPosition(FXYZ position);
	void SetAcceleration(float x, float y, float z);
	void SetAcceleration(FXYZ Acceleration);
	void SetVelocity(float x, float y, float z);
	void SetVelocity(FXYZ velocity);
	void SetDirection( float x, float y, float z );
	void SetDirection(FXYZ dir);

	void SetDefault();

	void SetModel(const std::string& model);







protected:
	std::string mName;

	FXYZ mPos;
	FXYZ mVel;
	FXYZ mAcc;
	FXYZ mDir;


};

