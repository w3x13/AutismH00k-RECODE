#include "global_count.h"

namespace global_count
{
	int hits[65];
	int shots_fired[65];

	bool didhit;
	bool on_fire;
}


void game_event::on_hurt(IGameEvent * Event)
{
	if (!Event)
		global_count::didhit = false;

	if (!strcmp(Event->GetName(), "player_hurt"))
	{
		int deadfag = Event->GetInt("userid");
		int attackingfag = Event->GetInt("attacker");
		if (Interfaces::Engine->GetPlayerForUserID(attackingfag) == Interfaces::Engine->GetLocalPlayer())
		{
			global_count::didhit = true;
			global_count::on_fire = true;
		}
	}
}
