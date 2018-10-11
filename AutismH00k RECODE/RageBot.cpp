#include "RageBot.h"
#include "RenderManager.h"
#include "Resolver.h"
#include "Autowall.h"
#include "edge.h"
#include <iostream>
#include <time.h>
#include "UTIL Functions.h"
#include "xostr.h"
#include <chrono>
#include "Hooks.h"
#include "global_count.h"

#define XM_2PI              6.283185307f

float current_real;
Vector LastAngleAA2;
static bool dir = false;
static bool back = false;
static bool up = false;
static bool jitter = false;

static bool backup = false;
static bool default_aa = true;
static bool panic = false;
float hitchance_custom;
bool is_broken;
#define TICK_INTERVAL			(Interfaces::Globals->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

void CRageBot::Init()
{
	IsAimStepping = false;
	IsLocked = false;
	TargetID = -1;
}
void CRageBot::Draw()
{
}
float curtime_fixed(CUserCmd* ucmd) {
	auto local_player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * Interfaces::Globals->interval_per_tick;
	return curtime;
}

int GetFPS()
{
	static int fps = 0;
	static int count = 0;
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	static auto last = high_resolution_clock::now();
	count++;

	if (duration_cast<milliseconds>(now - last).count() > 1000)
	{
		fps = count;
		count = 0;
		last = now;
	}

	return fps;
}
/*
void next_lby_update_func(CUserCmd* m_pcmd, const float yaw_to_break)
{
	auto m_local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	static int choked = 0;
	static bool wasMoving = true;
	static bool preBreak = false;
	static bool shouldBreak = false;
	static bool brokeThisTick = false;
	static float oldCurtime = curtime_fixed(m_pcmd);

	if (Options::Menu.MiscTab.antilby.GetState()) return;
	if (((m_pcmd->buttons & IN_USE) || (m_pcmd->buttons & IN_ATTACK) || m_local->GetMoveType() == MOVETYPE_LADDER))
		return ;
	if (!Interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::MOUSE_LEFT)) {


		brokeThisTick = false;
		choked = Interfaces::m_pClientState->chokedcommands;

		if (m_local->GetVelocity().Length2D() >= 0.1f && (m_local->GetFlags() & FL_ONGROUND))
		{
			oldCurtime = Interfaces::Globals->curtime;
			wasMoving = true;
		}
		else
		{
			if (wasMoving && Interfaces::Globals->curtime - oldCurtime > 0.22f)
			{
				wasMoving = false;
				shouldBreak = true;
				GlobalBREAK::NextPredictedLBYUpdate = Interfaces::Globals->curtime;
			}
			else if (Interfaces::Globals->curtime - oldCurtime > 1.1f)
			{
				shouldBreak = true;
				GlobalBREAK::NextPredictedLBYUpdate = Interfaces::Globals->curtime;
			}
			else if (Interfaces::Globals->curtime - oldCurtime > 1.1f - TICKS_TO_TIME(choked) - TICKS_TO_TIME(2))
				preBreak = true;
		}
		if (shouldBreak)
		{
			shouldBreak = false;
			brokeThisTick = true;
			oldCurtime = Interfaces::Globals->curtime;
			m_pcmd->viewangles.y += yaw_to_break;
		}
		if (preBreak)
		{
			preBreak = false;
			brokeThisTick = true;
			float addAngle = GetFPS() >= (TIME_TO_TICKS(1.f) * 0.5f) ? (2.9 * max(choked, GlobalBREAK::prevChoked) + 100) : 145.f;
			m_pcmd->viewangles.y += 75 + addAngle;
		}


	}
} */

inline float RandomFloat(float min, float max)
{
	static auto fn = (decltype(&RandomFloat))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat"));
	return fn(min, max);
}

static bool wasMoving = true;
static bool preBreak = false;
static bool shouldBreak = false;
static bool brokeThisTick = false;
static bool fake_walk = false;
static int chocked = 0;
static bool gaymode = false;
static bool doubleflick = false;
void CRageBot::anti_lby(CUserCmd* cmd, bool& bSendPacket)
{
	if (Options::Menu.MiscTab.antilby.GetIndex() < 1)
		return;

	auto local_player = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (!local_player)
		return;

	auto local_weapon = local_player->GetActiveWeaponHandle();

	if (!local_weapon)
		return;

	static float oldCurtime = Interfaces::Globals->curtime;

	if (bSendPacket)
	{
		brokeThisTick = false;
		chocked = Interfaces::m_pClientState->chokedcommands;

		if (local_player->GetVelocity().Length2D() >= 0.1f && (local_player->GetFlags() & FL_ONGROUND))
		{
			if (GetAsyncKeyState(VK_SHIFT))
			{
				wasMoving = false;
				oldCurtime = Interfaces::Globals->curtime;
				if (Interfaces::Globals->curtime - oldCurtime >= 1.1f)
				{
					shouldBreak = true;
					NextPredictedLBYUpdate = Interfaces::Globals->curtime;
				}
			}
			else
			{
				oldCurtime = Interfaces::Globals->curtime;
				wasMoving = true;
			}
		}

		else
		{
			if (wasMoving &&Interfaces::Globals->curtime - oldCurtime > 0.22f)
			{
				wasMoving = false;
				shouldBreak = true;
				NextPredictedLBYUpdate = Interfaces::Globals->curtime;
			}

			else if (Interfaces::Globals->curtime - oldCurtime > 1.1f)
			{
				shouldBreak = true;
				NextPredictedLBYUpdate = Interfaces::Globals->curtime;
			}

			else if (Interfaces::Globals->curtime - oldCurtime > 1.1f - TICKS_TO_TIME(chocked) - TICKS_TO_TIME(2))
				preBreak = true;

			else if (Interfaces::Globals->curtime - oldCurtime > 1.f - TICKS_TO_TIME(chocked + 4))
				gaymode = true;

			else if (Interfaces::Globals->curtime - oldCurtime > 1.f - TICKS_TO_TIME(chocked + 12))
				doubleflick = true;
		}
	}

	else if (shouldBreak)
	{
		if (Options::Menu.MiscTab.antilby.GetIndex() == 1)
		{
			brokeThisTick = true;
			oldCurtime = Interfaces::Globals->curtime;
			cmd->viewangles.y = cmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue();
			shouldBreak = false;
		}
		else if (Options::Menu.MiscTab.antilby.GetIndex() > 1)
		{
			brokeThisTick = true;
			oldCurtime = Interfaces::Globals->curtime;
			cmd->viewangles.y = cmd->viewangles.y + 90;
			shouldBreak = false;
		}
	}

	else if (preBreak && Options::Menu.MiscTab.antilby.GetIndex() > 1)
	{
		brokeThisTick = true;
		float addAngle = GetFPS() >= (TIME_TO_TICKS(1.f) * 0.5f) ? (2.3789 * max(chocked, GlobalBREAK::prevChoked) + 100) : 145.f;
		cmd->viewangles.y = cmd->viewangles.y + 105 + addAngle;
		preBreak = false;
	}
}
void FakeMove(CUserCmd* cmd)
{
	IClientEntity *g_LocalPlayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!g_LocalPlayer->IsAlive())
		return;
	if (g_LocalPlayer->IsAlive()) {

		Vector velocity = GlobalBREAK::vecUnpredictedVel;
		if (velocity.Length2D() < 0.1f)
			return;
		int32_t ticks_to_stop;
		for (ticks_to_stop = 0; ticks_to_stop < 15; ticks_to_stop++)
		{
			if (velocity.Length2D() < 0.1f)
				break;
			static auto predict_velocity = [](Vector* velocity)
			{
				float speed = velocity->Length();
				static auto sv_friction = Interfaces::CVar->FindVar("sv_friction");
				static auto sv_stopspeed = Interfaces::CVar->FindVar("sv_stopspeed");
				if (speed >= 0.1f)
				{
					float friction = sv_friction->GetFloat();
					float stop_speed = std::max< float >(speed, sv_stopspeed->GetFloat());
					float time = std::max< float >(Interfaces::Globals->interval_per_tick, Interfaces::Globals->frametime);
					*velocity *= std::max< float >(0.f, speed - friction * stop_speed * time / speed);
				}
			};

			predict_velocity(&velocity);
			if (g_LocalPlayer->GetFlags() & FL_ONGROUND)
			{
				velocity[2] = 0.5f;
				//Interfaces::GameMovement->Friction(g_LocalPlayer); this is needed
				//Interfaces::GameMovement->WalkMove(g_LocalPlayer);
			}
		}
		const int32_t max_ticks = 14;
		const int32_t chocked = GlobalBREAK::ChokeAmount;
		int32_t ticks_left = max_ticks - chocked;

		if (ticks_to_stop < ticks_left && !GlobalBREAK::bSendPacket)
		{
			cmd->viewangles.y += Options::Menu.MiscTab.BreakLBYDelta.GetValue();
		}
	}
}

