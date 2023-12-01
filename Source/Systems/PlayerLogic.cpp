#include "PlayerLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Visuals.h"
#include "../Components/Gameplay.h"
#include "../Entities/Prefabs.h"
#include "../Events/Playevents.h"
#include "../Events/Levelevents.h"
#include "../Systems/LevelLogic.h"

using namespace DSC; // descender namespace
using namespace GW::INPUT; // input libs
using namespace GW::AUDIO; // audio libs

// Connects logic to traverse any players and allow a controller to manipulate them
bool DSC::PlayerLogic::Init(	std::shared_ptr<flecs::world> _game, 
							std::weak_ptr<const GameConfig> _gameConfig, 
							GW::INPUT::GInput _immediateInput, 
							GW::INPUT::GBufferedInput _bufferedInput, 
							GW::INPUT::GController _controllerInput,
							GW::AUDIO::GAudio _audioEngine,
							GW::CORE::GEventGenerator _eventPusher,
							GW::CORE::GEventGenerator _levelEventGen)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	immediateInput = _immediateInput;
	bufferedInput = _bufferedInput;
	controllerInput =	_controllerInput;
	audioEngine = _audioEngine;
	levelEventGen = _levelEventGen;
	// Init any helper systems required for this task
	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	int width = (*readCfg).at("Window").at("width").as<int>();
	float verticalspeed = (*readCfg).at("Player1").at("verticalspeed").as<float>();
	float horizontalspeed = (*readCfg).at("Player1").at("horizontalspeed").as<float>();
	float minacceleration = (*readCfg).at("Player1").at("minacceleration").as<float>();
	float maxacceleration = (*readCfg).at("Player1").at("maxacceleration").as<float>();
	float xstart = (*readCfg).at("Player1").at("xstart").as<float>();
	float ystart = (*readCfg).at("Player1").at("ystart").as<float>();
	
	// initially match player facing direction to initial bullet direction
	SetPlayerFacingDirection(1, game->entity("Player One"));

	// add logic for updating players
	playerSystem = game->system<Player, Position, ControllerID>("Player System")
		.iter([this, verticalspeed, horizontalspeed, minacceleration, maxacceleration](flecs::iter it, Player*, Position* p, ControllerID* c) {
		for (auto i : it) {
			// left-right, up-down movement
			float xaxis = 0, yaxis = 0, xinput = 0, yinput = 0;
			// Use the controller/keyboard to move the player around the screen			
			if (c[i].index == 0) { // enable keyboard controls for player 1
				immediateInput.GetState(G_KEY_LEFT, xinput); xaxis -= xinput; 
				immediateInput.GetState(G_KEY_RIGHT, xinput); xaxis += xinput;
				immediateInput.GetState(G_KEY_DOWN, yinput); yaxis -= yinput;
				immediateInput.GetState(G_KEY_UP, yinput); yaxis += yinput;
			}
			// grab left-thumb stick
			controllerInput.GetState(c[i].index, G_LX_AXIS, xinput); xaxis += xinput;
			controllerInput.GetState(c[i].index, G_LY_AXIS, yinput); yaxis += yinput;
			controllerInput.GetState(c[i].index, G_DPAD_LEFT_BTN, xinput); xaxis -= xinput;
			controllerInput.GetState(c[i].index, G_DPAD_RIGHT_BTN, xinput); xaxis += xinput;
			controllerInput.GetState(c[i].index, G_DPAD_DOWN_BTN, yinput); yaxis -= yinput;
			controllerInput.GetState(c[i].index, G_DPAD_UP_BTN, yinput); yaxis += yinput;

			xaxis = G_LARGER(xaxis, -1);// cap right motion
			xaxis = G_SMALLER(xaxis, 1);// cap left motion
			yaxis = G_LARGER(yaxis, -1);// cap up motion
			yaxis = G_SMALLER(yaxis, 1);// cap down motion

			// only update facing direction if player has moved
			if (xaxis != 0) {
				SetPlayerFacingDirection(xaxis, it.entity(i));
			}

			// acceleration(fast the first half, slow the second half)
			if (abs(velocity) < 1)
			{
				velocity += xaxis * it.delta_time() * maxacceleration;
			}
			else
			{
				velocity += xaxis * it.delta_time() * minacceleration;
			}
			velocity = G_LARGER(velocity, -horizontalspeed);// cap horizontal speed to the right
			velocity = G_SMALLER(velocity, horizontalspeed);// cap horizontal speed to the left

			// inertia 
			if (xaxis == 0) 
			{
				if (velocity > 0)
				{
					velocity -= it.delta_time();
				}
				if (velocity < 0)
				{
					velocity += it.delta_time();
				}
				if (abs(velocity) < 0.004f)
				{
					velocity = 0;
				}
			}

			// horizontal movement
			p[i].value.x += it.delta_time() * velocity;
			// verical movement
			p[i].value.y += yaxis * it.delta_time() * verticalspeed;

			//when a player ship leave one side of the screen, it appears on the other
			//if (p[i].value.x < -1.05f)
			//	p[i].value.x = 1.0f;
			//if (p[i].value.x > 1.05f)
			//	p[i].value.x = -1.0f;
			//if (p[i].value.y < -1.05f)
			//	p[i].value.y = 1.0f;
			//if (p[i].value.y > 1.05f)
			//	p[i].value.y = -1.0f;

			// fire weapon if they are in a firing state
			if (it.entity(i).has<Firing>()) {
				
				controllerInput.StartVibration(c[i].index, 0, 0.1f, 1);

				CreatePlayerBullet(it.world(), it.entity(i), *it.entity(i).get<Position>());
				it.entity(i).remove<Firing>();
			}

			if (it.entity(i).has<Nuke>() && it.entity(i).get<BombCount>()->value > 0)
			{
				// need audio and particle effect for the nuke explosion
				// std::cout << "Nuke" << std::endl;
				// begin render flash effect
				game->add<FLASH_EFFECT>();
				it.entity(i).set<BombCount>({ it.entity(i).get<BombCount>()->value - 1 });
				DestroyAllActiveEnemies();
				it.entity(i).remove<Nuke>();
				GW::AUDIO::GSound nukeSound = it.entity(i).get<NukeSound>()->sound;
				nukeSound.Play();
				controllerInput.StartVibration(c[i].index, 0, 1, 1);
			}

			GW::AUDIO::GSound thrusterSound = it.entity(i).get<ThrusterSound>()->sound;
			thrusterSound.isPlaying(thrusterSoundPlaying);
			if ((xaxis != 0 || yaxis != 0) && !thrusterSoundPlaying) {
				thrusterSound.Play();
			}
			else if ((xaxis == 0 && yaxis == 0) && thrusterSoundPlaying) {
				thrusterSound.Stop();
			}
		}

		// process any cached button events after the loop (happens multiple times per frame)
		ProcessInputEvents(it.world());
	});


	auto b = game->system<EnemyHuman, PlayerCaptured>("Player Captures Human System")
		.each([this](flecs::entity& humanEntity, EnemyHuman& h, PlayerCaptured) {
			if (h.heTouchedMeBro.is_alive() && humanEntity.has<PlayerCaptured>())
			{
				float x = h.heTouchedMeBro.get<Position>()->value.x;
				float y = h.heTouchedMeBro.get<Position>()->value.y;
				humanEntity.set_override<Position>({ x, y - 0.1f });
			}
			else
			{
				humanEntity.remove<PlayerCaptured>();
				humanEntity.add<Falling>();
			}
		});
	systems.push_back(b);



	// player dies if hit by Enemy or EnemyBullet
	auto a = game->system<Player>("Health System")
		.each([this, xstart, ystart](flecs::entity e, Player){
			e.each<CollidedWith>([&e, xstart, ystart](flecs::entity hit){
				if (hit.has<Enemy>() || hit.has<EnemyBullet>()) {
					hit.set<IsAlive>({ false }); // make enemy hit die
					GW::AUDIO::GSound playerDeath = *e.get<GW::AUDIO::GSound>();
					playerDeath.Play();
					e.set<LifeCount>({ e.get<LifeCount>()->value - 1 });
					// std::cout << "Player Lives Left: " << e.get<LifeCount>()->value << "\n";
					e.set<Position>({ xstart, ystart });
					if (e.get<LifeCount>()->value <= 0)
					{
						e.set<IsAlive>({ false }); // make player die
					}
				}
			});
			
			// destroy player if not alive
			if (e.get<IsAlive>()->value == false)
			{
				game->entity("Player One").get_mut<Score>()->value = 0;
				game->entity("Player One").get_mut<AwardScore>()->value = 0;
				/*DSC::PLAY_EVENT_DATA x;
				GW::GEvent death;
				death.Write(DSC::PLAY_EVENT::PLAYER_DEAD, x);
				eventPusher.Push(death);*/

				GW::GEvent gameOver;
				DSC::LEVEL_EVENT_DATA y;
				gameOver.Write(DSC::LEVEL_EVENT::NEW_GAME, y);
				levelEventGen.Push(gameOver);

				//e.disable();
				//e.destruct();
			}
			
		});
	systems.push_back(a);

	// Create an event cache for when the spacebar/'A' button is pressed
	pressEvents.Create(Max_Frame_Events); // even 32 is probably overkill for one frame
		
	// register for keyboard and controller events
	bufferedInput.Register(pressEvents);
	controllerInput.Register(pressEvents);

	// create the on explode handler
	onExplode.Create([this, xstart, ystart](const GW::GEvent& e) {
		DSC::PLAY_EVENT event; DSC::PLAY_EVENT_DATA eventData;
		GW::AUDIO::GSound eventSound;

		if (+e.Read(event, eventData)) {
			bool decrementRemainingEnemyCounter = true;
			// only in here if event matches
			if (event == DSC::PLAY_EVENT::ENEMY_LANDER_DESTROYED) {
				game->entity("Player One").get_mut<Score>()->value += 150;
				game->entity("Player One").get_mut<AwardScore>()->value += 150;
				std::cout << "Lander Was Destroyed!\n";
			}
			else if (event == DSC::PLAY_EVENT::ENEMY_MUTANT_DESTROYED){
				game->entity("Player One").get_mut<Score>()->value += 150;
				game->entity("Player One").get_mut<AwardScore>()->value += 150;
				std::cout << "Mutant Was Destroyed!\n";
			}
			if (event == DSC::PLAY_EVENT::ENEMY_BAITER_DESTROYED) {
				game->entity("Player One").get_mut<Score>()->value += 200;
				game->entity("Player One").get_mut<AwardScore>()->value += 200;
				decrementRemainingEnemyCounter = false;
				std::cout << "Baiter Was Destroyed!\n";
			}
			else if (event == DSC::PLAY_EVENT::ENEMY_BOMBER_DESTROYED) {
				game->entity("Player One").get_mut<Score>()->value += 250;
				game->entity("Player One").get_mut<AwardScore>()->value += 250;
				std::cout << "Bomber Was Destroyed!\n";
			}
			if (event == DSC::PLAY_EVENT::ENEMY_POD_DESTROYED) {
				game->entity("Player One").get_mut<Score>()->value += 1000;
				game->entity("Player One").get_mut<AwardScore>()->value += 1000;
				std::cout << "Pod Was Destroyed!\n";
			}
			else if (event == DSC::PLAY_EVENT::ENEMY_SWARMER_DESTROYED) {
				game->entity("Player One").get_mut<Score>()->value += 150;
				game->entity("Player One").get_mut<AwardScore>()->value += 150;
				decrementRemainingEnemyCounter = false;
				std::cout << "Swarmer Was Destroyed!\n";
			}
			if (event == DSC::PLAY_EVENT::ENEMY_HUMAN_DESTROYED) {
				// decrement humans alive in leveldata
				game->add<HUMAN_KILLED>();
				decrementRemainingEnemyCounter = false;
				//std::cout << "Human Was Destroyed!\n";
			}
			else if (event == DSC::PLAY_EVENT::ENEMY_HUMAN_SAVED) {
				game->entity("Player One").get_mut<Score>()->value += 250;
				game->entity("Player One").get_mut<AwardScore>()->value += 250;
				decrementRemainingEnemyCounter = false;
				std::cout << "Human Was Saved!\n";
			}
			if (event == DSC::PLAY_EVENT::ENEMY_LANDER_ESCAPED) {
				// std::cout << "Lander abducted human!\n";
			}
			else if (event == DSC::PLAY_EVENT::RESET_PLAYER) {
				// reset score
				game->entity("Player One").get_mut<Score>()->value = 0;
				game->entity("Player One").get_mut<AwardScore>()->value = 0;
				// reset lives/bombs
				game->entity("Player One").get_mut<IsAlive>()->value = true;
				game->entity("Player One").get_mut<LifeCount>()->value = 3;
				game->entity("Player One").get_mut<BombCount>()->value = 3;
				// reset position
				game->entity("Player One").get_mut<Position>()->value.x = xstart;
				game->entity("Player One").get_mut<Position>()->value.y = ystart;
				// reset velocity
				game->entity("Player One").get_mut<Velocity>()->value.x = 0;
				game->entity("Player One").get_mut<Velocity>()->value.y = 0;
				velocity = 0;

			}
			if (game->entity("Player One").get<AwardScore>()->value >= 10000)
			{
				GW::AUDIO::GSound playerAward = game->entity("Player One").get<Award>()->award;
				playerAward.Play();
				game->entity("Player One").set<LifeCount>({ game->entity("Player One").get<LifeCount>()->value + 1 });
				game->entity("Player One").set<BombCount>({ game->entity("Player One").get<BombCount>()->value + 1 });
				game->entity("Player One").set<AwardScore>({ game->entity("Player One").get<AwardScore>()->value - 10000 });
			}
			
			// decrement number of alive enemies except when baiter/swarmer are killed
			if (decrementRemainingEnemyCounter) {
				game->get_mut<ENEMIES_REMAINING>()->numEnemies--;
			}

			std::cout << "Current Score: " << std::to_string(game->entity("Player One").get_mut<Score>()->value) << "\n";
		}
	});
	_eventPusher.Register(onExplode);

	return true;
}

