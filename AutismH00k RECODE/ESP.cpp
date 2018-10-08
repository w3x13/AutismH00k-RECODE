#include "ESP.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "GlowManager.h"
#include <stdarg.h>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <random>
#include <iostream>
#include <iomanip>
#include <random>
#include "RageBot.h"
#include "Autowall.h"
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <stdlib.h>
#include "edge.h"
#include "Hooks.h"
#include "kobe.h"
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
float lineLBY;
float lineLBY2;
float lineRealAngle;
float lineFakeAngle;
float lby2;
float lspeed;
float pitchmeme;
CRageBot rageXbot;

float inaccuracy;
void CEsp::Init()
{
	BombCarrier = nullptr;
}
void CEsp::Move(CUserCmd *pCmd, bool &bSendPacket)
{
}
bool screen_transformx(const Vector& point, Vector& screen)
{
	const matrix3x4& w2sMatrix = Interfaces::Engine->WorldToScreenMatrix();
	screen.x = w2sMatrix[0][0] * point.x + w2sMatrix[0][1] * point.y + w2sMatrix[0][2] * point.z + w2sMatrix[0][3];
	screen.y = w2sMatrix[1][0] * point.x + w2sMatrix[1][1] * point.y + w2sMatrix[1][2] * point.z + w2sMatrix[1][3];
	screen.z = 0.0f;
	float w = w2sMatrix[3][0] * point.x + w2sMatrix[3][1] * point.y + w2sMatrix[3][2] * point.z + w2sMatrix[3][3];
	if (w < 0.001f) {
		screen.x *= 100000;
		screen.y *= 100000;
		return true;
	}
	float invw = 1.0f / w;
	screen.x *= invw;
	screen.y *= invw;
	return false;
}
bool world_to_screen(const Vector &origin, Vector &screen)
{
	if (!screen_transformx(origin, screen)) {
		int iScreenWidth, iScreenHeight;
		Interfaces::Engine->GetScreenSize(iScreenWidth, iScreenHeight);
		screen.x = (iScreenWidth / 2.0f) + (screen.x * iScreenWidth) / 2;
		screen.y = (iScreenHeight / 2.0f) - (screen.y * iScreenHeight) / 2;
		return true;
	}
	return false;
}
void CEsp::Draw()
{
	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;
	IClientEntity *pLocal = hackManager.pLocal();
	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		player_info_t pinfo;
		if (pEntity &&  pEntity != pLocal && !pEntity->IsDormant())
		{
			if (Options::Menu.VisualsTab.aa_helper2.GetState())
			{

				int screen_width, screen_height;
				Interfaces::Engine->GetScreenSize(screen_width, screen_height);

				static const auto fake_color = Color(117, 245, 10, 255);


				auto client_viewangles = Vector();
				Interfaces::Engine->GetViewAngles(client_viewangles);

				constexpr auto radius = 90.f;

				const auto screen_center = Vector2D(screen_width / 2.f, screen_height / 2.f);
				const auto fake_rot = DEG2RAD(client_viewangles.y - lineFakeAngle - 90);

				auto draw_arrow = [&](float rot, Color color) -> void
				{
					std::vector<Vertex_t> vertices;
					vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot) * radius, screen_center.y + sinf(rot) * radius)));
					vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot + DEG2RAD(12)) * (radius - 20.f), screen_center.y + sinf(rot + DEG2RAD(9)) * (radius - 20.f)))); //25
					vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot - DEG2RAD(12)) * (radius - 20.f), screen_center.y + sinf(rot - DEG2RAD(9)) * (radius - 20.f)))); //25
					Render::TexturedPolygon(3, vertices, color);
				};

				draw_arrow(fake_rot, fake_color);


				static const auto real_color = Color(245, 238, 10, 255);

				const auto real_rot = DEG2RAD(client_viewangles.y - lineRealAngle - 90);

				draw_arrow(real_rot, real_color);


			static const auto lby_color = Color(250, 250, 250, 255);

				const auto lby_rot = DEG2RAD(client_viewangles.y - lineLBY - 90);

				draw_arrow(lby_rot, lby_color);
			}
			if (Options::Menu.VisualsTab.GrenadePrediction.GetState())
			{
				grenade_prediction::instance().Paint();
			}

			if (Options::Menu.VisualsTab.OtherRadar.GetState())
			{
				DWORD m_bSpotted = NetVar.GetNetVar(0x839EB159);
				*(char*)((DWORD)(pEntity)+m_bSpotted) = 1;
			}
			if (Options::Menu.VisualsTab.FiltersPlayers.GetState() && Interfaces::Engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
			{
				DrawPlayer(pEntity, pinfo);
			}
			if (Options::Menu.VisualsTab.SpecList.GetState())
			{
				SpecList();
			}
			ClientClass* cClass = (ClientClass*)pEntity->GetClientClass();
			if (Options::Menu.VisualsTab.FiltersNades.GetState() && strstr(cClass->m_pNetworkName, "Projectile"))
			{
				DrawThrowable(pEntity);
			}
			if (Options::Menu.VisualsTab.FiltersWeapons.GetState() && cClass->m_ClassID != (int)CSGOClassID::CBaseWeaponWorldModel && ((strstr(cClass->m_pNetworkName, "Weapon") || cClass->m_ClassID == (int)CSGOClassID::CDEagle || cClass->m_ClassID == (int)CSGOClassID::CAK47)))
			{
				DrawDrop(pEntity, cClass);
			}
			if (Options::Menu.VisualsTab.FiltersC4.GetState())
			{
				if (cClass->m_ClassID == (int)CSGOClassID::CPlantedC4)
					DrawBombus(pEntity);
				if (cClass->m_ClassID == (int)CSGOClassID::CC4)
					DrawBomb(pEntity, cClass);
			}
		}
	}
	static auto linegoesthrusmoke = Utilities::Memory::FindPattern("client_panorama.dll", (PBYTE)"\x55\x8B\xEC\x83\xEC\x08\x8B\x15\x00\x00\x00\x00\x0F\x57\xC0", "xxxxxxxx????xxx");
	if (Options::Menu.VisualsTab.OtherNoFlash.GetState())
	{
		float alp = Options::Menu.VisualsTab.flashAlpha.GetValue() / 255;
		DWORD m_flFlashMaxAlpha = NetVar.GetNetVar(0xFE79FB98);
		*(float*)((DWORD)pLocal + m_flFlashMaxAlpha) = alp;
	}
	if (Options::Menu.VisualsTab.FovArrows.GetState())
	{

	}
	if (Options::Menu.VisualsTab.OtherNoSmoke.GetState())
	{
			std::vector<const char*> vistasmoke_wireframe =
			{
				"particle/vistasmokev1/vistasmokev1_smokegrenade",
			};

			std::vector<const char*> vistasmoke_nodraw =
			{
				"particle/vistasmokev1/vistasmokev1_fire",
				"particle/vistasmokev1/vistasmokev1_emods",
				"particle/vistasmokev1/vistasmokev1_emods_impactdust",
			};

			for (auto mat_s : vistasmoke_wireframe)
			{
				IMaterial* mat = Interfaces::MaterialSystem->FindMaterial(mat_s, TEXTURE_GROUP_OTHER);
				mat->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, true); //wireframe
			}

			for (auto mat_n : vistasmoke_nodraw)
			{
				IMaterial* mat = Interfaces::MaterialSystem->FindMaterial(mat_n, TEXTURE_GROUP_OTHER);
				mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			}

			static auto smokecout = *(DWORD*)(linegoesthrusmoke + 0x8);
			*(int*)(smokecout) = 0;
		}
	}