float curtime_fixedX(CUserCmd* ucmd) {
	auto local_player = hackManager.pLocal();
	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * Interfaces::Globals->interval_per_tick;
	return curtime;
}
bool next_lby_update_funcX(CUserCmd* m_pcmd, const float yaw_to_break) {
	auto m_local = hackManager.pLocal();
	if (m_local) {
		static float last_attempted_yaw;
		static float old_lby;
		static float next_lby_update_time;
		const float current_time = curtime_fixedX(m_pcmd); 

		if (old_lby != m_local->GetLowerBodyYaw() && last_attempted_yaw != m_local->GetLowerBodyYaw()) {
			old_lby = m_local->GetLowerBodyYaw();
			if (m_local->GetVelocity().Length2D() < 0.1) {
				auto latency = (Interfaces::Engine->GetNetChannelInfo()->GetAvgLatency(FLOW_INCOMING) + Interfaces::Engine->GetNetChannelInfo()->GetAvgLatency(FLOW_OUTGOING));
				next_lby_update_time = current_time + 1.1f;
			}
		}
		if (m_local->GetVelocity().Length2D() < 0.1) {
			if ((next_lby_update_time < current_time) && m_local->GetFlags() & FL_ONGROUND) {
				last_attempted_yaw = yaw_to_break;
				next_lby_update_time = current_time + 1.1f;
				return true;
			}
		}
	}

	return false;
}
void fake979(CUserCmd* cmd)
{
	if (Options::Menu.MiscTab.antilby.GetIndex() < 1)
		return;
	if (GetAsyncKeyState(VK_SPACE))
		return;
	IClientEntity *g_LocalPlayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (g_LocalPlayer->IsAlive()) {
		static int choked = 0;
		static bool wasMoving = true;
		static bool preBreak = false;
		static bool shouldBreak = false;
		static bool brokeThisTick = false;
		static float oldCurtime = Interfaces::Globals->curtime;
		if (GlobalBREAK::bSendPacket)
		{
			brokeThisTick = false;
			choked = Interfaces::m_pClientState->chokedcommands;
			if (next_lby_update_funcX(cmd, Options::Menu.MiscTab.BreakLBYDelta.GetValue()))
			{
				shouldBreak = true;
				GlobalBREAK::NextPredictedLBYUpdate = Interfaces::Globals->curtime;
			}
			float value = 55.f;
			if (g_LocalPlayer->GetVelocity().Length2D() >= value)
			{

				oldCurtime = Interfaces::Globals->curtime;
				wasMoving = true;
			}
			else
			{
				if (wasMoving && next_lby_update_funcX(cmd, Options::Menu.MiscTab.BreakLBYDelta.GetValue()))
				{
					wasMoving = false;
					shouldBreak = true;
					FakeMove(cmd);
					GlobalBREAK::NextPredictedLBYUpdate = Interfaces::Globals->curtime;
				}
				if (next_lby_update_funcX(cmd, Options::Menu.MiscTab.BreakLBYDelta.GetValue()))
				{
					shouldBreak = true;
					GlobalBREAK::NextPredictedLBYUpdate = Interfaces::Globals->curtime;
				}
				if (Interfaces::Globals->curtime - oldCurtime > 1.1f - TICKS_TO_TIME(choked) - TICKS_TO_TIME(2) || next_lby_update_funcX(cmd, 119))
					preBreak = true;
			}
		}
		else if (shouldBreak)
		{
			shouldBreak = false;
			brokeThisTick = true;
			oldCurtime = Interfaces::Globals->curtime;
			if (!GetAsyncKeyState(VK_LSHIFT))
				FakeMove(cmd);
			if (GetAsyncKeyState(VK_LSHIFT))
				cmd->viewangles.y += 180;
			else if (!GetAsyncKeyState(VK_LSHIFT))
				cmd->viewangles.y += Options::Menu.MiscTab.BreakLBYDelta.GetValue();
		}
		else if (preBreak)
		{
			preBreak = false;
			brokeThisTick = true;
			if (!GetAsyncKeyState(VK_LSHIFT))
				FakeMove(cmd);
			float addAngle = GetFPS() >= (TIME_TO_TICKS(1.f) * 0.5f) ? (2.3789 * max(choked, GlobalBREAK::prevChoked) + 99.9) : 144.9f;
			if (!GetAsyncKeyState(VK_LSHIFT))
				cmd->viewangles.y += Options::Menu.MiscTab.BreakLBYDelta.GetValue() + addAngle;
			GlobalBREAK::breakangle = Options::Menu.MiscTab.BreakLBYDelta.GetValue() + addAngle;
		}
		if (g_LocalPlayer->IsAlive()) {
			if (g_LocalPlayer->GetWeapon2())
			{
				if (!g_LocalPlayer->GetWeapon2()->IsMiscGAY())
					cmd->buttons &= ~IN_ATTACK;
			}
		}
	}
}

int BreakLagCompensation()
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	auto g_LocalPlayer = pLocal;
	Vector velocity = g_LocalPlayer->GetVelocity();
	velocity.z = 0;
	float speed = velocity.Length();
	if (speed > 0.f)
	{
		auto distance_per_tick = speed * Interfaces::Globals->interval_per_tick;
		int choked_ticks = std::ceilf(65.f / distance_per_tick);
		return std::min<int>(choked_ticks, 14);
	}
}

void NormaliseViewAngle(Vector &angle)
{
	while (angle.y <= -180) angle.y += 360;
	while (angle.y > 180) angle.y -= 360;
	while (angle.x <= -180) angle.x += 360;
	while (angle.x > 180) angle.x -= 360;


	if (angle.x > 89.0f)
		angle.x = 89.0f;
	else if (angle.x < -89.0f)
		angle.x = -89.0f;

	if (angle.y > 179.99999f)
		angle.y = 179.99999f;
	else if (angle.y < -179.99999f)
		angle.y = -179.99999f;

	angle.z = 0;

}
inline void RandomSeed(int seed)
{
	static auto fn = (decltype(&RandomSeed))(GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomSeed"));

	return fn(seed);
}


void CRageBot::Fakelag(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	
	if (GameUtils::IsRevolver(pWeapon))
		return;

	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (GameUtils::IsBomb(pWeapon))
		return;

	static auto choked = 0;
	const auto FUCKING_IMMIGRANTS = 2; 
	const auto max_fake_lag3 = Options::Menu.MiscTab.FakelagStand.GetValue();
	const auto max_fake_lag = Options::Menu.MiscTab.FakelagBreakLC.GetState() ? BreakLagCompensation() : Options::Menu.MiscTab.FakelagMove.GetValue();
	const auto max_fake_lag2 = Options::Menu.MiscTab.FakelagBreakLC.GetState() ? BreakLagCompensation() : Options::Menu.MiscTab.Fakelagjump.GetValue();
	const auto legit_fakelag = Options::Menu.LegitBotTab.aafl.GetValue();

		if ((pCmd->buttons & IN_ATTACK))
		{
			if (Options::Menu.MiscTab.fl_spike.GetIndex() == 1)
			{
				if (max_fake_lag3 < 6)
				{
					if (choked < (max_fake_lag3 * 2))
					{
						choked++;
						bSendPacket = false;
					}
					else
					{
						choked = 0;
						bSendPacket = true;
					}
				}
				else if (max_fake_lag3 >= 6 && max_fake_lag3 < 13)
				{
					if (choked < (max_fake_lag3 + 2))
					{
						choked++;
						bSendPacket = false;
					}
					else
					{
						choked = 0;
						bSendPacket = true;
					}
				}
				else
				{
					if (choked < (max_fake_lag3 + 1))
					{
						choked++;
						bSendPacket = false;
					}
					else
					{
						choked = 0;
						bSendPacket = true;
					}
				}

			}
			else
			{

				if (choked < FUCKING_IMMIGRANTS - 1)
				{
					choked++;
					bSendPacket = false;
				}
				else
				{
					choked = 0;
					bSendPacket = true;
				}

			}
		}
		else
		{
			if (Options::Menu.MiscTab.fl_spike.GetIndex() == 2 && pLocal->GetBasePlayerAnimState()->m_bInHitGroundAnimation)
			{
				if (choked < 9)
				{
					choked++;
					bSendPacket = false;
				}
				else
				{
					choked = 13;
					bSendPacket = true;
				}
			}
			else
			{
				if (choked < max_fake_lag && pLocal->GetVelocity().Length2D() >= 1 && (pLocal->GetFlags() & FL_ONGROUND))
				{
					choked++;
					bSendPacket = false;
				}
				else if (choked < max_fake_lag2 && pLocal->GetVelocity().Length2D() >= 1 && !(pLocal->GetFlags() & FL_ONGROUND))
				{
					choked++;
					bSendPacket = false;
				}
				else if (choked < max_fake_lag3 && pLocal->GetVelocity().Length2D() < 1)
				{
					choked++;
					bSendPacket = false;
				}
				else
				{
					choked = 0;
					bSendPacket = true;
				}
			}
		}
}
bool IsAbleToShoot(IClientEntity* pLocal)
{
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (!pLocal)return false;
	if (!pWeapon)return false;
	float flServerTime = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
	return (!(pWeapon->GetNextPrimaryAttack() > flServerTime));
}
float hitchance(IClientEntity* pLocal, C_BaseCombatWeapon* pWeapon)
{
	float hitchance = 101;
	if (!pWeapon) return 0;
	if (Options::Menu.RageBotTab.AccuracyHitchance.GetValue() > 0 || hitchance_custom > 0)
	{
		float inaccuracy = pWeapon->GetInaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
	}
	return hitchance;
}
bool CanOpenFire()
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity)
		return false;
	C_BaseCombatWeapon* entwep = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocalEntity->GetActiveWeaponHandle());
	float flServerTime = (float)pLocalEntity->GetTickBase() * Interfaces::Globals->interval_per_tick;
	float flNextPrimaryAttack = entwep->GetNextPrimaryAttack();
	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;
	return !(flNextPrimaryAttack > flServerTime);
}
template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}
float GetLerpTimeX()
{
	int ud_rate = Interfaces::CVar->FindVar("cl_updaterate")->GetFloat();
	ConVar *min_ud_rate = Interfaces::CVar->FindVar("sv_minupdaterate");
	ConVar *max_ud_rate = Interfaces::CVar->FindVar("sv_maxupdaterate");
	if (min_ud_rate && max_ud_rate)
		ud_rate = max_ud_rate->GetFloat();
	float ratio = Interfaces::CVar->FindVar("cl_interp_ratio")->GetFloat();
	if (ratio == 0)
		ratio = 1.0f;
	float lerp = Interfaces::CVar->FindVar("cl_interp")->GetFloat();
	ConVar *c_min_ratio = Interfaces::CVar->FindVar("sv_client_min_interp_ratio");
	ConVar *c_max_ratio = Interfaces::CVar->FindVar("sv_client_max_interp_ratio");
	if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
		ratio = clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());
	return max(lerp, (ratio / ud_rate));
}
float InterpFix()
{
	static ConVar* cvar_cl_interp = Interfaces::CVar->FindVar("cl_interp");
	static ConVar* cvar_cl_updaterate = Interfaces::CVar->FindVar("cl_updaterate");
	static ConVar* cvar_sv_maxupdaterate = Interfaces::CVar->FindVar("sv_maxupdaterate");
	static ConVar* cvar_sv_minupdaterate = Interfaces::CVar->FindVar("sv_minupdaterate");
	static ConVar* cvar_cl_interp_ratio = Interfaces::CVar->FindVar("cl_interp_ratio");
	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	float cl_interp = cvar_cl_interp->GetFloat();
	int cl_updaterate = cvar_cl_updaterate->GetInt();
	int sv_maxupdaterate = cvar_sv_maxupdaterate->GetInt();
	int sv_minupdaterate = cvar_sv_minupdaterate->GetInt();
	int cl_interp_ratio = cvar_cl_interp_ratio->GetInt();
	if (sv_maxupdaterate <= cl_updaterate)
		cl_updaterate = sv_maxupdaterate;
	if (sv_minupdaterate > cl_updaterate)
		cl_updaterate = sv_minupdaterate;
	float new_interp = (float)cl_interp_ratio / (float)cl_updaterate;
	if (new_interp > cl_interp)
		cl_interp = new_interp;
	return max(cl_interp, cl_interp_ratio / cl_updaterate);
}
void CRageBot::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocalEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!pLocalEntity || !Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;
	Fakelag(pCmd, bSendPacket);
	if (Options::Menu.MiscTab.AntiAimEnable.GetState())
	{
		static int ChokedPackets = 1;
		C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
		if (!pWeapon)
			return;
		if (ChokedPackets < 1 && pLocalEntity->GetLifeState() == LIFE_ALIVE /*&& pCmd->buttons & IN_ATTACK && CanOpenFire() */ && GameUtils::IsBallisticWeapon(pWeapon))
		{
			bSendPacket = false;
		}
		else
		{
			if (pLocalEntity->GetLifeState() == LIFE_ALIVE)
			{
				DoAntiAim(pCmd, bSendPacket);
			}
			ChokedPackets = 1;
		}
	}
	float simtime = 0;
	float current_aim_simulationtime = simtime;
	if (Options::Menu.RageBotTab.AimbotEnable.GetState() && !Options::Menu.RageBotTab.lag_pred.GetState())
	{
		pCmd->tick_count = TIME_TO_TICKS(GetLerpTimeX());
		DoAimbot(pCmd, bSendPacket);
		DoNoRecoil(pCmd);
	}
	else if (Options::Menu.RageBotTab.AimbotEnable.GetState() && Options::Menu.RageBotTab.lag_pred.GetState())
	{
		pCmd->tick_count = TIME_TO_TICKS(current_aim_simulationtime) + TIME_TO_TICKS(GetLerpTimeX());
		DoAimbot(pCmd, bSendPacket);
		DoNoRecoil(pCmd);
	}
	if (Options::Menu.RageBotTab.AimbotAimStep.GetState())
	{
		Vector AddAngs = pCmd->viewangles - LastAngle;
		if (AddAngs.Length2D() > 25.f)
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 25;
			pCmd->viewangles = LastAngle + AddAngs;
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
		}
	}
	LastAngle = pCmd->viewangles;
}
Vector BestPoint(IClientEntity *targetPlayer, Vector &final)
{
	IClientEntity* pLocal = hackManager.pLocal();
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = targetPlayer;
	ray.Init(final + Vector(0, 0, Options::Menu.RageBotTab.pointscaleval.GetValue() / 10), final);
	Interfaces::Trace->TraceRay(ray, MASK_SHOT, &filter, &tr);
	final = tr.endpos;
	return final;
}
inline float FastSqrt(float x)
{
	unsigned int i = *(unsigned int*)&x;
	i += 127 << 23;
	i >>= 1;
	return *(float*)&i;

}
#define square( x ) ( x * x )
void ClampMovement(CUserCmd* pCommand, float fMaxSpeed)
{
	if (fMaxSpeed <= 0.f)
		return;
	float fSpeed = (float)(FastSqrt(square(pCommand->forwardmove) + square(pCommand->sidemove) + square(pCommand->upmove)));
	if (fSpeed <= 0.f)
		return;
	if (pCommand->buttons & IN_DUCK)
		fMaxSpeed *= 2.94117647f; // TO DO: Maybe look trough the leaked sdk for an exact value since this is straight out of my ass...
	if (fSpeed <= fMaxSpeed)
		return;
	float fRatio = fMaxSpeed / fSpeed;
	pCommand->forwardmove *= fRatio;
	pCommand->sidemove *= fRatio;
	pCommand->upmove *= fRatio;
}



