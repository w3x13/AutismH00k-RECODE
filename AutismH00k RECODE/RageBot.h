#pragma once
#include "Hacks.h"
struct AntiaimData_t
{
	AntiaimData_t(const float& dist, const bool& inair, int player)
	{
		this->flDist = dist;
		this->bInAir = inair;
		this->iPlayer = player;
	}
	float flDist;
	bool bInAir;
	int	iPlayer;
};
class CRageBot : public CHack
{
public:
	bool bUsingFakeAngles[65];
	void Init();
	void Draw();
	void SyncSettings();
	int HitScan(IClientEntity* pEntity);
	void Fakelag(CUserCmd * pCmd, bool & bSendPacket);
	bool ValidEntity(int i);
	void DoAimbot(CUserCmd *pCmd, bool &bSendPacket);
	void DoNoRecoil(CUserCmd *pCmd);
	void Move(CUserCmd *pCmd, bool &bSendPacket);
	float NextPredictedLBYUpdate = 0.f;

	
private:
	std::vector<AntiaimData_t> Entities;
	int GetTargetCrosshair();
	int GetTargetDistance();
	int GetTargetNextShot();
	int GetTargetThreat(CUserCmd * pCmd);
	int GetTargetHealth();
	bool TargetMeetsRequirements(IClientEntity* pEntity);
	void aimAtPlayer(CUserCmd * pCmd);
	bool AimAtPoint(IClientEntity * pLocal, Vector point, CUserCmd * pCmd, bool & bSendPacket);
	float FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int HitBox);
	void DoPitch(CUserCmd * pCmd);
	void fw(CUserCmd * pCmd, bool & bSendPacket);
	void anti_lby(CUserCmd * cmd, bool & bSendPacket);
	void DoAntiAim(CUserCmd *pCmd, bool&bSendPacket);
	bool IsAimStepping;
	Vector LastAimstepAngle;
	Vector LastAngle;
	IClientEntity * pTarget;
	bool IsLocked;
	int TargetID;
	int HitBox;
	Vector AimPoint;
};
extern CRageBot ragebot;