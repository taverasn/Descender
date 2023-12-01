#ifndef PLAYEVENTS_H
#define PLAYEVENTS_H

// descender namespace
namespace DSC
{
	enum PLAY_EVENT {
		ENEMY_LANDER_DESTROYED,
		ENEMY_MUTANT_DESTROYED,
		ENEMY_BAITER_DESTROYED,
		ENEMY_BOMBER_DESTROYED,
		ENEMY_POD_DESTROYED,
		ENEMY_SWARMER_DESTROYED,
		ENEMY_HUMAN_DESTROYED,
		ENEMY_HUMAN_SAVED,
		ENEMY_LANDER_ESCAPED,
		PLAYER_DEAD,
		EVENT_COUNT,
		RESET_PLAYER,
		RESET
	};
	struct PLAY_EVENT_DATA {
		flecs::id entity_id; // which entity was affected?
	};
}

#endif