void CRageBot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{

	if (!Options::Menu.RageBotTab.AimbotEnable.GetState())
		return;
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	bool FindNewTarget = true;
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (pWeapon != nullptr)
	{
		if (pWeapon->GetAmmoInClip() == 0 || !GameUtils::IsBallisticWeapon(pWeapon) || GameUtils::IsGrenade(pWeapon)) return;
	}
	else
		return;
	if (GameUtils::IsRevolver(pWeapon))
	{
		static int pasteme = 0;
		pasteme++;
		if (pasteme <= 14.5f) {
			pCmd->buttons |= IN_ATTACK;
		}
		else {
			pasteme = 0;

			float flPostponeFireReady = pWeapon->GetFireReadyTime();
			if (flPostponeFireReady > 0 && flPostponeFireReady < Interfaces::Globals->curtime) {
				pCmd->buttons &= ~IN_ATTACK;
			}
		}
		}
		

	// Make sure we have a good target
	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{
		pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			HitBox = HitScan(pTarget);
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
				Vector View; Interfaces::Engine->GetViewAngles(View);
				float FoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (FoV < Options::Menu.RageBotTab.AimbotFov.GetValue())
					FindNewTarget = false;
			}
		}
	}
	if (FindNewTarget)
	{
		Globals::Shots = 0;
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;
		TargetID = GetTargetCrosshair();
		if (TargetID >= 0)
		{
			pTarget = Interfaces::EntList->GetClientEntity(TargetID);
		}
	}
	if (TargetID >= 0 && pTarget)
	{
		HitBox = HitScan(pTarget);
		if (!CanOpenFire()) return;
		if (Options::Menu.RageBotTab.QuickStop.GetState() && pLocal->GetFlags() & FL_ONGROUND)
			ClampMovement(pCmd, 2);

		pCmd->tick_count = TIME_TO_TICKS(pTarget->GetSimulationTime() + (GetLerpTimeX()));
		pCmd->tick_count = TIME_TO_TICKS(backtracking->records->tick_count);

		Vector AimPoint = GetHitboxPosition(pTarget, HitBox);
		float hitchanceval = Options::Menu.RageBotTab.AccuracyHitchance.GetValue();
		GlobalBREAK::hitchance = hitchanceval;
		pTarget->GetPredicted(AimPoint);
		if (pTarget->GetVelocity().Length2D() > 25 && headPositions[pTarget->GetIndex()][10].simtime > 1 && Options::Menu.MiscTab.OtherSafeMode.GetIndex() > 2) { //doesn't seem to work well in Spread
			float bestTargetSimTime = headPositions[pTarget->GetIndex()][10].simtime;
			pCmd->tick_count = TIME_TO_TICKS(bestTargetSimTime);
			AimPoint = headPositions[pTarget->GetIndex()][10].hitboxPos;
			CBacktracking::Get().ShotBackTrackAimbotStart(pTarget);
			CBacktracking::Get().RestoreTemporaryRecord(pTarget);
			CBacktracking::Get().ShotBackTrackedTick(pTarget);
		}
		float yes = 100;
		bool IsAtPeakOfJump = fabs(pLocal->GetVelocity().z) <= 5.0f;
		if (Options::Menu.RageBotTab.pointscaleyes.GetState())
			AimPoint = BestPoint(pTarget, AimPoint);
		if (GameUtils::IsScopedWeapon(pWeapon) && !pWeapon->IsScoped() && Options::Menu.RageBotTab.AccuracyAutoScope.GetState()) pCmd->buttons |= IN_ATTACK2;
		else if ((hitchance(pLocal, pWeapon)) >= hitchanceval * 1.5 || (*pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() == 31 && (CanHit(AimPoint, &yes))) || (GameUtils::IsScout(pWeapon) && !pLocal->GetFlags() & FL_ONGROUND && IsAtPeakOfJump))
		{
			if (AimAtPoint(pLocal, AimPoint, pCmd, bSendPacket))
			{

				if (Options::Menu.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))
				{
					AimPoint -= pTarget->GetAbsOrigin();
					AimPoint += pTarget->GetOrigin();
					CBacktracking::Get().ShotBackTrackAimbotStart(pTarget);
					CBacktracking::Get().RestoreTemporaryRecord(pTarget);
					CBacktracking::Get().ShotBackTrackedTick(pTarget);
					pCmd->buttons |= IN_ATTACK;
				}
				else if (pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;

				if (Options::Menu.RageBotTab.AimbotAutoFire.GetState() && !(pCmd->buttons & IN_ATTACK))
				{
					if (GameUtils::IsZeus(pWeapon))
					{
						if ((pTarget->GetOrigin() - pLocal->GetOrigin()).Length() > 115.f)
							return;
						if ((pTarget->GetOrigin() - pLocal->GetOrigin()).Length() <= 110.f && pTarget->GetHealth() < 50)
							pCmd->buttons |= IN_ATTACK;
						else if ((pTarget->GetOrigin() - pLocal->GetOrigin()).Length() <= 100.f && pTarget->GetHealth() >= 75) pCmd->buttons |= IN_ATTACK;
					}
					else
						pCmd->buttons |= IN_ATTACK;
				}
				else if
					(pCmd->buttons & IN_ATTACK || pCmd->buttons & IN_ATTACK2)return;

			}
		}
		if (IsAbleToShoot(pLocal) && pCmd->buttons & IN_ATTACK) Globals::Shots += 1;



		if (global_count::didhit && pCmd->buttons & IN_ATTACK)
		{
			global_count::hits[pTarget->GetIndex()]++;
			global_count::shots_fired[pTarget->GetIndex()] = 0;
		}
		if (global_count::on_fire && pCmd->buttons & IN_ATTACK)
		{
			global_count::hits[pTarget->GetIndex()] = 0;
			global_count::shots_fired[pTarget->GetIndex()]++;
		}

		static bool WasFiring = false;
		if (pWeapon != nullptr)
		{
			if (GameUtils::IsPistol(pWeapon) && *pWeapon->m_AttributeManager()->m_Item()->ItemDefinitionIndex() != 64)
			{
				if (pCmd->buttons & IN_ATTACK && !GameUtils::IsGrenade(pWeapon) && !GameUtils::IsKnife(pWeapon))
				{
					if (WasFiring)
					{
						pCmd->buttons &= ~IN_ATTACK;
					}
				}
				WasFiring = pCmd->buttons & IN_ATTACK ? true : false;
			}
		}
	}
}
bool CRageBot::TargetMeetsRequirements(IClientEntity* pEntity)
{
	auto local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	// Is a valid player
	if (pEntity && pEntity->IsDormant() == false && pEntity->IsAlive() && pEntity->GetIndex() != local->GetIndex())
	{
		
		// Entity Type checks
		ClientClass *pClientClass = pEntity->GetClientClass();
		player_info_t pinfo;
		if (pClientClass->m_ClassID == (int)CSGOClassID::CCSPlayer && Interfaces::Engine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			// Team Check
			if (pEntity->GetTeamNum() != local->GetTeamNum())
			{
				// Spawn Check
				if (!pEntity->HasGunGameImmunity())
				{
					return true;
				}
			}
		}
	}
	// They must have failed a requirement
	return false;
}
float CRageBot::FovToPlayer(Vector ViewOffSet, Vector View, IClientEntity* pEntity, int aHitBox)
{

	CONST FLOAT MaxDegrees = 180.0f;

	Vector Angles = View;

	Vector Origin = ViewOffSet;

	Vector Delta(0, 0, 0);

	Vector Forward(0, 0, 0);

	AngleVectors(Angles, &Forward);

	Vector AimPos = GetHitboxPosition(pEntity, aHitBox);

	VectorSubtract(AimPos, Origin, Delta);

	Normalize(Delta, Delta);

	FLOAT DotProduct = Forward.Dot(Delta);
	// Time to calculate the field of view
	return (acos(DotProduct) * (MaxDegrees / PI));
}
float FovToPoint(Vector ViewOffSet, Vector View, Vector Point)
{
	// Get local view / eye position
	Vector Origin = ViewOffSet;
	// Create and intiialize vectors for calculations below
	Vector Delta(0, 0, 0);
	Vector Forward(0, 0, 0);
	// Convert angles to normalized directional forward vector
	AngleVectors(View, &Forward);
	Vector AimPos = Point;
	// Get delta vector between our local eye position and passed vector
	Delta = AimPos - Origin;
	//Delta = AimPos - Origin;
	// Normalize our delta vector
	Normalize(Delta, Delta);
	// Get dot product between delta position and directional forward vectors
	FLOAT DotProduct = Forward.Dot(Delta);
	// Time to calculate the field of view
	return (acos(DotProduct) * (180.f / PI));
}
int CRageBot::GetTargetCrosshair()
{
	int target = -1;
	float minFoV = Options::Menu.RageBotTab.AimbotFov.GetValue();
	IClientEntity * pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);
	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minFoV)
				{
					minFoV = fov;
					target = i;
				}
			}
		}
	}
	return target;
}
int CRageBot::GetTargetDistance()
{
	int target = -1;
	int minDist = 99999;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);
	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				int Distance = Difference.Length();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Distance < minDist && fov < Options::Menu.RageBotTab.AimbotFov.GetValue())
				{
					minDist = Distance;
					target = i;
				}
			}
		}
	}
	return target;
}
int CRageBot::GetTargetNextShot()
{
	int target = -1;
	int minfov = 361;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);
	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (fov < minfov && fov < Options::Menu.RageBotTab.AimbotFov.GetValue())
				{
					minfov = fov;
					target = i;
				}
				else
					minfov = 361;
			}
		}
	}
	return target;
}
float GetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	Vector ang, aim;
	AngleVectors(viewAngle, &aim);
	AngleVectors(aimAngle, &ang);
	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}
