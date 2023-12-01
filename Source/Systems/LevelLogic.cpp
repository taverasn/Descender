#include <random>
#include "LevelLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Entities/Prefabs.h"
#include "../Utils/Macros.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"


using namespace DSC; // descender namespace

// Connects logic to traverse any players and allow a controller to manipulate them
bool DSC::LevelLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::AUDIO::GAudio _audioEngine,
	GW::CORE::GEventGenerator _eventPusher,
	GW::CORE::GEventGenerator _levelEventGen)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	audioEngine = _audioEngine;
	mainEventGen = _eventPusher;
	levelEventGen = _levelEventGen;

	// create an asynchronus version of the world
	gameAsync = game->async_stage(); // just used for adding stuff, don't try to read data
	gameLock.Create();
	// Pull enemy Y start location from config file
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	// get world constraints
	worldxmin = (*readCfg).at("World").at("xsizemin").as<float>();
	worldxmax = (*readCfg).at("World").at("xsizemax").as<float>();
	worldymin = (*readCfg).at("World").at("ysizemin").as<float>();
	worldymax = (*readCfg).at("World").at("ysizemax").as<float>();

	// level variables
	// time before enemy baiters begin to spawn
	int baiterLevelSpawnDelay = (*readCfg).at("Level1").at("baiterleveldelay").as<int>();
	// time between each baiter spawn
	int baiterSpawnDelay = (*readCfg).at("Level1").at("baiterspawndelay").as<int>();

	// initialize global components
	ResetGameLevel();

	// local level event generator and responder
	//eventGenerator.Create();
	eventResponder.Create([this, baiterLevelSpawnDelay, baiterSpawnDelay](const GW::GEvent& event) {
		enum LEVEL_EVENT what;
		LEVEL_EVENT_DATA how;
		LEVEL_EVENT_SWARMER_DATA swarm;
		// if I have an event to process
		if (+event.Read(what, how)) {
			// do event processing things here
			if (what == LEVEL_EVENT::NEW_LEVEL) {
				std::cout << "NEW LEVEL " << levelData.level << '\n';
				// generate terrain or attack phase based on level and alive humans
				// track enemies remaining (done through a system)
				// display level complete on destroying all enemies (that system triggers level complete)
				game->entity("Player One").get_mut<Position>()->value.x = 6;
				game->entity("Player One").get_mut<Position>()->value.y = 1;
				GenerateNewLevel(how);
				StartEnemyBaiterSpawnTimedEvents(baiterLevelSpawnDelay, baiterSpawnDelay);
				//ResetLevelTimer();
			}
			else if (what == LEVEL_EVENT::LEVEL_COMPLETE) {
				std::cout << "LEVEL COMPLETE\n";
				// clear all enemies, enemy bullets, humans
				ClearGameWorldEntities();
				// display summary menu
				// 
				// stop processing timedEvents (TODO: need to ensure pause also pauses timedEvents)
				ResetEnemyBaiterSpawnTimedEvents();
				// set new event to be pushed
				levelEvent = LEVEL_EVENT::NEW_LEVEL;
				// increment level and track humans alive
				levelData.level++;
				if (numSpaceLevelsCleared == 0) {
					levelData.humansAlive = 5;
				}
				// spin up new level by pushing new level event
				WriteAndPushLevelEvent();
			}
			else if (what == LEVEL_EVENT::MAIN_MENU) {
				std::cout << "MAIN MENU\n";
				// pause game systems
				// display main menu
				GW::AUDIO::GMusic menuMusic;
				menuMusic.Create("../SoundFX/menu-music.wav", audioEngine, 0.15f);
				menuMusic.Play(true);
				// transition to new game
			}
			else if (what == LEVEL_EVENT::NEW_GAME) {
				std::cout << "NEW GAME\n";
				// display level 1
				// play start game audio
				// keep systems paused during start audio
				// 
				// begin level 1
				levelEvent = LEVEL_EVENT::NEW_LEVEL;
				ClearGameWorldEntities();
				ResetGameLevel();
				GW::AUDIO::GSound gameStart;
				gameStart.Create("../SoundFX/game-start.wav", audioEngine, 0.15f);
				gameStart.Play();
				// spin up new level by pushing new level event
				WriteAndPushLevelEvent();
				// reset score? etc?
			}
			else if (what == LEVEL_EVENT::GAME_OVER) {
				std::cout << "GAME OVER\n";
				// stop baiter system
				ResetEnemyBaiterSpawnTimedEvents();

				// play game over music
				// GW::AUDIO::GSound gameOver;
				// gameOver.Create("../SoundFX/game-over.wav", audioEngine, 0.15f);
				// gameOver.Play();
				// display high score list
				// display initials entry
				// exit back to main menu
			}
			else if (what == LEVEL_EVENT::CLEAR_ENTITIES) {
				DestroyActiveEnemies();
			}
		}
		if (+event.Read(what, swarm)) {
			SpawnSwarmers(swarm.xpos, swarm.ypos);
		}
	});
	levelEventGen.Register(eventResponder);

	// test to begin game, need main menu
	// should be NEW_GAME which then loads first level
	levelEvent = LEVEL_EVENT::NEW_GAME;
	GW::GEvent beginGame;
	beginGame.Write(levelEvent, levelData);
	// push first level event here
	levelEventGen.Push(beginGame);
	
	// initialize/start up systems that run every frame
	auto a = ConstrainPositionInsideGameWorld(worldxmin, worldxmax, worldymin + 0.05f, worldymax - 0.05f);
	systems.push_back(a);
	auto b = ActivateEnemiesNearPlayer();
	systems.push_back(b);
	auto c = EnemiesRemainingSystem();
	systems.push_back(c);
	auto d = LevelTimer();
	systems.push_back(d);
	auto e = EnemyOrientationSystem();
	systems.push_back(e);
	auto f = HumansRemainingSystem();
	systems.push_back(f);
	auto g = HumanOrientationSystem();
	systems.push_back(g);


	// create a system the runs at the end of the frame only once to merge async changes
	struct LevelSystem {}; // local definition so we control iteration counts
	game->entity("Level System").add<LevelSystem>();
	// only happens once per frame at the very start of the frame
	game->system<LevelSystem>().kind(flecs::OnLoad) // first defined phase
		.each([this](flecs::entity e, LevelSystem& s) {
		// merge any waiting changes from the last frame that happened on other threads
		gameLock.LockSyncWrite();
		gameAsync.merge();
		gameLock.UnlockSyncWrite();
	});

	return true;
}

