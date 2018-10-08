
#include "Hacks.h"
#include "Chams.h"
#include "Menu.h"
#include "circlestrafer.h"
#include "CBulletListener.h"
#include "Interfaces.h"
#include "autodefuse.h"
#include "RenderManager.h"
#include "lodepng.h"
#include "knifebot.h"
#include "faxzeebreaker.h"
#include <d3d9.h>
#include "EnginePrediction.h"
#include "MiscHacks.h"
#include "CRC32.h"
#include "Resolver.h"
#include "hitmarker.h"
#include "laggycompensation.h"
#include <intrin.h>
#include "DamageIndicator.h"
#include "RageBot.h"
#include "LagCompensation2.h"
#include "edge.h"
#include "engine_prd.h"
#include "lin_extp.h"
#include "radar.h"
static CPredictionSystem* Prediction = new CPredictionSystem();
static CRageBot* rage = new CRageBot();
CLagcompensation lagcompensation;
HANDLE worldmodel_handle;
C_BaseCombatWeapon* worldmodel;
#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr) + (DWORD)(addValue))
#ifdef NDEBUG
#define strenc( s ) std::string( cx_make_encrypted_string( s ) )
#define charenc( s ) strenc( s ).c_str()
#define wstrenc( s ) std::wstring( strenc( s ).begin(), strenc( s ).end() )
#define wcharenc( s ) wstrenc( s ).c_str()
#else
#define strenc( s ) ( s )
#define charenc( s ) ( s )
#define wstrenc( s ) ( s )
#define wcharenc( s ) ( s )
#endif
#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif

int bigboi::indicator;
bool bigboi::freestand;
int bigboi::freestandval;
std::vector<trace_info> trace_logs;
int currentfov;
Vector LastAngleAA;
extern Vector LastAngleAA2;
Vector LastAngleAAFake;
Vector last_fake;
bool Resolver::didhitHS;
CUserCmd* Globals::UserCmd;
IClientEntity* Globals::Target;
int Globals::Shots;
bool Globals::change;
int Globals::TargetID;
bool Resolver::hitbaim;
bool Globals::Up2date;
int Globals::fired;
int Globals::hit;
extern float lineLBY;
extern float lineLBY2;
extern float current_real;
extern float lineRealAngle;
extern float lineFakeAngle;
extern float last_real;
extern float lspeed;
extern float pitchmeme;
extern float lby2;
extern float inaccuracy;
Vector LastAngleAAReal;
Vector LBYThirdpersonAngle;

#define STUDIO_RENDER					0x00000001
std::map<int, QAngle>Globals::storedshit;
int Globals::missedshots;
static int missedLogHits[65];
float fakeangle;
typedef void(__thiscall* DrawModelEx_)(void*, void*, void*, const ModelRenderInfo_t&, matrix3x4*);
typedef void(__thiscall* PaintTraverse_)(PVOID, unsigned int, bool, bool);
typedef bool(__thiscall* InPrediction_)(PVOID);
typedef void(__stdcall *FrameStageNotifyFn)(ClientFrameStage_t);
typedef long(__stdcall *EndScene_t)(IDirect3DDevice9*);
typedef int(__thiscall* DoPostScreenEffects_t)(IClientModeShared*, int);
typedef bool(__thiscall *FireEventClientSideFn)(PVOID, IGameEvent*);
typedef long(__stdcall *Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
typedef void(__thiscall* RenderViewFn)(void*, CViewSetup&, CViewSetup&, int, int);
using OverrideViewFn = void(__fastcall*)(void*, void*, CViewSetup*);
typedef float(__stdcall *oGetViewModelFOV)();
typedef void(__thiscall *SceneEnd_t)(void *pEcx);
EndScene_t o_EndScene;
SceneEnd_t pSceneEnd;
Reset_t o_Reset;
DoPostScreenEffects_t o_DoPostScreenEffects;
PaintTraverse_ oPaintTraverse;
DrawModelEx_ oDrawModelExecute;
FrameStageNotifyFn oFrameStageNotify;
OverrideViewFn oOverrideView;
FireEventClientSideFn oFireEventClientSide;
RenderViewFn oRenderView;
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
bool __stdcall Hooked_InPrediction();
bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event);
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld);
bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd);
HRESULT __stdcall EndScene_hooked(IDirect3DDevice9 *pDevice);
int __stdcall Hooked_DoPostScreenEffects(int a1);
HRESULT __stdcall Reset_hooked(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters);
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage);
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup);
float __stdcall GGetViewModelFOV();
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw);
void __fastcall	hkSceneEnd(void *pEcx, void *pEdx);
typedef void(__thiscall* LockCursor)(void*);
LockCursor oLockCursor;

void	__stdcall Hooked_LockCursor()
{
	bool xd = Options::Menu.m_bIsOpen;
	if (xd) {
		Interfaces::Surface->unlockcursor();
		return;
	}
	oLockCursor(Interfaces::Surface);
}

namespace GlobalBREAK
{
	bool bVisualAimbotting = false;
	QAngle vecVisualAimbotAngs = QAngle(0.f, 0.f, 0.f);
	int ChokeAmount = 0;
	float flFakewalked = 0.f;
	bool NewRound = false;
	bool WeaponFire = false;
	QAngle fakeangleslocal;
	bool bRainbowCross = true;
	bool dohitmarker;
	float LastTimeWeFired = 0;
	int ShotsFiredLocally = 0;
	int ShotsHitPerEntity[65];
	bool HeadShottedEntity[65] = { false };
	float curFov = 0;
	bool bUsingFakeAngles[65];
	float HitMarkerAlpha = 0.f;
	int TicksOnGround = 0;
	char* breakmode;
	int AnimationPitchFix = 0;
	float hitchance;
	int NextPredictedLBYUpdate = 0;
	int breakangle;
	int prevChoked = 0;
	bool AAFlip = false;
	bool LEFT;
	bool RIGHT;
	bool BACK;
	char my_documents_folder[MAX_PATH];
	float smt = 0.f;
	QAngle visualAngles = QAngle(0.f, 0.f, 0.f);
	bool bSendPacket = false;
	bool bAimbotting = false;
	CUserCmd* userCMD = nullptr;
	char* szLastFunction = "<No function was called>";
	HMODULE hmDll = nullptr;
	bool bFakewalking = false;
	Vector vecUnpredictedVel = Vector(0, 0, 0);
	float flFakeLatencyAmount = 0.f;
	float flEstFakeLatencyOnServer = 0.f;
	matrix3x4_t traceHitboxbones[128];
	std::array<std::string, 64> resolverModes;
}
int ground_tick;
Vector OldOrigin;
namespace Hooks
{
	Utilities::Memory::VMTManager VMTPanel;
	Utilities::Memory::VMTManager VMTClient;
	Utilities::Memory::VMTManager VMTClientMode;
	Utilities::Memory::VMTManager VMTModelRender;
	Utilities::Memory::VMTManager VMTPrediction;
	Utilities::Memory::VMTManager VMTRenderView;
	Utilities::Memory::VMTManager VMTEventManager;
	Utilities::Memory::VMTManager VMTDIRECTX;
	Utilities::Memory::VMTManager VMTSurface;
};
void Hooks::UndoHooks()
{
	VMTPanel.RestoreOriginal();
	VMTPrediction.RestoreOriginal();
	VMTModelRender.RestoreOriginal();
	VMTClientMode.RestoreOriginal();
	VMTDIRECTX.RestoreOriginal();
	VMTEventManager.RestoreOriginal();
	VMTSurface.RestoreOriginal();
	VMTRenderView.RestoreOriginal();
	VMTClient.RestoreOriginal();
}
void Hooks::Initialise()
{
	Interfaces::Engine->ExecuteClientCmd("clear");
	//--------------- D3D ---------------//
	VMTDIRECTX.Initialise((DWORD*)Interfaces::g_pD3DDevice9);
	o_EndScene = (EndScene_t)VMTDIRECTX.HookMethod((DWORD)&EndScene_hooked, 42);
	VMTDIRECTX.Initialise((DWORD*)Interfaces::g_pD3DDevice9);
	o_Reset = (Reset_t)VMTDIRECTX.HookMethod((DWORD)&Reset_hooked, 16);
	//--------------- NORMAL HOOKS ---------------//
	VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
	o_DoPostScreenEffects = (DoPostScreenEffects_t)VMTClientMode.HookMethod((DWORD)Hooked_DoPostScreenEffects, 44);
	VMTPanel.Initialise((DWORD*)Interfaces::Panels);
	oPaintTraverse = (PaintTraverse_)VMTPanel.HookMethod((DWORD)&PaintTraverse_Hooked, Offsets::VMT::Panel_PaintTraverse);
	VMTPrediction.Initialise((DWORD*)Interfaces::Prediction);
	VMTPrediction.HookMethod((DWORD)&Hooked_InPrediction, 14);
	VMTModelRender.Initialise((DWORD*)Interfaces::ModelRender);
	oDrawModelExecute = (DrawModelEx_)VMTModelRender.HookMethod((DWORD)&Hooked_DrawModelExecute, Offsets::VMT::ModelRender_DrawModelExecute);
	VMTClientMode.Initialise((DWORD*)Interfaces::ClientMode);
	VMTClientMode.HookMethod((DWORD)CreateMoveClient_Hooked, 24);
	oOverrideView = (OverrideViewFn)VMTClientMode.HookMethod((DWORD)&Hooked_OverrideView, 18);
	VMTClientMode.HookMethod((DWORD)&GGetViewModelFOV, 35);
	VMTClient.Initialise((DWORD*)Interfaces::Client);
	oFrameStageNotify = (FrameStageNotifyFn)VMTClient.HookMethod((DWORD)&Hooked_FrameStageNotify, 37);
	VMTEventManager.Initialise((DWORD*)Interfaces::EventManager);
	oFireEventClientSide = (FireEventClientSideFn)VMTEventManager.HookMethod((DWORD)&Hooked_FireEventClientSide, 9);
	VMTRenderView.Initialise((DWORD*)Interfaces::RenderView);
	pSceneEnd = (SceneEnd_t)VMTRenderView.HookMethod((DWORD)&hkSceneEnd, 9);
	VMTSurface.Initialise((DWORD*)Interfaces::Surface);
	oLockCursor = (LockCursor)VMTSurface.HookMethod((DWORD)Hooked_LockCursor, 67);
	Interfaces::CVar->ConsoleColorPrintf(Color(50, 255, 50, 255), ("[AutismH00k RECODE] "));
	Interfaces::CVar->ConsoleColorPrintf(Color(255, 255, 255, 255), ("Injected Succesfully.\n-Fixed CFG\n-Redesigned menu\n-Added features (Grenade trace, BuyBot etc)\n-Fixed Color issues\n"));

	ConVar* nameVar = Interfaces::CVar->FindVar("name");
	//--------------- NAME CVAR ---------------//
	if (nameVar)
	{
		*(int*)((DWORD)&nameVar->fnChangeCallback + 0xC) = 0;
	}
	//--------------- EVENT LOG ---------------//
	static auto y = Interfaces::CVar->FindVar("sv_showanimstate"); //this probably isn't avaible in modern source
	y->SetValue(1);
	static auto developer = Interfaces::CVar->FindVar("developer");
	developer->SetValue(1);
	static auto con_filter_text_out = Interfaces::CVar->FindVar("con_filter_text_out");
	static auto con_filter_enable = Interfaces::CVar->FindVar("con_filter_enable");
	static auto con_filter_text = Interfaces::CVar->FindVar("con_filter_text");
	static auto dogstfu = Interfaces::CVar->FindVar("con_notifytime");
	dogstfu->SetValue(3);
	con_filter_text->SetValue(".     ");
	con_filter_text_out->SetValue("");
	con_filter_enable->SetValue(2);
}