float damage;
char bombdamagestringdead[24];
char bombdamagestringalive[24];
inline float CSGO_Armor(float flDamage, int ArmorValue)
{
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if (ArmorValue > 0) {
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;
		if (flArmor > static_cast<float>(ArmorValue)) {
			flArmor = static_cast<float>(ArmorValue) * (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}
		flDamage = flNew;
	}
	return flDamage;
}
void CEsp::DrawBombus(IClientEntity* pEntity) {
	BombCarrier = nullptr;
	auto entity = pEntity;
	Vector vOrig; Vector vScreen;
	vOrig = entity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)entity;
	float flBlow = Bomb->GetC4BlowTime();
	auto local = hackManager.pLocal();
	float lifetime = flBlow - (Interfaces::Globals->interval_per_tick * local->GetTickBase());
	int width = 0;
	int height = 0;
	Interfaces::Engine->GetScreenSize(width, height);
	if (world_to_screen(vOrig, vScreen))
	{
		Render::nonamegey(vScreen.x, vScreen.y, "BOMB", Render::Fonts::ESP, Color(255, 10, 10));
	}
	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		if (lifetime > 0.01f && !Bomb->IsBombDefused())
		{
			int boomval = (lifetime * 300) / 40;
			Render::gradient_horizontal(0, 0, 10, boomval, Color(250, 10, 50, 200), Color(10, 10, 10, 55));
			Render::textT(0, boomval, Render::Fonts::ESP, Color(255, 255, 255), "%.1f", lifetime);
			Render::textT(5, height / 2 - 140, Render::Fonts::niggerbomb, Color(250, 10, 50, 255), "%.1fs", lifetime);
		}
	}
	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		float flDistance = local->GetEyePosition().DistTo(entity->GetEyePosition());
		float a = 450.7f;
		float b = 75.68f;
		float c = 789.2f;
		float d = ((flDistance - b) / c);
		float flDamage = a * exp(-d * d);
		damage = float((std::max)((int)ceilf(CSGO_Armor(flDamage, local->ArmorValue())), 0));
		sprintf_s(bombdamagestringdead, sizeof(bombdamagestringdead) - 1, "");
		if (lifetime > 0.01f && !Bomb->IsBombDefused() && local->IsAlive())
		{
			if (damage >= local->GetHealth() && lifetime > 0.01f)
			{
				Render::textT(5, height / 2 - 120, Render::Fonts::niggerbomb, Color(255, 10, 10, 255), "FATAL");
			}
			else {
				if (lifetime > 0.01f && local->IsAlive()) {
					std::string gey;
					gey += "-";
					gey += std::to_string((int)(damage));
					gey += "HP";
					Render::textT(5, height / 2 - 120, Render::Fonts::niggerbomb, Color(255, 15, 15, 255), gey.c_str());
				}
			}
		}
	}
	if (Interfaces::Engine->IsConnected() && Interfaces::Engine->IsInGame())
	{
		if (Bomb->GetBombDefuser() > 0)
		{
			//IClientEntity *pDefuser = Interfaces::EntList->GetClientEntity(Bomb->GetBombDefuser());
			float countdown = Bomb->GetC4DefuseCountDown() - (local->GetTickBase() * Interfaces::Globals->interval_per_tick);
			//float maxdefuse = pDefuser->HasDefuser() ? 5.0f : 10.f;
			if (countdown > 0.01f)
			{
				if (lifetime > countdown)
				{
					Render::textT(5, height / 2 - 100, Render::Fonts::niggerbomb, Color(10, 140, 255, 255), "Defused in: %.1f", countdown);
				}
			}
		}
	}
	//i hate giving this out because it looks so p, but whatever
}
void CEsp::DrawWeaponIcon(IClientEntity* pEntity, Box size)
{
	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	if (pWeapon)
	{
		if (Options::Menu.VisualsTab.Weapons.GetIndex() == 2) {
			RECT nameSize = Render::GetTextSize(Render::Fonts::IconESP, pWeapon->GetGunIcon());
			Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 8,
				Color(Options::Menu.ColorsTab.Weapons.GetValue()), Render::Fonts::IconESP, pWeapon->GetGunIcon());
		}
	}
}
/*void CEsp::multipoint(IClientEntity* pEntity, Box size)
{
	if (Options::Menu.VisualsTab.DebugHitbones.GetState()) {
		for (int i = 0; i < 19; i++) {
			Vector screen_position;
			Vector hitbone_pos = GetHitboxPosition(pEntity, i);
			if (world_to_screen(hitbone_pos, screen_position)) {
				Render::rect(screen_position.x, screen_position.y, 4, 4, Color(255, 255, 255, 255));
				Render::rect(screen_position.x + 1, screen_position.y + 1, 2, 2, Color(255, 255, 255, 255));
			}
		}
	}
}*/
void CEsp::DrawWeapon(IClientEntity* pEntity, Box size)
{
	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	if (pWeapon)
	{
		if (Options::Menu.VisualsTab.Weapons.GetIndex() == 1) {
			RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, pWeapon->GetWeaponName());
			Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 5,
				Color(Options::Menu.ColorsTab.Weapons.GetValue()), Render::Fonts::ESP, pWeapon->GetWeaponName());
		}
	}
}
void DrawSkeletonBECKTRECK(IClientEntity* entity, const matrix3x4_t* bone_matrix)
{
	auto studio_model = Interfaces::ModelInfo->GetStudiomodel(entity->GetModel());
	if (!studio_model)
		return;
	for (int i = 0; i < studio_model->numbones; i++)
	{
		auto bone = studio_model->GetBone(i);
		if (!bone || bone->parent < 0 || !(bone->flags & BONE_USED_BY_HITBOX))
			continue;
		matrix3x4_t bone_matrix_1, bone_matrix_2;
		if (bone_matrix)
		{
			bone_matrix_1 = bone_matrix[i];
			bone_matrix_2 = bone_matrix[bone->parent];
		}
		else
		{
			bone_matrix_1 = entity->GetBoneMatrix(i);
			bone_matrix_2 = entity->GetBoneMatrix(bone->parent);
		}
		Vector bone_position_1 = Vector(bone_matrix_1[0][3], bone_matrix_1[1][3], bone_matrix_1[2][3]),
			bone_position_2 = Vector(bone_matrix_2[0][3], bone_matrix_2[1][3], bone_matrix_2[2][3]);
		Vector screen1, screen2;
		if (world_to_screen(bone_position_1, screen1) && world_to_screen(bone_position_2, screen2))
			Render::DrawLine(screen1.x, screen1.y, screen2.x, screen2.y, Color(120, 120, 120, 255));
	}
}
int GetSequenceActivity(IClientEntity* lole, int sequence)
{
	auto hdr = Interfaces::ModelInfo->GetStudiomodel(lole->GetModel());
	if (!hdr)
		return -1;
	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Offsets::Functions::GetSeqActivity);
	return get_sequence_activity(lole, hdr, sequence);
}
void CEsp::DrawAmmo(IClientEntity* pEntity, Box size)
{
	C_BaseCombatWeapon* pWeapon = pEntity->GetWeapon2();
	if (pWeapon)
	{
		if (pWeapon->isZeus27() || pWeapon->IsMiscGAY())
			return;
		CSWeaponInfo* weapInfo = pWeapon->GetCSWpnData();
		Color arc = Color(Options::Menu.ColorsTab.Ammo.GetValue());
		int ammoyes = pWeapon->GetAmmoInClip() * (size.w) / weapInfo->max_clip;
		Render::outlineyeti(size.x - 1, size.y + size.h + 1, size.w + 2, 4, Color(21, 21, 21, 255));
		Render::rect(size.x, size.y + size.h + 2, size.w, 2, Color(51, 51, 51, 255));
		Render::rect(size.x, size.y + size.h + 2, ammoyes, 2, arc);
		if (weapInfo->max_clip > pWeapon->GetAmmoInClip())
			Render::Text(size.x + ammoyes, size.y + size.h, Color(255, 255, 255), Render::Fonts::smallassfont, std::to_string(pWeapon->GetAmmoInClip()).c_str());
		//didn't bother adding reload anim, if you want it, go check on uc
	}
}
void CEsp::DrawPlayer(IClientEntity* pEntity, player_info_t pinfo)
{
	Box box;
	Color Color;
	Vector max = pEntity->GetCollideable()->OBBMaxs();
	Vector pos, pos3D;
	Vector top, top3D;
	pos3D = pEntity->GetOrigin();
	top3D = pos3D + Vector(0, 0, max.z);

	if (!Interfaces::Engine->IsConnected() || !Interfaces::Engine->IsInGame())
		return;

	if (!Render::TransformScreen(pos3D, pos) || !Render::TransformScreen(top3D, top))
		return;

	
	if (Options::Menu.VisualsTab.FiltersEnemiesOnly.GetState() && (pEntity->GetTeamNum() == hackManager.pLocal()->GetTeamNum()))
		return;
	
	if (Options::Menu.VisualsTab.OffscreenESP.GetState())
		drawgayarrows(pEntity);


	if (get_box(pEntity, box, Options::Menu.VisualsTab.Active.GetState()))
	{
		Color = GetPlayerColor(pEntity);
		if (Options::Menu.VisualsTab.OptionsBox.GetIndex() == 1)
			DrawBoxx(box, Options::Menu.ColorsTab.BoxCol.GetValue());
		else if (Options::Menu.VisualsTab.OptionsBox.GetIndex() == 2)
			DrawBoxx2(box, Options::Menu.ColorsTab.BoxCol.GetValue());
		else if (Options::Menu.VisualsTab.OptionsBox.GetIndex() == 3)
			Corners(box, Options::Menu.ColorsTab.BoxCol.GetValue(), pEntity);
		if (Options::Menu.VisualsTab.OptionsName.GetState())
			DrawName(pinfo, box, pEntity);
		if (Options::Menu.VisualsTab.OptionsHealth.GetIndex() == 1)
			DrawVitals(pEntity, box);
		if (Options::Menu.VisualsTab.OptionsHealth.GetIndex() == 2)
			DrawHealth(pEntity, box);
		if (Options::Menu.VisualsTab.OptionsHealth.GetIndex() == 3)
			DrawInfo22(pEntity, box);
		if (Options::Menu.VisualsTab.OptionsInfo.GetState() || Options::Menu.VisualsTab.OptionsWeapone.GetState())
		{
			DrawInfo(pEntity, box);
			DrawInfo2(pEntity, box);
		}

		if (Options::Menu.VisualsTab.OptionsSkeleton.GetState())
			DrawSkeleton(pEntity);
		//if (Options::Menu.VisualsTab.DebugHitbones.GetState())
			//multipoint(pEntity, box);
		if (Options::Menu.VisualsTab.Weapons.GetIndex() == 1) {
			DrawWeapon(pEntity, box);
		}
		if (Options::Menu.VisualsTab.Weapons.GetIndex() == 2) {
			DrawWeaponIcon(pEntity, box);
		}
		if (Options::Menu.VisualsTab.Ammo.GetState()) {
			DrawAmmo(pEntity, box);
		}

	}
}

