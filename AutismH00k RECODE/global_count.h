#pragma once
#include "Hacks.h"
#include "Hooks.h"

namespace global_count
{
	extern int hits[65];
	extern int shots_fired[65];
	
	extern bool didhit;
	extern bool on_fire;
}

namespace game_event
{
	void on_hurt(IGameEvent * Event);
}