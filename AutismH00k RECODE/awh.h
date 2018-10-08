
#pragma once

#include "SDK.h"
#include "singleton.hpp"
#include "MathFunctions.h"


struct ImpactInfo {

	float x, y, z;
	long long time;
};

struct HitmarkerInfo {

	ImpactInfo pImpact;
	int pAlpha;
	int pDamage;
};

class Hitmarker : IGameEventListener2 {

public:
	void Initialize();
	void Paint();

private:
	void FireGameEvent(IGameEvent* pEvent) override;
	int GetEventDebugID(void) override;
	void PlayerHurt(IGameEvent* pEvent);
	void BulletImpact(IGameEvent* pEvent);
	std::vector<ImpactInfo> pImpacts;
	std::vector<HitmarkerInfo> pHitmarkers;
};

extern Hitmarker* pHitmarker;