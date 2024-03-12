#include"Player.h"

Player::Player()
{

}

Player::Player(int chartype)
	: mCharaterType(chartype)
{
	SetDefault();
}

Player::Player(int chartype, FXYZ pos, FXYZ vel, FXYZ acc, FXYZ dir)
	: mCharaterType(chartype)
{
	SetPosition(pos);
	SetVelocity(vel);
	SetAcceleration(acc);
	SetDirection(dir);
}



Player::~Player()
{

}

bool Player::mNormalCoolTime()
{
	//if (time > 30 second) return true;
	return false;
}

bool Player::mHyperCoolTime()
{
	//if (time > 2 minute) return true;
	return false;
}
