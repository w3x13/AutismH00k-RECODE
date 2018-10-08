#include "Autowall.h"
#include "XorStr.hpp"
#define    HITGROUP_GENERIC    0
#define    HITGROUP_HEAD        1
#define    HITGROUP_CHEST        2
#define    HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4    
#define HITGROUP_RIGHTARM    5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG    7
#define HITGROUP_GEAR        10
inline bool CGameTrace::DidHitWorld() const
{
	return m_pEnt->GetIndex() == 0;
}
inline bool CGameTrace::DidHitNonWorldEntity() const
{
	return m_pEnt != NULL && !DidHitWorld();
}
bool HandleBulletPenetration(CSWeaponInfo *wpn_data, FireBulletData &data);
void trace_line(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, const IClientEntity *ignore, int collisionGroup, trace_t *ptr)
{
	typedef int(__fastcall* UTIL_TraceLine_t)(const Vector&, const Vector&, unsigned int, const IClientEntity*, int, trace_t*);
	static UTIL_TraceLine_t TraceLine = (UTIL_TraceLine_t)Utilities::Memory::FindPatternV2("client_panorama.dll", "55 8B EC 83 E4 F0 83 EC 7C 56 52");
	TraceLine(vecAbsStart, vecAbsEnd, mask, ignore, collisionGroup, ptr);
}
void inline sincos(float radians, float *sine, float *cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);
}
void angle_vectors2(const Vector &angles, Vector *forward) {
	float sp, sy, cp, cy;
	sincos(DEG2RAD(angles[1]), &sy, &cy);
	sincos(DEG2RAD(angles[0]), &sp, &cp);
	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}
void clip_trace_to_player(const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, trace_t* tr)
{
	static DWORD dwAddress = Utilities::Memory::FindPatternV2("client_panorama.dll", "53 8B DC 83 EC 08 83 E4 F0 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 81 EC ? ? ? ? 8B 43 10");
	if (!dwAddress)
		return;
	_asm
	{
		MOV		EAX, filter
		LEA		ECX, tr
		PUSH	ECX
		PUSH	EAX
		PUSH	mask
		LEA		EDX, vecAbsEnd
		LEA		ECX, vecAbsStart
		CALL	dwAddress
		ADD		ESP, 0xC
	}
}
bool trace_to_exitx(Vector& end, trace_t& tr, Vector start, Vector vEnd, trace_t* trace)
{
	typedef bool(__fastcall* TraceToExitFn)(Vector&, trace_t&, float, float, float, float, float, float, trace_t*);
	static TraceToExitFn TraceToExit = (TraceToExitFn)Utilities::Memory::FindPatternV2(XorStr("client_panorama.dll"), XorStr("55 8B EC 83 EC 30 F3 0F 10 75"));
	if (!TraceToExit)
		return false;
	return TraceToExit(end, tr, start.x, start.y, start.z, vEnd.x, vEnd.y, vEnd.z, trace);
}
void calculate_angleX(Vector src, Vector dst, Vector &angles)
{
	Vector delta = src - dst;
	double hyp = delta.Length2D();
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.0f;
	if (delta.x >= 0.0) angles.y += 180.0f;
}
void angle_vectors(const Vector &angles, Vector& forward)
{
	Assert(s_bMathlibInitialized);
	Assert(forward);
	float	sp, sy, cp, cy;
	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));
	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}
