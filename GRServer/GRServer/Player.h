#pragma once
#include "Protocol.h"
#include "GameObject.h"



class Player : public GameObject 
{
public:
	Player();
	Player(int chartype);

	Player(int chartype, FXYZ pos, FXYZ vel, FXYZ acc, FXYZ dir);

	~Player();

	bool mNormalCoolTime();
	bool mHyperCoolTime();


private:
	bool mNormalSkill = true;
	int mNormalSkillTimer;

	bool mHyperSkill = true;
	int mHyperSkillTimer;

	int mCharaterType;

	
};