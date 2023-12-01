// The powerup system is responsible for applying powerups to player
#ifndef POWERUPLOGIC_H
#define POWERUPLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Entities/PowerUpData.h"

// descender namespace
namespace DSC
{
	class PowerUpLogic
	{
		float fallingVelocity = 0.098f;
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// shared event generator
		GW::CORE::GEventGenerator eventPusher;
	public:
		// attach the required logic to the ECS 
		bool Init(std::shared_ptr<flecs::world> _game,
			std::weak_ptr<const GameConfig> _gameConfig,
			GW::CORE::GEventGenerator _eventPusher);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
	};

};

#endif