double inline __declspec (naked) __fastcall FASTSQRT(double n)
{
	_asm fld qword ptr[esp + 4]
		_asm fsqrt
	_asm ret 8
}
float VectorDistance(Vector v1, Vector v2)
{
	return FASTSQRT(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2) + pow(v1.z - v2.z, 2));
}
int CRageBot::GetTargetThreat(CUserCmd* pCmd)
{
	auto iBestTarget = -1;
	float flDistance = 8192.f;
	IClientEntity* pLocal = hackManager.pLocal();
	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			auto vecHitbox = pEntity->GetBonePos(NewHitBox);
			if (NewHitBox >= 0)
			{
				Vector Difference = pLocal->GetOrigin() - pEntity->GetOrigin();
				QAngle TempTargetAbs;
				CalcAngle(pLocal->GetEyePosition(), vecHitbox, TempTargetAbs);
				float flTempFOVs = GetFov(pCmd->viewangles, TempTargetAbs);
				float flTempDistance = VectorDistance(pLocal->GetOrigin(), pEntity->GetOrigin());
				if (flTempDistance < flDistance && flTempFOVs < Options::Menu.RageBotTab.AimbotFov.GetValue())
				{
					flDistance = flTempDistance;
					iBestTarget = i;
				}
			}
		}
	}
	return iBestTarget;
}
int CRageBot::GetTargetHealth()
{
	int target = -1;
	int minHealth = 101;
	IClientEntity* pLocal = hackManager.pLocal();
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; Interfaces::Engine->GetViewAngles(View);
	for (int i = 0; i < Interfaces::EntList->GetMaxEntities(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			int NewHitBox = HitScan(pEntity);
			if (NewHitBox >= 0)
			{
				int Health = pEntity->GetHealth();
				float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
				if (Health < minHealth && fov < Options::Menu.RageBotTab.AimbotFov.GetValue())
				{
					minHealth = Health;
					target = i;
				}
			}
		}
	}
	return target;
}
int CRageBot::HitScan(IClientEntity* pEntity)
{
	IClientEntity* pLocal = hackManager.pLocal();
	std::vector<int> HitBoxesToScan;
#pragma region GetHitboxesToScan
	int HitScanMode = Options::Menu.RageBotTab.TargetHitscan.GetIndex();
	float huso = (pEntity->GetHealth());
	float health = Options::Menu.RageBotTab.BaimIfUnderXHealth.GetValue();
	bool Multipoint = Options::Menu.RageBotTab.TargetMultipoint.GetState();
	int TargetHitbox = Options::Menu.RageBotTab.TargetHitbox.GetIndex();
	static bool enemyHP = false;

	int shots = global_count::shots_fired[pEntity->GetIndex()];

	if (shots > Options::Menu.RageBotTab.baim.GetValue() + 4)
		shots = 0;

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (huso < health)
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_UPPER_ARM);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_UPPER_ARM);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_HAND);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_HAND);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOREARM);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOREARM);
	}
	else if (Options::Menu.RageBotTab.AWPAtBody.GetState() && GameUtils::AWP(pWeapon))
	{

		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
		HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
	}
	else
	{
		if (HitScanMode == 0)
		{

			switch (Options::Menu.RageBotTab.TargetHitbox.GetIndex())
			{
			case 0:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
				break;
			case 1:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
				break;
			case 2:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LOWER_CHEST);
				break;
			case 3:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				break;
			case 4:
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
				break;
			}
		}
		else
		{
			if (GameUtils::IsZeus(pWeapon))
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LOWER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);

			}
			else if (pEntity->GetHealth() <= Options::Menu.RageBotTab.BaimIfUnderXHealth.GetValue() && pEntity->GetHealth() != 0)
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOREARM);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOREARM);
			}
			else if (!pEntity->GetFlags() & FL_ONGROUND && Options::Menu.RageBotTab.BaimInAir.GetState()) { //memed from fucking elysium jesus christ, thanks BlueeMods
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_THORAX);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LOWER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
			}
			else if (pEntity->GetVelocity().Length2D() < 0.10) //memed from fucking elysium jesus christ, thanks BlueeMods
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_THORAX);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LOWER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
			}
			else if (GetAsyncKeyState(Options::Menu.RageBotTab.bigbaim.GetKey()) && !Options::Menu.m_bIsOpen)
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_THORAX);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LOWER_CHEST);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
			}
			else if (shots >= Options::Menu.RageBotTab.baim.GetValue())
			{
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
				HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
			}
			else
			{
				switch (HitScanMode)
				{
				case 1:
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
					break;
				case 2:
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_THIGH);
					break;
				case 3:
				{
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_HEAD);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_NECK);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOOT);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOOT);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_UPPER_CHEST);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_BELLY);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_PELVIS);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_FOREARM);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_FOREARM);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_THIGH);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_LEFT_CALF);
					HitBoxesToScan.push_back((int)CSGOHitboxID::HITBOX_RIGHT_CALF);
				}
				break;
				}
			}
		}
	}

#pragma endregion Get the list of shit to scan
	float highestDamage;
	for (auto HitBoxID : HitBoxesToScan)
	{
		if (Options::Menu.RageBotTab.AccuracyAutoWall.GetState())
		{
			Vector Point = GetHitboxPosition(pEntity, HitBoxID);
			float dmg = 0.f;
			if (CanHit(Point, &dmg))
			{
				if (dmg >= Options::Menu.RageBotTab.AccuracyMinimumDamage.GetValue() || dmg > pEntity->GetHealth())
				{
					return HitBoxID;
					highestDamage = dmg;
				}
			}
		}
		else
		{
			if (GameUtils::IsVisible(hackManager.pLocal(), pEntity, HitBoxID))
			{
				Vector Point = GetHitboxPosition(pEntity, HitBoxID);
				float dmg = 0.f;
				if (CanHit(Point, &dmg))
				{
					if (dmg >= Options::Menu.RageBotTab.AccuracyMinimumDamage.GetValue() || dmg > pEntity->GetHealth())
					{
						return HitBoxID;
						highestDamage = dmg;
					}
				}
			}
		}
	}
	return -1;
}