float dot_product_t(const float* a, const float* b) {
	return (a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
}

#define rad_pi 57.295779513082f
#define pi 3.14159265358979323846f
#define rad(a) a * 0.01745329251
#define deg(a) a * 57.295779513082

float degrees_to_radians(const float deg)
{
	return deg * (pi / 180.f);
}
float radians_to_degrees(const float rad)
{
	return rad * rad_pi;
}
void rotate_triangle(std::array<Vector2D, 3>& points, float rotation)
{
	const auto points_center = (points.at(0) + points.at(1) + points.at(2)) / 3;
	for (auto& point : points)
	{
		point -= points_center;
		const auto temp_x = point.x;
		const auto temp_y = point.y;
		const auto theta = degrees_to_radians(rotation);
		const auto c = cosf(theta);
		const auto s = sinf(theta);
		point.x = temp_x * c - temp_y * s;
		point.y = temp_x * s + temp_y * c;
		point += points_center;
	}
}
template<class T, class U>
inline T clampx(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}
void VectorAngles(Vector &forward, QAngle &angles)
{
	Assert(s_bMathlibInitialized);
	float	tmp, yaw, pitch;
	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / pi);
		if (yaw < 0)
			yaw += 360;
		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / pi);
		if (pitch < 0)
			pitch += 360;
	}
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}
void CEsp::DrawInfo22(IClientEntity* pEntity, Box size)
{
	RECT defSize = Render::GetTextSize(Render::Fonts::ESP, "");
	std::vector<std::string> Info;
	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	static RECT Size = Render::GetTextSize(Render::Fonts::Default, "Hi");
	char hp[50];
	sprintf_s(hp, sizeof(hp), "HP: %i", pEntity->GetHealth());
	if (Options::Menu.VisualsTab.OptionsHealth.GetIndex() == 3)
	{
		RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, hp);
		sprintf_s(hp, sizeof(hp), "HP: %i", pEntity->GetHealth());
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 16, Color(0, 255, 120, 255), Render::Fonts::ESP, hp);
	}
	int i = 0;
	for (auto Text : Info)
	{
		Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::ESP, Text.c_str());
		i++;
	}
}
void VectorAnglesXX(const Vector& forward, Vector &angles)
{
	float tmp, yaw, pitch;
	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
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
	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}
