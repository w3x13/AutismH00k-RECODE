#pragma once

#include "Hacks.h"


class CVisuals : public CHack
{
public:
	void Init();
	void Draw();
	void Move(CUserCmd *pCmd, bool &bSendPacket);
private:
	void DrawCrosshair();
	void NoScopeCrosshair();
	void DrawRecoilCrosshair();
	void SpreadCrosshair();
	void AutowallCrosshair();
};


static const char *Ranks[] =
{
	"Unranked",
	"Silver I",
	"Silver II",
	"Silver III",
	"Silver IV",
	"Silver Elite",
	"Silver Elite Master",

	"pWhite Nova I",
	"pWhite Nova II",
	"pWhite Nova III",
	"pWhite Nova Master",
	"Master Guardian I",
	"Master Guardian II",

	"Master Guardian Elite",
	"Distinguished Master Guardian",
	"Legendary Eagle",
	"Legendary Eagle Master",
	"Supreme Master First Class",
	"The Global Elite"
};
