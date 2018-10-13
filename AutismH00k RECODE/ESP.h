#pragma once

#include "Hacks.h"
extern int missedLogHits[65];
struct Box
{
	int x = -1, y = -1, w = -1, h = -1;
};
class CEsp : public CHack
{
public:
	void Init();
	void Draw();
	void DrawBombus(IClientEntity * pEntity);
	void Move(CUserCmd *pCmd, bool &bSendPacket);
private:

	IClientEntity * BombCarrier;

	struct ESPBox
	{
		int x, y, w, h;
	};

	void DrawWeaponIcon(IClientEntity * pEntity, Box size);

	void multipoint(IClientEntity * pEntity, Box size);

	void DrawWeapon(IClientEntity * pEntity, Box size);

	void DrawAmmo(IClientEntity * pEntity, Box size);

	void DrawPlayer(IClientEntity* pEntity, player_info_t pinfo);
	void GoKys(IClientEntity * pEntity, Box size);
	void DrawInfo22(IClientEntity * pEntity, Box size);
	void drawgayarrows(IClientEntity * entity);
	void SpecList();

	Color GetPlayerColor(IClientEntity* pEntity);

	void DrawGun(IClientEntity * pEntity, CEsp::ESPBox size);
	void Corners(Box size, Color color, IClientEntity * pEntity);
	void Corners(CEsp::ESPBox size, Color color, IClientEntity * pEntity);

	void FilledBox(CEsp::ESPBox size, Color color);
	void DrawBoxx(Box size, Color color);
	void Rank(CUserCmd *pCmd);
	void DrawBoxx2(Box size, Color color);
	bool get_box(IClientEntity * m_entity, Box & box, bool dynamic);
	bool GetBox(IClientEntity* pEntity, ESPBox &result);



	void DrawName(player_info_t pinfo, Box size, IClientEntity* pEntity);
	void DrawHealth(IClientEntity * pEntity, Box size);


	void DrawHealth2(IClientEntity * pEntity, Box size);

	void DrawVitals(IClientEntity * pEntity, Box size);

	void DrawHealthOld(IClientEntity * pEntity, Box size);

	void DrawInfo21(IClientEntity * pEntity, Box size);

	void DrawInfo(IClientEntity * pEntity, Box size);

	void DrawInfo3(IClientEntity * pEntity, Box size);

	void DrawInfo2(IClientEntity * pEntity, Box size);

	void DrawCross(IClientEntity* pEntity);

	void DrawSkeleton(IClientEntity* pEntity);
	void DrawSkeletonBacktrack(IClientEntity * pEntity);
	void DrawHealthText(IClientEntity * pEntity, CEsp::ESPBox size);

	void BoxAndText(IClientEntity * entity, std::string text);
	void DrawThrowable(IClientEntity * throwable);

	void DrawDrop(IClientEntity* pEntity, ClientClass* cClass);
	void DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass);
	void DrawBomb(IClientEntity* pEntity, ClientClass* cClass);
};

