#include "EnemyBulletData.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "Prefabs.h"
#include "../Components/Gameplay.h"

bool DSC::EnemyBulletData::Load(	std::shared_ptr<flecs::world> _game,
							std::weak_ptr<const GameConfig> _gameConfig,
							GW::AUDIO::GAudio _audioEngine)
{
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	
	// attributes for enemy lander bullet weapon
	float bulletScaleX = (*readCfg).at("EnemyBullet").at("xscale").as<float>();
	float bulletScaleY = (*readCfg).at("EnemyBullet").at("yscale").as<float>();
	float bulletColliderSize = (*readCfg).at("EnemyBullet").at("colliderscale").as<float>();
	float bulletAngle = (*readCfg).at("EnemyBullet").at("angle").as<float>();
	float bulletLifetime = (*readCfg).at("EnemyBullet").at("lifetime").as<float>();
	DSC::Collider bulletCollider = { {{-bulletColliderSize, -bulletColliderSize},
		                              {-bulletColliderSize, bulletColliderSize},
		                              {bulletColliderSize, bulletColliderSize},
		                              {bulletColliderSize, -bulletColliderSize}} };

	// attributes for enemy bomb weapon, sharing scale/angle info for now
	float bombLifetime = (*readCfg).at("EnemyBomb").at("lifetime").as<float>();
	float bombScaleX = (*readCfg).at("EnemyBomb").at("xscale").as<float>();
	float bombScaleY = (*readCfg).at("EnemyBomb").at("yscale").as<float>();
	float bombAngle = (*readCfg).at("EnemyBomb").at("angle").as<float>();
	float bombColliderSize = (*readCfg).at("EnemyBomb").at("colliderscale").as<float>();
	std::string bombFX = (*readCfg).at("EnemyBomb").at("explode").as<std::string>();
	DSC::Collider bombCollider = { {{-bombColliderSize, -bombColliderSize},
									{-bombColliderSize,  bombColliderSize},
								 	 {bombColliderSize,  bombColliderSize},
									 {bombColliderSize, -bombColliderSize}} };
	
	GW::MATH2D::GMATRIX2F world;
	GW::MATH2D::GMatrix2D::Rotate2F(GW::MATH2D::GIdentityMatrix2F,
		G_DEGREE_TO_RADIAN_F(bulletAngle), world);
	GW::MATH2D::GMatrix2D::Scale2F(GW::MATH2D::GIdentityMatrix2F, 
		GW::MATH2D::GVECTOR2F{ bulletScaleX, bulletScaleY }, world);
	// add enemy bullet prefab
	auto enemyBulletPrefab = _game->prefab((*readCfg).at("EnemyBullet").at("name").as<std::string>().c_str())
		// .set<> in a prefab means components are shared (instanced)
		.set<Material>({})
		.set_override<Orientation>({ world })
		.set<Acceleration>({ 0, 0 })
		.set_override<Lifetime>({ bulletLifetime} )
		// .override<> ensures a component is unique to each entity created from a prefab 
		.override<Velocity>()
		.override<Position>()
		.set_override<Collider>({ bulletCollider })
		.override<EnemyBullet>() // Tag this prefab as a bullet (for queries/systems)
		.override<Collidable>(); // can be collided with
	// register this prefab by name so other systems can use it
	RegisterPrefab((*readCfg).at("EnemyBullet").at("name").as<std::string>().c_str(), enemyBulletPrefab);

	GW::MATH2D::GMatrix2D::Rotate2F(GW::MATH2D::GIdentityMatrix2F,
		G_DEGREE_TO_RADIAN_F(bombAngle), world);
	GW::MATH2D::GMatrix2D::Scale2F(world,
		GW::MATH2D::GVECTOR2F{ bombScaleX, bombScaleY }, world);
	GW::AUDIO::GSound bombImpact;
	bombImpact.Create(bombFX.c_str(), _audioEngine, 0.15f); // TODO: we need a global music & sfx volumes
	// create bomb-bullet prefab and register
	// TODO: needs separate soundfx
	auto enemyBombPrefab = _game->prefab((*readCfg).at("EnemyBomb").at("name").as<std::string>().c_str())
		// .set<> in a prefab means components are shared (instanced)
		.set<Material>({})
		.set_override<Orientation>({ world })
		.set<Acceleration>({ 0, 0 })
		.set<EnemyBomb>({ bombImpact.Relinquish() })
		.set_override<Lifetime>({ bombLifetime })
		// .override<> ensures a component is unique to each entity created from a prefab 
		.override<Velocity>()
		.override<Position>()
		.set_override<Collider>( bombCollider )
		.override<EnemyBullet>() // Tag this prefab as a bullet (for queries/systems)
		.override<Collidable>(); // can be collided with
	// register this prefab by name so other systems can use it
	RegisterPrefab((*readCfg).at("EnemyBomb").at("name").as<std::string>().c_str(), enemyBombPrefab);

	return true;
}

bool DSC::EnemyBulletData::Unload(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig)
{
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	// remove all bullets and their prefabs
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, EnemyBullet&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
	});
	_game->defer_end(); // required when removing while iterating!

	// unregister this prefab by name
	UnregisterPrefab((*readCfg).at("EnemyBullet").at("name").as<std::string>().c_str());

	return true;
}
