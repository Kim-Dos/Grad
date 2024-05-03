#include "GameObject.h"

void GameObject::SetPosition(float x, float y, float z)
{
	mPos.x = x;
	mPos.y = y;
	mPos.z = z;
}

void GameObject::SetPosition(FXYZ position)
{
	mPos = position;
}

void GameObject::SetAcceleration(float x, float y, float z)
{
	mAcc.x = x;
	mAcc.y = y;
	mAcc.z = z;
}

void GameObject::SetAcceleration(FXYZ Acceleration)
{
	mAcc = Acceleration;
}

void GameObject::SetVelocity(float x, float y, float z)
{
	mVel.x = x;
	mVel.y = y;
	mVel.z = z;
}

void GameObject::SetVelocity(FXYZ velocity)
{
	mVel = velocity;
}

void GameObject::SetDirection(float x, float y, float z)
{
	mDir.x = x;
	mDir.y = y;
	mDir.z = z;
}

void GameObject::SetDirection(FXYZ direction)
{
	mDir = direction;
}

void GameObject::SetModel(const std::string& model)
{
	mName = model;
}

void GameObject::SetDefault()
{
	mPos = { 0,0,0 };
	mVel = { 0,0,0 };
	mAcc = { 0,0,0 };
	mDir = { 0,0,0 };
	mName = "None";
}