int __stdcall Hooked_DoPostScreenEffects(int a1)
{
	auto m_local = hackManager.pLocal();

	for (auto i = 0; i < Interfaces::m_pGlowObjManager->size; i++)
	{
		auto glow_object = &Interfaces::m_pGlowObjManager->m_GlowObjectDefinitions[i];
		IClientEntity *m_entity = glow_object->m_pEntity;
		if (!glow_object->m_pEntity || glow_object->IsUnused() || !m_local)
			continue;
		if (m_entity->GetClientClass()->m_ClassID == 1 || m_entity->GetClientClass()->m_ClassID == 66 || m_entity->GetClientClass()->m_ClassID == 87 || m_entity->GetClientClass()->m_ClassID == 98 || m_entity->GetClientClass()->m_ClassID == 108 || m_entity->GetClientClass()->m_ClassID == 134 || m_entity->GetClientClass()->m_ClassID == 1 || m_entity->GetClientClass()->m_ClassID == 9 || m_entity->GetClientClass()->m_ClassID == 29 || m_entity->GetClientClass()->m_ClassID == 39 || m_entity->GetClientClass()->m_ClassID == 41 || strstr(m_entity->GetClientClass()->m_pNetworkName, "Weapon")) {
			if (Options::Menu.ColorsTab.OtherEntityGlow.GetState()) {
				float m_flRed = Options::Menu.ColorsTab.GlowOtherEnt.GetValue()[0], m_flGreen = Options::Menu.ColorsTab.GlowOtherEnt.GetValue()[1], m_flBlue = Options::Menu.ColorsTab.GlowOtherEnt.GetValue()[2];
				bool m_teammate = m_entity->GetTeamNum() == m_local->GetTeamNum();
				glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
				glow_object->m_flGlowAlpha = 1.f;
				glow_object->m_bRenderWhenOccluded = true;
				glow_object->m_bRenderWhenUnoccluded = false;
			}
		}
		if (m_entity->GetClientClass()->m_ClassID == 35) {
			if (m_entity == m_local && Options::Menu.VisualsTab.OptionsGlow_lcl.GetState()) {
				if (m_local->IsAlive())
				{
					float m_flRed = Options::Menu.ColorsTab.GlowLocal.GetValue()[0], m_flGreen = Options::Menu.ColorsTab.GlowLocal.GetValue()[1], m_flBlue = Options::Menu.ColorsTab.GlowLocal.GetValue()[2];
					glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
					glow_object->m_flGlowAlpha = Options::Menu.VisualsTab.Glowz_lcl.GetValue() / 100;
					glow_object->m_bRenderWhenOccluded = true;
					glow_object->m_bRenderWhenUnoccluded = false;
					if (Options::Menu.VisualsTab.PulsatingChams.GetState()){
						glow_object->m_bPulsatingChams = 1;
					}
				}
			}
			else if (!(m_entity->GetTeamNum() == m_local->GetTeamNum())) {
				if (Options::Menu.VisualsTab.OptionsGlow.GetState()) {
					float m_flRed = Options::Menu.ColorsTab.GlowEnemy.GetValue()[0], m_flGreen = Options::Menu.ColorsTab.GlowEnemy.GetValue()[1], m_flBlue = Options::Menu.ColorsTab.GlowEnemy.GetValue()[2];
					bool m_teammate = m_entity->GetTeamNum() == m_local->GetTeamNum();
					glow_object->m_vGlowColor = Vector(m_flRed / 255, m_flGreen / 255, m_flBlue / 255);
					glow_object->m_flGlowAlpha = Options::Menu.VisualsTab.GlowZ.GetValue() / 100;
					glow_object->m_bRenderWhenOccluded = true;
					glow_object->m_bRenderWhenUnoccluded = false;
				}
			}
		}
	}
	return o_DoPostScreenEffects(Interfaces::ClientMode, a1);
}
HRESULT __stdcall Reset_hooked(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	auto hr = o_Reset(pDevice, pPresentationParameters);
	if (hr >= 0)
	{
		bool gey;
		gey = true;
	}
	return hr;
}
struct CUSTOMVERTEX {
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
};
#define M_PI 3.14159265358979323846
void CircleFilledRainbowColor(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;
	std::vector<CUSTOMVERTEX> circle(resolution + 2);
	float angle = rotate * M_PI / 180, pi = M_PI;
	if (type == 1)
		pi = M_PI; // Full circle
	if (type == 2)
		pi = M_PI / 2; // 1/2 circle
	if (type == 3)
		pi = M_PI / 4; // 1/4 circle
	pi = M_PI / type; // 1/4 circle
	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);
	float hue = 0.f;
	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		auto clr = Color::FromHSB(hue, 1.f, 1.f);
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), clr.a() - 175);
		hue += 0.02;
	}
	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));
		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}
	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);
	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();
	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
#define M_PI 3.14159265358979323846
#define MASK_SHOT_BRUSHONLY			(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS)
void DrawTaserRange()
{
	Vector prev_scr_pos, scr_pos;
	IClientEntity* pLocalPlayer = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	C_BaseCombatWeapon *local_weapon = pLocalPlayer->GetWeapon2();
	if (!local_weapon->isZeus27())
		return;
	float step = M_PI * 2.0 / 2047; //adjust if you need 1-5 extra fps lol
	float rad = local_weapon->GetCSWpnData()->range;
	Vector origin = pLocalPlayer->GetEyePosition();
	static float hue_offset = 0;
	for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
	{
		Vector pos(rad * cos(rotation) + origin.x, rad * sin(rotation) + origin.y, origin.z);
		Ray_t ray;
		trace_t trace;
		CTraceFilter filter;
		filter.pSkip = pLocalPlayer;
		ray.Init(origin, pos);
		Interfaces::Trace->TraceRay(ray, MASK_SHOT_BRUSHONLY, &filter, &trace);
		if (Render::WorldToScreen(trace.endpos, scr_pos))
		{
			if (prev_scr_pos.IsValid())
			{
				int hue = RAD2DEG(rotation) + hue_offset;
				static float rainbow;
				rainbow += 0.0004f;
				if (rainbow > 1.f) rainbow = 0.f;
				Color color = Color::FromHSB(rainbow, 1.f, 1.f);
				color = Color::FromHSB(hue / 360.f, 1, 1);
				Interfaces::Surface->DrawSetColor(color);
				Interfaces::Surface->DrawLine(prev_scr_pos.x, prev_scr_pos.y, scr_pos.x, scr_pos.y);
			}
			prev_scr_pos = scr_pos;
		}
	}
	hue_offset += 0.25;
}
void greyone(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;
	std::vector<CUSTOMVERTEX> circle(resolution + 2);
	float angle = rotate * M_PI / 180, pi = M_PI;
	if (type == 1)
		pi = M_PI; // Full circle
	if (type == 2)
		pi = M_PI / 2; // 1/2 circle
	if (type == 3)
		pi = M_PI / 4; // 1/4 circle
	pi = M_PI / type; // 1/4 circle
	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);
	float hue = 0.f;
	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		auto clr = Color(15, 15, 15);
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), clr.a() - 175);
		hue += 0.02;
	}
	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));
		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}
	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);
	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();
	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
void colorboy69(float x, float y, float rad, float rotate, int type, int resolution, IDirect3DDevice9* m_device)
{
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;
	std::vector<CUSTOMVERTEX> circle(resolution + 2);
	float angle = rotate * M_PI / 180, pi = M_PI;
	if (type == 1)
		pi = M_PI; // Full circle
	if (type == 2)
		pi = M_PI / 2; // 1/2 circle
	if (type == 3)
		pi = M_PI / 4; // 1/4 circle
	pi = M_PI / type; // 1/4 circle
	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	circle[0].color = D3DCOLOR_RGBA(0, 0, 0, 0);
	float hue = 0.f;
	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		auto clr = (Color)Options::Menu.ColorsTab.spreadcrosscol.GetValue();
		circle[i].color = D3DCOLOR_RGBA(clr.r(), clr.g(), clr.b(), clr.a() - 175);
		hue += 0.02;
	}
	// Rotate matrix
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		float Vx1 = x + (cosf(angle) * (circle[i].x - x) - sinf(angle) * (circle[i].y - y));
		float Vy1 = y + (sinf(angle) * (circle[i].x - x) + cosf(angle) * (circle[i].y - y));
		circle[i].x = Vx1;
		circle[i].y = Vy1;
	}
	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);
	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();
	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}
HRESULT __stdcall EndScene_hooked(IDirect3DDevice9 *pDevice)
{
	//this will probably get drawn even over the console and other CSGO hud elements, but whatever
	//this will also draw over the menu so we should disable it if the menu is open
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	auto g_LocalPlayer = pLocal;
	if (g_LocalPlayer && g_LocalPlayer->IsAlive() && g_LocalPlayer->GetWeapon2()) {
		int w, h;
		Interfaces::Engine->GetScreenSize(w, h); w /= 2; h /= 2;
		if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
		{
			int w, h;
			static float rot = 0.f;
			Interfaces::Engine->GetScreenSize(w, h); w /= 2; h /= 2;
			C_BaseCombatWeapon* pWeapon = g_LocalPlayer->GetWeapon2();
			if (pWeapon)
			{
				short Index = (int)pWeapon->GetItemDefinitionIndex();
				if (Index == WEAPON_TASER) {
					//DrawTaserRange();
				}
				if (g_LocalPlayer && Index != 42 && Index != 59 && Index != 500)
				{
					if (Options::Menu.VisualsTab.SpreadCross.GetState() && !Options::Menu.m_bIsOpen) {
						auto accuracy = pWeapon->GetInaccuracy() * (Options::Menu.VisualsTab.SpreadCrossSize.GetValue() * 6.5);
						if (Options::Menu.VisualsTab.SpreadCrosshair.GetIndex() == 0)
							greyone(w, h, accuracy, 0, 1, 50, pDevice);
						if (Options::Menu.VisualsTab.SpreadCrosshair.GetIndex() == 1)
							colorboy69(w, h, accuracy, 0, 1, 50, pDevice);
						if (Options::Menu.VisualsTab.SpreadCrosshair.GetIndex() == 2)
							CircleFilledRainbowColor(w, h, accuracy, 0, 1, 50, pDevice);
						if (Options::Menu.VisualsTab.SpreadCrosshair.GetIndex() == 3)
							CircleFilledRainbowColor(w, h, accuracy, rot, 1, 50, pDevice);
						rot += 1.f;
						if (rot > 360.f) rot = 0.f;
					}
				}
			}
		}
	}
	return o_EndScene(pDevice);
}
void MovementCorrection(CUserCmd* userCMD)
{
	if (userCMD->forwardmove) {
		userCMD->buttons &= ~(userCMD->forwardmove < 0 ? IN_FORWARD : IN_BACK);
		userCMD->buttons |= (userCMD->forwardmove > 0 ? IN_FORWARD : IN_BACK);
	}
	if (userCMD->sidemove) {
		userCMD->buttons &= ~(userCMD->sidemove < 0 ? IN_MOVERIGHT : IN_MOVELEFT);
		userCMD->buttons |= (userCMD->sidemove > 0 ? IN_MOVERIGHT : IN_MOVELEFT);
	}

}
float clip(float n, float lower, float upper)
{
	return (std::max)(lower, (std::min)(n, upper));
}
int kek = 0;
int autism = 0;
int speed = 0;
static float testtimeToTick;
static float testServerTick;
static float testTickCount64 = 1;
void RotateMovement(CUserCmd * pCmd, float rotation)
{
	rotation = DEG2RAD(rotation);
	float cosr, sinr;
	cosr = cos(rotation);
	sinr = sin(rotation);
	float forwardmove, sidemove;
	forwardmove = (cosr * pCmd->forwardmove) - (sinr * pCmd->sidemove);
	sidemove = (sinr * pCmd->forwardmove) - (cosr * pCmd->sidemove);
	pCmd->forwardmove = forwardmove;
	pCmd->sidemove = sidemove;
}
void cstrafe_run(CUserCmd* m_pcmd)
{
	auto m_local = hackManager.pLocal();
	if (m_local && m_local->IsAlive())
	{
		if (Options::Menu.MiscTab.CircleStrafe.GetState()) {
			Vector View(m_pcmd->viewangles);
			float niglet = 0;
			m_pcmd->forwardmove = 450.f;
			int random = rand() % 100;
			int random2 = rand() % 1000;
			static bool dir;
			static float current_y = View.y;
			if (m_local->GetVelocity().Length() > 50.f)
			{
				niglet += 0.00007;
				current_y += 3 - niglet;
			}
			else
			{
				niglet = 0;
			}
			View.y = current_y;
			if (random == random2)
				View.y += random;
			// Clamp (View);
			char bufferxf[64];
			sprintf_s(bufferxf, "Circlestrafer running c_yaw: %1f", current_y);
			RotateMovement(m_pcmd, current_y);
		}
		else
		{
			float niglet = 0;
		}
	}
}

void cstrafe_start(CUserCmd* m_pcmd)
{
	auto m_local = hackManager.pLocal();
	if (m_local && m_local->IsAlive()) {
		if (Options::Menu.MiscTab.CircleStrafe.GetState()) {
			Vector View(m_pcmd->viewangles);
			float niglet = 0;
			m_pcmd->forwardmove = 450.f;
			int random = rand() % 100;
			int random2 = rand() % 1000;
			static bool dir;
			static float current_y = View.y;
			if (m_local->GetVelocity().Length() > 50.f)
			{
				niglet += 0.00007;
				current_y += 3 - niglet;
			}
			else
			{
				niglet = 0;
			}
			View.y = current_y;
			if (random == random2)
				View.y += random;
			// Clamp (View);
			RotateMovement(m_pcmd, current_y);
		}
		else
		{
			float niglet = 0;
		}
	}
}

