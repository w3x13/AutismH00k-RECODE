#include <array>
#include <string>
#include <deque>
#include <algorithm>
#include "Entities.h"
#include "CommonIncludes.h"
#include "Entities.h"
#include "Vector.h"
#include <map>
#include "Interfaces.h"
#include "Hacks.h"
#include "Hooks.h"


namespace FEATURES
{
	namespace RAGEBOT
	{
		class Resolver
		{
		public:
			Resolver();

			void DoFSN();
			void DoCM();

		public:
			/// resolve types, they're "flags" so to speak since a player can have more than 1 resolve type at once
			/// if angles overlap
			static const unsigned int RESOLVE_TYPE_NUM = 8;
			static const unsigned short RESOLVE_TYPE_OVERRIDE = 0b00000001,
				RESOLVE_TYPE_NO_FAKE = 0b00000010,
				RESOLVE_TYPE_LBY = 0b00000100,
				RESOLVE_TYPE_LBY_UPDATE = 0b00001000,
				RESOLVE_TYPE_PREDICTED_LBY_UPDATE = 0b00010000,
				RESOLVE_TYPE_LAST_MOVING_LBY = 0b00100000,
				RESOLVE_TYPE_NOT_BREAKING_LBY = 0b01000000,
				RESOLVE_TYPE_BRUTEFORCE = 0b10000000,
				RESOLVE_TYPE_LAST_MOVING_LBY_DELTA = 0b100000000,
				RESOLVE_TYPE_ANTI_FREESTANDING = 0b1000000000;

		public:
			/// a struct holding info the resolver needs, updated every frame for every player
			class PlayerResolveRecord
			{
			public:
				PlayerResolveRecord()
				{
					resolve_type = 0;
					shots_missed_moving_lby = 0;
					shots_missed_moving_lby_delta = 0;

					last_balance_adjust_trigger_time = 0.f;
					last_moving_lby_delta = 0.f;
					last_time_moving = 0.f;
					last_time_down_pitch = 0.f;
					next_predicted_lby_update = 0.f;

					has_fake = false;
					is_dormant = false, is_last_moving_lby_delta_valid = false;
					is_last_moving_lby_valid = false, is_fakewalking = false;
					is_balance_adjust_triggered = false, is_balance_adjust_playing = false;
					did_lby_flick = false, did_predicted_lby_flick = false;

					for (int i = 0; i < RESOLVE_TYPE_NUM; i++)
					{
						shots_hit[i] = 0;
						shots_fired[i] = 0;
					}
				}

			public:
				struct AntiFreestandingRecord
				{
					int right_damage = 0, left_damage = 0;
					float right_fraction = 0.f, left_fraction = 0.f;
				};

			public:
				CAnimationLayer anim_layers[15];
				AntiFreestandingRecord anti_freestanding_record;

				Vector resolved_angles, networked_angles;
				Vector velocity, origin;

				int shots_hit[RESOLVE_TYPE_NUM], shots_fired[RESOLVE_TYPE_NUM];
				int shots_missed_moving_lby, shots_missed_moving_lby_delta;
				unsigned short resolve_type;

				float lower_body_yaw;
				float last_moving_lby;
				float last_moving_lby_delta;
				float last_balance_adjust_trigger_time;
				float last_time_moving;
				float last_time_down_pitch;
				float next_predicted_lby_update;

				bool is_dormant;
				bool is_last_moving_lby_valid;
				bool is_fakewalking;
				bool is_last_moving_lby_delta_valid;
				bool is_balance_adjust_triggered, is_balance_adjust_playing;
				bool did_lby_flick, did_predicted_lby_flick;
				bool has_fake;
			};

			/// a snapshot holding info about the moment you shot, used to count shots missed / hit
			struct ShotSnapshot
			{
				IClientEntity* entity; /// person we shot at
				PlayerResolveRecord resolve_record; /// their resolve record when we shot

				float time; /// time when snapshot was created
				float first_processed_time; /// time when the shot was first processed
				bool was_shot_processed;
				int hitgroup_hit;
			};

		private:
			PlayerResolveRecord player_resolve_records[64];
			std::vector<ShotSnapshot> shot_snapshots;
			std::vector<Vector> last_eye_positions;

		public:
			PlayerResolveRecord& GetPlayerResolveInfo(IClientEntity* entity)
			{
				return player_resolve_records[entity->GetIndex()];
			}

			std::string TranslateResolveRecord(unsigned short resolve_type);
			int GetResolveTypeIndex(unsigned short resolve_type);
			int GetShotsMissed(IClientEntity* entity, unsigned short resolve_type);

			bool IsResolved(const unsigned short& resolve_type)
			{
				if (resolve_type & RESOLVE_TYPE_NO_FAKE ||
					resolve_type & RESOLVE_TYPE_LBY_UPDATE ||
					resolve_type & RESOLVE_TYPE_PREDICTED_LBY_UPDATE)
					return true;

				return false;
			}

			bool IsFakewalking(IClientEntity* entity)
			{
				return player_resolve_records[entity->GetIndex()].is_fakewalking;
			}

			bool IsMovingOnGround(IClientEntity* entity)
			{
				return player_resolve_records[entity->GetIndex()].velocity.Length2D() > 0.1f && entity->GetFlags() & FL_ONGROUND;
			}

			bool IsYawSideways(IClientEntity* entity, float yaw);
			bool IsFakingYaw(IClientEntity* entity);

			/// pushback a record onto the shot snapshot queue
			void AddShotSnapshot(IClientEntity* entity, PlayerResolveRecord resolve_record);

			void EventCallback(IGameEvent* game_event);

		private:
			void ProcessSnapShots();

			void UpdateResolveRecord(IClientEntity* entity);

			void ResolveYaw(IClientEntity* entity);
			void ResolvePitch(IClientEntity* entity);

			void ResolveYawBruteforce(IClientEntity* entity);
			float ResolveYawOverride´(IClientEntity* entity);
			bool AntiFreestanding(IClientEntity* entity, float& yaw);
		};

		extern Resolver resolver;
	}
}


class ResolverSetup
{
public:
	bool didhit;
	void FSN(IClientEntity* pEntity, ClientFrameStage_t stage); //used to get the fresh THINGS and the resolve
	void CM(IClientEntity* pEntity);
	void preso(IClientEntity * pEntity);
	bool AntiFreestanding(IClientEntity * pEntity);
	
	//cause this is slower than FSN so we are going to get older info and not updated at all ez to compare between etc.
	std::map<int, float>badangle;

	void Resolve(IClientEntity * pEntity, int CurrentTarget);
	//resolve

	float newsimtime;
	float storedsimtime;
	bool lbyupdated;
	float storedlbyFGE;
	float storedanglesFGE;
	float storedsimtimeFGE;
	float movinglbyFGE;
	bool didhitHS;

	static ResolverSetup GetInst()
	{
		static ResolverSetup instance;
		return instance;
	}
};
namespace Globals
{

	extern CUserCmd* UserCmd;
	extern IClientEntity* Target;
	extern int Shots;
	extern int fired;
	extern int hit;

	extern bool change;
	extern int TargetID;
	extern bool didhitHS;
	extern int missedshots;
	extern std::map<int, QAngle> storedshit;
	extern bool Up2date;
}