Vector CalcAngleXX(Vector src, Vector dst)
{
	Vector ret;
	VectorAnglesXX(dst - src, ret);
	return ret;
}
bool World2Screen(const Vector &origin, Vector &screen)
{
	if (!screen_transformx(origin, screen)) {
		int iScreenWidth, iScreenHeight;
		Interfaces::Engine->GetScreenSize(iScreenWidth, iScreenHeight);
		screen.x = (iScreenWidth / 2.0f) + (screen.x * iScreenWidth) / 2;
		screen.y = (iScreenHeight / 2.0f) - (screen.y * iScreenHeight) / 2;
		return true;
	}
	return false;
}
bool isOnScreen(Vector origin, Vector &screen)
{
	if (!World2Screen(origin, screen)) return false;
	int iScreenWidth, iScreenHeight;
	Interfaces::Engine->GetScreenSize(iScreenWidth, iScreenHeight);
	bool xOk = iScreenWidth > screen.x > 0, yOk = iScreenHeight > screen.y > 0;
	return xOk && yOk;
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
void CEsp::drawgayarrows(IClientEntity* entity) {
	auto local_player = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());
	if (!local_player) return;
	if (!local_player->IsAlive())
		return;
	if (entity->IsDormant()) return;
	if (entity->GetTeamNum() == local_player->GetTeamNum())
		return;
	Vector screenPos, client_viewangles;
	int screen_width = 0, screen_height = 0;
	float radius = 300.f;
	if (isOnScreen(GameUtils::get_hitbox_location(entity, 0), screenPos)) return;
	Interfaces::Engine->GetViewAngles(client_viewangles);
	Interfaces::Engine->GetScreenSize(screen_width, screen_height);
	const auto screen_center = Vector(screen_width / 2.f, screen_height / 2.f, 0);
	const auto rot = DEG2RAD(client_viewangles.y - CalcAngleXX(local_player->GetEyePosition(), GameUtils::get_hitbox_location(entity, 1)).y - 90);
	static auto alpha = 0.f;
	static auto plus_or_minus = false;
	if (alpha <= 0.f || alpha >= 255.f)
		plus_or_minus = !plus_or_minus;
	alpha += plus_or_minus ? (255.f / 0.5f * Interfaces::Globals->frametime) : -(255.f / 0.5f * Interfaces::Globals->frametime);
	alpha = clamp<float>(alpha, 0.f, 255.f);
	std::vector<Vertex_t> vertices;
	vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot) * radius, screen_center.y + sinf(rot) * radius)));
	vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot + DEG2RAD(2)) * (radius - 16), screen_center.y + sinf(rot + DEG2RAD(2)) * (radius - 16))));
	vertices.push_back(Vertex_t(Vector2D(screen_center.x + cosf(rot - DEG2RAD(2)) * (radius - 16), screen_center.y + sinf(rot - DEG2RAD(2)) * (radius - 16))));
	Render::TexturedPolygon(3, vertices, Color(Options::Menu.ColorsTab.Offscreen.GetValue()[0], Options::Menu.ColorsTab.Offscreen.GetValue()[1], Options::Menu.ColorsTab.Offscreen.GetValue()[2], alpha));
}
void CEsp::SpecList()
{
	IClientEntity *pLocal = hackManager.pLocal();
	RECT scrn = Render::GetViewport();
	int ayy = 0;
	// Loop through all active entitys
	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		// Get the entity
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		player_info_t pinfo;
		// The entity isn't some laggy peice of shit or something
		if (pEntity &&  pEntity != pLocal)
		{
			if (Interfaces::Engine->GetPlayerInfo(i, &pinfo) && !pEntity->IsAlive() && !pEntity->IsDormant())
			{
				HANDLE obs = pEntity->GetObserverTargetHandle();
				if (obs)
				{
					IClientEntity *pTarget = Interfaces::EntList->GetClientEntityFromHandle(obs);
					player_info_t pinfo2;
					if (pTarget)
					{
						if (Interfaces::Engine->GetPlayerInfo(pTarget->GetIndex(), &pinfo2))
						{
							if (strlen(pinfo.name) > 16)
							{
								pinfo.name[12] = 0;
								strcat(pinfo.name, "...");
								RECT TextSize = Render::GetTextSize(Render::Fonts::nameaiz, pinfo.name);
								RECT nameSize = Render::GetTextSize(Render::Fonts::nameaiz, pinfo.name);
								Render::Text(scrn.right - TextSize.right - 4, (scrn.top / 4) + (16 * ayy), pTarget->GetIndex() == pLocal->GetIndex() ? Color(255, 255, 255, 255) : Color(255, 255, 255, 255), Render::Fonts::nameaiz, pinfo.name);
								ayy++;
							}
						}
					}
				}
			}
		}
	}
}
void vector_transform_a(const float *in1, const matrix3x4& in2, float *out) {
	out[0] = dot_product_t(in1, in2[0]) + in2[0][3];
	out[1] = dot_product_t(in1, in2[1]) + in2[1][3];
	out[2] = dot_product_t(in1, in2[2]) + in2[2][3];
}
inline void vector_transform_z(const Vector& in1, const matrix3x4 &in2, Vector &out) {
	vector_transform_a(&in1.x, in2, &out.x);
}
bool CEsp::get_box(IClientEntity* m_entity, Box& box, bool dynamic) {
	DWORD m_rgflCoordinateFrame = (DWORD)0x470 - 0x30;
	const matrix3x4& trnsf = *(matrix3x4*)((DWORD)m_entity + (DWORD)m_rgflCoordinateFrame);
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;
	vOrigin = m_entity->GetOrigin();
	min = m_entity->collisionProperty()->GetMins();
	max = m_entity->collisionProperty()->GetMaxs();
	if (!dynamic) {
		min += vOrigin;
		max += vOrigin;
	}
	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };
	Vector vector_transformed[8];
	if (dynamic)
	{
		for (int i = 0; i < 8; i++)
		{
			vector_transform_z(points[i], trnsf, vector_transformed[i]);
			points[i] = vector_transformed[i];
		}
	}
	if (!Render::TransformScreen(points[3], flb) || !Render::TransformScreen(points[5], brt)
		|| !Render::TransformScreen(points[0], blb) || !Render::TransformScreen(points[4], frt)
		|| !Render::TransformScreen(points[2], frb) || !Render::TransformScreen(points[1], brb)
		|| !Render::TransformScreen(points[6], blt) || !Render::TransformScreen(points[7], flt))
		return false;
	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;
	for (int i = 1; i < 8; i++) {
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}
	box.x = left;
	box.y = top;
	box.w = right - left;
	box.h = bottom - top;
	return true;
}
bool CEsp::GetBox(IClientEntity* pEntity, CEsp::ESPBox &result)
{
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;
	vOrigin = pEntity->GetOrigin();
	min = pEntity->collisionProperty()->GetMins() + vOrigin;
	max = pEntity->collisionProperty()->GetMaxs() + vOrigin;
	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };
	if (!Render::TransformScreen(points[3], flb) || !Render::TransformScreen(points[5], brt)
		|| !Render::TransformScreen(points[0], blb) || !Render::TransformScreen(points[4], frt)
		|| !Render::TransformScreen(points[2], frb) || !Render::TransformScreen(points[1], brb)
		|| !Render::TransformScreen(points[6], blt) || !Render::TransformScreen(points[7], flt))
		return false;
	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };
	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;
	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}
	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;
	return true;
}
Color CEsp::GetPlayerColor(IClientEntity* pEntity)
{
	int TeamNum = pEntity->GetTeamNum();
	bool IsVis = GameUtils::IsVisible(hackManager.pLocal(), pEntity, (int)CSGOHitboxID::HITBOX_HEAD);
	Color color;
	if (TeamNum == TEAM_CS_T)
	{
	}
	else
	{
	}
	return color;
}
std::string CleanItemName(std::string name)
{
	std::string Name = name;
	if (Name[0] == 'C')
		Name.erase(Name.begin());
	auto startOfWeap = Name.find("Weapon");
	if (startOfWeap != std::string::npos)
		Name.erase(Name.begin() + startOfWeap, Name.begin() + startOfWeap + 6);
	return Name;
}
void CEsp::DrawGun(IClientEntity* pEntity, CEsp::ESPBox size)
{
	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	ClientClass* cClass = (ClientClass*)pWeapon->GetClientClass();
	if (cClass)
	{
		std::string meme = CleanItemName(cClass->m_pNetworkName);
		RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, meme.c_str());
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 1,
			Color(255, 255, 255, 255), Render::Fonts::ESP, meme.c_str());
	}
}
void CEsp::Corners(Box size, Color color, IClientEntity* pEntity)
{
	int VertLine = (((float)size.w) * (0.20f));
	int HorzLine = (((float)size.h) * (0.30f));
	Render::Clear(size.x, size.y - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x - 1, size.y, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - 1, size.y, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x, size.y, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
	Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x, size.y, 1, HorzLine, color);
	Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
}
void CEsp::FilledBox(CEsp::ESPBox size, Color color)
{
	int VertLine = (((float)size.w) * (0.20f));
	int HorzLine = (((float)size.h) * (0.20f));
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	Render::Clear(size.x, size.y, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
	Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	Render::Clear(size.x, size.y, 1, HorzLine, color);
	Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	CEsp::ESPBox box = size;
	Render::Outline(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(21, 21, 21, 150));
	Render::Outline(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(21, 21, 21, 150));
}
void CEsp::DrawBoxx(Box box, Color color)
{
	Render::Outline(box.x, box.y, box.w, box.h, color);
	Render::Outline(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(21, 21, 21, 150));
	Render::Outline(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(21, 21, 21, 150));
	Render::Outline(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(21, 21, 21, 150));
	Render::Outline(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(21, 21, 21, 150));
}
void CEsp::DrawBoxx2(Box size, Color color)
{
	int VertLine = (((float)size.w) * (0.20f));
	int HorzLine = (((float)size.h) * (1.00f));
	Render::Clear(size.x, size.y, VertLine, 1.2, color);
	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1.2, color);
	Render::Clear(size.x, size.y + size.h, VertLine, 1.2, color);
	Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1.2, color);
	Render::Clear(size.x, size.y, 1, HorzLine, color);
	Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
	Box box = size;
	Render::Outline(box.x - 1, box.y - 1, box.w + 2, box.h + 2, Color(21, 21, 21, 150));
	Render::Outline(box.x + 1, box.y + 1, box.w - 2, box.h - 2, Color(21, 21, 21, 150));
}
static wchar_t* CharToWideChar(const char* text)
{
	size_t size = strlen(text) + 1;
	wchar_t* wa = new wchar_t[size];
	mbstowcs_s(NULL, wa, size / 4, text, size);
	return wa;
}
//converting like that will fuck the unicode characters lmao who made that
void CEsp::DrawName(player_info_t pinfo, Box size, IClientEntity* pEntity)
{
	if (!pEntity || !pEntity->IsAlive() || !hackManager.pLocal())
		return;
	wchar_t buffer[36];
	auto name = pinfo.name;
	if (MultiByteToWideChar(CP_UTF8, 0, pinfo.name, -1, buffer, 36) > 0)
	{
		RECT nameSize = Render::GetTextSize(Render::Fonts::nameaiz, pinfo.name);
		Render::TEXTUNICODE(size.x + (size.w / 2) - (nameSize.right / 2), size.y - 11, pinfo.name, Render::Fonts::nameaiz, Color(Options::Menu.ColorsTab.NameCol.GetValue()));
	}
}
void CEsp::DrawHealth(IClientEntity* pEntity, Box size)
{
	Box HealthBar = size;
	HealthBar.y += (HealthBar.h + 7);
	HealthBar.h = 4;
	float HealthValue = pEntity->GetHealth();
	float HealthPerc = HealthValue / 100.f;
	float flBoxes = std::ceil(HealthValue / 10.f);
	float Width = (size.w * HealthPerc);
	HealthBar.w = Width;
	float h = (size.h);
	float offset = (h / 4.f) + 5;
	float w = h / 64.f;
	float health = pEntity->GetHealth();
	float flMultiplier = 12 / 360.f; flMultiplier *= flBoxes - 1;
	Color ColHealth = Color::FromHSB(flMultiplier, 1, 1);
	UINT hp = h - (UINT)((h * health) / 100);
	int Red = 255 - (health*2.55);
	int Green = health * 2.55;
	Render::Outline((size.x - 6), size.y - 2, 4, h + 2, Color(0, 0, 0, 255));
	Render::Outline(size.x - 5, size.y - 1, 1, (h / 10) * flBoxes + 1, ColHealth);
	for (int i = 0; i < 10; i++)
	{
		Render::Line((size.x - 5), size.y + i * (h / 10) - 0.4, size.x - 3.5, size.y + i * (h / 10), Color(0, 0, 0, 255));
	}
}
void CEsp::Corners(CEsp::ESPBox size, Color color, IClientEntity* pEntity)
{
	int VertLine = (((float)size.w) * (0.20f));
	int HorzLine = (((float)size.h) * (0.30f));
	Render::Clear(size.x, size.y - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x - 1, size.y, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - 1, size.y, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x, size.y, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
	Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x, size.y, 1, HorzLine, color);
	Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
}
void CEsp::DrawHealth2(IClientEntity* pEntity, Box size)
{
	Box HealthBar = size;
	HealthBar.y += (HealthBar.h + 7);
	HealthBar.h = 4;
	float HealthValue = pEntity->GetHealth();
	float HealthPerc = HealthValue / 100.f;
	float flBoxes = std::ceil(HealthValue / 10.f);
	float Width = (size.w * HealthPerc);
	HealthBar.w = Width;
	float h = (size.h);
	float offset = (h / 4.f) + 5;
	float w = h / 64.f;
	float health = pEntity->GetHealth();
	float flMultiplier = 12 / 360.f; flMultiplier *= flBoxes - 1;
	Color ColHealth = Color::FromHSB(flMultiplier, 1, 1);
	UINT hp = h - (UINT)((h * health) / 100);
	int Red = 255 - (health*2.55);
	int Green = health * 2.55;
	Render::Outline((size.x - 6), size.y - 2, 4, h + 2, Color(0, 0, 0, 255));
	Render::Outline(size.x - 5, size.y - 1, 1, (h / 10) * flBoxes + 1, ColHealth);
	for (int i = 0; i < 10; i++)
	{
		Render::Line((size.x - 5), size.y + i * (h / 10) - 0.4, size.x - 3.5, size.y + i * (h / 10), Color(0, 0, 0, 255));
	}
}
void CEsp::DrawVitals(IClientEntity* pEntity, Box size)
{
	Box box = size;
	int player_health = pEntity->GetHealth() > 100 ? 100 : pEntity->GetHealth();
	if (player_health) {
		if (player_health > 100) {
			player_health = 100;
		}
		int color[3] = { 0, 0, 0 };
		if (player_health >= 85) {
			color[0] = 83; color[1] = 200; color[2] = 84;
		}
		else if (player_health >= 70) {
			color[0] = 107; color[1] = 142; color[2] = 35;
		}
		else if (player_health >= 55) {
			color[0] = 173; color[1] = 255; color[2] = 47;
		}
		else if (player_health >= 40) {
			color[0] = 255; color[1] = 215; color[2] = 0;
		}
		else if (player_health >= 25) {
			color[0] = 255; color[1] = 127; color[2] = 80;
		}
		else if (player_health >= 10) {
			color[0] = 205; color[1] = 92; color[2] = 92;
		}
		else if (player_health >= 0) {
			color[0] = 178; color[1] = 34; color[2] = 34;
		}
		Render::Outline(box.x - 7, box.y - 1, 4, box.h + 2, Color(21, 21, 21, 255));
		int health_height = player_health * box.h / 100;
		int add_space = box.h - health_height;
		Color hec = Color(color[0], color[1], color[2], 255);
		Render::rect(box.x - 6, box.y, 2, box.h, Color(21, 21, 21, 255));
		Render::rect(box.x - 6, box.y + add_space, 2, health_height, hec);
		if (player_health < 100) {
			RECT text_size = Render::GetTextSize(Render::Fonts::slickESP, std::to_string(player_health).c_str());
			Render::Text(box.x - 5 - (text_size.right / 2), box.y + add_space - (text_size.bottom / 2), Color(255, 255, 255, 255), Render::Fonts::slickESP, std::to_string(player_health).c_str());
		}
	}
}
void CEsp::DrawHealthOld(IClientEntity* pEntity, Box size)
{
	int HPEnemy = 100;
	HPEnemy = pEntity->GetHealth();
	char nameBuffer[512];
	sprintf_s(nameBuffer, "%d", HPEnemy);
	float h = (size.h);
	float health = pEntity->GetHealth();
	UINT hp = h - (UINT)((h * health) / 100);
	int Health = pEntity->GetHealth();
	if (Health > 100)
		Health = 100;
	int healthG = Health * 2.55;
	int healthR = 255 - healthG;
	Interfaces::Surface->DrawSetColor(0, 0, 0, 200);
	Interfaces::Surface->DrawOutlinedRect(size.x - 5, size.y - 1, size.x - 1, size.y + size.h + 1);
	//Interfaces::Surface->DrawOutlinedRect(BOX.x + BOX.w + 1, BOX.y - 1, BOX.x + BOX.w + 5, BOX.y + BOX.h + 1);
	int hpBarH = Health * size.h / 100;
	if (Health > 0)
	{
		Interfaces::Surface->DrawSetColor(healthR, healthG, 0, 255);
		Interfaces::Surface->DrawFilledRect(size.x - 4, size.y + size.h - hpBarH, size.x - 2, size.y + size.h);
	}
	if (Health <= 100)
	{
		Render::Text(size.x - 10, size.y + hp - 2, Color(255, 255, 255, 255), Render::Fonts::ESP, nameBuffer);
	}
	if (Health > 104)
	{
		Interfaces::Surface->DrawSetColor(250, 10, (30 + rand() % 25), 255); // anti Cuntmode
		Render::Text(size.x - 10, size.y + hp - 2, Color((100 + rand() % 150), (100 + rand() % 150), (100 + rand() % 150), 255), Render::Fonts::ESP, nameBuffer);
	}
}
void CEsp::DrawInfo21(IClientEntity* pEntity, Box size)
{
	RECT defSize = Render::GetTextSize(Render::Fonts::ESP, "");
	std::vector<std::string> Info;
	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	static RECT Size = Render::GetTextSize(Render::Fonts::Default, "Hi");
	char hp[50];
	sprintf_s(hp, sizeof(hp), "HP: %i", pEntity->GetHealth());
	if (Options::Menu.VisualsTab.OptionsHealth.GetIndex() == 4)
	{
		RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, hp);
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 5.1, Color(0, 255, 120, 255), Render::Fonts::ESP, hp);
		int HPEnemy = 100;
		HPEnemy = pEntity->GetHealth();
		char nameBuffer[512];
		sprintf_s(nameBuffer, "%d", HPEnemy);
		float h = (size.h);
		float health = pEntity->GetHealth();
		UINT hp = h - (UINT)((h * health) / 100);
		int Health = pEntity->GetHealth();
		if (Health > 100)
			Health = 100;
		int healthG = Health * 2.55;
		int healthR = 255 - healthG;
		Interfaces::Surface->DrawSetColor(0, 0, 0, 200);
		Interfaces::Surface->DrawOutlinedRect(size.x - 5, size.y - 1, size.x - 1, size.y + size.h + 1);
		//Interfaces::Surface->DrawOutlinedRect(BOX.x + BOX.w + 1, BOX.y - 1, BOX.x + BOX.w + 5, BOX.y + BOX.h + 1);
		int hpBarH = Health * size.h / 100;
		if (Health > 0)
		{
			Interfaces::Surface->DrawSetColor(healthR, healthG, 0, 255);
			Interfaces::Surface->DrawFilledRect(size.x - 4, size.y + size.h - hpBarH, size.x - 2, size.y + size.h);
		}
		if (Health <= 100)
		{
			Render::Text(size.x - 10, size.y + hp - 2, Color(255, 255, 255, 255), Render::Fonts::ESP, nameBuffer);
		}
		if (Health > 104)
		{
			Interfaces::Surface->DrawSetColor(250, 10, (30 + rand() % 25), 255); // anti Cuntmode
			Render::Text(size.x - 10, size.y + hp - 2, Color((100 + rand() % 150), (100 + rand() % 150), (100 + rand() % 150), 255), Render::Fonts::ESP, nameBuffer);
		}
	}
	int i = 0;
	for (auto Text : Info)
	{
		Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::ESP, Text.c_str());
		i++;
	}
}
void CEsp::DrawInfo(IClientEntity* pEntity, Box size)
{
	IClientEntity *g_LocalPlayer = hackManager.pLocal();
	if (!g_LocalPlayer)
		return;
	std::vector<std::pair<std::string, Color>> stored_info;
	auto entity = pEntity;
	static float old_simtime[65];




	if (entity->GetFlashDuration() > 0)
		stored_info.push_back(std::pair<std::string, Color>("Flashed", Color(255, 255, 255, 250)));
	if (BombCarrier == entity)
		stored_info.push_back(std::pair<std::string, Color>("Bomb", Color(250, 10, 10, 220)));

	int i = 0;
	for (auto Text : stored_info)
	{
		RECT TextSize = Render::GetTextSize(Render::Fonts::ESP, Text.first.c_str());
		Render::Text(size.x + size.w + 1, size.y + i - ((TextSize.bottom / 2) - 5), Text.second, Render::Fonts::ESP, Text.first.c_str());
		i += 8;
	}
}


