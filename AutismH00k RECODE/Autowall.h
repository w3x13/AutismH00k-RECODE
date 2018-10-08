#pragma once
#include "Interfaces.h"
struct FireBulletData
{
	FireBulletData(const Vector &eye_pos)
		: src(eye_pos)
	{
	}

	Vector           src;
	trace_t          enter_trace;
	Vector           direction;
	CTraceFilter    filter;
	float           trace_length;
	float           trace_length_remaining;
	float           current_damage;
	int             penetrate_count;
};
bool CanHit(const Vector &point, float *damage_given);
bool trace_autowallable(float& dmg);
//bool SimulateFireBullet(IClientEntity * ent, IClientEntity *local, C_BaseCombatWeapon *weapon, FireBulletData &data);


namespace cut_wrists
{
	extern bool yes;
}

