#include "Menu.h"
#include "Controls.h"
#include "Hooks.h" 
#include "Interfaces.h"
#include "CRC32.h"
#include <fstream>
#include "XorStr.hpp"
#define WINDOW_WIDTH 690
#define WINDOW_HEIGHT 507
PhantomWindow Options::Menu;
struct Config_t {
	int id;
	std::string name;
};
std::vector<Config_t> configs;
typedef void(__cdecl* MsgFn)(const char* msg, va_list);
void MsgX(const char* msg, ...)
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
void save_callback()
{
	Interfaces::Engine->ExecuteClientCmd("clear");
	int should_save = Options::Menu.ColorsTab.ConfigListBox.GetIndex();
	std::string config_directory = "miroawr\\cfg\\";
	config_directory += configs[should_save].name; config_directory += ".xml";
	GUI.SaveWindowState(&Options::Menu, XorStr(config_directory.c_str()));
	Interfaces::CVar->ConsoleColorPrintf(Color(30, 255, 30, 255), "AutismH00k RECODE ");
	std::string uremam = "Saved configuration.     \n";
	MsgX(uremam.c_str());
}
void load_callback()
{
	Interfaces::Engine->ExecuteClientCmd("clear");
	int should_load = Options::Menu.ColorsTab.ConfigListBox.GetIndex();
	std::string config_directory = "miroawr\\cfg\\";
	config_directory += configs[should_load].name; config_directory += ".xml";
	GUI.LoadWindowState(&Options::Menu, XorStr(config_directory.c_str()));
	Interfaces::CVar->ConsoleColorPrintf(Color(30, 255, 30, 255), "AutismH00k RECODE ");
	std::string uremam = "Loaded configuration.     \n";
	MsgX(uremam.c_str());
}
void list_configs() {
	configs.clear();
	Options::Menu.ColorsTab.ConfigListBox.ClearItems();
	std::ifstream file_in;
	file_in.open("miroawr\\cfg\\miroawr_configs.txt");
	if (file_in.fail()) {
		std::ofstream("miroawr\\cfg\\miroawr_configs.txt");
		file_in.open("miroawr\\cfg\\miroawr_configs.txt");
	}
	int line_count;
	while (!file_in.eof()) {
		Config_t config;
		file_in >> config.name;
		config.id = line_count;
		configs.push_back(config);
		line_count++;
		Options::Menu.ColorsTab.ConfigListBox.AddItem(config.name);
	}
	file_in.close();
	if (configs.size() > 7) Options::Menu.ColorsTab.ConfigListBox.AddItem(" ");
}
void add_config() {
	std::fstream file;
	file.open("miroawr\\cfg\\miroawr_configs.txt", std::fstream::app);
	if (file.fail()) {
		std::fstream("miroawr\\cfg\\miroawr_configs.txt");
		file.open("miroawr\\cfg\\miroawr_configs.txt", std::fstream::app);
	}
	file << std::endl << Options::Menu.ColorsTab.NewConfigName.getText();
	file.close();
	list_configs();
	int should_add = Options::Menu.ColorsTab.ConfigListBox.GetIndex();
	std::string config_directory = "miroawr\\cfg\\";
	config_directory += Options::Menu.ColorsTab.NewConfigName.getText(); config_directory += ".xml";
	GUI.SaveWindowState(&Options::Menu, XorStr(config_directory.c_str()));
	Options::Menu.ColorsTab.NewConfigName.SetText("");
}
void remove_config() {
	int should_remove = Options::Menu.ColorsTab.ConfigListBox.GetIndex();
	std::string config_directory = "miroawr\\cfg\\";
	config_directory += configs[should_remove].name; config_directory += ".xml";
	std::remove(config_directory.c_str());
	std::ofstream ofs("miroawr\\cfg\\miroawr_configs.txt", std::ios::out | std::ios::trunc);
	ofs.close();
	std::fstream file;
	file.open("miroawr\\cfg\\miroawr_configs.txt", std::fstream::app);
	if (file.fail()) {
		std::fstream("miroawr\\cfg\\miroawr_configs.txt");
		file.open("miroawr\\cfg\\miroawr_configs.txt", std::fstream::app);
	}
	for (int i = 0; i < configs.size(); i++) {
		if (i == should_remove) continue;
		Config_t config = configs[i];
		file << std::endl << config.name;
	}
	file.close();
	list_configs();
}

