#include <time.h>
#include <chrono>
#include "faxzeebreaker.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "IClientMode.h"
#include <algorithm>
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
float curtime_fixedX(CUserCmd* ucmd) {
	auto local_player = hackManager.pLocal();
	static int g_tick = 0;
	static CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = local_player->GetTickBase();
	}
	else {
		// Required because prediction only runs on frames, not ticks
		// So if your framerate goes below tickrate, m_nTickBase won't update every tick
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
		const float current_time = curtime_fixedX(m_pcmd); // Fixes curtime to the frame so it breaks perfectly every time if delta is in range

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


void FakeMove(CUserCmd* cmd)
{

	if (!Options::Menu.MiscTab.experimental.GetState())
		return;

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
	//seems to fail at some statement
	void fake979(CUserCmd* cmd)
	{

		if (!Options::Menu.MiscTab.experimental.GetState())
			return;

		if ((cmd->buttons & IN_ATTACK))
			return;

		if (GetAsyncKeyState(VK_SPACE))
			return;


		IClientEntity *g_LocalPlayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		if (g_LocalPlayer->IsAlive())
		{
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

	void spookyfakeanimation(CUserCmd* m_pcmd, const float yaw_to_break) {
		auto m_local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		static int choked = 0;
		static bool wasMoving = true;
		static bool preBreak = false;
		static bool shouldBreak = false;
		static bool brokeThisTick = false;
		static float oldCurtime = curtime_fixedX(m_pcmd);
		if (((m_pcmd->buttons & IN_USE) || (m_pcmd->buttons & IN_ATTACK) || m_local->GetMoveType() == MOVETYPE_LADDER))
			return;
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
				m_pcmd->viewangles.y += yaw_to_break + addAngle;
			}
		}
	}