float NormalizeYaw(float value)
{
	while (value > 180)
		value -= 360.f;
	while (value < -180)
		value += 360.f;
	return value;
}
float random_float(float min, float max)
{
	typedef float(*RandomFloat_t)(float, float);
	static RandomFloat_t m_RandomFloat = (RandomFloat_t)GetProcAddress(GetModuleHandle(("vstdlib.dll")), ("RandomFloat"));
	return m_RandomFloat(min, max);
}
LinearExtrapolations linear_extraps;
bool __stdcall CreateMoveClient_Hooked(float frametime, CUserCmd* pCmd)
{
	
	if (!pCmd->command_number)
		return true;
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());


	uintptr_t* FPointer; __asm { MOV FPointer, EBP }
	byte* SendPacket = (byte*)(*FPointer - 0x1C);
	GlobalBREAK::bSendPacket = *SendPacket;
	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{


		static bool abc = false;
		static bool freakware = false;
		if (Options::Menu.ColorsTab.colmodupdate.GetState())
		{
			if (!freakware)
			{
				ConVar* staticdrop = Interfaces::CVar->FindVar("r_DrawSpecificStaticProp");
				SpoofedConvar* staticdrop_spoofed = new SpoofedConvar(staticdrop);
				staticdrop_spoofed->SetInt(0);
				for (MaterialHandle_t i = Interfaces::MaterialSystem->FirstMaterial(); i != Interfaces::MaterialSystem->InvalidMaterial(); i = Interfaces::MaterialSystem->NextMaterial(i))
				{
					IMaterial *pMaterial = Interfaces::MaterialSystem->GetMaterial(i);
					if (!pMaterial)
						continue;
					if (!pMaterial || pMaterial->IsErrorMaterial())
						continue;

					float sky_r = Options::Menu.ColorsTab.sky_r.GetValue() / 10;
					float sky_g = Options::Menu.ColorsTab.sky_g.GetValue() / 10;
					float sky_b = Options::Menu.ColorsTab.sky_b.GetValue() / 10;
					if (strstr(pMaterial->GetTextureGroupName(), ("SkyBox")))
					{
						Color geay = Color(Options::Menu.ColorsTab.Skyboxcolor.GetValue());
						if (Options::Menu.ColorsTab.ModulateSkyBox.GetState())
							pMaterial->ColorModulate(sky_r, sky_g, sky_b);
						else {
							pMaterial->ColorModulation(0.6, 0, 0.9);
						}
					}
					float test = Options::Menu.ColorsTab.asusamount.GetValue() / 100;
					float amountr = Options::Menu.ColorsTab.colmod.GetValue() / 100;
					if (Options::Menu.ColorsTab.colmod.GetValue() < 99)
					{
						if (Options::Menu.ColorsTab.customskies.GetIndex() == 0)
						{
							ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
							*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
							NightSkybox1->SetValue("sky_day02_05");
							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->AlphaModulate(test);
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
						}
						if (Options::Menu.ColorsTab.customskies.GetIndex() == 1)
						{
							ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
							*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
							NightSkybox1->SetValue("sky_csgo_night02b");
							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->AlphaModulate(test);
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
						}
						if (Options::Menu.ColorsTab.customskies.GetIndex() == 2)
						{
							ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
							*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
							NightSkybox1->SetValue("sky_csgo_night02");



							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->AlphaModulate(test);
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
						}
						if (Options::Menu.ColorsTab.customskies.GetIndex() == 3)
						{
							ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
							*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
							NightSkybox1->SetValue("sky_l4d_rural02_ldr");
							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->AlphaModulate(test);
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
						}
						if (Options::Menu.ColorsTab.customskies.GetIndex() == 4)
						{
							ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
							*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
							NightSkybox1->SetValue("vertigoblue_hdr");
							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->AlphaModulate(test);
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
						}
						if (Options::Menu.ColorsTab.customskies.GetIndex() == 5)
						{
							ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
						*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
						NightSkybox1->SetValue("jungle");
						if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
						{
							pMaterial->ColorModulation(amountr, amountr, amountr);
						}
						if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
						{
							pMaterial->AlphaModulate(test);
							pMaterial->ColorModulation(amountr, amountr, amountr);
						}
						}
						if (Options::Menu.ColorsTab.customskies.GetIndex() == 6)
						{
							ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
							*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
							NightSkybox1->SetValue("cs_baggage_skybox_");
							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->AlphaModulate(test);
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
						}
						if (Options::Menu.ColorsTab.customskies.GetIndex() == 7)
						{
							ConVar* NightSkybox1 = Interfaces::CVar->FindVar("sv_skyname");
							*(float*)((DWORD)&NightSkybox1->fnChangeCallback + 0xC) = NULL;
							NightSkybox1->SetValue("embassy");
							if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
							{
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
							if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
							{
								pMaterial->AlphaModulate(test);
								pMaterial->ColorModulation(amountr, amountr, amountr);
							}
						}

					}
					else
					{
						if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls	
						{
							pMaterial->ColorModulation(amountr, amountr, amountr);
						}
						if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
						{
							pMaterial->AlphaModulate(test);
							pMaterial->ColorModulation(amountr, amountr, amountr);
						}
					}
				}
			}
			freakware = true;
			Options::Menu.ColorsTab.colmodupdate.SetState(false);
		}
		else
		{
			freakware = false;
		}
		pCmd->sidemove = pCmd->sidemove;
		pCmd->upmove = pCmd->upmove;
		pCmd->forwardmove = pCmd->forwardmove;




		GlobalBREAK::bSendPacket = *SendPacket;
		if (GetAsyncKeyState(Options::Menu.MiscTab.manualleft.GetKey()))
		{
			bigboi::indicator = 1;
		}
		else if (GetAsyncKeyState(Options::Menu.MiscTab.manualright.GetKey()))
		{
			bigboi::indicator = 2;
		}
		else if (GetAsyncKeyState(Options::Menu.MiscTab.manualback.GetKey()))
		{
			bigboi::indicator = 3;
		}
		else if (GetAsyncKeyState(Options::Menu.MiscTab.manualfront.GetKey()))
		{
			bigboi::indicator = 4;
		}

		if (Options::Menu.MiscTab.AutoDefuse.GetState())
			defususmaximus(pCmd);
		if (Options::Menu.RageBotTab.extrapolation.GetState())
			linear_extraps.run();
		GlobalBREAK::smt = frametime;
		GlobalBREAK::userCMD = pCmd;
		GlobalBREAK::vecUnpredictedVel = pLocal->GetVelocity();
		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;
		uintptr_t* framePtr;
		__asm mov framePtr, ebp;
		GlobalBREAK::bSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		if (pLocal && pLocal->IsAlive()) {
			if (pLocal->GetFlags() & FL_ONGROUND && !(headPositions[pLocal->GetIndex()][0].flags & FL_ONGROUND)) {
				if (GetAsyncKeyState(VK_SPACE)) {
					*(bool*)(*(DWORD*)pebp - 0x1C) = true;
				}
			}
			if (pLocal->GetFlags() & FL_ONGROUND && !(headPositions[pLocal->GetIndex()][2].flags & FL_ONGROUND)) {
				if (GetAsyncKeyState(VK_SPACE)) {
					*(bool*)(*(DWORD*)pebp - 0x1C) = true;
				}
			}
			if (pLocal->GetFlags() & FL_ONGROUND && !(headPositions[pLocal->GetIndex()][1].flags & FL_ONGROUND)) {
				if (GetAsyncKeyState(VK_SPACE)) {
					*(bool*)(*(DWORD*)pebp - 0x1C) = true;
				}
			}
		}
		if (pLocal && pLocal->IsAlive()) {
			if (pLocal->GetFlags() & FL_ONGROUND && !(CMBacktracking::Get().current_record->m_nFlags & FL_ONGROUND)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = true;
			}
			if (pLocal->GetFlags() & FL_ONGROUND && Interfaces::m_iInputSys->IsButtonDown(KEY_SPACE)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = false;
			}
			if (!pLocal->GetFlags() & FL_ONGROUND && !Interfaces::m_iInputSys->IsButtonDown(KEY_SPACE)) {
				*(bool*)(*(DWORD*)pebp - 0x1C) = false;
			}
			for (int i = 1; i < 2; i++)
			{
				if (pLocal->GetFlags() & FL_ONGROUND && !(headPositions[pLocal->GetIndex()][i].flags)) {
					*(bool*)(*(DWORD*)pebp - 0x1C) = true;
				}
			}
		}
		if (bSendPacket || GlobalBREAK::bSendPacket) {
			GlobalBREAK::fakeangleslocal = pLocal->GetEyeAngles();
		}

		if (pLocal->GetFlags() & FL_ONGROUND)
			GlobalBREAK::TicksOnGround++;
		else
			GlobalBREAK::TicksOnGround = 0;
		if (GlobalBREAK::bSendPacket)
			GlobalBREAK::prevChoked = Interfaces::m_pClientState->chokedcommands;
		if (!GlobalBREAK::bSendPacket)
			GlobalBREAK::visualAngles = QAngle(pCmd->viewangles.x, pCmd->viewangles.y, pCmd->viewangles.z);
		if (GlobalBREAK::TicksOnGround == 1)
			*(bool*)(*(DWORD*)pebp - 0x1C) = false;
		if (GlobalBREAK::TicksOnGround == 1 && pLocal->getFlags() & FL_ONGROUND)
			*(bool*)(*(DWORD*)pebp - 0x1C) = false;
		if (GlobalBREAK::TicksOnGround == 0 && pLocal->GetFlags() & FL_ONGROUND)
			*(bool*)(*(DWORD*)pebp - 0x1C) = false;
		CMBacktracking::Get().StartLagCompensation(pLocal);
		if (Options::Menu.RageBotTab.EnginePrediction.GetState())
			start_prediction(pCmd);
		globalsh.bSendPaket = true;
		if (Options::Menu.MiscTab.FakeLagChoke.GetValue() > 0 || Options::Menu.MiscTab.FakeLagChoke2.GetValue() > 0)
			globalsh.bSendPaket = false;
		if (Interfaces::m_pClientState->chokedcommands > 13 || (Interfaces::m_pClientState->chokedcommands == globalsh.ChokeAmount &&
			(Options::Menu.MiscTab.FakeLagChoke.GetValue() > 0 || Options::Menu.MiscTab.FakeLagChoke2.GetValue() > 0)))
			globalsh.bSendPaket = false;
		globalsh.bSendPaket = (bool*)(*(DWORD*)pebp - 0x1C);
		Vector origView = pCmd->viewangles;
		Vector viewforward, viewright, viewup, aimforward, aimright, aimup;
		Vector qAimAngles;
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		if (globalsh.bSendPaket)
			globalsh.prevChoked = Interfaces::m_pClientState->chokedcommands;
		IClientEntity* pEntity;
		if (Options::Menu.MiscTab.CircleStrafe.GetState()) {
			if (!(pLocal->GetFlags() & FL_ONGROUND))
			{
				if (GUI.GetKeyState(Options::Menu.MiscTab.CircleStrafeKey.GetKey()))
				{
					cstrafe_run(pCmd);
				}
			}
		}
		if (Options::Menu.MiscTab.SniperCrosshair.GetState() && pLocal && pLocal->IsAlive() && !pLocal->IsScoped()) {
			if (pLocal->GetWeapon2() && pLocal->GetWeapon2()->m_bIsSniper()) {
				ConVar* cross = Interfaces::CVar->FindVar("weapon_debug_spread_show");
				cross->nFlags &= ~FCVAR_CHEAT;
				cross->SetValue(3);
			}
		}
		else {
			ConVar* cross = Interfaces::CVar->FindVar("weapon_debug_spread_show");
			cross->nFlags &= ~FCVAR_CHEAT;
			cross->SetValue(0);
		}
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal && pLocal->IsAlive())
		{
			Hacks::MoveHacks(pCmd, bSendPacket);
			if (pCmd->forwardmove) {
				pCmd->buttons &= ~(pCmd->forwardmove < 0 ? IN_FORWARD : IN_BACK);
				pCmd->buttons |= (pCmd->forwardmove > 0 ? IN_FORWARD : IN_BACK);
			}
			if (pCmd->sidemove) {
				pCmd->buttons &= ~(pCmd->sidemove < 0 ? IN_MOVERIGHT : IN_MOVELEFT);
				pCmd->buttons |= (pCmd->sidemove > 0 ? IN_MOVERIGHT : IN_MOVELEFT);
			}
			ResolverSetup::GetInst().CM(pEntity);
		}
		Prediction->EnginePrediction(pCmd);
		{
			Vector AimPoint;
			backtracking->legitBackTrack(pCmd, pLocal);
			backtracking->rageBackTrack(pCmd, pLocal);
			backtracking->RunLBYBackTrack(pCmd, AimPoint);
			rage->DoAimbot(pCmd, bSendPacket);
			rage->DoNoRecoil(pCmd);
		}
#pragma region Timer4LBY
		static float myOldLby;
		static float myoldTime;
		testtimeToTick = TIME_TO_TICKS(0.1);
		testServerTick = TIME_TO_TICKS(1);
		static int timerino;
		static float oneTickMinues;
		if (testServerTick == 128) {
			oneTickMinues = testServerTick / 128;
		}
		else {
			oneTickMinues = testServerTick / 64;
		}
#pragma endregion
		IClientEntity* LocalPlayer = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
		float flServerTime = (float)(LocalPlayer->GetTickBase()  * Interfaces::Globals->interval_per_tick);
		static float next_time = 0;
		if (!bSendPacket && LocalPlayer->IsAlive() && LocalPlayer->GetVelocity().Length2D() == 0) {
			float TickStuff = TICKS_TO_TIME(LocalPlayer->GetTickBase());
			Globals::Up2date = false;
			//flServerTime = next_time;
			if (next_time - TICKS_TO_TIME(LocalPlayer->GetTickBase()) > 1.1)
			{
				next_time = 0;
			}
			if (TickStuff > next_time + 1.1f)
			{
				next_time = TickStuff + TICKS_TO_TIME(1);
				Globals::Up2date = true;
			}
		}
		qAimAngles.Init(0.0f, GetAutostrafeView().y, 0.0f);
		AngleVectors(qAimAngles, &viewforward, &viewright, &viewup);
		qAimAngles.Init(0.0f, pCmd->viewangles.y, 0.0f);
		AngleVectors(qAimAngles, &aimforward, &aimright, &aimup);
		Vector vForwardNorm;		Normalize(viewforward, vForwardNorm);
		Vector vRightNorm;			Normalize(viewright, vRightNorm);
		Vector vUpNorm;				Normalize(viewup, vUpNorm);
		float forward = pCmd->forwardmove;
		float right = pCmd->sidemove;
		float up = pCmd->upmove;
		if (forward > 450) forward = 450;
		if (right > 450) right = 450;
		if (up > 450) up = 450;
		if (forward < -450) forward = -450;
		if (right < -450) right = -450;
		if (up < -450) up = -450;
		pCmd->forwardmove = DotProduct(forward * vForwardNorm, aimforward) + DotProduct(right * vRightNorm, aimforward) + DotProduct(up * vUpNorm, aimforward);
		pCmd->sidemove = DotProduct(forward * vForwardNorm, aimright) + DotProduct(right * vRightNorm, aimright) + DotProduct(up * vUpNorm, aimright);
		pCmd->upmove = DotProduct(forward * vForwardNorm, aimup) + DotProduct(right * vRightNorm, aimup) + DotProduct(up * vUpNorm, aimup);
		MovementCorrection(pCmd); //slide fix
		if (Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 0 || Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 1 || Options::Menu.MiscTab.OtherSafeMode.GetIndex() == 2)
		{
			GameUtils::NormaliseViewAngle(pCmd->viewangles);
			if (pCmd->viewangles.z != 0.0f)
			{
				pCmd->viewangles.z = 0.00;
			}
			if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
			{
				Utilities::Log(" Re-calculating angles");
				GameUtils::NormaliseViewAngle(pCmd->viewangles);
				if (pCmd->viewangles.x < -89 || pCmd->viewangles.x > 89 || pCmd->viewangles.y < -180 || pCmd->viewangles.y > 180)
				{
					pCmd->viewangles = origView;
					pCmd->sidemove = right;
					pCmd->forwardmove = forward;
				}
			}
		}
		if (pCmd->viewangles.x > 90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}
		if (pCmd->viewangles.x < -90)
		{
			pCmd->forwardmove = -pCmd->forwardmove;
		}
		if (bSendPacket == true)
		{
			LastAngleAA = pCmd->viewangles;
			current_real = pCmd->viewangles.y;
		}
		else if (bSendPacket == false) { //
			LastAngleAAReal = pCmd->viewangles;
			lineLBY2 = NormalizeYaw(pCmd->viewangles.y - *pLocal->GetLowerBodyYawTarget());
		}
		if (bSendPacket == true)
		{
			//last_fake = pLocal->StupidFuckingAbsOrigin();
			lineFakeAngle = pCmd->viewangles.y;
			LastAngleAAFake = Vector(pLocal->GetEyeAnglesXY()->x, pLocal->GetEyeAnglesXY()->y, pLocal->GetEyeAnglesXY()->z);
		}
		else if (bSendPacket == false) {
			lineRealAngle = pCmd->viewangles.y;
		}
		lineLBY = pLocal->GetLowerBodyYaw();

		if (pLocal->IsAlive() && !GameUtils::IsGrenade(pWeapon) && GameUtils::IsBallisticWeapon(pWeapon))
		{
			inaccuracy = pWeapon->GetInaccuracy() * 1000;
			lspeed = pLocal->GetVelocity().Length2D();
			pitchmeme = pCmd->viewangles.x;
		}
		else if (pLocal->IsAlive() && !GameUtils::IsGrenade(pWeapon) && !GameUtils::IsBallisticWeapon(pWeapon))
		{
			lspeed = pLocal->GetVelocity().Length2D();
			pitchmeme = pCmd->viewangles.x;
		}
	}
	return false;
}
void StartPrediction(IClientEntity* LocalPlayer, CUserCmd* pCmd) //in case you cant tell, this is pasted
{
	static bool done = false;
	if (LocalPlayer->IsAlive() && Options::Menu.RageBotTab.resolver.GetIndex() != 0 && !done)
	{
		Interfaces::CVar->FindVar("cl_interp")->SetValue(0.01f);
		Interfaces::CVar->FindVar("cl_cmdrate")->SetValue(66);
		Interfaces::CVar->FindVar("cl_updaterate")->SetValue(66);
		Interfaces::CVar->FindVar("cl_interp_all")->SetValue(0.0f);
		Interfaces::CVar->FindVar("cl_interp_ratio")->SetValue(1.0f);
		Interfaces::CVar->FindVar("cl_smooth")->SetValue(0.0f);
		Interfaces::CVar->FindVar("cl_smoothtime")->SetValue(0.01f);
		done = true;
	}
}
static void drawThiccLine(int x1, int y1, int x2, int y2, int type, Color color) {
	if (type > 1) {
		Render::Line(x1, y1 - 1, x2, y2 - 1, color);
		Render::Line(x1, y1, x2, y2, color);
		Render::Line(x1, y1 + 1, x2, y2 + 1, color);
		Render::Line(x1, y1 - 2, x2, y2 - 2, color);
		Render::Line(x1, y1 + 2, x2, y2 + 2, color);
	}
	else {
		Render::Line(x1 - 1, y1, x2 - 1, y2, color);
		Render::Line(x1, y1, x2, y2, color);
		Render::Line(x1 + 1, y1, x2 + 1, y2, color);
		Render::Line(x1 - 2, y1, x2 - 2, y2, color);
		Render::Line(x1 + 2, y1, x2 + 2, y2, color);
	}
}
const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y | %d | %X", &tstruct);
	return buf;
}
Color urmamasuckmylargegenetalia(int speed, int offset)
{
	float hue = (float)((GetCurrentTime() + offset) % speed);
	hue /= speed;
	return Color::FromHSB(hue, 1.0F, 1.0F);
}
int BreakuLagCompensation()
{
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	Vector velocity = pLocal->GetVelocity();
	velocity.z = 0;
	float speed = velocity.Length();

	if (speed > 0.f)
	{
		auto distance_per_tick = speed * Interfaces::Globals->interval_per_tick;
		int choked_ticks = std::ceilf(65.f / distance_per_tick);
		return std::min<int>(choked_ticks, 14);
	}
}
void __fastcall PaintTraverse_Hooked(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	if (Options::Menu.VisualsTab.Active.GetState() && Options::Menu.VisualsTab.OtherNoScope.GetState() && strcmp("HudZoom", Interfaces::Panels->GetName(vguiPanel)) == 0)
		return;
	int w, h;
	int centerW, centerh, topH;
	Interfaces::Engine->GetScreenSize(w, h);
	centerW = w / 2;
	centerh = h / 2;
	static unsigned int FocusOverlayPanel = 0;
	static bool FoundPanel = false;
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!FoundPanel)
	{
		PCHAR szPanelName = (PCHAR)Interfaces::Panels->GetName(vguiPanel);
		if (strstr(szPanelName, XorStr("MatSystemTopPanel")))
		{
			FocusOverlayPanel = vguiPanel;
			FoundPanel = true;
		}
	}
	else if (FocusOverlayPanel == vguiPanel)
	{
		Interfaces::m_iInputSys->EnableInput(!Options::Menu.m_bIsOpen);

		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal->IsAlive())
		{
			CUserCmd* cmdlist = *(CUserCmd**)((DWORD)Interfaces::pInput + 0xEC);
			CUserCmd* pCmd = cmdlist;
			RECT scrn = Render::GetViewport();
			if (Options::Menu.MiscTab.Radar.GetState())
				DrawRadar();
			if (globalsh.bSendPaket) {
				globalsh.prevChoked = Interfaces::m_pClientState->chokedcommands;
			}

			if (Options::Menu.ColorsTab.ambient.GetState())
			{
				float AmbientRedAmount = Options::Menu.ColorsTab.AmbientRed.GetValue() / 10;
				float AmbientGreenAmount = Options::Menu.ColorsTab.AmbientGreen.GetValue() / 10;
				float AmbientBlueAmount = Options::Menu.ColorsTab.AmbientBlue.GetValue() / 10;

				ConVar* AmbientRedCvar = Interfaces::CVar->FindVar("mat_ambient_light_r");
				*(float*)((DWORD)&AmbientRedCvar->fnChangeCallback + 0xC) = NULL;
				AmbientRedCvar->SetValue(AmbientRedAmount);

				ConVar* AmbientGreenCvar = Interfaces::CVar->FindVar("mat_ambient_light_g");
				*(float*)((DWORD)&AmbientGreenCvar->fnChangeCallback + 0xC) = NULL;
				AmbientGreenCvar->SetValue(AmbientGreenAmount);

				ConVar* AmbientBlueCvar = Interfaces::CVar->FindVar("mat_ambient_light_b");
				*(float*)((DWORD)&AmbientBlueCvar->fnChangeCallback + 0xC) = NULL;
				AmbientBlueCvar->SetValue(AmbientBlueAmount);
			}

			if (Options::Menu.RageBotTab.AimbotEnable.GetState())
			{
				if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && pLocal->IsAlive())
				{

					bool breaklagcomp = false;


					auto last_origin = pLocal->GetAbsOrigin2();
					if (pLocal->GetAbsOrigin2() != last_origin) {
						if (!(pLocal->GetFlags() & FL_ONGROUND) && pLocal->GetAbsOrigin2().Length2DSqr() > 4096) {
							breaklagcomp = true;
							last_origin = pLocal->GetAbsOrigin2();
						}
					}

					else if (!(pLocal->GetFlags() & FL_ONGROUND) && pLocal->GetVelocity().Length2D() > 110 && GlobalBREAK::bSendPacket && GlobalBREAK::ChokeAmount == BreakuLagCompensation() || GlobalBREAK::ChokeAmount == 5) {
						breaklagcomp = true;

					}
					else {
						breaklagcomp = false;
					}

					char angle[50];
					sprintf_s(angle, sizeof(angle), "%i", *pLocal->GetLowerBodyYawTarget());
					RECT TextSize = Render::GetTextSize(Render::Fonts::LBYIndicator, "LBY");
					if (pCmd->viewangles.y - pLocal->GetLowerBodyYaw() >= -35 && pCmd->viewangles.y - pLocal->GetLowerBodyYaw() <= 35)
					{
						Render::Text(6, scrn.bottom - 75, Color(255, 0, 30, 255), Render::Fonts::LBY, "LBY");
					}
					else
					{
						if (pCmd->viewangles.y - *pLocal->GetLowerBodyYawTarget() > 120)
						{
							Render::Text(6, scrn.bottom - 75, Color(255, 120, 30, 255), Render::Fonts::LBY, "LBY");
						}
						else
						{
							Render::Text(6, scrn.bottom - 75, Color(0, 250, 60, 255), Render::Fonts::LBY, "LBY");
						}
					}
				}
			}
			if (Options::Menu.VisualsTab.manualaa.GetState())
			{
				int W, H, cW, cH;
				Interfaces::Engine->GetScreenSize(W, H);
				cW = W / 2;
				cH = H / 2;
				int rounding = 3;
				if (bigboi::indicator == 1)
				{
					drawThiccLine(cW + 45, cH, cW + 35 - rounding, cH - 10, 1, Color(120, 120, 255));// RIGHT | UP
																									 //RIGHT TARROW
					drawThiccLine(cW + 45, cH, cW + 35 - rounding, cH + 10, 1, Color(120, 120, 255)); //RIGHT DOWN
				}
				else if (bigboi::indicator == 2)
				{
					drawThiccLine(cW - 45, cH, cW - 35 + rounding, cH - 10, 0, Color(120, 120, 255)); //LEFT | UP 
																									  //LEFT ARROW
					drawThiccLine(cW - 45, cH, cW - 35 + rounding, cH + 10, 0, Color(120, 120, 255)); //LEFT | DOWN 
				}
				else if (bigboi::indicator == 3)
				{
					drawThiccLine(cW, cH + 45, cW - 10, cH + 35 - rounding, 2, Color(120, 120, 255));// DOWN | LEFT
																									 //DOWN ARROW
					drawThiccLine(cW, cH + 45, cW + 10, cH + 35 - rounding, 2, Color(120, 120, 255)); // DOWN | RIGHT
				}

			}

		}
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
			Hacks::DrawHacks();
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && Options::Menu.VisualsTab.DamageIndicator.GetState())
			damage_indicators.paint();
		if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame() && Options::Menu.VisualsTab.OtherHitmarker.GetState())
			hitmarker::singleton()->on_paint();
		Options::DoUIFrame();
	}
	oPaintTraverse(pPanels, vguiPanel, forceRepaint, allowForce);
}

