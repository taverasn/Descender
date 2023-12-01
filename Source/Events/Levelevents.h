#ifndef LEVELEVENTS_H
#define LEVELEVENTS_H

namespace DSC {
	enum LEVEL_EVENT {
		LEVEL_COMPLETE,
		NEW_LEVEL,
		MAIN_MENU,
		NEW_GAME,
		GAME_OVER,
		CLEAR_ENTITIES,
		SPAWN_SWARMERS
	};
	struct LEVEL_EVENT_DATA {
		int level;
		int humansAlive;
	};
	struct LEVEL_EVENT_SWARMER_DATA {
		float xpos;
		float ypos;
		int count;
	};

	// globals often attached to game entity
	struct ENEMIES_REMAINING {
		int numEnemies;
	};
	struct LEVEL_TIMER {
		float timeElapsed;
	};
	struct CAMERA_X_POSITION {
		float value;
	};

	struct TERRAIN_LEVEL {};
	struct GENERATE_NEW_TERRAIN {};
	struct DESTROY_TERRAIN {};
	struct HUMAN_KILLED {};
	struct FLASH_EFFECT {};
	struct GAME_OVER {};
}

#endif
