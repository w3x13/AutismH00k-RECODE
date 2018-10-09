#pragma once

#include "Hacks.h"

Vector GetAutostrafeView();

class CMiscHacks : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool &bSendPacket);
	void FakeWalk0(CUserCmd * pCmd, bool & bSendPacket);

private:
	
	void AutoPistol(CUserCmd * pCmd);


	void SlowMo(CUserCmd * pCmd, bool & bSendPacket);
	void Namespam();
	void NoName();
	void NameSteal();
	
	void AutoJump(CUserCmd *pCmd);
	void RageStrafe(CUserCmd *pCmd);
	void left2();

	void PostProcces();

	void SlowMo2(CUserCmd * pCmd, bool & bSendPacket);

	int CircleFactor = 0;
	
}; extern CMiscHacks* g_Misc;



