// The level system is responsible for transitioning the various levels in the game
#ifndef LEVELLOGIC_H
#define LEVELLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
// Entities for players, enemies & bullets
#include "../Entities/PlayerData.h"
#include "../Entities/PlayerBulletData.h"
#include <vector>
#include "../Events/Levelevents.h"

// descender namespace
namespace DSC
{
	class LevelLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// async version of above for threaded operations
		flecs::world gameAsync; 
		// mutex used to protect access to gameAsync 
		GW::CORE::GThreadShared gameLock;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// main event generator connected to player
		GW::CORE::GEventGenerator mainEventGen;
		// local event generator
		GW::CORE::GEventGenerator levelEventGen;
		// local event responder
		GW::CORE::GEventResponder eventResponder;
		// Level system will also load and switch music
		GW::AUDIO::GAudio audioEngine;
		GW::AUDIO::GMusic currentTrack;
		// Used to spawn enemies at a regular intervals on another thread
		GW::SYSTEM::GDaemon timedEvents;
		// systems vector for managing systems easier
		std::vector<flecs::system> systems;
		// world sizes
		float worldxmin, worldxmax, worldymin, worldymax;
		int currentLevel, currentEnemies, totalEnemies;
		int numSpaceLevelsCleared;
		GW::GEvent levelEventPush;

		LEVEL_EVENT levelEvent;
		LEVEL_EVENT_DATA levelData;
	public:
		// attach the required logic to the ECS 
		bool Init(	std::shared_ptr<flecs::world> _game,
					std::weak_ptr<const GameConfig> _gameConfig,
					GW::AUDIO::GAudio _audioEngine,
					GW::CORE::GEventGenerator _eventPusher,
					GW::CORE::GEventGenerator _levelEventGen);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
	private:
		flecs::system ConstrainPositionInsideGameWorld(float xmin, float xmax, float ymin, float ymax);
		flecs::system ActivateEnemiesNearPlayer();
		void GenerateNewLevel(LEVEL_EVENT_DATA& data);
		flecs::system EnemiesRemainingSystem();
		void WriteAndPushLevelEvent();
		void ClearGameWorldEntities();
		void StartEnemyBaiterSpawnTimedEvents(int, int);
		void ResetEnemyBaiterSpawnTimedEvents();
		void PauseEnemyBaiterSpawnTimedEvents(bool);
		flecs::system LevelTimer();
		void ResetLevelTimer();
		void SpawnEnemyType(flecs::entity enemy);
		flecs::system EnemyOrientationSystem();
		void DecrementHumansAlive();
		flecs::system HumansRemainingSystem();
		flecs::system HumanOrientationSystem();
		void SpawnEnemyTypeAtLocation(flecs::entity enemy, float x_pos, float y_pos);
		void ResetGameLevel();
		void DestroyActiveEnemies();
		void SpawnSwarmers(float xpos, float ypos);
	};

};

#endif