inline vec_t vector_normalize(Vector& v)
{
	vec_t l = v.Length();
	if (l != 0.0f)
	{
		v /= l;
	}
	else
	{
		v.x = v.y = 0.0f; v.z = 1.0f;
	}
	return l;
}
float GetHitgroupDamageMult(int iHitGroup)
{
	switch (iHitGroup)
	{
	case HITGROUP_GENERIC:
		return 0.5f;
	case HITGROUP_HEAD:
		return 2.0f;
	case HITGROUP_CHEST:
		return 0.5f;
	case HITGROUP_STOMACH:
		return 0.75f;
	case HITGROUP_LEFTARM:
		return 0.5f;
	case HITGROUP_RIGHTARM:
		return 0.5f;
	case HITGROUP_LEFTLEG:
		return 0.375f;
	case HITGROUP_RIGHTLEG:
		return 0.375f;
	case HITGROUP_GEAR:
		return 0.5f;
	default:
		return 1.0f;
	}
	return 1.0f;
}
void ScaleDamage(int hitgroup, IClientEntity *enemy, float weapon_armor_ratio, float &current_damage)
{
	current_damage *= GetHitgroupDamageMult(hitgroup);
	if (enemy->ArmorValue() > 0)
	{
		if (hitgroup == HITGROUP_HEAD)
		{
			if (enemy->HasHelmet())
				current_damage *= (weapon_armor_ratio);
		}
		else
		{
			current_damage *= (weapon_armor_ratio);
		}
	}
}
bool SimulateFireBullet(IClientEntity *local, C_BaseCombatWeapon *weapon, FireBulletData &data)
{
	if (!local || !local->IsAlive() || !weapon)
		return false;
	data.penetrate_count = 4; // Max Amount Of Penitration
	data.trace_length = 0.0f; // wow what a meme
	auto *wpn_data = weapon->GetCSWpnData(); // Get Weapon Info
	if (!wpn_data)
		return false;
	data.current_damage = (float)wpn_data->damage;// Set Damage Memes
	while ((data.penetrate_count > 0) && (data.current_damage >= 1.0f))
	{
		data.trace_length_remaining = wpn_data->range - data.trace_length;
		Vector End_Point = data.src + data.direction * data.trace_length_remaining;
		trace_line(data.src, End_Point, 0x4600400B, local, 0, &data.enter_trace);
		clip_trace_to_player(data.src, End_Point * 40.f, 0x4600400B, &data.filter, &data.enter_trace);
		if (data.enter_trace.fraction == 1.0f) break;
		if ((data.enter_trace.hitgroup <= 7) && (data.enter_trace.hitgroup > 0) && (local->GetTeamNum() != data.enter_trace.m_pEnt->GetTeamNum()))
		{
			data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
			data.current_damage *= pow(wpn_data->range_modifier, data.trace_length * 0.002);
			ScaleDamage(data.enter_trace.hitgroup, data.enter_trace.m_pEnt, wpn_data->armor_ratio, data.current_damage);
			return true;
		}
		if (!HandleBulletPenetration(wpn_data, data)) break;
	}
	return false;
}
bool HandleBulletPenetration(CSWeaponInfo *wpn_data, FireBulletData &data)
{
	if (!wpn_data)
		return false;
	surfacedata_t *enter_surface_data = Interfaces::PhysProps->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;
	data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
	data.current_damage *= pow(wpn_data->range_modifier, (data.trace_length * 0.002));
	if ((data.trace_length > 3000.f) || (enter_surf_penetration_mod < 0.1f))data.penetrate_count = 0;
	if (data.penetrate_count <= 0)return false;
	Vector dummy;
	trace_t trace_exit;
	if (!trace_to_exitx(dummy, data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit)) return false;
	surfacedata_t *exit_surface_data = Interfaces::PhysProps->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_mod = exit_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;
	if (((data.enter_trace.contents & CONTENTS_GRATE) != 0) || (enter_material == 89) || (enter_material == 71)) { combined_penetration_modifier = 3.0f; final_damage_modifier = 0.05f; }
	else { combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f; }
	if (enter_material == exit_material)
	{
		if (exit_material == 87 || exit_material == 85)combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)combined_penetration_modifier = 2.0f;
	}
	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / wpn_data->penetration) * 1.25f);
	float thickness = VectorLength(trace_exit.endpos - data.enter_trace.endpos);
	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;
	float lost_damage = fmaxf(0.0f, v35 + thickness);
	if (lost_damage > data.current_damage)return false;
	if (lost_damage >= 0.0f)data.current_damage -= lost_damage;
	if (data.current_damage < 1.0f) return false;
	data.src = trace_exit.endpos;
	data.penetrate_count--;
	return true;
}
bool CanHit(const Vector &point, float *damage_given)
{
	auto *local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	auto data = FireBulletData(local->GetOrigin() + local->GetViewOffset());
	data.filter = CTraceFilter();
	data.filter.pSkip = local;
	Vector angles;
	calculate_angleX(data.src, point, angles);
	angle_vectors(angles, data.direction);
	vector_normalize(data.direction);
	if (SimulateFireBullet(local, (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle((HANDLE)local->GetActiveWeaponHandle()), data))
	{
		*damage_given = data.current_damage;
		return true;
	}
	return false;
}
bool trace_autowallable(float& dmg) {
	auto m_local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (m_local && m_local->IsAlive()) {
		FireBulletData data = FireBulletData(m_local->GetEyePosition());
		data.filter = CTraceFilter();
		data.filter.pSkip = m_local;
		Vector eye_angle; Interfaces::Engine->GetViewAngles(eye_angle);
		Vector dst, forward;
		angle_vectors2(eye_angle, &forward);
		dst = data.src + (forward * 8196.f);
		Vector angles;
		calculate_angleX(data.src, dst, angles);
		angle_vectors2(angles, &data.direction);
		vector_normalize(data.direction);
		auto m_weapon = m_local->GetWeapon2();
		if (m_weapon) {
			data.penetrate_count = 1;
			data.trace_length = 0.0f;
			CSWeaponInfo *weapon_data = m_weapon->GetCSWpnData();
			if (weapon_data) {
				data.current_damage = weapon_data->damage;
				data.trace_length_remaining = weapon_data->range - data.trace_length;
				Vector end = data.src + data.direction * data.trace_length_remaining;
				trace_line(data.src, end, MASK_SHOT | CONTENTS_GRATE, m_local, 0, &data.enter_trace);
				if (data.enter_trace.fraction == 1.0f) return false;
				if (HandleBulletPenetration(weapon_data, data)) {
					dmg = data.current_damage;
					return true;
				}
			}
		}
	}
	return false;
}