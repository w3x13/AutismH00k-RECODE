#include "Visuals.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "Autowall.h"
#include "Interfaces.h"
void CVisuals::Init()
{
}
void CVisuals::Move(CUserCmd *pCmd, bool &bSendPacket) {}


void CVisuals::Draw()
{
	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;
	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (Options::Menu.VisualsTab.AutowallCrosshair.GetState())
	{
		AutowallCrosshair();
		Interfaces::Engine->ClientCmd_Unrestricted("crosshair 0");
	}
	else
	{
		Interfaces::Engine->ClientCmd_Unrestricted("crosshair 1");
	}
}
void CVisuals::NoScopeCrosshair()
{
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;
#define TEXTURE_GROUP_OTHER                         "Other textures"

	IMaterial *xblur_mat = Interfaces::MaterialSystem->FindMaterial("dev/blurfilterx_nohdr", TEXTURE_GROUP_OTHER, true);
	IMaterial *yblur_mat = Interfaces::MaterialSystem->FindMaterial("dev/blurfiltery_nohdr", TEXTURE_GROUP_OTHER, true);
	IMaterial *scope = Interfaces::MaterialSystem->FindMaterial("dev/scope_bluroverlay", TEXTURE_GROUP_OTHER, true);
	xblur_mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);

	yblur_mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
	scope->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
	IClientEntity* pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (GameUtils::IsSniper(pWeapon) && GameUtils::IsScopedWeapon(pWeapon) && pWeapon->IsScoped())
	{
		Render::Line(MidX - 1000, MidY, MidX + 1000, MidY, Color(255, 255, 255, 255));
		Render::Line(MidX, MidY - 1000, MidX, MidY + 1000, Color(0, 0, 0, 255));
	}
}
void CVisuals::DrawCrosshair()
{
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;
	Render::Line(MidX - 10, MidY, MidX + 10, MidY, Color(0, 255, 0, 255));
	Render::Line(MidX, MidY - 10, MidX, MidY + 10, Color(0, 255, 0, 255));
}
void CVisuals::DrawRecoilCrosshair()
{
	IClientEntity *pLocal = hackManager.pLocal();
	Vector ViewAngles;
	Interfaces::Engine->GetViewAngles(ViewAngles);
	ViewAngles += pLocal->localPlayerExclusive()->GetAimPunchAngle() * 2.f;
	Vector fowardVec;
	AngleVectors(ViewAngles, &fowardVec);
	fowardVec *= 10000;
	Vector start = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector end = start + fowardVec, endScreen;
	if (Render::TransformScreen(end, endScreen) && pLocal->IsAlive())
	{
		Render::Line(endScreen.x - 10, endScreen.y, endScreen.x + 10, endScreen.y, Color(0, 255, 0, 255));
		Render::Line(endScreen.x, endScreen.y - 10, endScreen.x, endScreen.y + 10, Color(0, 255, 0, 255));
	}
}
void CVisuals::SpreadCrosshair()
{
	IClientEntity *pLocal = hackManager.pLocal();
	C_BaseCombatWeapon* pWeapon = (C_BaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	IClientEntity* WeaponEnt = Interfaces::EntList->GetClientEntityFromHandle(pLocal->GetActiveWeaponHandle());
	if (!hackManager.pLocal()->IsAlive())
		return;
	if (!GameUtils::IsBallisticWeapon(pWeapon))
		return;
	if (pWeapon == nullptr)
		return;
	int xs;
	int ys;
	Interfaces::Engine->GetScreenSize(xs, ys);
	xs /= 2; ys /= 2;
	float inaccuracy = pWeapon->GetInaccuracy() * 1000;
	char buffer4[64];
	sprintf_s(buffer4, "%.00f", inaccuracy);
	Render::DrawFilledCircle(Vector2D(xs, ys), Color(20, 20, 20, 124), inaccuracy, 60);
}
void CVisuals::AutowallCrosshair()
{
	IClientEntity *pLocal = hackManager.pLocal();
	RECT View = Render::GetViewport();
	int MidX = View.right / 2;
	int MidY = View.bottom / 2;
	Vector ViewAngles;
	Interfaces::Engine->GetViewAngles(ViewAngles);
	ViewAngles += pLocal->localPlayerExclusive()->GetAimPunchAngle() * 2.f;
	Vector fowardVec;
	AngleVectors(ViewAngles, &fowardVec);
	fowardVec *= 10000;
	Vector start = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector end = start + fowardVec, endScreen;
	int xs, ys;
	Interfaces::Engine->GetScreenSize(xs, ys);
	xs /= 2; ys /= 2;
	if (Render::TransformScreen(end, endScreen) && pLocal->IsAlive())
	{
		float damage = 0.f;
		Render::OutlinedRect(xs - 2, ys - 2, 5, 5, Color(27, 27, 27), trace_autowallable(damage) ? Color(0, 255, 0) : Color(255, 0, 0));
	}
}