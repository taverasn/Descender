#ifndef APPLICATION_H
#define APPLICATION_H

// include events
#include "Events/Playevents.h"
// Contains our global game settings
#include "GameConfig.h"
// Load all entities+prefabs used by the game 
#include "Entities/PlayerBulletData.h"
#include "Entities/EnemyBulletData.h"
#include "Entities/PlayerData.h"
#include "Entities/EnemyData.h"
#include "Entities/PowerUpData.h"
// Include all systems used by the game and their associated components
#include "Systems/PlayerLogic.h"
#include "Systems/Renderer.h"
#include "Systems/LevelLogic.h"
#include "Systems/PhysicsLogic.h"
#include "Systems/PlayerBulletLogic.h"
#include "Systems/EnemyLogic.h"
#include "Systems/PowerUpLogic.h"
#include "Systems/EnemyBulletLogic.h"


// Allocates and runs all sub-systems essential to operating the game
class Application 
{
	// gateware libs used to access operating system
	GW::SYSTEM::GWindow window; // gateware multi-platform window
	GW::GRAPHICS::GOpenGLSurface openGL; // gateware vulkan API wrapper
	GW::INPUT::GController gamePads; // controller support
	GW::INPUT::GInput immediateInput; // twitch keybaord/mouse
	GW::INPUT::GBufferedInput bufferedInput; // event keyboard/mouse
	GW::AUDIO::GAudio audioEngine; // can create music & sound effects
	GW::CORE::GEventCache pressEvents;// key press event cache (saves input events)
	// third-party gameplay & utility libraries
	std::shared_ptr<flecs::world> game; // ECS database for gameplay
	std::shared_ptr<GameConfig> gameConfig; // .ini file game settings
	// ECS Entities and Prefabs that need to be loaded
	DSC::PlayerBulletData playerWeapons;
	DSC::EnemyBulletData enemyWeapons;
	DSC::PlayerData players;
	DSC::EnemyData enemies;
	DSC::PowerUpData powerUps;
	// specific ECS systems used to run the game
	DSC::PlayerLogic playerSystem;
	DSC::Renderer OGLRenderingSystem;
	DSC::LevelLogic levelSystem;
	DSC::PhysicsLogic physicsSystem;
	DSC::PlayerBulletLogic playerBulletSystem;
	DSC::EnemyLogic enemySystem;
	DSC::PowerUpLogic powerUpSystem;
	DSC::EnemyBulletLogic enemyBulletSystem;

	// EventGenerator for Game Events
	GW::CORE::GEventGenerator eventPusher;
	GW::CORE::GEventResponder msgs;
	// levellogic events
	GW::CORE::GEventGenerator levelEventGen;
	float outEsc;
	float outR;
	bool isActive = true;
	// how big the input cache can be each frame
	static constexpr unsigned int Max_Frame_Events = 32;
public:
	bool Init();
	bool Run();
	bool Shutdown();
private:
	bool InitWindow();
	bool InitInput();
	bool InitAudio();
	bool InitGraphics();
	bool InitEntities();
	bool UnloadEntities();
	bool InitSystems();
	bool GameLoop();
	void TogglePause(bool _isActive);
	bool ProcessInputEvents();
};

#endif 