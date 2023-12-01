// The player system is responsible for allowing control over the main ship(s)
#ifndef PLAYERLOGIC_H
#define PLAYERLOGIC_H

// Contains our global game settings
#include "../GameConfig.h"
#include "../Components/Physics.h"

// descender namespace
namespace DSC 
{
	class PlayerLogic 
	{
		// shared connection to the main ECS engine
		std::shared_ptr<flecs::world> game;
		// non-ownership handle to configuration settings
		std::weak_ptr<const GameConfig> gameConfig;
		// handle to our running ECS system
		flecs::system playerSystem;
		// permananent handles input systems
		GW::INPUT::GInput immediateInput;
		GW::INPUT::GBufferedInput bufferedInput;
		GW::INPUT::GController controllerInput;
		// permananent handle to audio system
		GW::AUDIO::GAudio audioEngine;
		// key press event cache (saves input events)
		// we choose cache over responder here for better ECS compatibility
		GW::CORE::GEventCache pressEvents;
		// event responder
		GW::CORE::GEventResponder onExplode;
		// handle to events
		GW::CORE::GEventGenerator eventPusher;
		// level event gen
		GW::CORE::GEventGenerator levelEventGen;
		bool thrusterSoundPlaying = false;
		// manage systems easier
		std::vector<flecs::system> systems;
		int awardScore = 0;
		float velocity = 0;
		bool isDead;
	public:
		bool GetPlayerState();
		void SetPlayerState(bool _isDead);
		// attach the required logic to the ECS 
		bool Init(	std::shared_ptr<flecs::world> _game,
					std::weak_ptr<const GameConfig> _gameConfig,
					GW::INPUT::GInput _immediateInput,
					GW::INPUT::GBufferedInput _bufferedInput,
					GW::INPUT::GController _controllerInput,
					GW::AUDIO::GAudio _audioEngine,
					GW::CORE::GEventGenerator _eventPusher,
					GW::CORE::GEventGenerator _levelEventGen);
		// control if the system is actively running
		bool Activate(bool runSystem);
		// release any resources allocated by the system
		bool Shutdown();
		// receive
	private:
		// how big the input cache can be each frame
		static constexpr unsigned int Max_Frame_Events = 32;
		// helper routines
		bool ProcessInputEvents(flecs::world& stage);
		bool CreatePlayerBullet(flecs::world& stage, flecs::entity& e, const Position origin);
		bool SetPlayerFacingDirection(const float& xaxis, flecs::entity& e);
		void DSC::PlayerLogic::DestroyAllActiveEnemies();;
	};

};

#endif