bool DSC::PlayerLogic::GetPlayerState() 
{
	return isDead;
}
void DSC::PlayerLogic::SetPlayerState(bool _isDead)
{
	isDead = _isDead;
}
// Free any resources used to run this system
bool DSC::PlayerLogic::Shutdown()
{
	playerSystem.destruct();
	for (auto system : systems) {
		system.destruct();
	}
	game.reset();
	gameConfig.reset();

	return true;
}

// Toggle if a system's Logic is actively running
bool DSC::PlayerLogic::Activate(bool runSystem)
{
	if (playerSystem.is_alive()) {
		if (runSystem) {
			playerSystem.enable();
			for (auto system : systems) {
				system.enable();
			}
		}
		else {
			playerSystem.disable();
			for (auto system : systems) {
				system.disable();
			}
		}
		return true;
	}
	return false;
}

bool DSC::PlayerLogic::ProcessInputEvents(flecs::world& stage)
{
	// pull any waiting events from the event cache and process them
	GW::GEvent event;
	while (+pressEvents.Pop(event)) {
		bool fire = false;
		bool nuke = false;
		GController::Events controller;
		GController::EVENT_DATA c_data;
		GBufferedInput::Events keyboard;
		GBufferedInput::EVENT_DATA k_data;
		// these will only happen when needed
		if (+event.Read(keyboard, k_data)) {
			if (keyboard == GBufferedInput::Events::KEYPRESSED) {
				if (k_data.data == G_KEY_SPACE) {
					fire = true;
				}

				if (k_data.data == G_KEY_E) {
					if (game->entity("Player One").get<BombCount>()->value > 0) {
						nuke = true;
					}
				}
			}
		}
		else if (+event.Read(controller, c_data)) {
			if (controller == GController::Events::CONTROLLERBUTTONVALUECHANGED) {
				if (c_data.inputValue > 0 && c_data.inputCode == G_SOUTH_BTN)
					fire = true;
				if (c_data.inputValue > 0 && c_data.inputCode == G_EAST_BTN)
					nuke = true;
			}
		}
		if (fire) {
			// grab player one and set them to a firing state
			stage.entity("Player One").add<Firing>();
		}
		if (nuke) {
			// grab player one and set them to a nuking state
			stage.entity("Player One").add<Nuke>();
		}
	}
	return true;
}

