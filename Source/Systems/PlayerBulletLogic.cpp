#include <random>
#include "PlayerBulletLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"

using namespace DSC; // descender namespace

// Connects logic to traverse any players and allow a controller to manipulate them
bool DSC::PlayerBulletLogic::Init(	std::shared_ptr<flecs::world> _game,
							std::weak_ptr<const GameConfig> _gameConfig)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;

	// destroy any bullets that have the CollidedWith relationship except with player
	game->system<PlayerBullet>("PlayerBullet System")
		.each([](flecs::entity e, PlayerBullet) {
			// damage anything we come into contact with
			e.each<CollidedWith>([&e](flecs::entity hit) {
				if (hit.has<Enemy>() || hit.has<EnemyHuman>()) { // if hit enemy
					hit.set<IsAlive>({ false });
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
bool DSC::PlayerBulletLogic::Shutdown()
{
	game->entity("PlayerBullet System").destruct();
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool DSC::PlayerBulletLogic::Activate(bool runSystem)
{
	if (runSystem) {
		game->entity("PlayerBullet System").enable();
	}
	else {
		game->entity("PlayerBullet System").disable();
	}
	return false;
}