void CEsp::DrawInfo2(IClientEntity* pEntity, Box size)
{
	RECT defSize = Render::GetTextSize(Render::Fonts::ESP, "");
	std::vector<std::string> Info;

	// Player Weapon ESP
	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	static RECT Size = Render::GetTextSize(Render::Fonts::Default, "Hi");
	RECT named = Render::GetTextSize(Render::Fonts::ESP, "Scoped");

	if (Options::Menu.VisualsTab.OptionsInfo.GetState())
	{
		char hp[50];
		sprintf_s(hp, sizeof(hp), "%i", pEntity->ArmorValue());
		RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, hp);
		if (pEntity->ArmorValue() > 0)
		{
		}

	}
	if (Options::Menu.VisualsTab.OptionsInfo.GetState())
	{
		if (pEntity->IsScoped())
		{
			Render::Text(size.x + (size.w / 2) - (named.right / 2), size.y - 20, Color(250, 250, 250, 255), Render::Fonts::ESP, "Scoped");
		}


	}


	int i = 0;
	for (auto Text : Info)
	{
		Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::ESP, Text.c_str());
		i++;
	}
}


void CEsp::DrawCross(IClientEntity* pEntity)
{
	Vector cross = pEntity->GetHeadPos(), screen;
	vec_t Scale = 2;
	if (Render::TransformScreen(cross, screen))
	{
		Render::Clear(screen.x - Scale, screen.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen.x - (Scale * 2), screen.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen.x - Scale - 1, screen.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(250, 250, 250, 160));
		Render::Clear(screen.x - (Scale * 2) - 1, screen.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(250, 250, 250, 160));
	}
}
void CEsp::DrawDrop(IClientEntity* pEntity, ClientClass* cClass)
{
	Color color;
	Box Box;
	IClientEntity* Weapon = (IClientEntity*)pEntity;
	IClientEntity* plr = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)Weapon->GetOwnerHandle());
	if (Weapon && !plr)
	{
		if (get_box(pEntity, Box, Options::Menu.VisualsTab.FiltersWeapons.GetState()))
		{
			if (Options::Menu.VisualsTab.FiltersWeapons.GetState())
			{
				//DrawBoxx(Box, Color(255, 255, 255, 255));
			}
			RECT TextSize = Render::GetTextSize(Render::Fonts::nameaiz, Weapon->GetWeaponName());
			Render::Text(Box.x + (Box.w / 2) - (TextSize.right / 2), Box.y + Box.h, Color(255, 255, 255, 255), Render::Fonts::nameaiz, Weapon->GetWeaponName());
		}
	}
}
void CEsp::DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass)
{
	BombCarrier = nullptr;
	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)pEntity;
	float flBlow = Bomb->GetC4BlowTime();
	float TimeRemaining = flBlow - (Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase());
	char buffer[64];
	sprintf_s(buffer, "%.1fs", TimeRemaining);
	float TimeRemaining2;
	bool exploded = true;
	if (TimeRemaining < 0)
	{
		!exploded;
		TimeRemaining2 = 0;
	}
	else
	{
		exploded = true;
		TimeRemaining2 = TimeRemaining;
	}
	if (exploded)
	{
		sprintf_s(buffer, "Bomb: %.1f", TimeRemaining2);
	}
	else
	{
		sprintf_s(buffer, "Bomb Undefusable", TimeRemaining2);
	}
	//Render::Text(10, 45, Color(0, 255, 0, 255), Render::Fonts::Clock, buffer);
}
void CEsp::DrawBomb(IClientEntity* pEntity, ClientClass* cClass)
{
	BombCarrier = nullptr;
	C_BaseCombatWeapon *BombWeapon = (C_BaseCombatWeapon *)pEntity;
	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	bool adopted = true;
	HANDLE parent = BombWeapon->GetOwnerHandle();
	if (parent || (vOrig.x == 0 && vOrig.y == 0 && vOrig.z == 0))
	{
		IClientEntity* pParentEnt = (Interfaces::EntList->GetClientEntityFromHandle(parent));
		if (pParentEnt && pParentEnt->IsAlive())
		{
			BombCarrier = pParentEnt;
			adopted = false;
		}
	}
}
void DrawBoneArray(int* boneNumbers, int amount, IClientEntity* pEntity, Color color)
{
	Vector LastBoneScreen;
	for (int i = 0; i < amount; i++)
	{
		Vector Bone = pEntity->GetBonePos(boneNumbers[i]);
		Vector BoneScreen;
		if (Render::TransformScreen(Bone, BoneScreen))
		{
			if (i>0)
			{
				Render::Line(LastBoneScreen.x, LastBoneScreen.y, BoneScreen.x, BoneScreen.y, color);
			}
		}
		LastBoneScreen = BoneScreen;
	}
}
void DrawBoneTest(IClientEntity *pEntity)
{
	for (int i = 0; i < 127; i++)
	{
		Vector BoneLoc = pEntity->GetBonePos(i);
		Vector BoneScreen;
		if (Render::TransformScreen(BoneLoc, BoneScreen))
		{
			char buf[10];
			_itoa_s(i, buf, 10);
			Render::Text(BoneScreen.x, BoneScreen.y, Color(255, 255, 255, 180), Render::Fonts::ESP, buf);
		}
	}
}
void CEsp::DrawSkeleton(IClientEntity* pEntity)
{
	studiohdr_t* pStudioHdr = Interfaces::ModelInfo->GetStudiomodel(pEntity->GetModel());
	if (!pStudioHdr)
		return;
	Vector vParent, vChild, sParent, sChild;
	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);
		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);
			if (Render::TransformScreen(vParent, sParent) && Render::TransformScreen(vChild, sChild))
			{
				Render::Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(Options::Menu.ColorsTab.Skeleton.GetValue()[0], Options::Menu.ColorsTab.Skeleton.GetValue()[1], Options::Menu.ColorsTab.Skeleton.GetValue()[2], Options::Menu.ColorsTab.Skeleton.GetValue()[3]));
			}
		}
	}
}
void CEsp::DrawHealthText(IClientEntity* pEntity, CEsp::ESPBox size)
{
	ESPBox HealthBar = size;
	HealthBar.y += (HealthBar.h + 6);
	HealthBar.h = 4;
	float HealthValue = pEntity->GetHealth();
	float HealthPerc = HealthValue / 100.f;
	float Width = (size.w * HealthPerc);
	HealthBar.w = Width;
	int HPEnemy = 100;
	HPEnemy = pEntity->GetHealth();
	char nameBuffer[512];
	sprintf_s(nameBuffer, "%d HP", HPEnemy);
	Render::Text(size.x - 36, size.y + 0, Color(255, 255, 255, 255), Render::Fonts::ESP, nameBuffer);
}
void CEsp::BoxAndText(IClientEntity* entity, std::string text)
{
	Box Box;
	std::vector<std::string> Info;
	RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, "");
	if (get_box(entity, Box, Options::Menu.VisualsTab.FiltersNades.GetState()))
	{
		Info.push_back(text);
		if (Options::Menu.VisualsTab.FiltersNades.GetState())
		{
			int i = 0;
			for (auto kek : Info)
			{
				DrawBoxx(Box, Color(255, 255, 255, 255));
				Render::Text(Box.x + (Box.w / 2) - (nameSize.right / 2), Box.y - 11, Color(255, 255, 255, 255), Render::Fonts::ESP, kek.c_str());
				i++;
			}
		}
	}
}
void CEsp::DrawThrowable(IClientEntity* throwable)
{
	model_t* nadeModel = (model_t*)throwable->GetModel();
	if (!nadeModel)
		return;
	studiohdr_t* hdr = Interfaces::ModelInfo->GetStudiomodel(nadeModel);
	if (!hdr)
		return;
	if (!strstr(hdr->name, "thrown") && !strstr(hdr->name, "dropped"))
		return;
	std::string nadeName = "Unknown Grenade";
	IMaterial* mats[32];
	Interfaces::ModelInfo->GetModelMaterials(nadeModel, hdr->numtextures, mats);
	for (int i = 0; i < hdr->numtextures; i++)
	{
		IMaterial* mat = mats[i];
		if (!mat)
			continue;
		if (strstr(mat->GetName(), "flashbang"))
		{
			nadeName = "flash";
			break;
		}
		else if (strstr(mat->GetName(), "m67_grenade") || strstr(mat->GetName(), "hegrenade"))
		{
			nadeName = "hegrenade";
			break;
		}
		else if (strstr(mat->GetName(), "smoke"))
		{
			nadeName = "smoke";
			break;
		}
		else if (strstr(mat->GetName(), "decoy"))
		{
			nadeName = "decoy";
			break;
		}
		else if (strstr(mat->GetName(), "incendiary") || strstr(mat->GetName(), "molotov"))
		{
			nadeName = "fire";
			break;
		}
	}
	BoxAndText(throwable, nadeName);
}