// create player bullet and play sound
bool DSC::PlayerLogic::CreatePlayerBullet(flecs::world& stage, flecs::entity& e, const Position origin)
{
	// Grab the prefab for a laser round
	flecs::entity bullet;
	RetreivePrefab("Lazer Bullet", bullet);
		
	Velocity velocity;
	velocity.value.x = bullet.get<Velocity>()->value.x;
	velocity.value.y = bullet.get<Velocity>()->value.y;
	
	if (e.get<FacingDirection>()->value) {
		velocity.value.x = -1.0f * abs(velocity.value.x);
		bullet.get_mut<Orientation>()->value.data[0] = abs(bullet.get<Orientation>()->value.data[0]) * -1.0f;
	}
	else {
		velocity.value.x = abs(velocity.value.x);
		bullet.get_mut<Orientation>()->value.data[0] = abs(bullet.get<Orientation>()->value.data[0]);
	}

	bullet.set_override<Velocity>(velocity)
		.set<Position>(origin);

	auto laser = stage.entity().is_a(bullet);
		
	// play the sound of the Lazer prefab
	GW::AUDIO::GSound shoot = *bullet.get<GW::AUDIO::GSound>();
	shoot.Play();

	return true;
}

bool DSC::PlayerLogic::SetPlayerFacingDirection(const float& xaxis, flecs::entity& e) {
	// get direction player is facing
	auto o = e.get<Orientation>()->value;
	if (xaxis < 0) {
		// face left is 'true'
		e.set<FacingDirection>({ true });
		o.data[0] = abs(o.data[0]);
	}
	else {
		// face right is 'false'
		e.set<FacingDirection>({ false });
		o.data[0] = abs(o.data[0]) * -1.0f;
	}
	e.set<Orientation>({ o });
	return true;
}

void DSC::PlayerLogic::DestroyAllActiveEnemies()
{
	GW::GEvent clearEntities;
	DSC::LEVEL_EVENT_DATA x;
	clearEntities.Write(DSC::LEVEL_EVENT::CLEAR_ENTITIES, x);
	levelEventGen.Push(clearEntities);
	/*game->filter<Enemy, IsAlive, IsActive>()
		.each([](flecs::entity& e, Enemy, IsAlive, IsActive) {
			e.set<IsAlive>({ false });
		});*/
}