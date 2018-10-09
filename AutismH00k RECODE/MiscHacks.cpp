#include "MiscHacks.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "IClientMode.h"
#include <chrono>
#include <algorithm>
#include <time.h>
#include "Hooks.h"
#include "edge.h"
CMiscHacks* g_Misc = new CMiscHacks;
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
inline float bitsToFloat(unsigned long i)
{
	return *reinterpret_cast<float*>(&i);
}
inline float FloatNegate(float f)
{
	return bitsToFloat(FloatBits(f) ^ 0x80000000);
}
Vector AutoStrafeView;
void CMiscHacks::Init()
{
}
void CMiscHacks::Draw()
{
	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;
	switch (Options::Menu.MiscTab.NameChanger.GetIndex())
	{
	case 0:
		break;
	case 1:
		Namespam();
		break;
	case 2:
		NoName();
		break;
	case 3:
		NameSteal();
		break;
	}
}
void CMiscHacks::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (Options::Menu.RageBotTab.AimbotEnable.GetState())
		AutoPistol(pCmd);
	if (Options::Menu.MiscTab.OtherAutoJump.GetState() && Options::Menu.MiscTab.OtherSafeMode.GetIndex() != 2)
		AutoJump(pCmd);
	Interfaces::Engine->GetViewAngles(AutoStrafeView);
	if (Options::Menu.MiscTab.OtherAutoStrafe.GetState())
		RageStrafe(pCmd);

	if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
	{
		if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() != 2)
		{
		
				SlowMo(pCmd, bSendPacket);
				FakeWalk0(pCmd, bSendPacket);
			
		}
	}
	if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
	{
		if (Options::Menu.VisualsTab.left2.GetState())
			left2();

		if (Options::Menu.VisualsTab.DisablePostProcess.GetState())
			PostProcces();
	}
}
int GetFxPS()
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
float curtime_fixedx(CUserCmd* ucmd) {
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
bool next_lby_update_funcdd(CUserCmd* m_pcmd, const float yaw_to_break) {
	auto m_local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (m_local) {
		static float last_attempted_yaw;
		static float old_lby;
		static float next_lby_update_time;
		const float current_time = curtime_fixedx(m_pcmd);
		if (old_lby != m_local->GetLowerBodyYaw() && last_attempted_yaw != m_local->GetLowerBodyYaw()) {
			old_lby = m_local->GetLowerBodyYaw();
			if (m_local->GetVelocity().Length2D() < 140) {
				auto latency = (Interfaces::Engine->GetNetChannelInfo()->GetAvgLatency(FLOW_INCOMING) + Interfaces::Engine->GetNetChannelInfo()->GetAvgLatency(FLOW_OUTGOING));
				next_lby_update_time = current_time + 1.1f;
			}
		}
		if (m_local->GetVelocity().Length2D() < 1) {
			if ((next_lby_update_time < current_time) && m_local->GetFlags() & FL_ONGROUND) {
				last_attempted_yaw = yaw_to_break;
				next_lby_update_time = current_time + 1.1f;
				return true;
			}
		}
	}
	return false;
}
void VectorAnglesXXX(Vector forward, Vector &angles)
{
	float tmp, yaw, pitch;

	if (forward[2] == 0 && forward[0] == 0)
	{
		yaw = 0;

		if (forward[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / PI);

		if (yaw < 0)
			yaw += 360;
		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / PI);

		if (pitch < 0)
			pitch += 360;
	}

	if (pitch > 180)
		pitch -= 360;
	else if (pitch < -180)
		pitch += 360;

	if (yaw > 180)
		yaw -= 360;
	else if (yaw < -180)
		yaw += 360;

	if (pitch > 89)
		pitch = 89;
	else if (pitch < -89)
		pitch = -89;

	if (yaw > 180)
		yaw = 180;
	else if (yaw < -180)
		yaw = -180;

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}
Vector CalcAngleFakewalk(Vector src, Vector dst)
{
	Vector ret;
	VectorAnglesXXX(dst - src, ret);
	return ret;
}

void rotate_movement(float yaw, CUserCmd* cmd)
{
	Vector viewangles;
	QAngle yamom;
	Interfaces::Engine->GetViewAngles(viewangles);
	float rotation = DEG2RAD(viewangles.y - yaw);
	float cos_rot = cos(rotation);
	float sin_rot = sin(rotation);
	float new_forwardmove = (cos_rot * cmd->forwardmove) - (sin_rot * cmd->sidemove);
	float new_sidemove = (sin_rot * cmd->forwardmove) + (cos_rot * cmd->sidemove);
	cmd->forwardmove = new_forwardmove;
	cmd->sidemove = new_sidemove;
}

float fakewalk_curtime(CUserCmd* ucmd)
{
	auto local_player = hackManager.pLocal();

	if (!local_player)
		return 0;

	int g_tick = 0;
	CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted)
	{
		g_tick = (float)local_player->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = ucmd;
	float curtime = g_tick * Interfaces::Globals->interval_per_tick;
	return curtime;
}
void CMiscHacks::FakeWalk0(CUserCmd* pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	int key1 = Options::Menu.MiscTab.fw.GetKey();
	if (key1 >0 && GetAsyncKeyState(key1) && !Options::Menu.m_bIsOpen)
	{
		globalsh.fakewalk = true;
		static int iChoked = -1;
		iChoked++;
		if (pCmd->forwardmove > 0)
		{
			pCmd->buttons |= IN_BACK;
			pCmd->buttons &= ~IN_FORWARD;
		}
		if (pCmd->forwardmove < 0)
		{
			pCmd->buttons |= IN_FORWARD;
			pCmd->buttons &= ~IN_BACK;
		}
		if (pCmd->sidemove < 0)
		{
			pCmd->buttons |= IN_MOVERIGHT;
			pCmd->buttons &= ~IN_MOVELEFT;
		}
		if (pCmd->sidemove > 0)
		{
			pCmd->buttons |= IN_MOVELEFT;
			pCmd->buttons &= ~IN_MOVERIGHT;
		}
		static int choked = 0;
		choked = choked > 14 ? 0 : choked + 1;
		
		float nani = Options::Menu.MiscTab.FakeWalkSpeed.GetValue() / 14;

		pCmd->forwardmove = choked < nani || choked > 14 ? 0 : pCmd->forwardmove;
		pCmd->sidemove = choked < nani || choked > 14 ? 0 : pCmd->sidemove; //100:6 are about 16,6, quick maths
		bSendPacket = choked < 1;

	}
}
static __declspec(naked) void __cdecl Invoke_NET_SetConVar(void* pfn, const char* cvar, const char* value)
{
	__asm
	{
		push    ebp
		mov     ebp, esp
		and     esp, 0FFFFFFF8h
		sub     esp, 44h
		push    ebx
		push    esi
		push    edi
		mov     edi, cvar
		mov     esi, value
		jmp     pfn
	}
}
void DECLSPEC_NOINLINE NET_SetConVar(const char* value, const char* cvar)
{
	static DWORD setaddr = Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x8D\x4C\x24\x1C\xE8\x00\x00\x00\x00\x56", "xxxxx????x");
	if (setaddr != 0)
	{
		void* pvSetConVar = (char*)setaddr;
		Invoke_NET_SetConVar(pvSetConVar, cvar, value);
	}
}
void change_name(const char* name)
{
	if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
		NET_SetConVar(name, "name");
	//fuck is this? fixing convars didn't used to be like that.
}
void CMiscHacks::AutoPistol(CUserCmd* pCmd) {

	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (pWeapon) {

		if (GameUtils::IsBomb(pWeapon)) { return; }

		if (GameUtils::IsPistol(pWeapon)) { return; }

		if (GameUtils::IsGrenade(pWeapon)) { return; }

	}

	static bool WasFiring = false;

	if (GameUtils::IsPistol) {

		if (pCmd->buttons & IN_ATTACK && WasFiring) { pCmd->buttons &= ~IN_ATTACK; }
	}

	WasFiring = pCmd->buttons & IN_ATTACK ? true : false;
}
void CMiscHacks::SlowMo(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (GetAsyncKeyState(Options::Menu.MiscTab.FakeWalk.GetKey()) && !Options::Menu.m_bIsOpen)
	{
		static int iChoked = -1;
		iChoked++;
		static bool slowmo;
		slowmo = !slowmo;
		const auto lag = 90;
		INetChannel* gayes = (INetChannel*)Interfaces::m_pClientState->m_NetChannel;
		gayes->m_nOutSequenceNr += 1;
		gayes->m_nOutSequenceNrAck -= 1;
		Interfaces::m_pClientState->lastoutgoingcommand += 1;
		if (iChoked < lag)
		{
			bSendPacket = false;
			if (slowmo)
			{
				pCmd->tick_count = INT_MAX;
				pCmd->command_number += INT_MAX + pCmd->tick_count % 2 ? 1 : 0;
				pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
				pCmd->buttons |= pLocal->GetMoveType() == IN_LEFT;
				pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
				INetChannel* gayes = (INetChannel*)Interfaces::m_pClientState->m_NetChannel;
				gayes->m_nOutSequenceNr += 1;
				gayes->m_nOutSequenceNrAck -= 1;
				Interfaces::m_pClientState->lastoutgoingcommand += 1;
				pCmd->forwardmove = pCmd->sidemove = 0.f;
			}
			else
			{
				bSendPacket = true;
				iChoked = -1;
				Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 1.2;
				pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
			}
		}
		else
		{
			if (!bSendPacket)
			{
				if (slowmo)
				{
					pCmd->tick_count = INT_MAX;
					pCmd->command_number += INT_MAX + pCmd->tick_count % 2 ? 1 : 0;
					pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
					pCmd->buttons |= pLocal->GetMoveType() == IN_LEFT;
					pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
					pCmd->forwardmove = pCmd->sidemove = 0.f;
				}
			}
			else
			{
				if (slowmo)
				{
					bSendPacket = true;
					iChoked = -1;
					pCmd->tick_count = INT_MAX;
					Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 1.25;
					pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
				}
			}
		}
	}
}
void CMiscHacks::Namespam()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;
	static bool wasSpamming = true;
	if (wasSpamming)
	{
		static bool useSpace = true;
		if (useSpace)
		{
			change_name("™Mirror");
			useSpace = !useSpace;
		}
		else
		{
			change_name("™ Mirror");
			useSpace = !useSpace;
		}
	}
	start_t = clock();
}
void CMiscHacks::NoName()
{
	change_name("\n­­­");
}
void CMiscHacks::NameSteal()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;
	std::vector < std::string > Names;
	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *entity = Interfaces::EntList->GetClientEntity(i);
		player_info_t pInfo;
		if (entity && hackManager.pLocal()->GetTeamNum() == entity->GetTeamNum() && entity != hackManager.pLocal())
		{
			ClientClass* cClass = (ClientClass*)entity->GetClientClass();
			if (cClass->m_ClassID == (int)CSGOClassID::CCSPlayer)
			{
				if (Interfaces::Engine->GetPlayerInfo(i, &pInfo))
				{
					if (!strstr(pInfo.name, "GOTV"))
						Names.push_back(pInfo.name);
				}
			}
		}
	}
	static bool wasSpamming = true;
	int randomIndex = rand() % Names.size();
	char buffer[128];
	sprintf_s(buffer, "%s ", Names[randomIndex].c_str());
	if (wasSpamming)
	{
		change_name(buffer);
	}
	else
	{
		change_name("p$i 1337");
	}
	start_t = clock();
}
void CMiscHacks::AutoJump(CUserCmd *pCmd)
{
	auto userCMD = pCmd;
	auto g_LocalPlayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());;
	if (g_LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->GetMoveType() == MOVETYPE_LADDER) return;
	if (userCMD->buttons & IN_JUMP && !(g_LocalPlayer->GetFlags() & FL_ONGROUND)) {
		userCMD->buttons &= ~IN_JUMP;
	}
}
template<class T, class U>
inline T clampangle(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}
void CMiscHacks::RageStrafe(CUserCmd *userCMD)
{
	auto g_LocalPlayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());;
	if (g_LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP || g_LocalPlayer->GetMoveType() == MOVETYPE_LADDER || !g_LocalPlayer->IsAlive()) return;
	if (!Interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_SPACE) ||
		Interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_A) ||
		Interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_D) ||
		Interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_S) ||
		Interfaces::m_iInputSys->IsButtonDown(ButtonCode_t::KEY_W))
		return;
	if (!(g_LocalPlayer->GetFlags() & FL_ONGROUND)) {
		if (userCMD->mousedx > 1 || userCMD->mousedx < -1) {
			userCMD->sidemove = clamp(userCMD->mousedx < 0.f ? -400.f : 400.f, -400, 400);
		}
		else {
			if (g_LocalPlayer->GetVelocity().Length2D() == 0 || g_LocalPlayer->GetVelocity().Length2D() == NAN || g_LocalPlayer->GetVelocity().Length2D() == INFINITE)
			{
				userCMD->forwardmove = 400;
				return;
			}
			userCMD->forwardmove = clamp(5850.f / g_LocalPlayer->GetVelocity().Length2D(), -400, 400);
			if (userCMD->forwardmove < -400 || userCMD->forwardmove > 400)
				userCMD->forwardmove = 0;
			userCMD->sidemove = clamp((userCMD->command_number % 2) == 0 ? -400.f : 400.f, -400, 400);
			if (userCMD->sidemove < -400 || userCMD->sidemove > 400)
				userCMD->sidemove = 0;
		}
	}
}
Vector GetAutostrafeView()
{
	return AutoStrafeView;
}
void CMiscHacks::left2()
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (!GameUtils::IsBallisticWeapon(pWeapon))
		Interfaces::Engine->ExecuteClientCmd("cl_righthand 0");
	else
		Interfaces::Engine->ExecuteClientCmd("cl_righthand 1");
}


void CMiscHacks::PostProcces()
{
	ConVar* Meme = Interfaces::CVar->FindVar("mat_postprocess_enable");
	SpoofedConvar* meme_spoofed = new SpoofedConvar(Meme);
	meme_spoofed->SetString("mat_postprocess_enable 0");
}
