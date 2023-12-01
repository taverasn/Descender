#include <random>
#include "EnemyBulletLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"

using namespace DSC;

// Connects logic to traverse any players and allow a controller to manipulate them
bool DSC::EnemyBulletLogic::Init(	std::shared_ptr<flecs::world> _game,
							std::weak_ptr<const GameConfig> _gameConfig)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;

	// destroy any bullets that have the CollidedWith relationship
	game->system<EnemyBullet>("Enemy Bullet System")
		.each([](flecs::entity e, EnemyBullet) {
		// destroy any player we contact
		e.each<CollidedWith>([&e](flecs::entity hit) {
			if (hit.has<Player>()) {
				//hit.set<IsAlive>({ false });
				if (e.has<EnemyBomb>()){
					GW::AUDIO::GSound bomb = e.get<EnemyBomb>()->explosion;
					bomb.Play();
				}
				e.destruct();
			}	
		});

		float life = e.get<Lifetime>()->value;
		life = life - e.delta_time();
		e.set<Lifetime>({ life });
		if (life <= 0.0f) {
			e.destruct();
		}
	});
	
	return true;
}

// Free any resources used to run this system
bool DSC::EnemyBulletLogic::Shutdown()
{
	game->entity("Enemy Bullet System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool DSC::EnemyBulletLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("Enemy Bullet System").enable();
	}
	else {
		game->entity("Enemy Bullet System").disable();
	}
	return false;
}