#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4    
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_GEAR 10

bool __stdcall Hooked_InPrediction()
{
	bool result;
	static InPrediction_ origFunc = (InPrediction_)Hooks::VMTPrediction.GetOriginalFunction(14);
	static DWORD *ecxVal = Interfaces::Prediction;
	result = origFunc(ecxVal);
	if (Options::Menu.VisualsTab.OtherNoVisualRecoil.GetState() && (DWORD)(_ReturnAddress()) == Offsets::Functions::dwCalcPlayerView)
	{
		IClientEntity* pLocalEntity = NULL;
		float* m_LocalViewAngles = NULL;
		__asm
		{
			MOV pLocalEntity, ESI
			MOV m_LocalViewAngles, EBX
		}
		Vector viewPunch = pLocalEntity->localPlayerExclusive()->GetViewPunchAngle();
		Vector aimPunch = pLocalEntity->localPlayerExclusive()->GetAimPunchAngle();
		m_LocalViewAngles[0] -= (viewPunch[0] + (aimPunch[0] * 2 * 0.4499999f));
		m_LocalViewAngles[1] -= (viewPunch[1] + (aimPunch[1] * 2 * 0.4499999f));
		m_LocalViewAngles[2] -= (viewPunch[2] + (aimPunch[2] * 2 * 0.4499999f));
		return true;
	}
	return result;
}
int Kills2 = 0;
int Kills = 0;
bool RoundInfo = false;
size_t Delay = 0;
bool flipAA;
player_info_t GetInfo(int Index) {
	player_info_t Info;
	Interfaces::Engine->GetPlayerInfo(Index, &Info);
	return Info;
}
typedef void(__cdecl* MsgFn)(const char* msg, va_list);
void Msg(const char* msg, ...)
{
	if (msg == nullptr)
		return; //If no string was passed, or it was null then don't do anything
	static MsgFn fn = (MsgFn)GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"); 	char buffer[989];
	va_list list;
	va_start(list, msg);
	vsprintf(buffer, msg, list);
	va_end(list);
	fn(buffer, list); //Calls the function, we got the address above.
}
bool warmup = false;
void draw_hitboxes(IClientEntity* pEntity, int r, int g, int b, int a, float duration, float diameter) {
	matrix3x4 matrix[128];
	if (!pEntity->SetupBones(matrix, 128, 0x00000100, pEntity->GetSimulationTime()))
		return;
	studiohdr_t* hdr = Interfaces::ModelInfo->GetStudiomodel(pEntity->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	for (int i = 0; i < set->numhitboxes; i++) {
		mstudiobbox_t* hitbox = set->GetHitbox(i);
		if (!hitbox)
			continue;
		Vector vMin, vMax;
		auto VectorTransform_Wrapperx = [](const Vector& in1, const matrix3x4 &in2, Vector &out)
		{
			auto VectorTransform = [](const float *in1, const matrix3x4& in2, float *out)
			{
				auto DotProducts = [](const float *v1, const float *v2)
				{
					return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
				};
				out[0] = DotProducts(in1, in2[0]) + in2[0][3];
				out[1] = DotProducts(in1, in2[1]) + in2[1][3];
				out[2] = DotProducts(in1, in2[2]) + in2[2][3];
			};
			VectorTransform(&in1.x, in2, &out.x);
		};
		VectorTransform_Wrapperx(hitbox->bbmin, matrix[hitbox->bone], vMin);
		VectorTransform_Wrapperx(hitbox->bbmax, matrix[hitbox->bone], vMax);
		Interfaces::DebugOverlay->DrawPill(vMin, vMax, hitbox->m_flRadius, r, g, b, a, duration);
	}
}

auto HitgroupToString = [](int hitgroup) -> std::string
{
	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return "head";
	case HITGROUP_CHEST:
		return "chest";
	case HITGROUP_STOMACH:
		return "stomach";
	case HITGROUP_LEFTARM:
		return "left arm";
	case HITGROUP_RIGHTARM:
		return "right arm";
	case HITGROUP_LEFTLEG:
		return "left leg";
	case HITGROUP_RIGHTLEG:
		return "right leg";
	default:
		return "body";
	}
};
void ConColorMsg(Color const &color, const char* buf, ...)
{
	using ConColFn = void(__stdcall*)(Color const &, const char*, ...);
	auto ConCol = reinterpret_cast<ConColFn>((GetProcAddress(GetModuleHandle("tier0.dll"), "?ConColorMsg@@YAXABVColor@@PBDZZ")));
	ConCol(color, buf);
}