void CRageBot::DoNoRecoil(CUserCmd *pCmd)
{
	if (!Options::Menu.RageBotTab.AimbotEnable.GetState())
		return;
	IClientEntity* pLocal = hackManager.pLocal();
	if (pLocal)
	{
		Vector AimPunch = pLocal->localPlayerExclusive()->GetAimPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() < 2)
			{
				pCmd->viewangles -= AimPunch * 2.00;
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
			}
			else if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2)
			{
				pCmd->viewangles -= AimPunch * 1.70;
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
			}
			else
			{
				pCmd->viewangles -= AimPunch * 2.00;
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
			}
		}
	}
}
void CRageBot::aimAtPlayer(CUserCmd *pCmd)
{
	if (!Options::Menu.RageBotTab.AimbotEnable.GetState())
		return;
	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	if (!pLocal || !pWeapon)
		return;
	Vector eye_position = pLocal->GetEyePosition();
	float best_dist = pWeapon->GetCSWpnData()->range;
	IClientEntity* target = nullptr;
	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		if (TargetMeetsRequirements(pEntity))
		{
			if (Globals::TargetID != -1)
				target = Interfaces::EntList->GetClientEntity(Globals::TargetID);
			else
				target = pEntity;
			Vector target_position = target->GetEyePosition();
			float temp_dist = eye_position.DistTo(target_position);
			if (best_dist > temp_dist)
			{
				best_dist = temp_dist;
				CalcAngle(eye_position, target_position, pCmd->viewangles);
			}
		}
	}
}
bool CRageBot::AimAtPoint(IClientEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{
	bool ReturnValue = false;
	if (point.Length() == 0) return ReturnValue;
	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();
	CalcAngle(src, point, angles);
	GameUtils::NormaliseViewAngle(angles);
	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return ReturnValue;
	}
	IsLocked = true;
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	if (!IsAimStepping)
		LastAimstepAngle = LastAngle;
	float fovLeft = FovToPlayer(ViewOffset, LastAimstepAngle, Interfaces::EntList->GetClientEntity(TargetID), 0);
	Vector AddAngs = angles - LastAimstepAngle;
	if (fovLeft > 37.0f && Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 1)
	{
		if (!pLocal->IsMoving())
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 37;
			LastAimstepAngle += AddAngs;
			GameUtils::NormaliseViewAngle(LastAimstepAngle);
			angles = LastAimstepAngle;
		}
		else
		{
			Normalize(AddAngs, AddAngs);
			AddAngs *= 39;
			LastAimstepAngle += AddAngs;
			GameUtils::NormaliseViewAngle(LastAimstepAngle);
			angles = LastAimstepAngle;
		}
	}
	else if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2)
	{
		if (fovLeft > 3.5)
		{
			if (Globals::Shots > 3)
			{
				Normalize(AddAngs, AddAngs);
				AddAngs *= 2.6;
				LastAimstepAngle += AddAngs;
				GameUtils::NormaliseViewAngle(LastAimstepAngle);
				angles = LastAimstepAngle;
			}
			else
			{
				Normalize(AddAngs, AddAngs);
				AddAngs *= 2.14;
				LastAimstepAngle += AddAngs;
				GameUtils::NormaliseViewAngle(LastAimstepAngle);
				angles = LastAimstepAngle;
			}
		}
		else
		{
			if (pLocal->GetVelocity().Length2D() > 210)
			{
				Normalize(AddAngs, AddAngs);
				AddAngs *= (3.12 - (pLocal->GetHealth() / 95));
				LastAimstepAngle += AddAngs;
				GameUtils::NormaliseViewAngle(LastAimstepAngle);
				angles = LastAimstepAngle;
			}
			else
			{
				Normalize(AddAngs, AddAngs);
				AddAngs *= (2.9 - (pLocal->GetHealth() / 90));
				LastAimstepAngle += AddAngs;
				GameUtils::NormaliseViewAngle(LastAimstepAngle);
				angles = LastAimstepAngle;
			}
		}
	}
	else
	{
		ReturnValue = true;
	}
	if (Options::Menu.RageBotTab.AimbotSilentAim.GetState())
	{
		pCmd->viewangles = angles;
	}
	if (!Options::Menu.RageBotTab.AimbotSilentAim.GetState())
	{
		Interfaces::Engine->SetViewAngles(angles);
	}
	return ReturnValue;
}
void CorrectMovement(Vector old_angles, CUserCmd* cmd, float old_forwardmove, float old_sidemove)
{
	float delta_view, first_function, second_function;
	if (old_angles.y < 0.f) first_function = 360.0f + old_angles.y;
	else first_function = old_angles.y;
	if (cmd->viewangles.y < 0.0f) second_function = 360.0f + cmd->viewangles.y;
	else second_function = cmd->viewangles.y;
	if (second_function < first_function) delta_view = abs(second_function - first_function);
	else delta_view = 360.0f - abs(first_function - second_function);
	delta_view = 360.0f - delta_view;
	cmd->forwardmove = cos(DEG2RAD(delta_view)) * old_forwardmove + cos(DEG2RAD(delta_view + 90.f)) * old_sidemove;
	cmd->sidemove = sin(DEG2RAD(delta_view)) * old_forwardmove + sin(DEG2RAD(delta_view + 90.f)) * old_sidemove;
}
float GetLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float Latency = nci->GetAvgLatency(FLOW_OUTGOING) + nci->GetAvgLatency(FLOW_INCOMING);
		return Latency;
	}
	else
	{
		return 0.0f;
	}
}
float GetOutgoingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float OutgoingLatency = nci->GetAvgLatency(FLOW_OUTGOING);
		return OutgoingLatency;
	}
	else
	{
		return 0.0f;
	}
}
float GetIncomingLatency()
{
	INetChannelInfo *nci = Interfaces::Engine->GetNetChannelInfo();
	if (nci)
	{
		float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING);
		return IncomingLatency;
	}
	else
	{
		return 0.0f;
	}
}
float OldLBY;
float LBYBreakerTimer;
float LastLBYUpdateTime;
bool bSwitch;
float CurrentVelocity(IClientEntity* LocalPlayer)
{
	int vel = LocalPlayer->GetVelocity().Length2D();
	return vel;
}
bool NextLBYUpdate()
{
	IClientEntity* LocalPlayer = hackManager.pLocal();
	float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);
	if (OldLBY != LocalPlayer->GetLowerBodyYaw())
	{
		LBYBreakerTimer++;
		OldLBY = LocalPlayer->GetLowerBodyYaw();
		bSwitch = !bSwitch;
		LastLBYUpdateTime = flServerTime;
	}
	if (CurrentVelocity(LocalPlayer) > 0.5)
	{
		LastLBYUpdateTime = flServerTime;
		return false;
	}
	if ((LastLBYUpdateTime + 1 - (GetLatency() * 2) < flServerTime) && (LocalPlayer->GetFlags() & FL_ONGROUND))
	{
		if (LastLBYUpdateTime + 1.1 - (GetLatency() * 2) < flServerTime)
		{
			LastLBYUpdateTime += 1.1;
		}
		return true;
	}
	return false;

}
static bool fuckassnig = false;
bool LBYBREAKTEST()
{
	bool lby_flip = false;
	IClientEntity* LocalPlayer = hackManager.pLocal();
	if (!LocalPlayer)//null check
		return false;
	float curtime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);
	static float NextLBYUpdate1 = 0;
	if (NextLBYUpdate1 > curtime + 1.1)
	{
		NextLBYUpdate1 = 0;
	}
	auto key1 = Options::Menu.MiscTab.FakeWalk.GetKey();
	if (LocalPlayer->GetVelocity().Length2D() > 0.1f && !GetAsyncKeyState(key1))
	{
		NextLBYUpdate1 = curtime + 0.22 + Interfaces::Globals->interval_per_tick;
		lby_flip = false;
		return false;
	}
	if ((NextLBYUpdate1 < curtime) && (LocalPlayer->GetFlags() & FL_ONGROUND) && LocalPlayer->GetVelocity().Length2D() < 1.f)
	{
		NextLBYUpdate1 = curtime + 1.1 + Interfaces::Globals->interval_per_tick;
		lby_flip = true;
		fuckassnig = true;
		return true;
	}
	lby_flip = false;
	fuckassnig = false;
	return false;
}
float RandomFloat2(float min, float max)
{
	typedef float(*RandomFloat_t)(float, float);
	return ((RandomFloat_t)GetProcAddress(GetModuleHandle("vstdlib.dll"), "RandomFloat")) (min, max);
}

void AngleVectors3(const QAngle &angles, Vector& forward)
{
	float	sp, sy, cp, cy;
	SinCos(DEG2RAD(angles[0]), &sp, &cp);
	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}
//--------------------------------------------------------------------------------
void AngleVectors3(const QAngle &angles, Vector& forward, Vector& right, Vector& up)
{
	float sr, sp, sy, cr, cp, cy;
	SinCos(DEG2RAD(angles[0]), &sp, &cp);
	SinCos(DEG2RAD(angles[1]), &sy, &cy);
	SinCos(DEG2RAD(angles[2]), &sr, &cr);
	forward.x = (cp * cy);
	forward.y = (cp * sy);
	forward.z = (-sp);
	right.x = (-1 * sr * sp * cy + -1 * cr * -sy);
	right.y = (-1 * sr * sp * sy + -1 * cr *  cy);
	right.z = (-1 * sr * cp);
	up.x = (cr * sp * cy + -sr * -sy);
	up.y = (cr * sp * sy + -sr * cy);
	up.z = (cr * cp);
}
static bool wilupdate;
static float LastLBYUpdateTime2 = 0;
#define RandomInt(min, max) (rand() % (max - min + 1) + min)
#define	MASK_ALL				(0xFFFFFFFF)
#define	MASK_SOLID				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 			/**< everything that is normally solid */
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) 	/**< everything that blocks player movement */
#define	MASK_NPCSOLID			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE) /**< blocks npc movement */
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME) 							/**< water physics in these contents */
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE) 							/**< everything that blocks line of sight for AI, lighting, etc */
#define MASK_OPAQUE_AND_NPCS	(MASK_OPAQUE|CONTENTS_MONSTER)										/**< everything that blocks line of sight for AI, lighting, etc, but with monsters added. */
#define	MASK_VISIBLE			(MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE) 								/**< everything that blocks line of sight for players */
#define MASK_VISIBLE_AND_NPCS	(MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE) 							/**< everything that blocks line of sight for players, but with monsters added. */
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX) 	/**< bullets see these as solid */
#define MASK_SHOT_HULL			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE) 	/**< non-raycasted weapons see this as solid (includes grates) */
#define MASK_SHOT_PORTAL		(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW) 							/**< hits solids (not grates) and passes through everything else */
#define MASK_SHOT_BRUSHONLY			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS) // non-raycasted weapons see this as solid (includes grates)
#define MASK_SOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE) 					/**< everything normally solid, except monsters (world+brush only) */
#define MASK_PLAYERSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for player movement, except monsters (world+brush only) */
#define MASK_NPCSOLID_BRUSHONLY	(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 			/**< everything normally solid for npc movement, except monsters (world+brush only) */
#define MASK_NPCWORLDSTATIC		(CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE) 					/**< just the world, used for route rebuilding */
#define MASK_SPLITAREAPORTAL	(CONTENTS_WATER|CONTENTS_SLIME) 		

void NormalizeNum(Vector &vIn, Vector &vOut)
{
	float flLen = vIn.Length();
	if (flLen == 0) {
		vOut.Init(0, 0, 1);
		return;
	}
	flLen = 1 / flLen;
	vOut.Init(vIn.x * flLen, vIn.y * flLen, vIn.z * flLen);
}

float fov_player(Vector ViewOffSet, Vector View, IClientEntity* entity, int hitbox)
{
	const float MaxDegrees = 180.0f;
	Vector Angles = View, Origin = ViewOffSet;
	Vector Delta(0, 0, 0), Forward(0, 0, 0);
	Vector AimPos = GameUtils::get_hitbox_location(entity, hitbox);
	AngleVectors(Angles, &Forward);
	VectorSubtract(AimPos, Origin, Delta);
	NormalizeNum(Delta, Delta);
	float DotProduct = Forward.Dot(Delta);
	return (acos(DotProduct) * (MaxDegrees / PI));
}

int closest_to_crosshair()
{
	int index = -1;
	float lowest_fov = INT_MAX;
	IClientEntity* local_player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!local_player)
		return -1;

	if (local_player->IsAlive())
	{
		Vector local_position = local_player->GetAbsOriginlol() + local_player->GetViewOffset();
		Vector angles;
		Interfaces::Engine->GetViewAngles(angles);
		for (int i = 1; i <= Interfaces::Globals->maxClients; i++)
		{
			IClientEntity *entity = Interfaces::EntList->GetClientEntity(i);
			if (!entity || entity->GetHealth() <= 0 || entity->GetTeamNum() == local_player->GetTeamNum() || entity->IsDormant() || entity == local_player)
				continue;
			float fov = fov_player(local_position, angles, entity, 0);
			if (fov < lowest_fov)
			{
				lowest_fov = fov;
				index = i;
			}
		}
		return index;
	}
}

int RandomIntf(int min, int max)
{
	return rand() % max + min;
}

int GetFPSXOXO()
{
	static int fps = 0;
	static int count = 0;
	using namespace std::chrono;
	auto now = high_resolution_clock::now();
	static auto last = high_resolution_clock::now();
	count++;
	if (duration_cast<milliseconds>(now - last).count() > 1000)
	{
		fps = count;
		count = 0;
		last = now;
	}
	return fps;
}

void fallback(CUserCmd* pCmd)
{

	IClientEntity* player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	float range = Options::Menu.MiscTab.freerange.GetValue();
	static int Ticks = 0;
	if (player->GetVelocity().Length2D() > 1 && (player->GetFlags() & FL_ONGROUND))
	{
		if (jitter)
			pCmd->viewangles.y -= (180 - range);
		else
			pCmd->viewangles.y += (180 - range);

		jitter = !jitter;
	}
	else if (player->GetVelocity().Length2D() <= 1 && (player->GetFlags() & FL_ONGROUND))
	{
		if (jitter)
			pCmd->viewangles.y -= (180 - range);
		else
			pCmd->viewangles.y += (180 - range);

		jitter = !jitter;
	}
	else
	{
		pCmd->viewangles.y -= Ticks;
		Ticks += 9;
		if (Ticks > (180 - range))
			Ticks = (-180 + range);
	}
}

