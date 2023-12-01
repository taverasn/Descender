#include <random>
#include "PowerUpLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"

using namespace DSC;

bool DSC::PowerUpLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	eventPusher = _eventPusher;

	// human alive system
	game->system<EnemyHuman, IsAlive>("Human Alive System")
		.each([this](flecs::entity& e, EnemyHuman, IsAlive) {
			if (e.get<IsAlive>()->value == false) {
				DSC::PLAY_EVENT_DATA x;
				GW::GEvent humanDeath;
				humanDeath.Write(DSC::PLAY_EVENT::ENEMY_HUMAN_DESTROYED, x);
				eventPusher.Push(humanDeath);
				GW::AUDIO::GSound enemyDeath = *e.get<GW::AUDIO::GSound>();
				enemyDeath.Play();
				e.destruct();
			}
		});

	// human movement system
	game->system<EnemyHuman, Position>("Human System")
		.each([this](flecs::entity e, EnemyHuman, Position& p) {
			
			float ground = 0.1f;
			float dt = e.delta_time();
			float humanGroundedSpeed = 0.1f;

			std::random_device rd;  // Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()

			std::uniform_real_distribution<float> x_range(1.0f, 11.0f);

			if (e.has<Destination>() && e.get<Destination>()->value == -1)
			{
				float wtf = x_range(gen);
				e.get_mut<Destination>()->value = wtf;
			}

			float currentDestination = e.get<Destination>()->value;

			if (!e.has<Captured>())
			{
				//adding falling state if a human is in the air
				if (!e.has<PlayerCaptured>() && !e.has<Falling>() && p.value.y > ground)
				{
					if (e.has<Grounded>())
					{
						e.remove<Grounded>();
					}
					e.add<Falling>();
					//std::cout << "Human falling" << std::endl;
				}
				else if (!e.has<PlayerCaptured>() && !e.has<Grounded>() && e.has<Falling>() && p.value.y < ground)
				{
					// if the human is in falling state and it hits the ground then it dies
					//std::cout << "Human died from falling" << std::endl;
					e.set<IsAlive>({ false });
				}


				if (p.value.y <= ground)
				{
					// adding grounded tag if the human is on the ground level 
					if (!e.has<Grounded>())
					{
						e.add<Grounded>();
						e.remove<CollidedWith>(game->entity("Player One"));
						//std::cout << "Human grounded" << std::endl;
					}
					if (e.has<PlayerCaptured>())
					{
						e.remove<PlayerCaptured>();
						e.get_mut<EnemyHuman>()->heTouchedMeBro.remove<PlayerCaptured>();
						DSC::PLAY_EVENT_DATA x;
						GW::GEvent humanSaved;
						humanSaved.Write(DSC::PLAY_EVENT::ENEMY_HUMAN_SAVED, x);
						eventPusher.Push(humanSaved);
						//std::cout << "Human is no longer captured by the player" << std::endl;
					}
					// handle grounded state
					if (p.value.x > (currentDestination - 1) && p.value.x < (currentDestination + 1))
					{
						//std::cout << "Human is at the destination. New Destination is " << currentDestination << std::endl;
						e.set_override<Destination>({ x_range(gen) });
					}
					if (p.value.x > currentDestination)
					{
						p.value.x -= humanGroundedSpeed * dt;
					}
					if (p.value.x < currentDestination)
					{
						p.value.x += humanGroundedSpeed * dt;
					}
				}

				// handle falling state
				if (e.has<Falling>() && !e.has<PlayerCaptured>())
				{
					//will fall... if hits the ground then deadge 
					p.value.y -= fallingVelocity * e.delta_time();
				}
			}

			e.each<CollidedWith>([&e, &p](flecs::entity EnemyEntity) {
				if (!EnemyEntity.has<Captured>() && EnemyEntity.has<EnemyLander>()) {
					e.add<Captured>();
					e.remove<Grounded>();
					EnemyEntity.add<Captured>();
					e.set<EnemyHuman>({ EnemyEntity });
					//std::cout << "HUMAN IS CAPTURED BY LANDER\n";
					// remove enemy from collide cache
					e.remove<CollidedWith>(EnemyEntity);
				}
			});

			e.each<CollidedWith>([&e, &p](flecs::entity PlayerEntity) {
				if (!e.has<Captured>() && !e.has<PlayerCaptured>() && PlayerEntity.has<Player>() && !e.has<Grounded>() && !PlayerEntity.has<PlayerCaptured>()) {
					e.add<PlayerCaptured>();
					e.remove<Falling>();
					PlayerEntity.add<PlayerCaptured>();
					e.set<EnemyHuman>({ PlayerEntity });
					//std::cout << "HUMAN IS CAPTURED BY PLAYER\n";

				}
			});
		});

	return true;
}

// Free any resources used to run this system
bool DSC::PowerUpLogic::Shutdown()
{
	game->entity("Human System").destruct();
	game->entity("Human Alive System").destruct();
	//game->entity("Human Collisions").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool DSC::PowerUpLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Human System").enable();
		game->entity("Human Alive System").enable();
		//game->entity("Human Collisions").enable();
	}
	else {
		game->entity("Human System").disable();
		game->entity("Human Alive System").disable();
		//game->entity("Human Collisions").disable();
	}
	return false;
}