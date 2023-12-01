#include "Application.h"
#include "Events/Levelevents.h"

// open some Gateware namespaces for conveinence 
// NEVER do this in a header file!
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;

bool Application::Init()
{
	eventPusher.Create();
	levelEventGen.Create();

	// load all game settigns
	gameConfig = std::make_shared<GameConfig>();
	// create the ECS system
	game = std::make_shared<flecs::world>();
	// init all other systems
	if (InitWindow() == false)
		return false;
	if (InitInput() == false)
		return false;
	if (InitAudio() == false)
		return false;
	if (InitGraphics() == false)
		return false;
	if (InitEntities() == false)
		return false;
	if (InitSystems() == false)
		return false;
	return true;
}

bool Application::Run()
{
	bool winClosed = false;
	float clr[] = { 0.0f / 255.0f,  0.0f / 255.0f, 0.0f / 255.0f, 1 }; // DONE: Part 1a (optional)
	msgs.Create([&](const GW::GEvent& e) {
		GW::SYSTEM::GWindow::Events q;
	if (+e.Read(q) && q == GWindow::Events::RESIZE)
		clr[2] += 0.01f; // shift color as they resize
		});
	window.Register(msgs);
	while (+window.ProcessWindowEvents())
	{

		if (winClosed == true)
			return true;


		OGLRenderingSystem.ClearRenderer();
		
		if (GameLoop() == false)
			return false;
	}


	return true;
}

bool Application::Shutdown()
{
	// disconnect systems from global ECS
	if (playerSystem.Shutdown() == false)
		return false;
	if (levelSystem.Shutdown() == false)
		return false;
	if (OGLRenderingSystem.Shutdown() == false)
		return false;
	if (physicsSystem.Shutdown() == false)
		return false;
	if (playerBulletSystem.Shutdown() == false)
		return false;
	if (enemyBulletSystem.Shutdown() == false)
		return false;
	if (enemySystem.Shutdown() == false)
		return false;
	if (powerUpSystem.Shutdown() == false)
		return false;

	return true;
}

bool Application::InitWindow()
{
	// grab settings
	int width = gameConfig->at("Window").at("width").as<int>();
	int height = gameConfig->at("Window").at("height").as<int>();
	int xstart = gameConfig->at("Window").at("xstart").as<int>();
	int ystart = gameConfig->at("Window").at("ystart").as<int>();
	std::string title = gameConfig->at("Window").at("title").as<std::string>();
	// open window
	if (+window.Create(xstart, ystart, width, height, GWindowStyle::WINDOWEDLOCKED) &&
		+window.SetWindowName(title.c_str())) {
		return true;
	}
	return false;
}

bool Application::InitInput()
{
	if (-gamePads.Create())
		return false;
	if (-immediateInput.Create(window))
		return false;
	if (-bufferedInput.Create(window))
		return false;
	if (-pressEvents.Create(Max_Frame_Events))
		return false;
	else
	{
		bufferedInput.Register(pressEvents);
		gamePads.Register(pressEvents);
	}

	return true;
}

bool Application::InitAudio()
{
	if (-audioEngine.Create())
		return false;
	return true;
}

