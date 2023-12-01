// The Enemy system is responsible for enemy behaviors
#ifndef ENEMYLOGIC_H
#define ENEMYLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Entities/EnemyData.h"
#include "../Components/Physics.h"
#include <vector>

// descender namespace
namespace DSC
{
	class EnemyLogic
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to events
		GW::CORE::GEventGenerator eventPusher;
		// level event gen
		GW::CORE::GEventGenerator levelEventGen;
		// easier to enable/disable systems
		std::vector<flecs::system> systems;

		GW::CORE::GEventResponder onExplode;

		GW::MATH2D::GVECTOR2F podPosition;

	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::CORE::GEventGenerator _eventPusher,
			GW::CORE::GEventGenerator _levelEventGen);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
	private:
		bool EnemyFireWeapon(flecs::world& stage, flecs::entity& e, Position origin);
		bool GetAimTarget(flecs::world& stage, flecs::entity& e, const Position& origin, Velocity& v);
		flecs::system StartEnemyFiringSystem();
		flecs::system StartEnemyCooldownSystem();
		flecs::system StartEnemyMutantMovementSystem(float speed);
		flecs::system StartEnemyBomberMovementSystem(float speed);
		flecs::system StartEnemyBaiterMovementSystem(float baiterSpeed);
		flecs::system StartEnemyPodMovementSystem(float podSpeed, float podDistmin, float podDistmax);
		flecs::system StartEnemySwarmerMovementSystem(float swarmerSpeed, float jitterAmount);
		void OnPodDeath(GW::CORE::GEventGenerator _eventPusher);
		void DSC::EnemyLogic::SpawnSwarmers(float podX, float podY, int i);
	};

};

#endif