void Do(CUserCmd* pCmd) 
{
	if (warmup) return;

	float range = Options::Menu.MiscTab.freerange.GetValue();
	static int Ticks = 0;
	static float last_real;
	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	Vector besthead;
	IClientEntity* local_player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	auto leyepos = local_player->GetOrigin() + local_player->GetViewOffset();
	auto headpos = GetHitboxPosition(local_player, 0);
	auto origin = local_player->GetAbsOriginDOG();

	if (!local_player->IsAlive())
		return;
	auto checkWallThickness = [&](IClientEntity* pPlayer, Vector newhead) -> float
	{
		Vector endpos1, endpos2;
		Vector eyepos = pPlayer->GetOrigin() + pPlayer->GetViewOffset();
		Ray_t ray;
		ray.Init(newhead, eyepos);
		CTraceFilterSkipTwoEntities filter(pPlayer, local_player);
		trace_t trace1, trace2;
		Interfaces::Trace->JewishFuckingTraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace1);
		if (trace1.DidHit())
			endpos1 = trace1.endpos;
		else
			return 0.f;
		ray.Init(eyepos, newhead);
		Interfaces::Trace->JewishFuckingTraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace2);
		if (trace2.DidHit())
			endpos2 = trace2.endpos;
		float add = newhead.Dist(eyepos) - leyepos.Dist(eyepos) + 3.f;
		return endpos1.Dist(endpos2) + add / 3;
	};
	int index = closest_to_crosshair();
	static IClientEntity* entity;
	if (index != -1)
		entity = Interfaces::EntList->GetClientEntity(index);
	if (!entity->isValidPlayer() || !entity)
	{
		pCmd->viewangles.y -= 180.f + RandomFloat(-range, range);

		return;
	}

	float step = (2 * PI) / 18.f;
	float radius = fabs(Vector(headpos - origin).Length2D());
	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float rotation = 0; rotation < (PI * 2.0); rotation += step)
		{
			Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);
			float totalthickness = 0.f;
			no_active = false;
			totalthickness += checkWallThickness(entity, newhead);
			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				bestrotation = rotation;
				besthead = newhead;
			}
		}
	}
	if (!no_active)
	{
		switch (Options::Menu.MiscTab.freestandtype.GetIndex())
		{
		case 0:
			pCmd->viewangles.y = RAD2DEG(bestrotation);
			break;
		case 1:
		{
			if (jitter)
				pCmd->viewangles.y = RAD2DEG(bestrotation) - range;
			else
				pCmd->viewangles.y = RAD2DEG(bestrotation) + range;

			jitter = !jitter;
		}
		break;
		case 2:
		{
			pCmd->viewangles.y = RAD2DEG(bestrotation) + RandomFloat(range, -range);
		}
		break;
		case 3:
		{	
			pCmd->viewangles.y -= Ticks;
			Ticks += 5;
			if (Ticks > (RAD2DEG(bestrotation) + range))
				Ticks = (RAD2DEG(bestrotation) - range);
		}
		break;
		}
	}
	else
		fallback(pCmd);
	last_real = pCmd->viewangles.y;
}

void Do3(CUserCmd* pCmd)
{
	if (warmup) return;

	float range = Options::Menu.MiscTab.freerange.GetValue();
	static int Ticks = 0;
	static float last_real;
	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;

	Vector besthead;
	IClientEntity* local_player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	auto leyepos = local_player->GetOrigin() + local_player->GetViewOffset();
	auto headpos = GetHitboxPosition(local_player, 0);
	auto origin = local_player->GetAbsOriginDOG();
	float val = Options::Menu.LegitBotTab.aatyp3.GetValue();
	if (!local_player->IsAlive())
		return;
	auto checkWallThickness = [&](IClientEntity* pPlayer, Vector newhead) -> float
	{
		Vector endpos1, endpos2;
		Vector eyepos = pPlayer->GetOrigin() + pPlayer->GetViewOffset();
		Ray_t ray;
		ray.Init(newhead, eyepos);
		CTraceFilterSkipTwoEntities filter(pPlayer, local_player);
		trace_t trace1, trace2;
		Interfaces::Trace->JewishFuckingTraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace1);
		if (trace1.DidHit())
			endpos1 = trace1.endpos;
		else
			return 0.f;
		ray.Init(eyepos, newhead);
		Interfaces::Trace->JewishFuckingTraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace2);
		if (trace2.DidHit())
			endpos2 = trace2.endpos;
		float add = newhead.Dist(eyepos) - leyepos.Dist(eyepos) + 3.f;
		return endpos1.Dist(endpos2) + add / 3;
	};
	int index = closest_to_crosshair();
	static IClientEntity* entity;
	if (index != -1)
		entity = Interfaces::EntList->GetClientEntity(index);
	if (!entity->isValidPlayer() || !entity)
	{
		pCmd->viewangles.y -= 180.f + RandomFloat(-20, 20);

		return;
	}

	float step = (2 * PI) / 18.f;
	float radius = fabs(Vector(headpos - origin).Length2D());
	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float rotation = 0; rotation < (PI * 2.0); rotation += step)
		{
			Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);
			float totalthickness = 0.f;
			no_active = false;
			totalthickness += checkWallThickness(entity, newhead);
			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				bestrotation = rotation;
				
				besthead = newhead;
			}
		}
	}
	if (!no_active)
	{
		switch (Options::Menu.LegitBotTab.aatyp2.GetIndex())
		{
		case 0:
		{
			pCmd->viewangles.y = RAD2DEG(bestrotation);
		}
		break;
		case 1:
		{
			if (jitter)
				pCmd->viewangles.y = RAD2DEG(bestrotation) - val;
			else
				pCmd->viewangles.y = RAD2DEG(bestrotation) + val;
		
			jitter = !jitter;
		}
		break;
		case 2:
		{
			pCmd->viewangles.y = RAD2DEG(bestrotation) + RandomFloat(-val, val);
		}
		break;
		case 3:
		{
			pCmd->viewangles.y -= Ticks;
			Ticks += 7;
			if (Ticks > (pCmd->viewangles.y = RAD2DEG(bestrotation) + val))
				Ticks = pCmd->viewangles.y = RAD2DEG(bestrotation) - val;
		}
		break;
		}
		
	
	}
	else
		fallback(pCmd);
	last_real = pCmd->viewangles.y;
}


void Do2(CUserCmd* pCmd) {

	if (warmup) return;
	static float last_real;
	bool no_active = true;
	float bestrotation = 0.f;
	float highestthickness = 0.f;
	Vector besthead;
	IClientEntity* local_player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	auto leyepos = local_player->GetOrigin() + local_player->GetViewOffset();
	auto headpos = GetHitboxPosition(local_player, 0);
	auto origin = local_player->GetAbsOriginDOG();

	if (!local_player->IsAlive())
		return;
	auto checkWallThickness = [&](IClientEntity* pPlayer, Vector newhead) -> float
	{
		Vector endpos1, endpos2;
		Vector eyepos = pPlayer->GetOrigin() + pPlayer->GetViewOffset();
		Ray_t ray;
		ray.Init(newhead, eyepos);
		CTraceFilterSkipTwoEntities filter(pPlayer, local_player);
		trace_t trace1, trace2;
		Interfaces::Trace->JewishFuckingTraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace1);
		if (trace1.DidHit())
			endpos1 = trace1.endpos;
		else
			return 0.f;
		ray.Init(eyepos, newhead);
		Interfaces::Trace->JewishFuckingTraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace2);
		if (trace2.DidHit())
			endpos2 = trace2.endpos;
		float add = newhead.Dist(eyepos) - leyepos.Dist(eyepos) + 3.f;
		return endpos1.Dist(endpos2) + add / 3;
	};
	int index = closest_to_crosshair();
	static IClientEntity* entity;
	if (index != -1)
		entity = Interfaces::EntList->GetClientEntity(index);
	if (!entity->isValidPlayer() || !entity)
	{

		pCmd->viewangles.y -= 0 + RandomFloat(-45, 45);

		return;
	}

	float step = (2 * PI) / 18.f; // One PI = half a circle ( for stacker cause low iq :sunglasses: ), 28
	float radius = fabs(Vector(headpos - origin).Length2D());
	if (index == -1)
	{
		no_active = true;
	}
	else
	{
		for (float rotation = 0; rotation < (PI * 2.0); rotation += step)
		{
			Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);
			float totalthickness = 0.f;
			no_active = false;
			totalthickness += checkWallThickness(entity, newhead);
			if (totalthickness > highestthickness)
			{
				highestthickness = totalthickness;
				bestrotation = rotation;
				besthead = newhead;
			}
		}
	}
	if (!no_active)
		pCmd->viewangles.y = (RAD2DEG(bestrotation) + 180) + RandomFloat(60, -60);
	else
		pCmd->viewangles.y -= 0 + RandomFloat(-90, 90);
	last_real = pCmd->viewangles.y;
}