bool Application::InitGraphics()
{
#ifndef NDEBUG
	const char* debugLayers[] = {
		"OGL_LAYER_KHRONOS_validation", // standard validation layer
		//"VK_LAYER_RENDERDOC_Capture" // add this if you have installed RenderDoc
	};
	if (+openGL.Create(window, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		return true;
#else
	if (+openGL.Create(window, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		return true;
#endif
	return false;
}

bool Application::InitEntities()
{
	// Load bullet prefabs
	if (playerWeapons.Load(game, gameConfig, audioEngine) == false)
		return false;
	// Load enemy bullet prefabs
	if (enemyWeapons.Load(game, gameConfig, audioEngine) == false)
		return false;
	// Load the player entities
	if (players.Load(game, gameConfig, audioEngine) == false)
		return false;
	// Load the enemy entities
	if (enemies.Load(game, gameConfig, audioEngine) == false)
		return false;
	// Load the power up entities
	if (powerUps.Load(game, gameConfig, audioEngine) == false)
		return false;

	return true;
}

bool Application::UnloadEntities()
{
	if (playerWeapons.Unload(game, gameConfig) == false)
		return false;
	if (enemyWeapons.Unload(game, gameConfig) == false)
		return false;
	if (players.Unload(game) == false)
		return false;
	if (enemies.Unload(game) == false)
		return false;

	return true;
}

bool Application::InitSystems()
{
	// connect systems to global ECS
	if (playerSystem.Init(game, gameConfig, immediateInput, bufferedInput,
		gamePads, audioEngine, eventPusher, levelEventGen) == false)
		return false;
	if (levelSystem.Init(game, gameConfig, audioEngine, eventPusher, levelEventGen) == false)
		return false;
	if (OGLRenderingSystem.Init(game, gameConfig, window, openGL) == false)
		return false;
	if (physicsSystem.Init(game, gameConfig) == false)
		return false;
	if (playerBulletSystem.Init(game, gameConfig) == false)
		return false;
	if (enemyBulletSystem.Init(game, gameConfig) == false)
		return false;
	if (enemySystem.Init(game, gameConfig, eventPusher, levelEventGen) == false)
		return false;
	if (powerUpSystem.Init(game, gameConfig, eventPusher) == false)
		return false;

	return true;
}

bool Application::GameLoop()
{
	// compute delta time and pass to the ECS system
	static auto start = std::chrono::steady_clock::now();
	double elapsed = std::chrono::duration<double>(
		std::chrono::steady_clock::now() - start).count();
	start = std::chrono::steady_clock::now();

	ProcessInputEvents();

	return game->progress(static_cast<float>(elapsed));
}

void Application::TogglePause(bool _isActive)
{
	playerSystem.Activate(_isActive);
	levelSystem.Activate(_isActive);
	OGLRenderingSystem.Activate(_isActive);
	physicsSystem.Activate(_isActive);
	playerBulletSystem.Activate(_isActive);
	enemyBulletSystem.Activate(_isActive);
	enemySystem.Activate(_isActive);
	powerUpSystem.Activate(_isActive);
}

bool Application::ProcessInputEvents()
{
	// pull any waiting events from the event cache and process them
	GW::GEvent event;
	while (+pressEvents.Pop(event)) {
		bool pause = false;
		bool reset = false;
		GW::INPUT::GController::Events controller;
		GW::INPUT::GController::EVENT_DATA c_data;
		GW::INPUT::GBufferedInput::Events keyboard;
		GW::INPUT::GBufferedInput::EVENT_DATA k_data;
		// these will only happen when needed
		if (+event.Read(keyboard, k_data)) {
			if (keyboard == GW::INPUT::GBufferedInput::Events::KEYPRESSED) {
				if (k_data.data == G_KEY_ESCAPE) {
					pause = true;
				}

				if (k_data.data == G_KEY_R) {
					reset = true;
				}
			}
		}
		else if (+event.Read(controller, c_data)) {
			if (controller == GW::INPUT::GController::Events::CONTROLLERBUTTONVALUECHANGED) {
				if (c_data.inputValue > 0 && c_data.inputCode == G_NORTH_BTN)
					pause = true;
			}
		}
		if (pause) {
			//set  a pause state
			std::cout << "Pause\n";
			isActive = !isActive;
			TogglePause(isActive);
		}
		if (reset) {
			//set a reset state
			std::cout << "Restart\n";
			TogglePause(true);
			GW::GEvent newGame;
			DSC::LEVEL_EVENT_DATA x;
			newGame.Write(DSC::LEVEL_EVENT::NEW_GAME, x);
			//override it with a new event pushed for the level logic
			levelEventGen.Push(newGame);
		}
	}
	return true;
}