// Free any resources used to run this system
bool DSC::LevelLogic::Shutdown()
{
	timedEvents = nullptr; // stop adding enemies
	gameAsync.merge(); // get rid of any remaining commands
	for (auto system : systems) {
		system.destruct();
	}
	game->entity("Level System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

void DSC::LevelLogic::DecrementHumansAlive() {
	levelData.humansAlive--;
}

flecs::system DSC::LevelLogic::HumansRemainingSystem()
{
	return game->system<HUMAN_KILLED>()
		.each([this](flecs::entity& e, HUMAN_KILLED) {
			if (e.has<HUMAN_KILLED>()) {
				DecrementHumansAlive();
				std::cout << "Humans remaining: " << levelData.humansAlive << '\n';
				e.remove<HUMAN_KILLED>();
				if (levelData.humansAlive == 0) {
					numSpaceLevelsCleared = 2;
				}
			}
		});
}

// Toggle if a system's Logic is actively running
bool DSC::LevelLogic::Activate(bool runSystem)
{
	if (runSystem) {
		for (auto system : systems) {
			system.enable();
		}
		game->entity("Level System").enable();
		PauseEnemyBaiterSpawnTimedEvents(false);
	}
	else {
		for (auto system : systems) {
			system.disable();
		}
		game->entity("Level System").disable();
		PauseEnemyBaiterSpawnTimedEvents(true);
	}
	return false;
}

// create baiter every spawn-delay seconds after level-delay seconds player is active in level
void DSC::LevelLogic::StartEnemyBaiterSpawnTimedEvents(int baiterLevelSpawnDelay, int baiterSpawnDelay) {
	timedEvents.Create(baiterSpawnDelay * 1000, [this]() {
		if (game->entity("Player One").has<IsAlive>()) {
			// compute random spawn location away from player
			std::random_device rd;  // Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
			std::uniform_real_distribution<float> x_range(-0.5f, 0.5f);
			std::uniform_real_distribution<float> y_range(-0.5f, 0.5f);
			// adjust offset from player's current position
			float x = x_range(gen);
			float y = y_range(gen);
			// don't spawn on top of player
			float x_offset = ((x < 0.0f) ? x -= 0.5f : x += 0.5f) + game->entity("Player One").get<Position>()->value.x;
			float y_offset = ((y < 0.0f) ? y -= 0.5f : y += 0.5f) + game->entity("Player One").get<Position>()->value.y;
			// grab enemy baiter prefab
			flecs::entity baiter; 
			if (RetreivePrefab("Enemy Type Baiter", baiter)) {
				// you must ensure the async_stage is thread safe as it has no built-in synchronization
				gameLock.LockSyncWrite();
				gameAsync.entity().is_a(baiter)
					.set<Velocity>({ 0,0 })
					.set<Acceleration>({ 0,0 })
					// spawn baiter at player position + x/y offset
					.set<Position>({ x_offset, y_offset })
					.set<IsAlive>({ true });
				// be sure to unlock when done so the main thread can safely merge the changes
				gameLock.UnlockSyncWrite();
			}
		}
	}, baiterLevelSpawnDelay * 1000); // wait x seconds to start enemy baiter spawning
}

// stop all timedEvents logic for new level/new game
void DSC::LevelLogic::ResetEnemyBaiterSpawnTimedEvents() {
	timedEvents = nullptr;
}

// pause or unpause the timedEvents
void DSC::LevelLogic::PauseEnemyBaiterSpawnTimedEvents(bool shouldPause) {
	if (shouldPause) {
		timedEvents.Pause(true, 0);
	}
	else {
		timedEvents.Resume();
	}
}

// maintain a level timer for enemy baiter spawning (may no longer need)
flecs::system DSC::LevelLogic::LevelTimer() {
	return game->system<LEVEL_TIMER>("LevelTimer")
		.iter([this](flecs::iter& i, LEVEL_TIMER* lt) {
			lt->timeElapsed += game->delta_time();
			//std::cout << lt->timeElapsed << '\n';
	});
}

// resets level timer to zero
void DSC::LevelLogic::ResetLevelTimer() {
	game->set<LEVEL_TIMER>({ 0.0f });
}

// clears game world entities
void DSC::LevelLogic::ClearGameWorldEntities() {
	gameLock.LockAsyncRead();
	gameAsync.filter<Enemy>()
		.each([](flecs::entity& e, Enemy) {
			e.destruct();
		});
	gameAsync.filter<EnemyBullet>()
		.each([](flecs::entity& e, EnemyBullet) {
			e.destruct();
		});
	gameAsync.filter<EnemyHuman>()
		.each([](flecs::entity& e, EnemyHuman) {
			e.destruct();
		});
	gameAsync.filter<PlayerBullet>()
		.each([](flecs::entity& e, PlayerBullet) {
			e.destruct();
		});
	gameLock.UnlockAsyncRead();
}

// should work this into the enemy death event system
// for now, tracks enemy count every frame
flecs::system DSC::LevelLogic::EnemiesRemainingSystem() {
	return game->system<ENEMIES_REMAINING>().kind(flecs::PostUpdate)
		.each([this](flecs::entity& e, ENEMIES_REMAINING& er) {
			if (er.numEnemies == 0) {
				er.numEnemies = -1;
				levelEvent = LEVEL_EVENT::LEVEL_COMPLETE;
				WriteAndPushLevelEvent();
			}
		});
}

void DSC::LevelLogic::WriteAndPushLevelEvent() {
	levelEventPush.Write(levelEvent, levelData);
	levelEventGen.Push(levelEventPush);
}

void DSC::LevelLogic::GenerateNewLevel(LEVEL_EVENT_DATA &data) {
	
	// gameLock.LockSyncWrite();

	int maxEnemies = 30;
	int randEnemies = 5;
	float enemyCount = 0;

	// data.humansAlive > 0 then make a terrain level
	if (data.humansAlive > 0) {

		game->add<TERRAIN_LEVEL>();
		game->add<GENERATE_NEW_TERRAIN>();
		// generate terrain with humansalive humans
		// disable enemy vertical looping
		// will be only level that enemy landers spawn on
	}
	else {
		game->remove<TERRAIN_LEVEL>();
		game->add<GENERATE_NEW_TERRAIN>();
		numSpaceLevelsCleared--;
		// generate an open space level
		// enable enemy veritcal looping
	}

	// retrieve all enemy prefabs
	flecs::entity lander, mutant, bomber, baiter, pod, swarmer, human;
	RetreivePrefab("Enemy Type Lander", lander);
	RetreivePrefab("Enemy Type Mutant", mutant);
	RetreivePrefab("Enemy Type Bomber", bomber);
	RetreivePrefab("Enemy Type Baiter", baiter);
	RetreivePrefab("Enemy Type Pod", pod);
	RetreivePrefab("Enemy Type Swarmer", swarmer); // no need as doesn't spawn in a level
	RetreivePrefab("Enemy Type Human", human);

	// counts of enemies per level
	int landerCount = 6;
	int mutantCount = 4;
	int bomberCount = 3;
	int podCount = 2;

	std::random_device rd;  // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<float> x_range(worldxmin, worldxmax);
	std::uniform_real_distribution<float> y_range(worldymin, worldymax);
	// spawn enemies outside of player at start
	float worldMidMinX = 5.0f;
	float worldMidMaxX = 7.0f;
	
	// spawn humans and landers
	if (data.humansAlive > 0) {
		// spawn humans
		for (int i = 0; i < data.humansAlive; i++) {
			float humanPositionX = x_range(gen);
			SpawnEnemyTypeAtLocation(human, humanPositionX, 0.1f);
		}
		for (int i = 0; i < landerCount + (data.level * 1.25); i++) {
			float landerPositionX;
			do {
				landerPositionX = x_range(gen);
			} while (landerPositionX > worldMidMinX && landerPositionX < worldMidMaxX);
			SpawnEnemyTypeAtLocation(lander, landerPositionX, 1.95f);
			enemyCount++;
		}
	}
	// normal spawning
	if (data.level != 1 && data.level < 4) {
		// bomber
		for (int i = 0; i < bomberCount + (data.level); i++) {
			float bomberPositionX, bomberPositionY;
			do {
				bomberPositionX = x_range(gen);
			} while (bomberPositionX > worldMidMinX && bomberPositionX < worldMidMaxX);
			bomberPositionY = y_range(gen);
			if (enemyCount < maxEnemies) {
				SpawnEnemyTypeAtLocation(bomber, bomberPositionX, bomberPositionY);
				enemyCount++;
			}
		}
		// mutant
		for (int i = 0; i < mutantCount + (data.level * 2.0f); i++) {
			float mutantPositionX, mutantPositionY;
			do {
				mutantPositionX = x_range(gen);
			} while (mutantPositionX > worldMidMinX && mutantPositionX < worldMidMaxX);
			mutantPositionY = y_range(gen);
			if (enemyCount < maxEnemies) {
				SpawnEnemyTypeAtLocation(mutant, mutantPositionX, mutantPositionY);
				enemyCount++;
			}
		}
		// pod
		for (int i = 0; i < podCount + (data.level); i++) {
			float podPositionX, podPositionY;
			do {
				podPositionX = x_range(gen);
			} while (podPositionX > worldMidMinX && podPositionX < worldMidMaxX);
			podPositionY = y_range(gen);
			if (enemyCount < maxEnemies) {
				SpawnEnemyTypeAtLocation(pod, podPositionX, podPositionY);
				enemyCount++;
			}
		}
	}
	else if (data.level != 1 && data.level >= 4) {
		// bomber
		for (int i = 0; i < bomberCount + (data.level * 1.25f); i++) {
			float bomberPositionX, bomberPositionY;
			do {
				bomberPositionX = x_range(gen);
			} while (bomberPositionX > worldMidMinX && bomberPositionX < worldMidMaxX);
			bomberPositionY = y_range(gen);
			if (enemyCount < maxEnemies) {
				SpawnEnemyTypeAtLocation(bomber, bomberPositionX, bomberPositionY);
				enemyCount++;
			}
		}
		// pod
		for (int i = 0; i < podCount + (data.level); i++) {
			float podPositionX, podPositionY;
			do {
				podPositionX = x_range(gen);
			} while (podPositionX > worldMidMinX && podPositionX < worldMidMaxX);
			podPositionY = y_range(gen);
			if (enemyCount < maxEnemies) {
				SpawnEnemyTypeAtLocation(pod, podPositionX, podPositionY);
				enemyCount++;
			}
		}
		// mutant
		for (int i = 0; i < mutantCount + (data.level * 2.0f); i++) {
			float mutantPositionX, mutantPositionY;
			do {
				mutantPositionX = x_range(gen);
			} while (mutantPositionX > worldMidMinX && mutantPositionX < worldMidMaxX);
			mutantPositionY = y_range(gen);
			if (enemyCount < maxEnemies) {
				SpawnEnemyTypeAtLocation(mutant, mutantPositionX, mutantPositionY);
				enemyCount++;
			}
		}
		// spawn swarmers for extras
		for (int i = 0; i < randEnemies; i++) {
			float enemyPositionX, enemyPositionY;
			do {
				enemyPositionX = x_range(gen);
			} while (enemyPositionX > worldMidMinX && enemyPositionX < worldMidMaxX);
			enemyPositionY = y_range(gen);
			if (enemyCount < maxEnemies) {
				SpawnEnemyTypeAtLocation(swarmer, enemyPositionX, enemyPositionY);
				enemyCount++;
			}
		}
	}
	// keep it easy on level one...
	else if (data.level == 1) {
		/*for (int i = 0; i < landerCount; i++) {
			float landerPositionX;
			do {
				landerPositionX = x_range(gen);
			} while (landerPositionX > worldMidMinX && landerPositionX < worldMidMaxX);
			SpawnEnemyTypeAtLocation(lander, landerPositionX, 1.95f);
			game->get_mut<ENEMIES_REMAINING>()->numEnemies++;
		}*/
		for (int i = 0; i < bomberCount/2; i++) {
			float bomberPositionX;
			do {
				bomberPositionX = x_range(gen);
			} while (bomberPositionX > worldMidMinX && bomberPositionX < worldMidMaxX);
			SpawnEnemyTypeAtLocation(bomber, bomberPositionX, 0.5f);
			enemyCount++;
		}
		for (int i = 0; i < 2; i++) {
			float mutantPositionX;
			do {
				mutantPositionX = x_range(gen);
			} while (mutantPositionX > worldMidMinX && mutantPositionX < worldMidMaxX);
			SpawnEnemyTypeAtLocation(mutant, mutantPositionX, 1.75f);
			enemyCount++;
		}
		//for (int i = 0; i < 2; i++) {
		//	float podPositionX;
		//	do {
		//		podPositionX = x_range(gen);
		//	} while (podPositionX > worldMidMinX && podPositionX < worldMidMaxX);
		//	SpawnEnemyTypeAtLocation(pod, podPositionX, 1.75f);
		//	enemyCount++;
		//}
	}

	// set enemies remaining for level
	game->get_mut<ENEMIES_REMAINING>()->numEnemies = enemyCount;

	// gameLock.UnlockSyncWrite();

	/*std::vector<flecs::entity> enemies;
	enemies.push_back(lander);
	enemies.push_back(mutant);
	enemies.push_back(bomber);
	enemies.push_back(pod);
	enemies.push_back(baiter);
	enemies.push_back(swarmer);
	enemies.push_back(human);*/

	// need some way to spawn different numbers of types of enemies
	//totalEnemies = enemies.size() - 3; // don't count human, baiter, swarmer
	//game->set<ENEMIES_REMAINING>({ totalEnemies });
	
	// spawn one of each enemy
	//for (auto enemy : enemies) {
	//	SpawnEnemyType(enemy);
	//}
}

void DSC::LevelLogic::ResetGameLevel() {
	game->set<ENEMIES_REMAINING>({ -1 });
	ResetLevelTimer();
	ResetEnemyBaiterSpawnTimedEvents();
	// hardcode current level at init
	currentLevel = 1;
	numSpaceLevelsCleared = -1;
	// hardcode level and humansalive level 1
	levelData.humansAlive = 5;
	levelData.level = 1;
	// push event to playerlogic for score reset
	DSC::PLAY_EVENT_DATA x;
	GW::GEvent resetPlayer;
	resetPlayer.Write(DSC::PLAY_EVENT::RESET_PLAYER, x);
	mainEventGen.Push(resetPlayer);
	
	
}

void DSC::LevelLogic::SpawnEnemyType(flecs::entity enemy) {
	// get spawn constraints of enemies
	std::random_device rd;  // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<float> x_range(worldxmin, worldxmax);
	std::uniform_real_distribution<float> y_range(worldymin, worldymax);
	float x_start = x_range(gen);
	float y_start = y_range(gen);
	// gamelock
	gameLock.LockSyncWrite();
	// add enemy
	gameAsync.entity().is_a(enemy)
		.set<Velocity>({ 0,0 })
		.set<Acceleration>({ 0,0 })
		.set<Position>({ x_start, y_start })
		.set_override<IsAlive>({ true });
	// be sure to unlock when done so the main thread can safely merge the changes
	gameLock.UnlockSyncWrite();
}

void DSC::LevelLogic::SpawnEnemyTypeAtLocation(flecs::entity enemy, float x_pos, float y_pos) {
	// gamelock
	gameLock.LockSyncWrite();
	// add enemy
	gameAsync.entity().is_a(enemy)
		.set<Velocity>({ 0,0 })
		.set<Acceleration>({ 0,0 })
		.set<Position>({ x_pos, y_pos })
		.set_override<IsAlive>({ true });
	// be sure to unlock when done so the main thread can safely merge the changes
	gameLock.UnlockSyncWrite();
}

// orient enemies based off their last x position
flecs::system DSC::LevelLogic::EnemyOrientationSystem() {
	return game->system<Enemy, Position, Orientation>()
		.each([this](flecs::entity& e, Enemy, Position& p, Orientation& o) {
			// face left
			if (p.value.x < o.lastX) {
				o.value.data[0] = abs(o.value.data[0]);
			}
			// face right
			else {
				o.value.data[0] = abs(o.value.data[0]) * -1.0f;
			}
			// update last x value
			o.lastX = p.value.x;
		});
}

flecs::system DSC::LevelLogic::HumanOrientationSystem() {
	return game->system<EnemyHuman, Position, Orientation>()
		.each([this](flecs::entity& e, EnemyHuman, Position& hp, Orientation& ho) {
			// face left
			if (hp.value.x < ho.lastX) {
				ho.value.data[0] = abs(ho.value.data[0]);
			}
			// face right
			else {
				ho.value.data[0] = abs(ho.value.data[0]) * -1.0f;
			}
			// update last x value
			ho.lastX = hp.value.x;
		});
}

// find all entities with position and loop them around the game world's min/max constraints
// will first limit up/down movement to constraints, but will need that to be flexible for enemies
flecs::system DSC::LevelLogic::ConstrainPositionInsideGameWorld(float xmin, float xmax, float ymin, float ymax) {
	return game->system<Position>().kind(flecs::PostUpdate)
		.each([xmin, xmax, ymin, ymax](flecs::entity e, Position& p) {
			// every object should traverse the x-axis
			if (p.value.x < xmin) {
				p.value.x += xmax;
			}
			else if (p.value.x > xmax) {
				p.value.x -= xmax;
			}
			// constrain player and enemy within y-axis, letting bullets go out of y-constaints for cleanup
			if (e.has<Player>() || (e.has<Enemy>() && !e.has<Captured>())) {
				if (p.value.y < ymin) {
					p.value.y = ymin;
				}
				else if (p.value.y > ymax) {
					p.value.y = ymax;
				}
			}
			else if (e.has<Enemy>()) {
				if (p.value.y < ymin) {
					p.value.y = ymin;
				}
			}
		});
}

// only enable firing from enemies near player (ideally within render range)
flecs::system DSC::LevelLogic::ActivateEnemiesNearPlayer() {
	return game->system<Enemy, Position>()
		.iter([this](flecs::iter it, Enemy*, Position* p) {
			float cameraPosition = abs(game->get<CAMERA_X_POSITION>()->value);
			if (it.world().entity("Player One").has<IsAlive>()) {
				for (auto i : it) {
					if (abs(p->value.x - cameraPosition) < 1.0f) {
						it.entity(i).add<IsActive>();
					}
					else {
						it.entity(i).remove<IsActive>();
					}
				}
			}
			// remove any enemy bullet out of camera render
			game->filter<EnemyBullet, Position>()
				.each([this, cameraPosition](flecs::entity& e, EnemyBullet, Position &p) {
					if (abs(p.value.x - cameraPosition) > 1.0f) {
						e.destruct();
					}
				});
			// remove any player bullet out of camera render
			game->filter<PlayerBullet, Position>()
				.each([this, cameraPosition](flecs::entity& e, PlayerBullet, Position& p) {
				if (abs(p.value.x - cameraPosition) > 1.0f) {
					e.destruct();
				}
			});
		});
}

void DSC::LevelLogic::DestroyActiveEnemies() {
	gameLock.LockAsyncRead();
	gameAsync.filter<Enemy, IsActive>()
		.each([](flecs::entity& e, Enemy, IsActive) {
			e.set<IsAlive>({ false });
		});
	gameLock.UnlockAsyncRead();
}

void DSC::LevelLogic::SpawnSwarmers(float xpos, float ypos) {
	flecs::entity swarmer;
	RetreivePrefab("Enemy Type Swarmer", swarmer);
	gameLock.LockSyncWrite();
	gameAsync.entity().is_a(swarmer)
		.set<Velocity>({ 0,0 })
		.set<Acceleration>({ 0,0 })
		.set<Position>({ xpos, ypos })
		.set_override<IsAlive>({ true });
	gameLock.UnlockSyncWrite();
}

// **** SAMPLE OF MULTI_THREADED USE ****
//flecs::world world; // main world
//flecs::world async_stage = world.async_stage();
//
//// From thread
//lock(async_stage_lock);
//flecs::entity e = async_stage.entity().child_of(parent)...
//unlock(async_stage_lock);
//
//// From main thread, periodic
//lock(async_stage_lock);
//async_stage.merge(); // merge all commands to main world
//unlock(async_stage_lock);