void DoRealAA(CUserCmd* pCmd, IClientEntity* pLocal, bool& bSendPacket)
{

	if (GetAsyncKeyState(Options::Menu.MiscTab.manualleft.GetKey())) // right
	{
		dir = true;
		back = false;
		up = false;
		bigboi::indicator = 1;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.manualright.GetKey())) // left
	{
		dir = false;
		back = false;
		up = false;
		bigboi::indicator = 2;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.manualback.GetKey()))
	{
		dir = false;
		back = true;
		up = false;
		bigboi::indicator = 3;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.manualfront.GetKey()))
	{
		dir = false;
		back = false;
		up = true;
		bigboi::indicator = 4;
	}

	float real = pCmd->viewangles.y;
	static QAngle angles;
	Vector oldAngle = pCmd->viewangles;
	float oldForward = pCmd->forwardmove;
	float oldSideMove = pCmd->sidemove;

	if ((pCmd->buttons & IN_ATTACK))
		return;

	IClientEntity* localp = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	float speed = localp->GetVelocity().Length2D();

	if (GameUtils::IsGrenade(pWeapon))
		return;

	static int Ticks = 0;
	float temp_base = pCmd->viewangles.y;

	float twitchr = Options::Menu.MiscTab.twitchr.GetValue();
	float twitchr2 = Options::Menu.MiscTab.twitchr2.GetValue();
	float lby1 = Options::Menu.MiscTab.lby1.GetValue();
	float randlbyr = Options::Menu.MiscTab.randlbyr.GetValue();
	float twitch_assist = rand() % 100;

	if (!Options::Menu.MiscTab.AntiAimEnable.GetState())
		return;


	if (!localp->IsAlive())
		return;

	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2) return;

	if (speed <= 10 && (pLocal->GetFlags() & FL_ONGROUND))
	{
		if (Options::Menu.MiscTab.AntiAimYaw.GetIndex() == 1)
		{
			pCmd->viewangles.y -= 180;
		}

		else if (Options::Menu.MiscTab.AntiAimYaw.GetIndex() == 2)
		{
			if (twitch_assist < 10)
				pCmd->viewangles.y -= 160;
			else if (twitch_assist > 90)
				pCmd->viewangles.y += 160;
			else
				pCmd->viewangles.y += 180 + RandomFloat(-twitchr, twitchr);
		}

		else if (Options::Menu.MiscTab.AntiAimYaw.GetIndex() == 3)
		{
			pCmd->viewangles.y -= Ticks;
			Ticks += 11;
			if (Ticks > 240)
				Ticks = 120;
		}

		else if (Options::Menu.MiscTab.AntiAimYaw.GetIndex() == 4)
		{
			if (dir && !back && !up)
				pCmd->viewangles.y -= 90.f;
			else if (!dir && !back && !up)
				pCmd->viewangles.y += 90.f;
			else if (!dir && back && !up)
				pCmd->viewangles.y -= 180.f;
			else if (!dir && !back && up)
				pCmd->viewangles.y -= 0.f;
		}

		else if (Options::Menu.MiscTab.AntiAimYaw.GetIndex() == 5)
		{
			if (dir && !back && !up)
			{
				if (twitch_assist < 5)
					pCmd->viewangles.y -= 110.f;
				else if (twitch_assist > 95)
					pCmd->viewangles.y -= 70.f;
				else
					pCmd->viewangles.y -= 90.f + RandomFloat(-twitchr, twitchr);
			}
			else if (!dir && !back && !up)
			{
				if (twitch_assist < 10)
					pCmd->viewangles.y += 110.f;
				else if (twitch_assist > 90)
					pCmd->viewangles.y += 70.f;
				else
					pCmd->viewangles.y += 90.f + RandomFloat(-twitchr, twitchr);
			}
			else if (!dir && back && !up)
			{
				if (twitch_assist < 10)
					pCmd->viewangles.y -= 160.f;
				else if (twitch_assist > 90)
					pCmd->viewangles.y += 160.f;
				else
					pCmd->viewangles.y -= 180.f + RandomFloat(-twitchr, twitchr);
			}
			else if (!dir && !back && up)
				pCmd->viewangles.y -= 0.f + RandomFloat(-twitchr, twitchr);
		}

		else if (Options::Menu.MiscTab.AntiAimYaw.GetIndex() == 6)
		{

			if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
			{
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + lby1;
			}
			else
			{
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + lby1;
			}
		}

		else if (Options::Menu.MiscTab.AntiAimYaw.GetIndex() == 7)
		{
			pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + RandomFloat(randlbyr, -randlbyr);
		}
		else if (Options::Menu.MiscTab.AntiAimYaw.GetIndex() == 8)
		{
			Do(pCmd);
		}



	}
	else if (speed > 10 && (pLocal->GetFlags() & FL_ONGROUND))
	{
		if (Options::Menu.MiscTab.AntiAimYawrun.GetIndex() == 1)
		{
			pCmd->viewangles.y -= 180;
		}

		else if (Options::Menu.MiscTab.AntiAimYawrun.GetIndex() == 2)
		{
			if (twitch_assist < 5)
				pCmd->viewangles.y -= 160;
			else if (twitch_assist > 95)
				pCmd->viewangles.y += 160;
			else
				pCmd->viewangles.y += 180 + RandomFloat(-twitchr2, twitchr2);
		}

		else if (Options::Menu.MiscTab.AntiAimYawrun.GetIndex() == 3)
		{
			pCmd->viewangles.y -= Ticks;
			Ticks += 11;

			if (Ticks > 240)
				Ticks = 120;
		}

		else if (Options::Menu.MiscTab.AntiAimYawrun.GetIndex() == 4)
		{
			if (dir && !back && !up)
				pCmd->viewangles.y -= 90.f;
			else if (!dir && !back && !up)
				pCmd->viewangles.y += 90.f;
			else if (!dir && back && !up)
				pCmd->viewangles.y -= 180.f;
			else if (!dir && !back && up)
				pCmd->viewangles.y -= 0.f;
		}

		else if (Options::Menu.MiscTab.AntiAimYawrun.GetIndex() == 5)
		{
			if (dir && !back && !up)
				pCmd->viewangles.y -= 90.f + RandomFloat(-twitchr, twitchr);
			else if (!dir && !back && !up)
				pCmd->viewangles.y += 90.f + RandomFloat(-twitchr, twitchr);
			else if (!dir && back && !up)
				pCmd->viewangles.y -= 180.f + RandomFloat(-twitchr, twitchr);
			else if (!dir && !back && up)
				pCmd->viewangles.y -= 0.f + RandomFloat(-twitchr, twitchr);
		}
		else if (Options::Menu.MiscTab.AntiAimYawrun.GetIndex() == 6)
		{
			pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + RandomFloat(randlbyr, -randlbyr);
		}

		else if (Options::Menu.MiscTab.AntiAimYawrun.GetIndex() == 7)
		{

			if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
			{

				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + lby1;
			}
			else
			{
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + lby1;
			}
		}
		else if (Options::Menu.MiscTab.AntiAimYawrun.GetIndex() == 8)
		{
			Do(pCmd);
		}


	}
	else if (speed > 1 && !(pLocal->GetFlags() & FL_ONGROUND))
	{


		if (Options::Menu.MiscTab.AntiAimYaw3.GetIndex() == 1)
		{
			pCmd->viewangles.y -= 180;
		}

		else if (Options::Menu.MiscTab.AntiAimYaw3.GetIndex() == 2)
		{
			if (twitch_assist < 10)
				pCmd->viewangles.y -= 160;
			else if (twitch_assist > 90)
				pCmd->viewangles.y += 160;
			else
				pCmd->viewangles.y += 180 + RandomFloat(-twitchr, twitchr);
		}

		else if (Options::Menu.MiscTab.AntiAimYaw3.GetIndex() == 3)
		{
			pCmd->viewangles.y -= Ticks;
			Ticks += 5;

			if (Ticks > 240)
				Ticks = 120;
		}

		else if (Options::Menu.MiscTab.AntiAimYaw3.GetIndex() == 4)
		{
			if (dir && !back && !up)
				pCmd->viewangles.y -= 90.f;
			else if (!dir && !back && !up)
				pCmd->viewangles.y += 90.f;
			else if (!dir && back && !up)
				pCmd->viewangles.y -= 180.f;
			else if (!dir && !back && up)
				pCmd->viewangles.y -= 0.f;
		}

		else if (Options::Menu.MiscTab.AntiAimYaw3.GetIndex() == 5)
		{
			if (dir && !back && !up)
				pCmd->viewangles.y -= 90.f + RandomFloat(-twitchr, twitchr);
			else if (!dir && !back && !up)
				pCmd->viewangles.y += 90.f + RandomFloat(-twitchr, twitchr);
			else if (!dir && back && !up)
				pCmd->viewangles.y -= 180.f + RandomFloat(-twitchr, twitchr);
			else if (!dir && !back && up)
				pCmd->viewangles.y -= 0.f + RandomFloat(-twitchr, twitchr);
		}
		else if (Options::Menu.MiscTab.AntiAimYaw3.GetIndex() == 6)
		{
			pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + RandomFloat(randlbyr, -randlbyr);
		}

		else if (Options::Menu.MiscTab.AntiAimYaw3.GetIndex() == 7)
		{
			pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + lby1;
		}
		else if (Options::Menu.MiscTab.AntiAimYaw3.GetIndex() == 8)
		{
			Do(pCmd);
		}


	}
	

}





void DoFakeAA(CUserCmd* pCmd, bool& bSendPacket, IClientEntity* pLocal)
{
	static QAngle angles;
	IClientEntity* localp = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	QAngle RealAngle;

	float t = rand() % 200;


	static int Ticks;
	static int Side;

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (GameUtils::IsBomb(pWeapon))
		return;

	if (Interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::MOUSE_LEFT)) return;

	float twitchf = Options::Menu.MiscTab.twitchf.GetValue();

	float randlbyf = Options::Menu.MiscTab.randlbyf.GetValue();
	float curtime = (float)(pLocal->GetTickBase()  * Interfaces::Globals->interval_per_tick);
	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2) return;

	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 1 && Options::Menu.MiscTab.AntiAimEnable.GetState()) //legit AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
	{
		if (bSendPacket)
			pCmd->viewangles.y -= 180;
	}
	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2 && Options::Menu.MiscTab.AntiAimEnable.GetState())
	{
		pCmd->viewangles.y = pCmd->viewangles.y;
	}
	else
	{

		if (pLocal->GetFlags() & FL_ONGROUND)
		{
			if (localp->GetVelocity().Length2D() < 10.0f)
			{

				if (Options::Menu.MiscTab.FakeYaw.GetIndex() == 1)
				{
					pCmd->viewangles.y = LastAngleAA2.y - 180;
				}

				else if (Options::Menu.MiscTab.FakeYaw.GetIndex() == 2)
				{
					if (t < 7)
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 150;
					else if (t > 93)
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 150;
					else
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 180 + RandomFloat(twitchf, -twitchf);
				}

				else if (Options::Menu.MiscTab.FakeYaw.GetIndex() == 3)
				{
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + RandomFloat(randlbyf, -randlbyf);
				}

				else if (Options::Menu.MiscTab.FakeYaw.GetIndex() == 4)
				{
					float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
					double speed = 250;
					double exe = fmod(static_cast<double>(server_time)*speed, 180);
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90 + static_cast<float>(exe);
				}

				else if (Options::Menu.MiscTab.FakeYaw.GetIndex() == 5)
				{
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + RandomFloat(180, -180);
				}

				else if (Options::Menu.MiscTab.FakeYaw.GetIndex() == 6)
				{
					if (is_broken)
					{
						if (jitter)
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 50;
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 100;
						jitter = !jitter;
					}
					else
					{
						if (jitter)
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 150;
						else
							pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 150;
						jitter = !jitter;
					}
					
				}
				else if (Options::Menu.MiscTab.FakeYaw.GetIndex() == 7)
				{
					Do2(pCmd);
				}
			}
			else
			{
				if (Options::Menu.MiscTab.FakeYaw2.GetIndex() == 1)
				{
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 180;
				}

				else if (Options::Menu.MiscTab.FakeYaw2.GetIndex() == 2)
				{
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 180 + RandomFloat(twitchf, -twitchf);
				}

				else if (Options::Menu.MiscTab.FakeYaw2.GetIndex() == 3)
				{
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + RandomFloat(randlbyf, -randlbyf);
				}

				else if (Options::Menu.MiscTab.FakeYaw2.GetIndex() == 4)
				{
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + RandomFloat(170, -170);
				}

				else if (Options::Menu.MiscTab.FakeYaw2.GetIndex() == 5)
				{
					Do2(pCmd);
				}
			}
		}
		else
		{
			if (Options::Menu.MiscTab.FakeYaw3.GetIndex() == 1)
			{
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 180;
			}
			else if (Options::Menu.MiscTab.FakeYaw3.GetIndex() == 2)
			{
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 180 + RandomFloat(twitchf, -twitchf);
			}
			else if (Options::Menu.MiscTab.FakeYaw3.GetIndex() == 3)
			{
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + RandomFloat(randlbyf, -randlbyf);
			}
			else if (Options::Menu.MiscTab.FakeYaw3.GetIndex() == 4)
			{
				float server_time = pLocal->GetTickBase() * Interfaces::Globals->interval_per_tick;
				double speed = 250;
				double exe = fmod(static_cast<double>(server_time)*speed, 180);
				pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 90 + static_cast<float>(exe);
			}
			else if (Options::Menu.MiscTab.FakeYaw3.GetIndex() == 5)
			{
				pCmd->viewangles.y = pCmd->viewangles.y + RandomFloat(170, -170);

			}
			else if (Options::Menu.MiscTab.FakeYaw3.GetIndex() == 6)
			{
				pCmd->viewangles.y = hackManager.pLocal()->GetEyeAnglesXY()->y - 180;

			}

		}


	}

}
static bool respecc = false;

