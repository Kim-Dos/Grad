#pragma once
#include "Protocol.h"
#include "GameObject.h"



class Player : public GameObject 
{
public:
	Player();
	Player(int CharType);

	Player(int CharType, FXYZ pos, FXYZ vel, FXYZ Acc, FXYZ dir);

	~Player();

	bool mNormalCoolTime();
	bool mHyperCoolTime();

private:
	bool mNormalSkill = true;
	int mNormalSkillTimer;

	bool mHyperSkill = true;
	int mHyperSkillTimer;
};