#include "GameObject.h"

Object::Object() {

	mPos = { 0,0,0 };
}

void Object::SetModelname(const std::string& modelname)
{
	mName = modelname;
}

void Object::SetPosition(float x, float y, float z)
{
	mPos.x = x;
	mPos.y = y;
	mPos.z = z;
}

void Object::SetPosition(FXYZ position)
{
	mPos = position;
}
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------
//------------------------------------------------------------------


GameActor::GameActor() : Object()
{
	userNumber = 0;
	mVel = { 0,0,0 };
	mAcc = { 0,0,0 };
	mDir = { 0,0,0 };
}

void GameActor::SetAcceleration(float x, float y, float z)
{
	mAcc.x = x;
	mAcc.y = y;
	mAcc.z = z;
}

void GameActor::SetAcceleration(FXYZ Acceleration)
{
	mAcc = Acceleration;
}

void GameActor::SetVelocity(float x, float y, float z)
{
	mVel.x = x;
	mVel.y = y;
	mVel.z = z;
}

void GameActor::SetVelocity(FXYZ velocity)
{
	mVel = velocity;
}

void GameActor::SetDirection(float x, float y, float z)
{
	mDir.x = x;
	mDir.y = y;
	mDir.z = z;
}

void GameActor::SetDirection(FXYZ direction)
{
	mDir = direction;
}



void GameActor::SetDefault()
{
	mVel = { 0,0,0 };
	mAcc = { 0,0,0 };
	mDir = { 0,0,0 };
}