void KnifeApplyCallbk()
{
	static ConVar* Meme = Interfaces::CVar->FindVar("cl_fullupdate");
	Meme->nFlags &= ~FCVAR_CHEAT;
	Interfaces::Engine->ClientCmd_Unrestricted("cl_fullupdate");
}
void UnLoadCallbk()
{
	DoUnload = true;
}
void PhantomWindow::Setup()
{
	SetPosition(350, 50);
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	SetTitle("");
	RegisterTab(&RageBotTab);
	RegisterTab(&LegitBotTab);
	RegisterTab(&VisualsTab);
	RegisterTab(&MiscTab);
	RegisterTab(&ColorsTab);
	RECT Client = GetClientArea();
	Client.bottom -= 29;
	RageBotTab.Setup();
	LegitBotTab.Setup();
	VisualsTab.Setup();
	MiscTab.Setup();
	ColorsTab.Setup();
#pragma region Bottom Buttons
#pragma endregion
}
void CRageBotTab::Setup()
{
	SetTitle("A");
#pragma region Aimbot
	AimbotGroup.SetPosition(10, 25);
	AimbotGroup.SetText("Aimbot");
	AimbotGroup.SetSize(650, 413);
	AimbotGroup.AddTab(CGroupTab("Main", 1));
	AimbotGroup.AddTab(CGroupTab("Accuracy", 2));
	RegisterControl(&AimbotGroup);
	AimbotEnable.SetFileId("aim_enable");
	AimbotGroup.PlaceLabledControl(1, "Enable", this, &AimbotEnable);
	AimbotAutoFire.SetFileId("aim_autofire");
	AimbotGroup.PlaceLabledControl(1, "AutoShoot", this, &AimbotAutoFire);
	AimbotSilentAim.SetFileId("aim_silent");
	AimbotGroup.PlaceLabledControl(1, "Silent Aim", this, &AimbotSilentAim);
	AccuracyAutoWall.SetFileId("acc_awall");
	AimbotGroup.PlaceLabledControl(1, "Auto Wall", this, &AccuracyAutoWall);
	AccuracyAutoScope.SetFileId("acc_scope");
	AimbotGroup.PlaceLabledControl(1, "Auto Scope", this, &AccuracyAutoScope);
	lag_pred.SetFileId("lag_pred");
	AimbotGroup.PlaceLabledControl(1, "Position Adjustment", this, &lag_pred);


	TargetSelection.SetFileId("tgt_selection");
	TargetSelection.AddItem("Closest To Crosshair");
	TargetSelection.AddItem("Distance");
	TargetSelection.AddItem("Lowest Health");
	TargetSelection.AddItem("Threat");
	TargetSelection.AddItem("Next Shot");
	AimbotGroup.PlaceLabledControl(1, "Selection", this, &TargetSelection);
	TargetHitbox.SetFileId("tgt_hitbox");
	TargetHitbox.AddItem("Head");
	TargetHitbox.AddItem("Neck");
	TargetHitbox.AddItem("Chest");
	TargetHitbox.AddItem("Pelvis");
	TargetHitbox.AddItem("Shin");
	AimbotGroup.PlaceLabledControl(1, "Hitbox Priority", this, &TargetHitbox);
	TargetHitscan.SetFileId("tgt_hitscan");
	TargetHitscan.AddItem("Off");
	TargetHitscan.AddItem("Minimal");
	TargetHitscan.AddItem("Essential");
	TargetHitscan.AddItem("Maximal");
	AimbotGroup.PlaceLabledControl(1, "Hitscan", this, &TargetHitscan);
	preso.SetFileId("acc_zeusisgay");
	preso.AddItem("Default");
	preso.AddItem("Down");
	preso.AddItem("Up");
	preso.AddItem("Zero");
	preso.AddItem("Automatic Spread");
	preso.AddItem("Automatic NoSpread");
	AimbotGroup.PlaceLabledControl(1, "Pitch Adjustment", this, &preso);
	resolver.SetFileId("acc_aaa");
	resolver.AddItem("Off");
	resolver.AddItem("Default");
	resolver.AddItem("NoSpread");
	AimbotGroup.PlaceLabledControl(1, "Resolver", this, &resolver);
	AimbotFov.SetFileId("aim_fov");
	AimbotFov.SetBoundaries(0.f, 180.f);
	AimbotFov.extension = XorStr("°");
	AimbotFov.SetValue(0.f);
	AimbotGroup.PlaceLabledControl(1, "Maximum Fov", this, &AimbotFov);
	AccuracyHitchance.SetFileId("acc_AccuracyHitchance");
	AccuracyHitchance.SetBoundaries(0, 100);
	AccuracyHitchance.extension = XorStr("%%");
	AccuracyHitchance.SetValue(20);
	AimbotGroup.PlaceLabledControl(2, "Minimum Hitchance", this, &AccuracyHitchance);
	AccuracyMinimumDamage.SetFileId("acc_AccuracyMinimumDamage");
	AccuracyMinimumDamage.SetBoundaries(0, 100);
	AccuracyMinimumDamage.SetValue(20);
	AccuracyMinimumDamage.extension = XorStr("HP");
	AimbotGroup.PlaceLabledControl(2, "Minimum Damage", this, &AccuracyMinimumDamage);
	BaimIfUnderXHealth.SetFileId("acc_BaimIfUnderXHealth");
	BaimIfUnderXHealth.SetBoundaries(0, 100);
	BaimIfUnderXHealth.extension = XorStr("HP");
	BaimIfUnderXHealth.SetValue(0);
	AimbotGroup.PlaceLabledControl(2, "Body Aim If HP Lower Than", this, &BaimIfUnderXHealth);
	baim.SetFileId("tgt_bruteforce2");
	baim.SetBoundaries(0.f, 15.f);
	baim.extension = XorStr(" Shots");
	baim.SetValue(4.f);
	AimbotGroup.PlaceLabledControl(2, "Body Aim After Shots", this, &baim);
	OverrideKey.SetFileId("acc_overrridekey");
	AimbotGroup.PlaceLabledControl(2, "Override Key", this, &OverrideKey);
	bigbaim.SetFileId("acc_bigbaim");
	AimbotGroup.PlaceLabledControl(2, "Force Body Aim Key", this, &bigbaim);
	QuickStop.SetFileId("schnellerstop");
	AimbotGroup.PlaceLabledControl(2, "Quickstop", this, &QuickStop);
	EnginePrediction.SetFileId("acc_engine_prediciton");
	AimbotGroup.PlaceLabledControl(2, "Engine Prediction", this, &EnginePrediction);
	pointscaleyes.SetFileId("godiefag");
	AimbotGroup.PlaceLabledControl(2, "Point scale", this, &pointscaleyes);
	pointscaleval.SetFileId("stfudognigger");
	pointscaleval.SetBoundaries(0, 100);
	pointscaleval.SetValue(20);
	pointscaleval.extension = XorStr("%%");
	AimbotGroup.PlaceLabledControl(2, "", this, &pointscaleval);
	Multienable.SetFileId("improper_yes");
	AimbotGroup.PlaceLabledControl(2, "Multi point", this, &Multienable);
	Multival2.SetFileId("yesdeseses");
	Multival2.SetBoundaries(0, 100);
	Multival2.SetValue(20);
	Multival2.extension = XorStr("%%");
	AimbotGroup.PlaceLabledControl(2, "Head", this, &Multival2);
	Multival.SetFileId("yesdeseses2");
	Multival.SetBoundaries(0, 100);
	Multival.SetValue(20);
	Multival.extension = XorStr("%%");
	AimbotGroup.PlaceLabledControl(2, "Body", this, &Multival);
	extrapolation.SetFileId("richnblind");
	AimbotGroup.PlaceLabledControl(2, "Extrapolation", this, &extrapolation);
	/*----------------------------------------------------------------------
	--------------------------------------------------*/
#pragma endregion  AntiAim controls get setup in here
}
void CLegitBotTab::Setup()
{	/*
	_      ______ _____ _____ _______
	| |    |  ____/ ____|_   _|__   __|
	| |    | |__ | |  __  | |    | |
	| |    |  __|| | |_ | | |    | |
	| |____| |___| |__| |_| |_   | |
	|______|______\_____|_____|  |_|

	*/
	SetTitle("B");
	AimbotGroup.SetText("LegitBot");
	AimbotGroup.SetPosition(15, 25);
	AimbotGroup.SetSize(640, 222);
	AimbotGroup.AddTab(CGroupTab("Aimbot", 1));
	AimbotGroup.AddTab(CGroupTab("Triggerbot", 2));
	AimbotGroup.AddTab(CGroupTab("Filters", 3));
	RegisterControl(&AimbotGroup);
	RegisterControl(&Active);
	Active.SetFileId("active");
	AimbotGroup.PlaceLabledControl(1, "Activate Legitbot", this, &Active);
	AimbotEnable.SetFileId("l_aimbot");
	AimbotGroup.PlaceLabledControl(1, "Aimbot Enable", this, &AimbotEnable);
	aimbotfiremode.SetFileId("l_autoaimbot");
	aimbotfiremode.AddItem("On Shot");
	aimbotfiremode.AddItem("Automatic");
	AimbotGroup.PlaceLabledControl(1, "Fire Mode", this, &aimbotfiremode);
	AimbotKeyBind.SetFileId("l_aimkey");
	AimbotGroup.PlaceLabledControl(1, "KeyBind", this, &AimbotKeyBind);
	BackTrack.SetFileId("l_backtrack");
	AimbotGroup.PlaceLabledControl(1, "Backtrack", this, &BackTrack);
	AimbotSmokeCheck.SetFileId("l_smokeaimbot");
	AimbotGroup.PlaceLabledControl(1, "Smoke Check", this, &AimbotSmokeCheck);

	legitresolver.SetFileId("l_B1GresolverTappingSkeet");
	legitresolver.AddItem("Off");
	legitresolver.AddItem("Default");
	legitresolver.AddItem("Alternative");
	AimbotGroup.PlaceLabledControl(1, "Resolver", this, &legitresolver);

	//---- Trigger ---//
	TriggerEnable.SetFileId("t_triggerbotenable");
	AimbotGroup.PlaceLabledControl(2, "Activate Trigger", this, &TriggerEnable);
	triggertype.SetFileId("t_triggerbottyp");
	triggertype.AddItem("None");
	triggertype.AddItem("Automatic");
	AimbotGroup.PlaceLabledControl(2, "Trigger Mode", this, &triggertype);
	TriggerHitChanceAmmount.SetFileId("l_trigHC");
	TriggerHitChanceAmmount.SetBoundaries(0, 100);
	TriggerHitChanceAmmount.extension = XorStr("%%");
	TriggerHitChanceAmmount.SetValue(0);
	AimbotGroup.PlaceLabledControl(2, "Hitchance", this, &TriggerHitChanceAmmount);
	TriggerSmokeCheck.SetFileId("l_trigsmoke");
	AimbotGroup.PlaceLabledControl(2, "Smoke Check", this, &TriggerSmokeCheck);
	TriggerRecoil.SetFileId("l_trigRCS");
	TriggerRecoil.SetBoundaries(0.f, 100.f);
	TriggerRecoil.extension = XorStr("%%");
	TriggerRecoil.SetValue(5.00f);
	AimbotGroup.PlaceLabledControl(2, "Recoil", this, &TriggerRecoil);
	TriggerKeyBind.SetFileId("l_trigkey");
	AimbotGroup.PlaceLabledControl(2, "KeyBind", this, &TriggerKeyBind);
	// ---- Hitboxes ---- //
	TriggerHead.SetFileId("l_trighead");
	AimbotGroup.PlaceLabledControl(3, "Head", this, &TriggerHead);
	TriggerChest.SetFileId("l_trigchest");
	AimbotGroup.PlaceLabledControl(3, "Chest", this, &TriggerChest);
	TriggerStomach.SetFileId("l_trigstomach");
	AimbotGroup.PlaceLabledControl(3, "Stomach", this, &TriggerStomach);
	TriggerArms.SetFileId("l_trigarms");
	AimbotGroup.PlaceLabledControl(3, "Arms", this, &TriggerArms);
	TriggerLegs.SetFileId("l_triglegs");
	AimbotGroup.PlaceLabledControl(3, "Legs", this, &TriggerLegs);

	//--------------------------legitaa--------------------------((
	aaenable.SetFileId("AA_anable");
	AimbotGroup.PlaceLabledControl(4, "Legit AA Enable", this, &aaenable);

	aatyp.SetFileId("AA_aatyp");
	aatyp.AddItem("Static");
	aatyp.AddItem("Lowerbody");
	aatyp.AddItem("Freestanding");
	AimbotGroup.PlaceLabledControl(4, "Anti-Aim", this, &aatyp);

	aatyp2.SetFileId("AA_aatyp2");
	aatyp2.AddItem("Default");
	aatyp2.AddItem("Jitter");
	aatyp2.AddItem("Shuffle");
	aatyp2.AddItem("Spin");
	AimbotGroup.PlaceLabledControl(4, "Anti-Aim Type", this, &aatyp2);

	aatyp3.SetFileId("AA_aatyp3");
	aatyp3.SetBoundaries(0, 90);
	aatyp3.SetValue(20);
	AimbotGroup.PlaceLabledControl(4, "Anti-Aim Value", this, &aatyp3);

	aafl.SetFileId("AA_aaFL");
	aafl.SetBoundaries(1, 4);
	aafl.SetValue(1);
	AimbotGroup.PlaceLabledControl(4, "Legit FakeLag", this, &aafl);
	//----------------------solid kys---------------------//
	weapongroup.SetText("Main");
	weapongroup.SetPosition(15, 256);
	weapongroup.SetSize(640, 183);
	weapongroup.AddTab(CGroupTab("Rifle", 1));
	weapongroup.AddTab(CGroupTab("Pistol", 2));
	weapongroup.AddTab(CGroupTab("Sniper", 3));
	weapongroup.AddTab(CGroupTab("SMG", 4));
	weapongroup.AddTab(CGroupTab("Heavy", 5));
	RegisterControl(&weapongroup);
	RegisterControl(&Active);
	WeaponMainHitbox.SetFileId("l_rhitbox");
	WeaponMainHitbox.AddItem("Head");
	WeaponMainHitbox.AddItem("Neck");
	WeaponMainHitbox.AddItem("Chest");
	WeaponMainHitbox.AddItem("Stomach");
	WeaponMainHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(1, "Hitbox", this, &WeaponMainHitbox);
	WeaponMainSpeed.SetFileId("l_rspeed");
	WeaponMainSpeed.SetBoundaries(0, 75);
	WeaponMainSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(1, "Speed", this, &WeaponMainSpeed);
	WeaponMainRecoil.SetFileId("l_rRecoil");
	WeaponMainRecoil.SetBoundaries(0, 200);
	WeaponMainRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(1, "Recoil", this, &WeaponMainRecoil);
	WeaponMainFoV.SetFileId("l_fov");
	WeaponMainFoV.SetBoundaries(0, 45);
	WeaponMainFoV.SetValue(10);
	weapongroup.PlaceLabledControl(1, "Field Of View", this, &WeaponMainFoV);
	// --- Pistols --- //
	WeaponPistHitbox.SetFileId("l_phitbox");
	WeaponPistHitbox.AddItem("Head");
	WeaponPistHitbox.AddItem("Neck");
	WeaponPistHitbox.AddItem("Chest");
	WeaponPistHitbox.AddItem("Stomach");
	WeaponPistHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(2, "Hitbox", this, &WeaponPistHitbox);
	WeaponPistSpeed.SetFileId("l_pspeed");
	WeaponPistSpeed.SetBoundaries(0, 75);
	WeaponPistSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(2, "Speed", this, &WeaponPistSpeed);
	WeaponPistRecoil.SetFileId("l_pRecoil");
	WeaponPistRecoil.SetBoundaries(0, 200);
	WeaponPistRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(2, "Recoil", this, &WeaponPistRecoil);
	WeaponPistFoV.SetFileId("l_pfov");
	WeaponPistFoV.SetBoundaries(0, 45);
	WeaponPistFoV.SetValue(10);
	weapongroup.PlaceLabledControl(2, "Field Of View", this, &WeaponPistFoV);
	// --- Sniper --- //
	WeaponSnipHitbox.SetFileId("l_shitbox");
	WeaponSnipHitbox.AddItem("Head");
	WeaponSnipHitbox.AddItem("Neck");
	WeaponSnipHitbox.AddItem("Chest");
	WeaponSnipHitbox.AddItem("Stomach");
	WeaponSnipHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(3, "Hitbox", this, &WeaponSnipHitbox);
	WeaponSnipSpeed.SetFileId("l_sspeed");
	WeaponSnipSpeed.SetBoundaries(0, 75);
	WeaponSnipSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(3, "Speed", this, &WeaponSnipSpeed);
	WeaponSnipRecoil.SetFileId("l_sRecoil");
	WeaponSnipRecoil.SetBoundaries(0, 200);
	WeaponSnipRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(3, "Recoil", this, &WeaponSnipRecoil);
	WeaponSnipFoV.SetFileId("l_sfov");
	WeaponSnipFoV.SetBoundaries(0, 45);
	WeaponSnipFoV.SetValue(10);
	weapongroup.PlaceLabledControl(3, "Field Of View", this, &WeaponSnipFoV);
	// --- SMG --- //
	WeaponMpHitbox.SetFileId("l_sniphitbox");
	WeaponMpHitbox.AddItem("Head");
	WeaponMpHitbox.AddItem("Neck");
	WeaponMpHitbox.AddItem("Chest");
	WeaponMpHitbox.AddItem("Stomach");
	WeaponMpHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(4, "Hitbox", this, &WeaponMpHitbox);
	WeaponMpSpeed.SetFileId("l_sspeed");
	WeaponMpSpeed.SetBoundaries(0, 75);
	WeaponMpSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(4, "Speed", this, &WeaponMpSpeed);
	WeaponMpRecoil.SetFileId("l_sRecoil");
	WeaponMpRecoil.SetBoundaries(0, 200);
	WeaponMpRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(4, "Recoil", this, &WeaponMpRecoil);
	WeaponMpFoV.SetFileId("l_sfov");
	WeaponMpFoV.SetBoundaries(0, 45);
	WeaponMpFoV.SetValue(10);
	weapongroup.PlaceLabledControl(4, "Field Of View", this, &WeaponMpFoV);
	// --- MachineGun --- //
	WeaponMGHitbox.SetFileId("l_mghitbox");
	WeaponMGHitbox.AddItem("Head");
	WeaponMGHitbox.AddItem("Neck");
	WeaponMGHitbox.AddItem("Chest");
	WeaponMGHitbox.AddItem("Stomach");
	WeaponMGHitbox.AddItem("Nearest");
	weapongroup.PlaceLabledControl(5, "Hitbox", this, &WeaponMGHitbox);
	WeaponMGSpeed.SetFileId("l_mgspeed");
	WeaponMGSpeed.SetBoundaries(0, 75);
	WeaponMGSpeed.SetValue(5);
	weapongroup.PlaceLabledControl(5, "Speed", this, &WeaponMGSpeed);
	WeaponMGRecoil.SetFileId("l_mgRecoil");
	WeaponMGRecoil.SetBoundaries(0, 200);
	WeaponMGRecoil.SetValue(165);
	weapongroup.PlaceLabledControl(5, "Recoil", this, &WeaponMGRecoil);
	WeaponMGFoV.SetFileId("l_mgfov");
	WeaponMGFoV.SetBoundaries(0, 45);
	WeaponMGFoV.SetValue(10);
	weapongroup.PlaceLabledControl(5, "Field Of View", this, &WeaponMGFoV);
}
void CVisualTab::Setup()
{

	SetTitle("C");
#pragma region Options
	OptionsGroup.SetText("Options");
	OptionsGroup.SetPosition(15, 30);
	OptionsGroup.SetSize(200, 390);
	OptionsGroup.AddTab(CGroupTab("Main", 1));
	OptionsGroup.AddTab(CGroupTab("Filters", 2));
	RegisterControl(&OptionsGroup);
	RegisterControl(&Active);
	Active.SetFileId("active");
	OptionsGroup.PlaceLabledControl(1, "Activate Visuals", this, &Active);
	OptionsBox.SetFileId("opt_box");
	OptionsBox.AddItem("Off");
	OptionsBox.AddItem("Default");
	OptionsBox.AddItem("Genuine");
	OptionsGroup.PlaceLabledControl(1, "Box", this, &OptionsBox);
	OptionsName.SetFileId("opt_name");
	OptionsGroup.PlaceLabledControl(1, "Name", this, &OptionsName);
	OptionsHealth.SetFileId("opt_hp");
	OptionsHealth.AddItem("Off");
	OptionsHealth.AddItem("Default");
	OptionsHealth.AddItem("Battery");
	OptionsGroup.PlaceLabledControl(1, "Health", this, &OptionsHealth);
	OptionsInfo.SetFileId("opt_info");
	OptionsGroup.PlaceLabledControl(1, "Info", this, &OptionsInfo);
	OtherHitmarker.SetFileId("otr_hitmarker");
	OptionsGroup.PlaceLabledControl(1, "Hitmarker", this, &OtherHitmarker);
	OptionsSkeleton.SetFileId("opt_bone");
	OptionsGroup.PlaceLabledControl(1, "Skeleton", this, &OptionsSkeleton);
	OptionsGlow.SetFileId("opt_glow");
	OptionsGroup.PlaceLabledControl(1, "Glow Enemy", this, &OptionsGlow);
	GlowZ.SetFileId("opt_glowz");
	GlowZ.SetValue(99.f);
	GlowZ.SetBoundaries(0.f, 100.f);
	GlowZ.extension = XorStr("%%");
	OptionsGroup.PlaceLabledControl(1, "Opacity percentage", this, &GlowZ);
	OptionsGlow_lcl.SetFileId("opt_glow_lcl");
	OptionsGroup.PlaceLabledControl(1, "Glow Local", this, &OptionsGlow_lcl);
	Glowz_lcl.SetFileId("opt_glowz_lcl");
	Glowz_lcl.SetBoundaries(0.f, 100.f);
	Glowz_lcl.extension = XorStr("%%");
	OptionsGroup.PlaceLabledControl(1, "Opacity percentage", this, &Glowz_lcl);
	Weapons.SetFileId("kysquecest");
	Weapons.AddItem("Off");
	Weapons.AddItem("Text");
	Weapons.AddItem("Icon");
	OptionsGroup.PlaceLabledControl(1, "Weapons", this, &Weapons);
	Ammo.SetFileId("urmomsucksass");
	OptionsGroup.PlaceLabledControl(1, "Ammo", this, &Ammo);
	OffscreenESP.SetFileId("otr_offscreenESP");
	OptionsGroup.PlaceLabledControl(1, "Offscreen ESP", this, &OffscreenESP);
	FiltersAll.SetFileId("ftr_all");
	OptionsGroup.PlaceLabledControl(2, "All", this, &FiltersAll);
	FiltersPlayers.SetFileId("ftr_players");
	FiltersPlayers.SetState(true);
	OptionsGroup.PlaceLabledControl(2, "Players", this, &FiltersPlayers);
	FiltersEnemiesOnly.SetFileId("ftr_enemyonly");
	OptionsGroup.PlaceLabledControl(2, "Enemies only", this, &FiltersEnemiesOnly);
	ChamsEnemyVis.SetFileId("chams_en_vis");
	OptionsGroup.PlaceLabledControl(2, "Enemies (Visible)", this, &ChamsEnemyVis);
	ChamsEnemyNoVis.SetFileId("chams_en_novis");
	OptionsGroup.PlaceLabledControl(2, "Enemies (Invisible)", this, &ChamsEnemyNoVis);
	ChamsLocal.SetFileId("chams_local");
	OptionsGroup.PlaceLabledControl(2, "Local Chams", this, &ChamsLocal);
	FiltersWeapons.SetFileId("ftr_weaps");
	OptionsGroup.PlaceLabledControl(2, "Weapons", this, &FiltersWeapons);
	FiltersNades.SetFileId("ftr_nades");
	OptionsGroup.PlaceLabledControl(2, "Nades", this, &FiltersNades);
	FiltersC4.SetFileId("ftr_c4");
	OptionsGroup.PlaceLabledControl(2, "C4", this, &FiltersC4);
	PulsatingChams.SetFileId("PulsatingChams");
	OptionsGroup.PlaceLabledControl(2, "Pulsating Chams", this, &PulsatingChams);

#pragma endregion Setting up the Options controls
	ChamsGroup.SetText("Options 2");
	ChamsGroup.SetPosition(225, 30);
	ChamsGroup.SetSize(200, 300);
	RegisterControl(&ChamsGroup);

	BuyBot.SetFileId("buy");
	BuyBot.AddItem("None"); // 0
	BuyBot.AddItem("Auto + Deagle"); // 1
	BuyBot.AddItem("Auto + Duals"); // 2
	BuyBot.AddItem("Scout + Deagle"); // 3
	BuyBot.AddItem("Scout + Dualies");
	ChamsGroup.PlaceLabledControl(1, "BuyBot", this, &BuyBot);

	aa_helper.SetFileId("aahelper");
	ChamsGroup.PlaceLabledControl(0, "Fake Angle Ghost", this, &aa_helper);

	SniperCrosshair.SetFileId("misc_snip_cross");
	ChamsGroup.PlaceLabledControl(0, "Sniper Crosshair", this, &SniperCrosshair);

	left2.SetFileId("aa_l23");
	ChamsGroup.PlaceLabledControl(0, "Left Hand On Knife", this, &left2);

	OtherThirdperson.SetFileId("aa_1thirpbind");
	ChamsGroup.PlaceLabledControl(0, "Thirdperson", this, &OtherThirdperson);
	ThirdPersonKeyBind.SetFileId("aa_thirpbind");
	ChamsGroup.PlaceLabledControl(0, "", this, &ThirdPersonKeyBind);

	ThirdPersonVal.SetFileId("thirdpersonrange");
	ThirdPersonVal.SetBoundaries(10, 300);
	ThirdPersonVal.SetValue(200);
	ChamsGroup.PlaceLabledControl(0, "Range", this, &ThirdPersonVal);


	aa_helper2.SetFileId("aahelper2");
	ChamsGroup.PlaceLabledControl(0, "Anti Aim Arrows", this, &aa_helper2);

	logs.SetFileId("otr_skeetpaste");
	ChamsGroup.PlaceLabledControl(0, "Event Log", this, &logs);
	DisablePostProcess.SetFileId("DisablePostProces");
	ChamsGroup.PlaceLabledControl(0, "Disable Post Processing", this, &DisablePostProcess);
	GrenadePrediction.SetFileId("GrenadePrediction");
	ChamsGroup.PlaceLabledControl(0, "Grenade Prediction", this, &GrenadePrediction);
	LBYIndicator.SetFileId("LBYIndicator");
	ChamsGroup.PlaceLabledControl(0, "LBYIndicator", this, &LBYIndicator);


#pragma region Other
	OtherGroup.SetText("Other");
	OtherGroup.SetPosition(436, 30);
	OtherGroup.SetSize(210, 390);
	RegisterControl(&OtherGroup);
	AutowallCrosshair.SetFileId("otr_autowallcross");
	OtherGroup.PlaceLabledControl(0, "Autowall crosshair", this, &AutowallCrosshair);
	SpreadCross.SetFileId("otr_spreadcross");
	OtherGroup.PlaceLabledControl(0, "Spread crosshair", this, &SpreadCross);
	SpreadCrosshair.SetFileId(XorStr("v_spreadcrosshair"));
	SpreadCrosshair.AddItem("Standard");
	SpreadCrosshair.AddItem("Colour");
	SpreadCrosshair.AddItem("Rainbow");
	SpreadCrosshair.AddItem("Rainbow Rotate");
	OtherGroup.PlaceLabledControl(0, XorStr("Spread crosshair"), this, &SpreadCrosshair);
	SpreadCrossSize.SetFileId("otr_spreadcross_size");
	SpreadCrossSize.SetBoundaries(1.f, 100.f); //we should take smth like 650 as max so i guess *6.5?
	SpreadCrossSize.extension = XorStr("%%");
	SpreadCrossSize.SetValue(45.f);
	OtherGroup.PlaceLabledControl(0, "Size", this, &SpreadCrossSize);
	OtherNoVisualRecoil.SetFileId("otr_visrecoil");
	OtherGroup.PlaceLabledControl(0, "No visual recoil", this, &OtherNoVisualRecoil);
	OtherNoFlash.SetFileId("otr_noflash");
	OtherGroup.PlaceLabledControl(0, "Remove flash effect", this, &OtherNoFlash);
	flashAlpha.SetFileId("otr_stolen_from_punknown_muahahaha");
	flashAlpha.SetBoundaries(0, 100);
	flashAlpha.extension = XorStr("%%");
	flashAlpha.SetValue(10);
	OtherGroup.PlaceLabledControl(0, "Flash Alpha", this, &flashAlpha);
	OtherNoSmoke.SetFileId("otr_nosmoke");
	OtherGroup.PlaceLabledControl(0, "Wireframe smoke", this, &OtherNoSmoke);
	OtherNoScope.SetFileId("otr_noscope");
	OtherGroup.PlaceLabledControl(0, "Remove scope", this, &OtherNoScope);
	RemoveZoom.SetFileId("otr_remv_zoom");
	OtherGroup.PlaceLabledControl(0, "Remove zoom", this, &RemoveZoom);
	OtherViewmodelFOV.SetFileId("otr_viewfov");
	OtherViewmodelFOV.SetBoundaries(30.f, 120.f);
	OtherViewmodelFOV.SetValue(90.f);
	OtherGroup.PlaceLabledControl(0, "Viewmodel fov", this, &OtherViewmodelFOV);
	OtherFOV.SetFileId("otr_fov");
	OtherFOV.SetBoundaries(0.f, 50.f);
	OtherFOV.SetValue(0.f);
	OtherGroup.PlaceLabledControl(0, "Override fov", this, &OtherFOV);
	BulletTracers.SetFileId("otr_btracers");
	OtherGroup.PlaceLabledControl(0, "Bullet tracers", this, &BulletTracers);
	beamtime.SetFileId("otr_beamtime");
	beamtime.SetBoundaries(0.1, 10);
	beamtime.extension = XorStr("s");
	beamtime.SetValue(2);
	OtherGroup.PlaceLabledControl(0, "Tracer Time", this, &beamtime);
	manualaa.SetFileId("manualaa");
	OtherGroup.PlaceLabledControl(0, "Manual Indicator", this, &manualaa);
	cheatinfo.SetFileId("cheatinfox");
	OtherGroup.PlaceLabledControl(0, "Localplayer Info", this, &cheatinfo);

	


#pragma endregion Setting up the Other controls
}
void CMiscTab::Setup()
{
	/*
	__  __ _____  _____  _____
	|  \/  |_   _|/ ____|/ ____|
	| \  / | | | | (___ | |
	| |\/| | | |  \___ \| |
	| |  | |_| |_ ____) | |____
	|_|  |_|_____|_____/ \_____|

	*/
	SetTitle("F");
#pragma region Other
	OtherGroup.SetPosition(10, 30);
	OtherGroup.SetSize(320, 400);
	OtherGroup.SetText("Other");
	RegisterControl(&OtherGroup);

	OtherSafeMode.SetFileId("otr_safemode");
	OtherSafeMode.AddItem("MatchMaking");
	OtherSafeMode.AddItem("Anti VAC Kick");
	OtherSafeMode.AddItem("FaceIt Server");
	OtherSafeMode.AddItem("Unrestricted (!)");
	OtherGroup.PlaceLabledControl(0, "Safety Mode", this, &OtherSafeMode);

	OtherAutoJump.SetFileId("otr_autojump");
	OtherGroup.PlaceLabledControl(0, "BunnyHop", this, &OtherAutoJump);

	OtherAutoStrafe.SetFileId("otr_strafe");
	OtherGroup.PlaceLabledControl(0, "Air Strafe", this, &OtherAutoStrafe);

	CircleStrafe.SetFileId("otr_circle_strafe_yes");
	OtherGroup.PlaceLabledControl(0, "Circle Strafe", this, &CircleStrafe);
	CircleStrafeKey.SetFileId("otr_circle_key_suck");
	OtherGroup.PlaceLabledControl(0, "", this, &CircleStrafeKey);
	


	experimental.SetFileId("experimental_stuff");
	OtherGroup.PlaceLabledControl(0, "Enable Experimental Features", this, &experimental);



	Radar.SetFileId("Radar");
	OtherGroup.PlaceLabledControl(0, "Draw Radar", this, &Radar);

	RadarX.SetFileId("misc_radar_xax1");
	RadarX.SetBoundaries(0, 3840);
	RadarX.SetValue(0);
	OtherGroup.PlaceLabledControl(0, "X-Axis", this, &RadarX);

	RadarY.SetFileId("misc_radar_yax2");
	RadarY.SetBoundaries(0, 2160);
	RadarY.SetValue(0);
	OtherGroup.PlaceLabledControl(0, "Y-Axis", this, &RadarY);



	/*		 ___________		*/
	// _______ //
	//|       |//
	//|-------|//
	//|_     _|//
	//  |   |  //
	//--|---|--//
	//  |   |  //
	//--|---|--//
	//  |   |  //
	//--|---|--//
	//  |   |  //
	//--|---|--//
	//  |   |  //
	//--|---|--//
	//  |   |  //
	//--|---|--//
	//  |   |  //
	//--|___|--//

	/*idk what this is*/

	AntiAimGroup.SetPosition(346, 30);
	AntiAimGroup.SetText("Anti-Aim");
	AntiAimGroup.SetSize(320, 400);
	AntiAimGroup.AddTab(CGroupTab("Main", 1));
	AntiAimGroup.AddTab(CGroupTab("Builder 1", 2));
	AntiAimGroup.AddTab(CGroupTab("Builder 2", 3));
	AntiAimGroup.AddTab(CGroupTab("Misc", 4));
	RegisterControl(&AntiAimGroup);
	AntiAimEnable.SetFileId("aa_enable");
	AntiAimGroup.PlaceLabledControl(1, "Enable", this, &AntiAimEnable);

	AntiAimPitch.SetFileId("aa_x");
	AntiAimPitch.AddItem("Off");
	AntiAimPitch.AddItem("Down");
	AntiAimPitch.AddItem("Up");
	AntiAimPitch.AddItem("Jitter");
	AntiAimPitch.AddItem("Random");
	AntiAimPitch.AddItem("Fake Down (!)");
	AntiAimPitch.AddItem("Fake Up (!)");
	AntiAimPitch.AddItem("Fake Mixed (!)");
	AntiAimGroup.PlaceLabledControl(1, "Pitch", this, &AntiAimPitch);

	AntiAimYaw.SetFileId("aa_y");
	AntiAimYaw.AddItem("Off");
	AntiAimYaw.AddItem("Static");
	AntiAimYaw.AddItem("Jitter");
	AntiAimYaw.AddItem("180 Z");
	AntiAimYaw.AddItem("Manual Static");
	AntiAimYaw.AddItem("Manual Jitter");
	AntiAimYaw.AddItem("Lowerbody");
	AntiAimYaw.AddItem("Random Lowerbody");
	AntiAimYaw.AddItem("Freestanding");
	AntiAimGroup.PlaceLabledControl(1, "Real_Stand", this, &AntiAimYaw);

	AntiAimYawrun.SetFileId("aa_y2");
	AntiAimYawrun.AddItem("Off");
	AntiAimYawrun.AddItem("Static");
	AntiAimYawrun.AddItem("Jitter");
	AntiAimYawrun.AddItem("180 Z");
	AntiAimYawrun.AddItem("Manual Static");
	AntiAimYawrun.AddItem("Manual Jitter");
	AntiAimYawrun.AddItem("Lowerbody");
	AntiAimYawrun.AddItem("Random Lowerbody");
	AntiAimYawrun.AddItem("FreeStanding");
	AntiAimGroup.PlaceLabledControl(1, "Real_Move", this, &AntiAimYawrun);

	AntiAimYaw3.SetFileId("aa_y3");
	AntiAimYaw3.AddItem("Off");
	AntiAimYaw3.AddItem("Static");
	AntiAimYaw3.AddItem("Jitter");
	AntiAimYaw3.AddItem("180 Z");
	AntiAimYaw3.AddItem("Random Lowerbody");
	AntiAimYaw3.AddItem("Manual Static");
	AntiAimYaw3.AddItem("Manual Jitter");
	AntiAimYaw3.AddItem("Lowerbody");
	AntiAimYaw3.AddItem("FreeStanding");
	AntiAimGroup.PlaceLabledControl(1, "Real_Air", this, &AntiAimYaw3);

	FakeYaw.SetFileId("fakeaa");
	FakeYaw.AddItem("Off");
	FakeYaw.AddItem("Opposite");
	FakeYaw.AddItem("Jitter");
	FakeYaw.AddItem("Random Lowerbody");
	FakeYaw.AddItem("Half Spin");
	FakeYaw.AddItem("Random");
	FakeYaw.AddItem("Magic");
	FakeYaw.AddItem("Dynamic");
	AntiAimGroup.PlaceLabledControl(1, "Fake_Stand", this, &FakeYaw);

	FakeYaw2.SetFileId("fakeaa2");
	FakeYaw2.AddItem("Off");
	FakeYaw2.AddItem("Opposite");
	FakeYaw2.AddItem("Jitter");
	FakeYaw2.AddItem("Random Lowerbody");
	FakeYaw2.AddItem("Random");
	FakeYaw2.AddItem("Dynamic");
	AntiAimGroup.PlaceLabledControl(1, "Fake_Move", this, &FakeYaw2);

	FakeYaw3.SetFileId("fakeaaaa"); // aaaaaaaaaaaaaaaaaaa sp00ky
	FakeYaw3.AddItem("Off");
	FakeYaw3.AddItem("Opposite");
	FakeYaw3.AddItem("Jitter");
	FakeYaw3.AddItem("Random Lowerbody");
	AntiAimGroup.PlaceLabledControl(1, "Fake_Air", this, &FakeYaw3);


	//-<------------------------------------->-//

	twitchr.SetFileId("b_twitch1");
	twitchr.SetBoundaries(0, 90);
	twitchr.SetValue(35);
	AntiAimGroup.PlaceLabledControl(2, "Real Standing Jitter Range", this, &twitchr);

	twitchr2.SetFileId("b_twitch2");
	twitchr2.SetBoundaries(0, 90);
	twitchr2.SetValue(60);
	AntiAimGroup.PlaceLabledControl(2, "Real Moving Jitter Range", this, &twitchr2);

	randlbyr.SetFileId("b_randlbyr");
	randlbyr.SetBoundaries(20, 180);
	randlbyr.SetValue(60);
	AntiAimGroup.PlaceLabledControl(2, "Real Random LBY Range", this, &randlbyr);

	lby1.SetFileId("b_lbyr");
	lby1.SetBoundaries(20, 180);
	lby1.SetValue(60);
	AntiAimGroup.PlaceLabledControl(2, "Real Lowerbody Range", this, &lby1);

	twitchf.SetFileId("b_st2");
	twitchf.SetBoundaries(0, 180);
	twitchf.SetValue(60);
	AntiAimGroup.PlaceLabledControl(2, "Fake Jitter Range", this, &twitchf);

	randlbyf.SetFileId("b_randlbyf");
	randlbyf.SetBoundaries(20, 180);
	randlbyf.SetValue(90);
	AntiAimGroup.PlaceLabledControl(2, "Fake Random LBY Range", this, &randlbyf);

	backup_aa.SetFileId("stopfreakyoufag");
	backup_aa.AddItem("Off");
	backup_aa.AddItem("Backwards / Adaptive");
	backup_aa.AddItem("Manual / Inverse");
	backup_aa.AddItem("Lowerbody 180 / Lowerbody Random");
	backup_aa.AddItem("Freestanding / Adaptive");
	AntiAimGroup.PlaceLabledControl(2, "Backup Anti Aim", this, &backup_aa);
	//-----
	antilby.SetFileId("otr_meh");
	antilby.AddItem("Off");
	antilby.AddItem("Default");
	antilby.AddItem("Smart");
	AntiAimGroup.PlaceLabledControl(3, "Anti-LBY", this, &antilby);


	BreakLBYDelta.SetFileId("b_antilby");
	BreakLBYDelta.SetBoundaries(35, 180);
	BreakLBYDelta.SetValue(119);
	AntiAimGroup.PlaceLabledControl(3, "Anti-LBY Range", this, &BreakLBYDelta);

	freestandtype.SetFileId("freestanding_type");
	freestandtype.AddItem("Default");
	freestandtype.AddItem("Jitter");
	freestandtype.AddItem("Shuffle");
	freestandtype.AddItem("Spin");
	AntiAimGroup.PlaceLabledControl(3, "Freestanding Type", this, &freestandtype);

	freerange.SetFileId("freestanding_range");
	freerange.SetBoundaries(0, 60);
	freerange.SetValue(30);
	AntiAimGroup.PlaceLabledControl(3, "Freestanding Range", this, &freerange);



	FakelagStand.SetFileId("fakelag_stand_val");
	FakelagStand.SetBoundaries(0, 14);
	FakelagStand.SetValue(1);
	AntiAimGroup.PlaceLabledControl(3, "Fakelag Standing", this, &FakelagStand);
	FakelagMove.SetFileId("fakelag_move_val");
	FakelagMove.SetBoundaries(0, 14);
	FakelagMove.SetValue(1);
	AntiAimGroup.PlaceLabledControl(3, "Fakelag Moving", this, &FakelagMove);
	Fakelagjump.SetFileId("fakelag_jump_val");
	Fakelagjump.SetBoundaries(0, 14);
	Fakelagjump.SetValue(1);
	AntiAimGroup.PlaceLabledControl(3, "Fakelag in Air", this, &Fakelagjump);


	fl_spike.SetFileId("fakelag_spike");
	fl_spike.AddItem("Off");
	fl_spike.AddItem("While Shooting");
	fl_spike.AddItem("On Land");
	AntiAimGroup.PlaceLabledControl(3, "Fakelag Spike", this, &fl_spike);

	FakelagBreakLC.SetFileId("fakelag_breaklc");
	AntiAimGroup.PlaceLabledControl(3, "Break Lag Compensation", this, &FakelagBreakLC);

	backup_key.SetFileId("learn_how_to_cfg_ffs");
	AntiAimGroup.PlaceLabledControl(4, "Backup AA Enable", this, &backup_key);
	default_key.SetFileId("learn_how_to_cfg_ffs2");
	AntiAimGroup.PlaceLabledControl(4, "Backup AA Disable", this, &default_key);

	manualleft.SetFileId("otr_keybasedleft");
	AntiAimGroup.PlaceLabledControl(4, "Manual Right", this, &manualleft);
	manualright.SetFileId("otr_keybasedright");
	AntiAimGroup.PlaceLabledControl(4, "Manual Left", this, &manualright);
	manualback.SetFileId("otr_keybasedback");
	AntiAimGroup.PlaceLabledControl(4, "Manual Back", this, &manualback);
	manualfront.SetFileId("otr_manualfrontk");
	AntiAimGroup.PlaceLabledControl(4, "Manual Front", this, &manualfront);
	fw.SetFileId("fakewalk_key");
	AntiAimGroup.PlaceLabledControl(4, "FakeWalk Key", this, &fw);
	FakeWalk.SetFileId("fakewalk_key2");
	FakeWalkSpeed.SetFileId("fakewalk_speed");
	FakeWalkSpeed.SetBoundaries(1, 100);
	FakeWalkSpeed.SetValue(50);
	FakeWalkSpeed.extension = XorStr("%%");
	AntiAimGroup.PlaceLabledControl(4, "Fakewalk Slowdown", this, &FakeWalkSpeed);
	AntiAimGroup.PlaceLabledControl(4, "GlitchWalk Key", this, &FakeWalk);
}
void CColorTab::Setup()
{
	SetTitle("D");
#pragma region Visual Colors
	ColorsGroup.SetText("Settings");
	ColorsGroup.SetPosition(16, 30);
	ColorsGroup.AddTab(CGroupTab("ESP", 1));
	ColorsGroup.AddTab(CGroupTab("ESP2", 2));
	ColorsGroup.AddTab(CGroupTab("ESP3", 3));
	ColorsGroup.SetSize(314, 320);
	RegisterControl(&ColorsGroup);
	/*---------------------- COL ----------------------*/
	/*---------------------- COL ----------------------*/
	/*---------------------- COL ----------------------*/
	NameCol.SetFileId(XorStr("player_espname_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Name"), this, &NameCol);
	BoxCol.SetFileId(XorStr("player_espbox_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Box"), this, &BoxCol);
	Skeleton.SetFileId(XorStr("player_skeleton_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Skeleton"), this, &Skeleton);
	GlowEnemy.SetFileId(XorStr("player_glowenemy_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Glow Enemy"), this, &GlowEnemy);
	GlowOtherEnt.SetFileId(XorStr("player_glowother_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Glow World"), this, &GlowOtherEnt);
	GlowLocal.SetFileId(XorStr("player_glowlocal_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Glow Local"), this, &GlowLocal);
	Weapons.SetFileId(XorStr("player_weapons_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Weapons"), this, &Weapons);
	Ammo.SetFileId(XorStr("player_ammo_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Ammo"), this, &Ammo);
	Money.SetFileId(XorStr("player_money_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Money"), this, &Money);
	Offscreen.SetFileId(XorStr("player_offscreen_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Offscreen"), this, &Offscreen);
	ChamsLocal.SetFileId(XorStr("player_chamslocal_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Chams Local"), this, &ChamsLocal);
	ChamsEnemyVis.SetFileId(XorStr("player_chamsEVIS_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Chams Enemy"), this, &ChamsEnemyVis);
	ChamsEnemyNotVis.SetFileId(XorStr("player_chamsENVIS_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Chams Enemy (Behind Wall)"), this, &ChamsEnemyNotVis);
	//ChamsTeamVis.SetFileId(XorStr("player_chamsTVIS_color"));
	//ColorsGroup.PlaceLabledControl(1, XorStr("Chams Team"), this, &ChamsTeamVis);
	//ChamsTeamNotVis.SetFileId(XorStr("player_chamsTNVIS_color"));
	//ColorsGroup.PlaceLabledControl(1, XorStr("Chams Team (Behind Wall)"), this, &ChamsTeamNotVis);


	Bullettracer.SetFileId(XorStr("player_beam_color"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Bullet tracers"), this, &Bullettracer);

	scoped_c.SetFileId(XorStr("scope_c"));
	ColorsGroup.PlaceLabledControl(1, XorStr("Local Scoped Material Colour"), this, &scoped_c);

	spreadcrosscol.SetFileId(XorStr("weapon_spreadcross_col"));
	ColorsGroup.PlaceLabledControl(2, XorStr("Spread Crosshair"), this, &spreadcrosscol);
	//HandChamsCol.SetFileId(XorStr("handchams_col"));
	//ColorsGroup.PlaceLabledControl(2, XorStr("Hand Chams"), this, &HandChamsCol);
	//GunChamsCol.SetFileId(XorStr("gunchams_col"));
	//ColorsGroup.PlaceLabledControl(2, XorStr("Weapon Chams"), this, &GunChamsCol);


	ModulateSkyBox.SetFileId(XorStr("sky_box_color_enable"));
	ColorsGroup.PlaceLabledControl(2, XorStr("Enable Sky Color Changer"), this, &ModulateSkyBox);
	sky_r.SetFileId("sky_r");
	sky_r.SetBoundaries(0.f, 25.f);
	sky_r.SetValue(10.f);
	ColorsGroup.PlaceLabledControl(2, "Sky: Red", this, &sky_r);

	sky_g.SetFileId("sky_g");
	sky_g.SetBoundaries(0.f, 25.f);
	sky_g.SetValue(1.f);
	ColorsGroup.PlaceLabledControl(2, "Sky: Green", this, &sky_g);

	sky_b.SetFileId("sky_b");
	sky_b.SetBoundaries(0.f, 25.f);
	sky_b.SetValue(20.f);
	ColorsGroup.PlaceLabledControl(2, "Sky: Blue", this, &sky_b);

	colmodupdate.SetFileId("otr_night");
	ColorsGroup.PlaceLabledControl(3, "Force update Materials", this, &colmodupdate); //you could've just made this a button lol
	customskies.SetFileId("otr_skycustom");
	customskies.AddItem("Default");
	customskies.AddItem("Night 1");
	customskies.AddItem("Night 2");
	customskies.AddItem("NoSky");
	customskies.AddItem("Vertigo");
	customskies.AddItem("Jungle");
	customskies.AddItem("Baggage");
	customskies.AddItem("Embassy");
	ColorsGroup.PlaceLabledControl(3, "Skybox Changer", this, &customskies);
	colmod.SetFileId("night_amm");
	colmod.SetBoundaries(000.000f, 100.00f);
	colmod.extension = XorStr("%%");
	colmod.SetValue(020.0f);
	ColorsGroup.PlaceLabledControl(3, "Brightness percantage", this, &colmod);
	asusamount.SetFileId("otr_asusprops");
	asusamount.SetBoundaries(1.f, 100.f);
	asusamount.extension = XorStr("%%");
	asusamount.SetValue(95.f);
	ColorsGroup.PlaceLabledControl(3, "Asus percantage", this, &asusamount);
	OtherEntityGlow.SetFileId("otr_world_ent_glow");
	ColorsGroup.PlaceLabledControl(3, "Glow world entities", this, &OtherEntityGlow);

	/*
	ambient.SetFileId(XorStr("ambient_toggle"));
	ColorsGroup.PlaceLabledControl(2, XorStr("Enable Ambient Glow"), this, &ambient);

	AmbientRed.SetFileId("otr_ambientred");
	AmbientRed.SetBoundaries(0.f, 10.f);
	AmbientRed.SetValue(0.f);
	ColorsGroup.PlaceLabledControl(2, "Ambient Red", this, &AmbientRed);

	AmbientGreen.SetFileId("otr_ambientgreen");
	AmbientGreen.SetBoundaries(0.f, 10.f);
	AmbientGreen.SetValue(0.f);
	ColorsGroup.PlaceLabledControl(2, "Ambient Green", this, &AmbientGreen);

	AmbientBlue.SetFileId("otr_ambientblue");
	AmbientBlue.SetBoundaries(0.f, 10.f);
	AmbientBlue.SetValue(0.f);
	ColorsGroup.PlaceLabledControl(2, "Ambient Blue", this, &AmbientBlue);

	MenuBar.SetFileId(XorStr("menu_bar_mode"));
	MenuBar.AddItem("Static");
	MenuBar.AddItem("Animated");
	MenuBar.AddItem("Fade");
	ColorsGroup.PlaceLabledControl(3, XorStr("Menu Bar"), this, &MenuBar);
	outl_r.SetFileId("outlred");
	outl_r.SetBoundaries(0.f, 255.f);
	outl_r.SetValue(55.f);
	ColorsGroup.PlaceLabledControl(3, "Outer Left: Red", this, &outl_r);
	outl_g.SetFileId("outlgreen");
	outl_g.SetBoundaries(0.f, 255.f);
	outl_g.SetValue(15.f);
	ColorsGroup.PlaceLabledControl(3, "Outer Left: Green", this, &outl_g);
	outl_b.SetFileId("outlblue");
	outl_b.SetBoundaries(0.f, 255.f);
	outl_b.SetValue(210.f);
	ColorsGroup.PlaceLabledControl(3, "Outer Left: Blue", this, &outl_b);
	inl_r.SetFileId("inlred");
	inl_r.SetBoundaries(0.f, 255.f);
	inl_r.SetValue(185.f);
	ColorsGroup.PlaceLabledControl(3, "Inner Left: Red", this, &inl_r);
	inl_g.SetFileId("inlgreen");
	inl_g.SetBoundaries(0.f, 255.f);
	inl_g.SetValue(25.f);
	ColorsGroup.PlaceLabledControl(3, "Inner Left: Green", this, &inl_g);
	inl_b.SetFileId("inlblue");
	inl_b.SetBoundaries(0.f, 255.f);
	inl_b.SetValue(230.f);
	ColorsGroup.PlaceLabledControl(3, "Inner Left: Blue", this, &inl_b);
	inr_r.SetFileId("inrred");
	inr_r.SetBoundaries(0.f, 255.f);
	inr_r.SetValue(185.f);
	ColorsGroup.PlaceLabledControl(3, "Inner Right: Red", this, &inr_r);

	inr_g.SetFileId("inrgreen");
	inr_g.SetBoundaries(0.f, 255.f);
	inr_g.SetValue(25.f);
	ColorsGroup.PlaceLabledControl(4, "Inner Right: Green", this, &inr_g);

	inr_b.SetFileId("inrblue");
	inr_b.SetBoundaries(0.f, 255.f);
	inr_b.SetValue(230.f);
	ColorsGroup.PlaceLabledControl(4, "Inner Right: Blue", this, &inr_b);
	outr_r.SetFileId("outrred");
	outr_r.SetBoundaries(0.f, 255.f);
	outr_r.SetValue(55.f);
	ColorsGroup.PlaceLabledControl(4, "Outer Right: Red", this, &outr_r);
	outr_g.SetFileId("outrgreen");
	outr_g.SetBoundaries(0.f, 255.f);
	outr_g.SetValue(15.f);
	ColorsGroup.PlaceLabledControl(4, "Outer Right: Green", this, &outr_g);
	outr_b.SetFileId("outrblue");
	outr_b.SetBoundaries(0.f, 255.f);
	outr_b.SetValue(210.f);
	ColorsGroup.PlaceLabledControl(4, "Outer Right: Blue", this, &outr_b);

	Menu.SetFileId(XorStr("menu_color"));
	Menu.SetColor(170, 20, 250, 255);
	ColorsGroup.PlaceLabledControl(4, XorStr("Controls"), this, &Menu);
	*/


	ConfigGroup.SetText("Configs");
	ConfigGroup.SetPosition(340, 30);
	ConfigGroup.SetSize(310, 320);
	RegisterControl(&ConfigGroup); ConfigListBox.SetHeightInItems(7);
	list_configs();
	ConfigGroup.PlaceLabledControl(0, XorStr(""), this, &ConfigListBox);
	LoadConfig.SetText(XorStr("Load"));
	LoadConfig.SetCallback(&load_callback);
	ConfigGroup.PlaceLabledControl(0, "", this, &LoadConfig);
	SaveConfig.SetText(XorStr("Save"));
	SaveConfig.SetCallback(&save_callback);
	ConfigGroup.PlaceLabledControl(0, "", this, &SaveConfig);
	RemoveConfig.SetText(XorStr("Remove"));
	RemoveConfig.SetCallback(&remove_config);
	ConfigGroup.PlaceLabledControl(0, "", this, &RemoveConfig);
	ConfigGroup.PlaceLabledControl(0, "", this, &NewConfigName);
	AddConfig.SetText(XorStr("Add"));
	AddConfig.SetCallback(&add_config);
	ConfigGroup.PlaceLabledControl(0, "", this, &AddConfig);
	/*---------------------- OTHERS ----------------------*/
	/*---------------------- OTHERS ----------------------*/
	/*---------------------- OTHERS ----------------------*/
	OtherOptions.SetText("other");
	OtherOptions.SetPosition(16, 358);
	OtherOptions.SetSize(314, 80);
	RegisterControl(&OtherOptions);


	BackTrackBones.SetFileId(XorStr("spookybones"));
	OtherOptions.PlaceLabledControl(2, XorStr("BackTrack Chams"), this, &BackTrackBones);
	DebugLagComp.SetFileId(XorStr("lagcompensationyes"));
	OtherOptions.PlaceLabledControl(2, XorStr("Draw Lag Compensation"), this, &DebugLagComp);
#pragma endregion
#pragma region GloveModel
#pragma endregion
}
void Options::SetupMenu()
{
	Menu.Setup();
	GUI.RegisterWindow(&Menu);
	GUI.BindWindow(VK_INSERT, &Menu);
}
void Options::DoUIFrame()
{
	if (Menu.VisualsTab.FiltersAll.GetState())
	{
		Menu.VisualsTab.FiltersC4.SetState(true);
		Menu.VisualsTab.FiltersPlayers.SetState(true);
		Menu.VisualsTab.FiltersWeapons.SetState(true);
		Menu.VisualsTab.FiltersNades.SetState(true);
	}
	GUI.Update();
	GUI.Draw();
}