bool __fastcall Hooked_FireEventClientSide(PVOID ECX, PVOID EDX, IGameEvent *Event)
{
	CBulletListener::singleton()->OnStudioRender();
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		std::string event_name = Event->GetName();
		if (event_name.find("round_prestart") != std::string::npos || event_name.find("round_end") != std::string::npos)
		{
			if (event_name.find("round_end") != std::string::npos)
			{
				if (Options::Menu.VisualsTab.DamageIndicator.GetState())
				{
					if (strcmp(Event->GetName(), "player_hurt") == 0)
					{
						IClientEntity* hurt = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetPlayerForUserID(Event->GetInt("userid")));
						IClientEntity* attacker = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetPlayerForUserID(Event->GetInt("attacker")));
						if (hurt != localplayer && attacker == localplayer)
						{
							DamageIndicator_t DmgIndicator;
							DmgIndicator.iDamage = Event->GetInt("dmg_health");
							DmgIndicator.Player = hurt;
							DmgIndicator.flEraseTime = localplayer->GetTickBase() * Interfaces::Globals->interval_per_tick + 3.f;
							DmgIndicator.bInitialized = false;
							damage_indicators.data.push_back(DmgIndicator);


						}
					}
				}
				if (event_name.find("round_start") != std::string::npos)
				{
					Interfaces::Engine->ExecuteClientCmd("clear");
					Interfaces::CVar->ConsoleColorPrintf(Color(50, 255, 50, 255), ("[AutismH00k RECODE] TEST "));
					switch (Options::Menu.VisualsTab.BuyBot.GetIndex())
					{
					case 0:
					{
						break;
					}
					case 1:
					{
						Interfaces::Engine->ClientCmd_Unrestricted("buy scar20; buy g3sg1; buy revolver; buy vest; buy vesthelm; buy taser 34; buy defuser; buy incgrenade; buy molotov; buy hegrenade; buy smokegrenade;");
						break;
					}
					case 2:
					{
						Interfaces::Engine->ClientCmd_Unrestricted("buy scar20; buy g3sg1; buy elite; buy vest; buy vesthelm; buy taser 34; buy defuser; buy incgrenade; buy molotov; buy hegrenade; buy smokegrenade;");
						break;
					}
					case 3:
					{
						Interfaces::Engine->ClientCmd_Unrestricted("buy ssg08; buy deagle; buy vesthelm; buy defuser; buy molotov; buy incgrenade; buy hegrenade; buy smokegrenade; buy; taser");
						break;
					}
					case 4:
					{
						Interfaces::Engine->ClientCmd_Unrestricted("buy ssg08; buy elite; buy vesthelm; buy defuser; buy molotov; buy incgrenade; buy hegrenade; buy smokegrenade; buy; taser");
						break;
					}
					}
				}
				warmup = false;
			}
			else
			{
			}
			{
				warmup = true;
			}
		}
		if (event_name.find("round_freeze_end") != std::string::npos)
		{
			warmup = false;
		}
		if (event_name.find("round_end") != std::string::npos)
		{
			warmup = true;
		}

		

		if (!strcmp(Event->GetName(), "player_hurt"))
		{
			if (!localplayer->IsAlive()) {
				Interfaces::Engine->ClientCmd_Unrestricted("firstperson");
				Interfaces::pInput->m_fCameraInThirdPerson = false;
				*(float*)((DWORD)Interfaces::pInput + 0xA8 + 0x8) = 0;
			}
			Interfaces::Engine->ExecuteClientCmd("clear");
			int attackerid = Event->GetInt("attacker");
			int entityid = Interfaces::Engine->GetPlayerForUserID(attackerid);
			if (entityid == Interfaces::Engine->GetLocalPlayer())
			{
				Interfaces::Engine->ClientCmd_Unrestricted("play buttons\\arena_switch_press_02.wav");
				int nUserID = Event->GetInt("attacker");
				int nDead = Event->GetInt("userid");
				int gaylol = Event->GetInt("hitgroup");
				if (nUserID || nDead)
				{
					player_info_t killed_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nDead));
					player_info_t killer_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nUserID));
					IClientEntity* hurt = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetPlayerForUserID(Event->GetInt("userid")));
					if (Options::Menu.ColorsTab.DebugLagComp.GetState())
						draw_hitboxes(hurt, 220, 220, 220, 255, 0.9, 0);
					auto remaining_health = Event->GetString("health");
					int remainaing = Event->GetInt("health");
					auto dmg_to_health = Event->GetString("dmg_health");
					std::string szHitgroup = HitgroupToString(gaylol);
					Interfaces::CVar->ConsoleColorPrintf(Color(255, 255, 255, 255), "");
					std::string One = "Hit ";
					std::string Two = killed_info.name;
					std::string Three = " in the ";
					std::string Four = szHitgroup;
					std::string gey = " for ";
					std::string yes = dmg_to_health;
					std::string yyes = " damage ";
					std::string yyyes = remaining_health;
					std::string yyyyes = " health remaining";
					std::string newline = ".     \n";//no,i'm not retarded, i tried with stringstream but it didn't work.
					std::string uremam = One + Two + Three + Four + gey + yes + yyes + yyyes + yyyyes + newline;

					if (Options::Menu.VisualsTab.logs.GetState() && remainaing > 1 && gaylol != 1)
					{
						Msg(uremam.c_str());
					}
					else if (remainaing == 0 && Options::Menu.VisualsTab.logs.GetState() && gaylol != 1) {
						ConColorMsg(Color(250, 250, 250), uremam.c_str());
					}
					if (gaylol == 1 && Options::Menu.VisualsTab.logs.GetState()) {
						ConColorMsg(Color(Options::Menu.ColorsTab.Menu.GetValue()), uremam.c_str());
					}


				}
			}
		}



		if (!strcmp(Event->GetName(), "item_purchase"))
		{
			Interfaces::Engine->ExecuteClientCmd("clear");
			int nUserID = Event->GetInt("attacker");
			int nDead = Event->GetInt("userid");
			if (nUserID || nDead)
			{
				if (!(localplayer->GetIndex() == Interfaces::Engine->GetPlayerForUserID(nDead))) {
					player_info_t killed_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nDead));
					player_info_t killer_info = GetInfo(Interfaces::Engine->GetPlayerForUserID(nUserID));
					Interfaces::CVar->ConsoleColorPrintf(Color(255, 255, 255, 255), "");
					std::string One = killed_info.name;
					std::string Two = (" bought ");
					std::string Three = Event->GetString("weapon");
					std::string Four = ".     \n";
					if (Options::Menu.VisualsTab.logs.GetState())
					{
						std::string msg = One + Two + Three + Four;
						std::string uremam = msg;
						Msg(uremam.c_str());
					}
				}
			}
		}
		if (Options::Menu.RageBotTab.resolver.GetIndex() != 0 || Options::Menu.LegitBotTab.legitresolver.GetIndex() > 0)
		{
			IClientEntity* pLocal = hackManager.pLocal();
			if (!strcmp(Event->GetName(), "weapon_fire")) {
				auto userID = Event->GetInt("userid");
				auto attacker = Interfaces::Engine->GetPlayerForUserID(userID);
				if (attacker) {
					if (attacker == Interfaces::Engine->GetLocalPlayer()) {
						if (Globals::Target) {
							Globals::fired++;
						}
					}
				}
			}
			if (!strcmp(Event->GetName(), "player_hurt")) {
				int deadfag = Event->GetInt("userid");
				int attackingfag = Event->GetInt("attacker");
				if (Interfaces::Engine->GetPlayerForUserID(deadfag) != Interfaces::Engine->GetLocalPlayer() && Interfaces::Engine->GetPlayerForUserID(attackingfag) == Interfaces::Engine->GetLocalPlayer()) {
					Globals::hit++;
				}
			}
			if (Globals::fired > 7) {
				Globals::fired = 0;
				Globals::hit = 0;
			}
			Globals::missedshots = Globals::fired - Globals::hit;
		}
	}
	return oFireEventClientSide(ECX, Event);
}
#define TEXTURE_GROUP_LIGHTMAP                      "Lightmaps"
#define TEXTURE_GROUP_WORLD                         "World textures"
#define TEXTURE_GROUP_MODEL                         "Model textures"
#define TEXTURE_GROUP_VGUI                          "VGUI textures"
#define TEXTURE_GROUP_PARTICLE                      "Particle textures"
#define TEXTURE_GROUP_DECAL                         "Decal textures"
#define TEXTURE_GROUP_SKYBOX                        "SkyBox textures"
#define TEXTURE_GROUP_CLIENT_EFFECTS                "ClientEffect textures"
#define TEXTURE_GROUP_OTHER                         "Other textures"
#define TEXTURE_GROUP_PRECACHED                     "Precached"
#define TEXTURE_GROUP_CUBE_MAP                      "CubeMap textures"
#define TEXTURE_GROUP_RENDER_TARGET                 "RenderTargets"
#define TEXTURE_GROUP_UNACCOUNTED                   "Unaccounted textures"
#define TEXTURE_GROUP_STATIC_INDEX_BUFFER           "Static Indices"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP     "Displacement Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR    "Lighting Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD    "World Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS   "Model Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER    "Other Verts"
#define TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER          "Dynamic Indices"
#define TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER         "Dynamic Verts"
#define TEXTURE_GROUP_DEPTH_BUFFER                  "DepthBuffer"
#define TEXTURE_GROUP_VIEW_MODEL                    "ViewModel"
#define TEXTURE_GROUP_PIXEL_SHADERS                 "Pixel Shaders"
#define TEXTURE_GROUP_VERTEX_SHADERS                "Vertex Shaders"
#define TEXTURE_GROUP_RENDER_TARGET_SURFACE         "RenderTarget Surfaces"
#define TEXTURE_GROUP_MORPH_TARGETS                 "Morph Targets"
void GEYXSXS(IClientEntity* pEntity, int r, int g, int b, int a, float duration, float diameter) {
	matrix3x4 matrix[128];
	if (!pEntity->SetupBones(matrix, 128, 0x00000100, pEntity->GetSimulationTime()))
		return;
	studiohdr_t* hdr = Interfaces::ModelInfo->GetStudiomodel(pEntity->GetModel());
	mstudiohitboxset_t* set = hdr->GetHitboxSet(0);
	for (int i = 0; i < set->numhitboxes; i++) {
		mstudiobbox_t* hitbox = set->GetHitbox(i);
		if (!hitbox)
			continue;
		Vector vMin, vMax;
		auto VectorTransform_Wrapperx = [](const Vector& in1, const matrix3x4 &in2, Vector &out)
		{
			auto VectorTransform = [](const float *in1, const matrix3x4& in2, float *out)
			{
				auto DotProducts = [](const float *v1, const float *v2)
				{
					return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
				};
				out[0] = DotProducts(in1, in2[0]) + in2[0][3];
				out[1] = DotProducts(in1, in2[1]) + in2[1][3];
				out[2] = DotProducts(in1, in2[2]) + in2[2][3];
			};
			VectorTransform(&in1.x, in2, &out.x);
		};
		VectorTransform_Wrapperx(hitbox->bbmin, matrix[hitbox->bone], vMin);
		VectorTransform_Wrapperx(hitbox->bbmax, matrix[hitbox->bone], vMax);
		Interfaces::DebugOverlay->DrawPill(vMin, vMax, diameter, r, g, b, a, duration);
	}
}
void __fastcall  hkSceneEnd(void *pEcx, void *pEdx) {
	Hooks::VMTRenderView.GetMethod<SceneEnd_t>(9)(pEcx);
	pSceneEnd(pEcx);
	IClientEntity* pLocalPlayer = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (pLocalPlayer && pLocalPlayer->IsAlive())
	{
		for (int i = 1; i <= Interfaces::Globals->maxClients; i++)
		{
			auto ent = Interfaces::EntList->GetClientEntity(i);
			if (ent  && ent->GetClientClass()->m_ClassID == (int)CSGOClassID::CCSPlayer && ent->IsAlive() && !ent->IsDormant()) {
				int teamnum = ent->GetTeamNum();
				bool flat = false;
				bool wireframe = false;
				bool glass = false;
				static IMaterial* wire = CreateMaterial(true, false, true);
				static IMaterial* materialLIT = CreateMaterialLit();
				static IMaterial* materialIGNOREZ = CreateMaterialIgnorez();
				if (pLocalPlayer->GetIndex() == ent->GetIndex()) {
					if (ent == pLocalPlayer) {
						if (!pLocalPlayer->IsScoped() && Options::Menu.VisualsTab.ChamsLocal.GetState()) {
							Color color = Color(Options::Menu.ColorsTab.ChamsLocal.GetValue());
							static float kys[3] = { 0.f, 0.f, 0.f };
							kys[0] = color.r() / 255.f;
							kys[1] = color.g() / 255.f;
							kys[2] = color.b() / 255.f;
							kys[3] = color.a() / 255.f;
							Interfaces::RenderView->SetColorModulation(kys);
							Interfaces::ModelRender->ForcedMaterialOverride(materialLIT);
							ent->draw_model(1, 255);
							if (pLocalPlayer->GetBasePlayerAnimState()->m_bInHitGroundAnimation)
								kys[3] = 10 / 255.f;
							Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
						}
					}
				}
				if (Options::Menu.VisualsTab.ChamsEnemyNoVis.GetState() && teamnum != pLocalPlayer->GetTeamNum())
				{
					Color color = Color(Options::Menu.ColorsTab.ChamsEnemyNotVis.GetValue());
					Vector oldOrigin = ent->GetOrigin();
					QAngle oldAngs = ent->GetAngles();
					if (Options::Menu.ColorsTab.BackTrackBones.GetState())
					{
						for (int i = 1; i < 11; i++)
						{
							auto record = headPositions[ent->GetIndex()][i];
							ent->SetAbsOriginal(record.origin);
							ent->SetAbsAngles(record.angs);
							Vector temp = ent->GetOrigin();
							Interfaces::ModelRender->ForcedMaterialOverride(wire);
							Interfaces::RenderView->SetBlend(0.35);
							ent->draw_model(1, 255);
						}
						ent->SetAbsAngles(oldAngs);
						ent->SetAbsOriginal(oldOrigin);

						static float kys[3] = { 0.f, 0.f, 0.f };
						kys[0] = 210 / 255.f;
						kys[1] = 210 / 255.f;
						kys[2] = 210 / 255.f;
						Interfaces::RenderView->SetBlend(0.9);
						Interfaces::RenderView->SetColorModulation(kys);
						Interfaces::ModelRender->ForcedMaterialOverride(materialIGNOREZ);
						ent->draw_model(1, 255);
						Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
					}
					else
					{
						static float kys[3] = { 0.f, 0.f, 0.f };
						kys[0] = color.r() / 255.f;
						kys[1] = color.g() / 255.f;
						kys[2] = color.b() / 255.f;
						Interfaces::RenderView->SetBlend(1);
						Interfaces::RenderView->SetColorModulation(kys);
						Interfaces::ModelRender->ForcedMaterialOverride(materialIGNOREZ);
						ent->draw_model(1, 255);
						Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
					}
					static float kys[3] = { 0.f, 0.f, 0.f };
					kys[0] = color.r() / 255.f;
					kys[1] = color.g() / 255.f;
					kys[2] = color.b() / 255.f;
					Interfaces::RenderView->SetBlend(1);
					Interfaces::RenderView->SetColorModulation(kys);
					Interfaces::ModelRender->ForcedMaterialOverride(materialIGNOREZ);
					ent->draw_model(1, 255);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
				}
				else if (Options::Menu.VisualsTab.ChamsTeamNoVis.GetState() && teamnum == pLocalPlayer->GetTeamNum() && !(pLocalPlayer->GetIndex() == pLocalPlayer->GetIndex())) {


					Color color = Color(Options::Menu.ColorsTab.ChamsTeamNotVis.GetValue());

					static float kys[3] = { 0.f, 0.f, 0.f };
					kys[0] = color.r() / 255.f;
					kys[1] = color.g() / 255.f;
					kys[2] = color.b() / 255.f;
					Interfaces::RenderView->SetColorModulation(kys);
					Interfaces::ModelRender->ForcedMaterialOverride(materialIGNOREZ);
					ent->draw_model(1, 255);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
				}
				if (Options::Menu.VisualsTab.ChamsEnemyVis.GetState() && teamnum != pLocalPlayer->GetTeamNum()) {
					Color color = Color(Options::Menu.ColorsTab.ChamsEnemyVis.GetValue());
					static float kys[3] = { 0.f, 0.f, 0.f };
					kys[0] = color.r() / 255.f;
					kys[1] = color.g() / 255.f;
					kys[2] = color.b() / 255.f;
					Interfaces::RenderView->SetColorModulation(kys);
					Interfaces::ModelRender->ForcedMaterialOverride(materialLIT);
					ent->draw_model(1, 255);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
				}
				else if (Options::Menu.VisualsTab.ChamsTeamVis.GetState() && teamnum == pLocalPlayer->GetTeamNum() && !(pLocalPlayer->GetIndex() == pLocalPlayer->GetIndex())) {
					Color color = Color(Options::Menu.ColorsTab.ChamsTeamVis.GetValue());
					static float kys[3] = { 0.f, 0.f, 0.f };
					kys[0] = color.r() / 255.f;
					kys[1] = color.g() / 255.f;
					kys[2] = color.b() / 255.f;
					Interfaces::RenderView->SetColorModulation(kys);
					Interfaces::ModelRender->ForcedMaterialOverride(materialLIT);
					ent->draw_model(1, 255);
					Interfaces::ModelRender->ForcedMaterialOverride(nullptr);
				}
			}
		}
		if (Options::Menu.VisualsTab.aa_helper.GetState())
		{
			if (warmup) return;
			static  IMaterial* CoveredLit = CreateMaterial(false, true, false);
			if (CoveredLit)
			{

				Vector OrigAng;
				OrigAng = pLocalPlayer->GetEyeAngles();
				pLocalPlayer->SetAngle2(Vector(0, hackManager.pLocal()->GetEyeAnglesXY()->y, 0));
				float	NormalColor[3] = { 0.75, 0.75 , 0.75 };
				bool LbyColor = false;
				float lbyUpdateColor[3] = { 0, 1, 0 };
				Interfaces::RenderView->SetColorModulation(LbyColor ? lbyUpdateColor : NormalColor);
				Interfaces::ModelRender->ForcedMaterialOverride(CoveredLit);
				pLocalPlayer->draw_model(STUDIO_RENDER, 255);
				Interfaces::ModelRender->ForcedMaterialOverride(nullptr);

			}

		}

	}
}
void Hooks::DrawBeamd(Vector src, Vector end, Color color)
{
	float time = Options::Menu.VisualsTab.beamtime.GetValue();
	float size = Options::Menu.VisualsTab.beamsize.GetValue() / 100;
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMPOINTS;
	beamInfo.m_pszModelName = "sprites/physbeam.vmt";
	beamInfo.m_nModelIndex = -1; // will be set by CreateBeamPoints if its -1
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = time;
	beamInfo.m_flWidth = 3.1;
	beamInfo.m_flEndWidth = 3.3;
	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 2.0f;
	beamInfo.m_flBrightness = color.a();
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_flRed = Options::Menu.ColorsTab.Bullettracer.GetValue()[0];
	beamInfo.m_flGreen = Options::Menu.ColorsTab.Bullettracer.GetValue()[1];
	beamInfo.m_flBlue = Options::Menu.ColorsTab.Bullettracer.GetValue()[2];
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM;
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	Beam_t* myBeam = Interfaces::g_pViewRenderBeams->CreateBeamPoints(beamInfo);
	if (myBeam)
		Interfaces::g_pViewRenderBeams->DrawBeam(myBeam);

	Interfaces::DebugOverlay->AddBoxOverlay(end, Vector(-2.5, -2.5, -2.5), Vector(2.5, 2.5, 2.5),
		QAngle(0, 0, 0), Options::Menu.ColorsTab.Bullettracer.GetValue()[0], Options::Menu.ColorsTab.Bullettracer.GetValue()[1], Options::Menu.ColorsTab.Bullettracer.GetValue()[2], Options::Menu.ColorsTab.Bullettracer.GetValue()[3], 2.5f);
}
bool random() {
	if (rand() % 2 == 0)
		return true;
	else return false;
}
void __fastcall Hooked_DrawModelExecute(void* thisptr, int edx, void* ctx, void* state, const ModelRenderInfo_t &pInfo, matrix3x4 *pCustomBoneToWorld)
{
	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;
	Color color;
	float flColor[3] = { 0.f };
	bool DontDraw = false;
	static IMaterial* mat = CreateMaterialLit();
	int HandsStyle = Options::Menu.VisualsTab.HandCHAMS.GetIndex();

	int gunstyle = Options::Menu.VisualsTab.GunCHAMS.GetIndex();

	const char* ModelName = Interfaces::ModelInfo->GetModelName((model_t*)pInfo.pModel);

	IClientEntity* pModelEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(pInfo.entity_index);

	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	int ChamsStyle = Options::Menu.VisualsTab.OptionsChams.GetIndex();
	static IMaterial* CoveredFlat = CreateMaterial(true, false);
	static IMaterial* OpenFlat = CreateMaterial(false, false);
	static IMaterial* wire = CreateMaterial(true, false, true);
	static IMaterial* CoveredLit = CreateMaterial(true);
	static IMaterial* OpenLit = CreateMaterial(false);

	IMaterial *covered = ChamsStyle == 1 ? CoveredLit : CoveredFlat;
	if (pLocal->IsScoped() && strstr(ModelName, "models/player") && pModelEntity != nullptr && pModelEntity->GetTeamNum() == pLocal->GetTeamNum() && pModelEntity == pLocal)
	{
		if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 1)
			Interfaces::RenderView->SetBlend(0.20f);
		else if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 2)
		{
			if (Options::Menu.VisualsTab.aa_helper.GetState())
			{
				Interfaces::RenderView->SetBlend(0.25f);
			}
			else
			{
				flColor[0] = Options::Menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = Options::Menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = Options::Menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;

				Interfaces::RenderView->SetBlend(0.05);
				Interfaces::RenderView->SetColorModulation(flColor);
				Interfaces::ModelRender->ForcedMaterialOverride(covered);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
		}
		else if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 3)
		{
			if (Options::Menu.VisualsTab.aa_helper.GetState())
			{
				Interfaces::RenderView->SetBlend(0.15f);
			}
			else
			{
				flColor[0] = Options::Menu.ColorsTab.scoped_c.GetValue()[0] / 255.f;
				flColor[1] = Options::Menu.ColorsTab.scoped_c.GetValue()[1] / 255.f;
				flColor[2] = Options::Menu.ColorsTab.scoped_c.GetValue()[2] / 255.f;
				Interfaces::RenderView->SetColorModulation(flColor);
				Interfaces::RenderView->SetBlend(1.f);
				Interfaces::ModelRender->ForcedMaterialOverride(wire);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
		}
		else if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 4)
		{
			Interfaces::RenderView->SetBlend(0);
		}
		else if (Options::Menu.VisualsTab.localmaterial.GetIndex() == 5)
		{
			if (Options::Menu.VisualsTab.FakeAngleChams.GetIndex() != 0)
			{
				Interfaces::RenderView->SetBlend(0.15f);
			}
			else
			{
				flColor[0] = rand() % 250 / 255.f;
				flColor[1] = rand() % 250 / 255.f;
				flColor[2] = rand() % 250 / 255.f;
				Interfaces::RenderView->SetColorModulation(flColor);
				Interfaces::RenderView->SetBlend(0.10);
				Interfaces::ModelRender->ForcedMaterialOverride(covered);
				oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
			}
		}
	}
	if (gunstyle != 0 && strstr(ModelName, "models/weapons/v_") && !strstr(ModelName, "arms") && pLocal && pLocal->IsAlive() && !warmup)
	{
		//			Interfaces::RenderView->SetBlend(0.20f);
		if (gunstyle == 1)
		{
			flColor[0] = Options::Menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
			flColor[1] = Options::Menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
			flColor[2] = Options::Menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
			Interfaces::RenderView->SetColorModulation(flColor);
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			Interfaces::ModelRender->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (gunstyle == 2)
		{
			flColor[0] = Options::Menu.ColorsTab.GunChamsCol.GetValue()[0] / 255.f;
			flColor[1] = Options::Menu.ColorsTab.GunChamsCol.GetValue()[1] / 255.f;
			flColor[2] = Options::Menu.ColorsTab.GunChamsCol.GetValue()[2] / 255.f;
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			Interfaces::RenderView->SetColorModulation(flColor);
			Interfaces::ModelRender->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (gunstyle == 3)
		{
			flColor[0] = 234 / 255.f;
			flColor[1] = 234 / 255.f;
			flColor[2] = 100 / 255.f;
			Interfaces::RenderView->SetBlend(0);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			Interfaces::RenderView->SetColorModulation(flColor);
			Interfaces::ModelRender->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (gunstyle == 4)
		{
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
			IMaterial *material = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
			Interfaces::ModelRender->ForcedMaterialOverride(material);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (gunstyle == 5)
		{
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
			IMaterial *material = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER);
			Interfaces::ModelRender->ForcedMaterialOverride(material);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (HandsStyle == 6)
		{
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_ENVMAPSPHERE, true);
			Color colRainbow = urmamasuckmylargegenetalia(5000, 1022);
			flColor[0] = colRainbow.r() / 255.f;
			flColor[1] = colRainbow.g() / 255.f;
			flColor[2] = colRainbow.b() / 255.f;
			Interfaces::RenderView->SetColorModulation(flColor);
			Interfaces::ModelRender->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
	}
	if (HandsStyle != 0 && strstr(ModelName, XorStr("arms")) && pLocal && pLocal->IsAlive())
	{
		//			Interfaces::RenderView->SetBlend(0.20f);
		if (HandsStyle == 1)
		{
			flColor[0] = Options::Menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = Options::Menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = Options::Menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
			Interfaces::RenderView->SetColorModulation(flColor);
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			Interfaces::ModelRender->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (HandsStyle == 2)
		{
			flColor[0] = Options::Menu.ColorsTab.HandChamsCol.GetValue()[0] / 255.f;
			flColor[1] = Options::Menu.ColorsTab.HandChamsCol.GetValue()[1] / 255.f;
			flColor[2] = Options::Menu.ColorsTab.HandChamsCol.GetValue()[2] / 255.f;
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true);
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			Interfaces::RenderView->SetColorModulation(flColor);
			Interfaces::ModelRender->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (HandsStyle == 3)
		{
			flColor[0] = 234 / 255.f;
			flColor[1] = 234 / 255.f;
			flColor[2] = 100 / 255.f;
			Interfaces::RenderView->SetBlend(0);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			Interfaces::RenderView->SetColorModulation(flColor);
			Interfaces::ModelRender->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (HandsStyle == 4)
		{
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
			IMaterial *material = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
			Interfaces::ModelRender->ForcedMaterialOverride(material);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (HandsStyle == 5)
		{
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, false);
			IMaterial *material = Interfaces::MaterialSystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER);
			Interfaces::ModelRender->ForcedMaterialOverride(material);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
		else if (HandsStyle == 6)
		{
			Interfaces::RenderView->SetBlend(1.f);
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
			mat->SetMaterialVarFlag(MATERIAL_VAR_ENVMAPSPHERE, true);
			Color colRainbow = urmamasuckmylargegenetalia(5000, 1022);
			flColor[0] = colRainbow.r() / 255.f;
			flColor[1] = colRainbow.g() / 255.f;
			flColor[2] = colRainbow.b() / 255.f;
			Interfaces::RenderView->SetColorModulation(flColor);
			Interfaces::ModelRender->ForcedMaterialOverride(mat);
			oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
		}
	}
	if (pLocal->IsScoped() && pModelEntity == pLocal)
		Interfaces::RenderView->SetBlend(0.33f);
	if (!DontDraw)
		oDrawModelExecute(thisptr, ctx, state, pInfo, pCustomBoneToWorld);
	Interfaces::ModelRender->ForcedMaterialOverride(NULL);
}
int RandomInt(int min, int max)
{
	return rand() % max + min;
}

std::vector<const char*> vistasmoke_mats =
{
	"particle/vistasmokev1/vistasmokev1_fire",
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
};

void faggots(IClientEntity *player)
{
	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{

		//this is utter bullshit and not even close to a proper animfix, i just call this because it clears up one animation.
		if (!player || !player->IsAlive() || !player->GetFlags() & FL_ONGROUND) //only call this in air so my walk anim doesn't get fucked
			return;
		auto entity = player;
		auto player_index = entity->GetIndex() - 1;
		auto old_curtime = Interfaces::Globals->curtime;
		auto old_frametime = Interfaces::Globals->frametime;
		auto old_ragpos = entity->get_ragdoll_pos();
		Interfaces::Globals->curtime = entity->GetSimulationTime();
		Interfaces::Globals->frametime = Interfaces::Globals->interval_per_tick;
		auto player_animation_state = reinterpret_cast<DWORD*>(entity + 0x3884);
		auto player_model_time = reinterpret_cast<int*>(player_animation_state + 112);
		if (player_animation_state != nullptr && player_model_time != nullptr)
			if (*player_model_time == Interfaces::Globals->framecount)
				*player_model_time = Interfaces::Globals->framecount - 1;
		entity->get_ragdoll_pos() = old_ragpos;
		entity->UpdateClientSideAnimation();
		entity->setAbsAechse(Vector(0.f, entity->GetBasePlayerAnimState()->m_flGoalFeetYaw, 0.f));
		Interfaces::Globals->curtime = old_curtime;
		Interfaces::Globals->frametime = old_frametime;
	}
}
class CBaseAnimating
{
public:
	std::array<float, 24>* m_flPoseParameter()
	{
		static int offset = 0;
		if (!offset)
			offset = 0x2764;
		return (std::array<float, 24>*)((uintptr_t)this + offset);
	}
	model_t* GetModel()
	{
		void* pRenderable = reinterpret_cast<void*>(uintptr_t(this) + 0x4);
		typedef model_t* (__thiscall* fnGetModel)(void*);
		return call_vfunc<fnGetModel>(pRenderable, 8)(pRenderable);
	}
	void SetBoneMatrix(matrix3x4_t* boneMatrix)
	{
		//Offset found in C_BaseAnimating::GetBoneTransform, string search ankle_L and a function below is the right one
		const auto model = this->GetModel();
		if (!model)
			return;
		matrix3x4_t* matrix = *(matrix3x4_t**)((DWORD)this + 9880);
		studiohdr_t *hdr = Interfaces::ModelInfo->GetStudiomodel(model);
		if (!hdr)
			return;
		int size = hdr->numbones;
		if (matrix) {
			for (int i = 0; i < size; i++)
				memcpy(matrix + i, boneMatrix + i, sizeof(matrix3x4_t));
		}
	}
	void GetDirectBoneMatrix(matrix3x4_t* boneMatrix)
	{
		const auto model = this->GetModel();
		if (!model)
			return;
		matrix3x4_t* matrix = *(matrix3x4_t**)((DWORD)this + 9880);
		studiohdr_t *hdr = Interfaces::ModelInfo->GetStudiomodel(model);
		if (!hdr)
			return;
		int size = hdr->numbones;
		if (matrix) {
			for (int i = 0; i < size; i++)
				memcpy(boneMatrix + i, matrix + i, sizeof(matrix3x4_t));
		}
	}
};
void layersstore(int stage)
{
	if (!Interfaces::Engine->IsInGame() || !Interfaces::Engine->IsConnected())
		return;
	auto local_player = hackManager.pLocal();
	if (!local_player)
		return;
	if (local_player->GetHealth() <= 0)
		return;
	static int userId[64];
	static AnimationLayer
		backupLayersUpdate[64][15],
		backupLayersInterp[64][15];
	for (int i = 1; i < Interfaces::Globals->maxClients; i++)
	{
		IClientEntity* player = Interfaces::EntList->GetClientEntity(i);
		if (!player ||
			player->GetTeamNum() == local_player->GetTeamNum() ||
			!player ||
			player->IsDormant())
			continue;
		player_info_t player_info;
		Interfaces::Engine->GetPlayerInfo(i, &player_info);
		switch (stage)
		{
		case ClientFrameStage_t::FRAME_NET_UPDATE_START: // Copy new, server layers to use when drawing.
			userId[i] = player_info.userid;
			memcpy(&backupLayersUpdate[i], player->GetAnimOverlays(), (sizeof AnimationLayer) * player->GetNumAnimOverlays());
			break;
		case ClientFrameStage_t::FRAME_RENDER_START: // Render started, don't use inaccurately extrapolated layers but save them to not mess shit up either.
			if (userId[i] != player_info.userid) continue;
			memcpy(&backupLayersInterp[i], player->GetAnimOverlays(), (sizeof AnimationLayer) * player->GetNumAnimOverlays());
			memcpy(player->GetAnimOverlays(), &backupLayersUpdate[i], (sizeof AnimationLayer) * player->GetNumAnimOverlays());
			break;
		case ClientFrameStage_t::FRAME_RENDER_END: // Restore layers to keep being accurate when backtracking.
			if (userId[i] != player_info.userid) continue;
			memcpy(player->GetAnimOverlays(), &backupLayersInterp[i], (sizeof AnimationLayer) * player->GetNumAnimOverlays());
			break;
		default:
			return;
		}
	}
}
void AnimFix(IClientEntity* entity)
{
	if (!Interfaces::Engine)
		return;
	if (!Interfaces::Engine->IsInGame() || !Interfaces::Engine->IsConnected())
		return;
	auto local_player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!local_player || !entity)
		return;
	bool is_local_player = entity == local_player;
	bool is_teammate = local_player->GetTeamNum() == entity->GetTeamNum() && !is_local_player;
	if (is_local_player)
		return;
	if (!entity || !entity->IsAlive() || !hackManager.pLocal() || !hackManager.pLocal()->IsAlive())
		return;
	struct clientanimating_t
	{
		CBaseAnimating *pAnimating;
		unsigned int	flags;
		clientanimating_t(CBaseAnimating *_pAnim, unsigned int _flags) : pAnimating(_pAnim), flags(_flags) {}
	};
	clientanimating_t *animating = nullptr;
	int animflags;
	const unsigned int FCLIENTANIM_SEQUENCE_CYCLE = 0x00000001;
	CAnimationLayer AnimLayer[15];
	int cnt = 15;
	for (int i = 0; i < cnt; i++)
	{
		AnimLayer[i] = entity->GetAnimOverlayX(i);
	}
	float flPoseParameter[24];
	float* pose = (float*)((uintptr_t)entity + 0x2764);
	memcpy(&flPoseParameter, pose, sizeof(float) * 24);
	Vector TargetEyeAngles = *entity->GetEyeAnglesPointer();
	bool bForceAnimationUpdate = entity->GetEyeAnglesPointer()->x != TargetEyeAngles.x || entity->GetEyeAnglesPointer()->y != TargetEyeAngles.y;
	if (bForceAnimationUpdate)
	{
		//Update animations and pose parameters
		clientanimating_t *animating = nullptr;
		int animflags;
		//Make sure game is allowed to client side animate. Probably unnecessary
		for (unsigned int i = 0; i < Interfaces::g_ClientSideAnimationList->count; i++)
		{
			clientanimating_t *tanimating = (clientanimating_t*)Interfaces::g_ClientSideAnimationList->Retrieve(i, sizeof(clientanimating_t));
			IClientEntity *pAnimEntity = (IClientEntity*)tanimating->pAnimating;
			if (pAnimEntity == entity)
			{
				animating = tanimating;
				animflags = tanimating->flags;
				tanimating->flags |= FCLIENTANIM_SEQUENCE_CYCLE;
				break;
			}
		}
		//Update animations/poses
		//	entity->UpdateClientSideAnimation();
		//Restore anim flags
		if (animating)
			animating->flags = animflags;
	}
	for (unsigned int i = 0; i < Interfaces::g_ClientSideAnimationList->count; i++)
	{
		clientanimating_t *animating = (clientanimating_t*)Interfaces::g_ClientSideAnimationList->Retrieve(i, sizeof(clientanimating_t));
		IClientEntity *Entity = (IClientEntity*)animating->pAnimating;
		if (Entity != local_player && !Entity->IsDormant() && Entity->GetHealth() > 0)
		{
			int TickReceivedNetUpdate[65];
			TickReceivedNetUpdate[entity->GetIndex()] = Interfaces::Globals->tickcount;
			bool HadClientAnimSequenceCycle[65];
			int ClientSideAnimationFlags[65];
			bool IsBreakingLagCompensation[65];
			IsBreakingLagCompensation[entity->GetIndex()] = !Entity->IsDormant() && entity->GetOrigin2().LengthSqr() > (64.0f * 64.0f);
			unsigned int flags = animating->flags;
			ClientSideAnimationFlags[entity->GetIndex()] = flags;
			HadClientAnimSequenceCycle[entity->GetIndex()] = (flags & FCLIENTANIM_SEQUENCE_CYCLE);

		}
	}
	if (is_local_player) {
		for (unsigned int i = 0; i < Interfaces::g_ClientSideAnimationList->count; i++)
		{
			clientanimating_t *animating = (clientanimating_t*)Interfaces::g_ClientSideAnimationList->Retrieve(i, sizeof(clientanimating_t));
			IClientEntity *Entity = (IClientEntity*)animating->pAnimating;
			if (Entity != local_player && !Entity->IsDormant() && Entity->GetHealth() > 0)
			{
				bool HadClientAnimSequenceCycle[65];
				int ClientSideAnimationFlags[65];
				unsigned int flags = animating->flags;
				ClientSideAnimationFlags[entity->GetIndex()] = flags;
				HadClientAnimSequenceCycle[entity->GetIndex()] = (flags & FCLIENTANIM_SEQUENCE_CYCLE);
				if (HadClientAnimSequenceCycle[entity->GetIndex()])
				{
					animating->flags |= FCLIENTANIM_SEQUENCE_CYCLE;
				}
			}
		}
	}
}

void NotReallyUsefull(IClientEntity *player)
{
	//this is utter bullshit and not even close to a proper animfix, i just call this because it clears up one animation.
	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		auto local_player = hackManager.pLocal();
		if (!local_player)
			return;
		if (!player || !player->IsAlive() || !player->GetFlags() & FL_ONGROUND) //only call this in air so my walk anim doesn't get fucked
			return;
		auto old_curtime = Interfaces::Globals->curtime;
		auto old_frametime = Interfaces::Globals->frametime;
		auto old_ragpos = local_player->get_ragdoll_pos();
		Interfaces::Globals->curtime = local_player->GetSimulationTime();
		Interfaces::Globals->frametime = Interfaces::Globals->interval_per_tick;
		auto player_animation_state = reinterpret_cast<DWORD*>(local_player + 0x3884);
		auto player_model_time = reinterpret_cast<int*>(player_animation_state + 112);
		if (player_animation_state != nullptr && player_model_time != nullptr)
			if (*player_model_time == Interfaces::Globals->framecount)
				*player_model_time = Interfaces::Globals->framecount - 1;
		local_player->get_ragdoll_pos() = old_ragpos;
		//		local_player->UpdateClientSideAnimation();
		local_player->setAbsAechse(Vector(0.f, local_player->GetBasePlayerAnimState()->m_flGoalFeetYaw, 0.f));
		Interfaces::Globals->curtime = old_curtime;
		Interfaces::Globals->frametime = old_frametime;
	}
}
struct CIncomingSequence
{
	CIncomingSequence::CIncomingSequence(int instate, int outstate, int seqnr, float time)
	{
		inreliablestate = instate;
		outreliablestate = outstate;
		sequencenr = seqnr;
		curtime = time;
	}
	int inreliablestate;
	int outreliablestate;
	int sequencenr;
	float curtime;
};
std::deque<CIncomingSequence> sequences;
int32_t lastincomingsequencenumber;

void UpdateIncomingSequences()
{
	auto clientState = Interfaces::m_pClientState; //DONT HARDCODE OFFESTS


	if (!clientState)
		return;

	auto intnetchan = clientState->m_NetChannel; //Can optimise, already done in CM hook, make a global

	INetChannel* netchan = reinterpret_cast<INetChannel*>(intnetchan);
	if (netchan)
	{
		if (netchan->m_nInSequenceNr > lastincomingsequencenumber)
		{
			//sequences.push_front(netchan->m_nInSequenceNr);
			lastincomingsequencenumber = netchan->m_nInSequenceNr;

			sequences.push_front(CIncomingSequence(netchan->m_nInReliableState, netchan->m_nOutReliableState, netchan->m_nInSequenceNr, Interfaces::Globals->realtime));
		}
		if (sequences.size() > 2048)
			sequences.pop_back();
	}

}
void  __stdcall Hooked_FrameStageNotify(ClientFrameStage_t curStage)
{
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	IClientEntity* pEntity = nullptr;
	static auto linegoesthrusmoke = Utilities::Memory::FindPattern("client_panorama.dll", (PBYTE)"\x55\x8B\xEC\x83\xEC\x08\x8B\x15\x00\x00\x00\x00\x0F\x57\xC0", "xxxxxxxx????xxx");
	static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);

	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{
			UpdateIncomingSequences();
		}

		if (Options::Menu.RageBotTab.resolver.GetIndex() != 0)
		{
			for (int i = 0; i < Interfaces::Globals->maxClients; i++)
			{
				auto m_entity = Interfaces::EntList->GetClientEntity(i);
				if (!m_entity || m_entity == pLocal || m_entity->GetClientClass()->m_ClassID != (int)CSGOClassID::CCSPlayer) continue; // !is alive
				{
					lagcompensation.disable_interpolation(m_entity);
				}
			}
		}

		if (Options::Menu.VisualsTab.OtherThirdperson.GetState() && pLocal)
		{
			Vector thirdpersonMode;
			thirdpersonMode = LastAngleAAReal;
			static bool rekt = false;
			if (!rekt)
			{
				ConVar* sv_cheats = Interfaces::CVar->FindVar("sv_cheats");
				SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
				sv_cheats_spoofed->SetInt(1);
				rekt = true;
			}
			static bool kek = false;
			if (!kek)
			{
				Interfaces::Engine->ClientCmd_Unrestricted("thirdperson");
				kek = true;
			}
			static bool toggleThirdperson;
			static float memeTime;
			int ThirdPersonKey = Options::Menu.VisualsTab.ThirdPersonKeyBind.GetKey();
			if (ThirdPersonKey >= 0 && GUI.GetKeyState(ThirdPersonKey) && abs(memeTime - Interfaces::Globals->curtime) > 0.5)
			{
				toggleThirdperson = !toggleThirdperson;
				memeTime = Interfaces::Globals->curtime;
			}
			if (pLocal->GetBasePlayerAnimState()->m_bInHitGroundAnimation) {
				thirdpersonMode.x = -10;
			}if (!pLocal->IsAlive()) {
				Interfaces::Engine->ClientCmd_Unrestricted("firstperson");
				Interfaces::pInput->m_fCameraInThirdPerson = false;
				*(float*)((DWORD)Interfaces::pInput + 0xA8 + 0x8) = 0;
			}
			if (toggleThirdperson)
			{
				Interfaces::pInput->m_fCameraInThirdPerson = true;
				*(float*)((DWORD)Interfaces::pInput + 0xA8 + 0x8) = Options::Menu.VisualsTab.ThirdPersonVal.GetValue(); //Thirdperson range
				if (*(bool*)((DWORD)Interfaces::pInput + 0xA5)) {
					*(Vector*)((DWORD)pLocal + 0x31C8) = thirdpersonMode;
				}
			}
			else
			{
				Interfaces::Engine->ClientCmd_Unrestricted("firstperson");
			}
		}
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
			CMBacktracking::Get().FrameUpdatePostEntityThink();
			CMBacktracking::Get().StartLagCompensation(pLocal);
		}
		if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_END) {
			CMBacktracking::Get().FrameUpdatePostEntityThink();
			CMBacktracking::Get().FinishLagCompensation(pLocal);

		}
		if (curStage == FRAME_RENDER_START)
		{
			//
			for (int i = 1; i <= Interfaces::Globals->maxClients; i++)
			{
				if (Interfaces::Engine) {
					if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected()) {
						if (pLocal) {
							if (pLocal->IsAlive())
							{
								auto local_player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

								local_player->setAbsAechse(Vector(0.f, local_player->GetBasePlayerAnimState()->m_flGoalFeetYaw, 0.f));
							}
						}
					}
				}
				//		AnimFix(pLocal);
			}

			for (int i = 1; i <= Interfaces::Globals->maxClients; i++)
			{
				if (i == Interfaces::Engine->GetLocalPlayer()) continue;
				IClientEntity* pEnt = Interfaces::EntList->GetClientEntity(i);
				if (!pEnt) continue;
				*(int*)((uintptr_t)pEnt + 0xA30) = Interfaces::Globals->framecount; //we'll skip occlusion checks now
				*(int*)((uintptr_t)pEnt + 0xA28) = 0;//clear occlusion flags
			}

		}
		if (curStage == FRAME_NET_UPDATE_END)
		{
			CMBacktracking::Get().FrameUpdatePostEntityThink();
		}
		if (Options::Menu.RageBotTab.resolver.GetIndex() != 0)
		{
			for (int i = 1; i < Interfaces::Globals->maxClients; i++)
			{
				if (i == Interfaces::Engine->GetLocalPlayer()) continue;
				IClientEntity* pCurEntity = Interfaces::EntList->GetClientEntity(i);
				if (!pCurEntity) continue;
				CTickRecord trans = CTickRecord(pCurEntity);
				CBacktracking::Get().ClearRecord(pCurEntity);
				CBacktracking::Get().SaveTemporaryRecord(pCurEntity, trans);
				*(int*)((uintptr_t)pCurEntity + 0xA30) = Interfaces::Globals->framecount;
				*(int*)((uintptr_t)pCurEntity + 0xA28) = 0; //literally the same shit as above
			}
		}
		ResolverSetup::GetInst().FSN(pEntity, curStage);
		//		layersstore(curStage);	
	}
	oFrameStageNotify(curStage);
}
void __fastcall Hooked_OverrideView(void* ecx, void* edx, CViewSetup* pSetup)
{
	auto local = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!local || !Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;
	if (local)
	{
		if (Options::Menu.VisualsTab.Active.GetState())
		{
			if (Options::Menu.VisualsTab.RemoveZoom.GetState()) {
				if (Options::Menu.VisualsTab.RemoveZoom.GetState() && local->IsScoped())
					pSetup->fov = 60 + Options::Menu.VisualsTab.OtherFOV.GetValue();
			}
			if (!Options::Menu.VisualsTab.OtherFOV.GetValue() > 0 && Options::Menu.VisualsTab.RemoveZoom.GetState() && local->IsScoped())
				pSetup->fov = 90;
			if (local->IsScoped())
			{
				if (Options::Menu.VisualsTab.OtherFOV.GetValue() > 0)
					pSetup->fov = (pSetup->fov + Options::Menu.VisualsTab.OtherFOV.GetValue());
				else
					pSetup->fov += Options::Menu.VisualsTab.OtherFOV.GetValue();
			}
			else
			{
				if (GetAsyncKeyState(Options::Menu.MiscTab.zoom.GetKey()) && !Options::Menu.m_bIsOpen)
					pSetup->fov = 30;
				else
					pSetup->fov += Options::Menu.VisualsTab.OtherFOV.GetValue();
			}
		}
	}
	oOverrideView(ecx, edx, pSetup);
}
void GetViewModelFOV(float& fov)
{
	IClientEntity* localplayer = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		if (!localplayer)
			return;
		if (Options::Menu.VisualsTab.Active.GetState())
			fov = Options::Menu.VisualsTab.OtherViewmodelFOV.GetValue();
	}
}
float __stdcall GGetViewModelFOV()
{
	float fov = Hooks::VMTClientMode.GetMethod<oGetViewModelFOV>(35)(); // view me through a FUCKING window because you're a perverted JEW
	GetViewModelFOV(fov);
	return fov;
}
void __fastcall Hooked_RenderView(void* ecx, void* edx, CViewSetup &setup, CViewSetup &hudViewSetup, int nClearFlags, int whatToDraw)
{
	static DWORD oRenderView = Hooks::VMTRenderView.GetOriginalFunction(6);
	IClientEntity* pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	__asm
	{
		PUSH whatToDraw
		PUSH nClearFlags
		PUSH hudViewSetup
		PUSH setup
		MOV ECX, ecx
		CALL oRenderView
	}
}