void CRageBot::DoPitch(CUserCmd * pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (GameUtils::IsGrenade(pWeapon))
		return;

	if ((pCmd->buttons & IN_ATTACK))
		return;

	if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() != 2)
	{
		switch (Options::Menu.MiscTab.AntiAimPitch.GetIndex())
		{
		case 0:
			break;
		case 1:
			pCmd->viewangles.x = 89;
			break;
		case 2:
			pCmd->viewangles.x = -89;
			break;
		case 3:
		{
			if (jitter)
				pCmd->viewangles.x = 89;
			else
				pCmd->viewangles.x = -89;
			jitter = !jitter;
		}
		break;
		case 4:
		{
			pCmd->viewangles.x = 0 + RandomFloat(-89.f, 89);
		}
		break;
		case 5:
		{
			if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() > 2)
				pCmd->viewangles.x = 991;
			else
				pCmd->viewangles.x = -89;
		}
		break;
		case 6:
		{
			if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() > 2)
				pCmd->viewangles.x = -991;
			else
				pCmd->viewangles.x = 89;
		}
		break;
		case 7:
		{
			if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() > 2)
			{
				pCmd->viewangles.x = -1080 + RandomFloat(-89, 89);
			}
			else
				pCmd->viewangles.x = 89;
		}
		break;
		}

	}





}

void backup_aa(CUserCmd* pCmd, bool& bSendPacket, IClientEntity* pLocal)
{

	IClientEntity* localp = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (GetAsyncKeyState(Options::Menu.MiscTab.manualleft.GetKey())) // right
	{
		dir = true;
		back = false;
		up = false;
		bigboi::indicator = 1;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.manualright.GetKey())) // left
	{
		dir = false;
		back = false;
		up = false;
		bigboi::indicator = 2;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.manualback.GetKey()))
	{
		dir = false;
		back = true;
		up = false;
		bigboi::indicator = 3;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.manualfront.GetKey()))
	{
		dir = false;
		back = false;
		up = true;
		bigboi::indicator = 4;
	}

	static int Ticks;
	static int Side;

	switch (Options::Menu.MiscTab.backup_aa.GetIndex())
	{
	case 1:
	{
		if (localp->GetFlags() & FL_ONGROUND)
		{
			if (localp->GetVelocity().Length2D() > 3.5)
			{
				if (bSendPacket)
				{
					if (jitter)
						pCmd->viewangles.y = 90;
					else
						pCmd->viewangles.y = -90;

					jitter = !jitter;

				}
				else
				{
					if (jitter)
						pCmd->viewangles.y -= 150 - rand() % 15;
					else
						pCmd->viewangles.y += 150 + rand() % 15;

					jitter = !jitter;
				}
			}
			else
			{
				if (bSendPacket)
				{
					pCmd->viewangles.y -= Ticks;
					Ticks += 6;

					if (Ticks > 90)
						Ticks = -90;
				}
				else
				{
					pCmd->viewangles.y -= 180;
				}
			}
		}
		else
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y = rand() % 360;
			}
			else
			{
				if (jitter)
					pCmd->viewangles.y -= (165.f + rand() % 10);
				else
					pCmd->viewangles.y += (165.f - rand() % 10);

				jitter = !jitter;
			}
		}
	}
	case 2:
	{
		if (localp->GetFlags() & FL_ONGROUND)
		{
			if (localp->GetVelocity().Length2D() > 3.5)
			{
				if (bSendPacket)
				{
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - (30 + rand() & 150);
				}
				else
				{
					if (dir && !back && !up)
					{
						if (jitter)
							pCmd->viewangles.y -= ((85) - rand() % 10);
						else
							pCmd->viewangles.y -= ((95) + rand() % 10);

						jitter = !jitter;
					}
					else if (!dir && !back && !up)
					{
						if (jitter)
							pCmd->viewangles.y += ((85) - rand() % 10);
						else
							pCmd->viewangles.y += ((95) - rand() % 10);

						jitter = !jitter;
					}
					else if (!dir && back && !up)
					{
						if (jitter)
							pCmd->viewangles.y -= ((175) - rand() % 10);
						else
							pCmd->viewangles.y += ((175) - rand() % 10);

						jitter = !jitter;
					}
					else if (!dir && !back && up)
					{
						if (jitter)
							pCmd->viewangles.y += (5);
						else
							pCmd->viewangles.y -= (5);

						jitter = !jitter;
					}
				}

			}
			else
			{
				if (bSendPacket)
				{
					if (dir && !back && !up)
					{
						if (jitter)
							pCmd->viewangles.y += (125 - rand() % 15);
						else
							pCmd->viewangles.y += (55 + rand() % 15);

						jitter = !jitter;
					}
					else if (!dir && !back && !up)
					{
						if (jitter)
							pCmd->viewangles.y -= (125 - rand() % 15);
						else
							pCmd->viewangles.y -= (55 + rand() % 15);

						jitter = !jitter;
					}
					else if (!dir && back && !up)
					{
						if (jitter)
							pCmd->viewangles.y -= (15 - rand() % 15);
						else
							pCmd->viewangles.y += (15 - rand() % 15);

						jitter = !jitter;
					}
					else if (!dir && !back && up)
					{
						if (jitter)
							pCmd->viewangles.y += (145 + rand() % 15);
						else
							pCmd->viewangles.y -= (145 + rand() % 15);

						jitter = !jitter;
					}

				}
				else
				{
					if (dir && !back && !up)
						pCmd->viewangles.y -= 90.f;
					else if (!dir && !back && !up)
						pCmd->viewangles.y += 90.f;
					else if (!dir && back && !up)
						pCmd->viewangles.y -= 180.f;
					else if (!dir && !back && up)
						pCmd->viewangles.y -= 0.f;
				}
			}
		}
		else
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y -= hackManager.pLocal()->GetLowerBodyYaw() + 100 + RandomFloat(-35, 35);
			}
			else
			{
				if (dir && !back && !up)
				{
					if (jitter)
						pCmd->viewangles.y -= ((80) - rand() % 10);
					else
						pCmd->viewangles.y -= ((100) + rand() % 10);

					jitter = !jitter;
				}
				else if (!dir && !back && !up)
				{
					if (jitter)
						pCmd->viewangles.y += ((80) - rand() % 10);
					else
						pCmd->viewangles.y += ((100) - rand() % 10);

					jitter = !jitter;
				}
				else if (!dir && back && !up)
				{
					if (jitter)
						pCmd->viewangles.y -= ((165) - rand() % 10);
					else
						pCmd->viewangles.y += ((165) - rand() % 10);

					jitter = !jitter;
				}
				else if (!dir && !back && up)
				{
					if (jitter)
						pCmd->viewangles.y += (15);
					else
						pCmd->viewangles.y -= (15);

					jitter = !jitter;
				}
			}
		}
	}
	case 3:
	{
		if (localp->GetFlags() & FL_ONGROUND)
		{
			if (localp->GetVelocity().Length2D() > 3.5)
			{
				if (bSendPacket)
				{
					pCmd->viewangles.y = RandomFloat(-90, 90);
				}
				else
				{
					pCmd->viewangles.y -= 180;
				}

			}
			else
			{
				if (bSendPacket)
				{
					if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
					{

						pCmd->viewangles.y = (hackManager.pLocal()->GetLowerBodyYaw() - 90) + 180;
					}
					else
					{
						pCmd->viewangles.y = (hackManager.pLocal()->GetLowerBodyYaw() + 90) - 180;
					}
				}
				else
				{
					if (pCmd->viewangles.y + Options::Menu.MiscTab.BreakLBYDelta.GetValue())
					{

						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 180;
					}
					else
					{
						pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() + 180;
					}
				}
			}
		}
		else
		{
			if (bSendPacket)
			{
				pCmd->viewangles.y = RandomFloat(-90, 90);
			}
			else
			{
				pCmd->viewangles.y -= 180 + RandomFloat(-30, 30);
			}
		}
	}
	case 4:
	{
		if (!bSendPacket)
			Do(pCmd);
		else
		{
			if (!(localp->GetFlags() & FL_ONGROUND))
			{
				pCmd->viewangles.y = RandomFloat(-160, 160);
			}
			else
			{
				if (localp->IsMoving())
				{
					pCmd->viewangles.y = hackManager.pLocal()->GetLowerBodyYaw() - 180 + RandomFloat(-30, 30);
				}
				else
				{
					Do2(pCmd);
				}
			}
		}
	}
	}

}

void CRageBot::DoAntiAim(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());
	Vector oldview = pCmd->viewangles;
	if (!Interfaces::Engine->IsInGame() || !Interfaces::Engine->IsConnected())
		return;

	if (!pLocal->IsAlive())
		return;
	if ((pCmd->buttons & IN_USE) || pLocal->GetMoveType() == MOVETYPE_LADDER)
		return;
	if (pLocal->GetMoveType() == MOVETYPE_NOCLIP)
		return;
	if (GameUtils::IsBomb(pWeapon))
		return;

	if (IsAimStepping)
		return;

	if (warmup)
		return;

	if (pWeapon)
	{
		if (!GameUtils::IsBallisticWeapon(pWeapon))
		{
			if (!CanOpenFire() || pCmd->buttons & IN_ATTACK2)
				return;
		}
	}

	if (GetAsyncKeyState(Options::Menu.MiscTab.backup_key.GetKey()))
	{
		backup = true;
		default_aa = false;
		bigboi::indicator = 5;
	}
	else if (GetAsyncKeyState(Options::Menu.MiscTab.default_key.GetKey()))
	{
		backup = false;
		default_aa = true;
		bigboi::indicator = 6;
	}
	if (Options::Menu.MiscTab.AntiAimEnable.GetState())
	{
	//	int choked = Interfaces::m_pClientState->chokedcommands;
	//	float addAngle = GetFPS() >= (TIME_TO_TICKS(1.f) * 0.5f) ? (2.3789 * max(choked, GlobalBREAK::prevChoked) + 99.9) : 144.9f;
		
		if (!backup && default_aa)
		{
				DoPitch(pCmd);
				if (bSendPacket)
					DoFakeAA(pCmd, bSendPacket, pLocal);
				else
					DoRealAA(pCmd, pLocal, bSendPacket);

		//		if (!Interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::MOUSE_LEFT))
		//		{
		//			next_lby_update_func(pCmd, Options::Menu.MiscTab.BreakLBYDelta.GetValue());
		//		}

			if (Options::Menu.MiscTab.antilby.GetIndex() > 0 && (pLocal->GetFlags() & FL_ONGROUND))
					anti_lby(pCmd, bSendPacket);
		}
		else if (backup && !default_aa)
		{
				DoPitch(pCmd);
				backup_aa(pCmd, bSendPacket, pLocal);
		}
	}
		
	
}
