#include "PowerUpData.h"
#include "../Components/Identification.h"
#include "../Components/visuals.h"
#include "../Components/Physics.h"
#include "../Entities/Prefabs.h"
#include "../Components/Gameplay.h"

bool DSC::PowerUpData::Load(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::AUDIO::GAudio _audioEngine)
{
	// Grab init settings
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();

	// Create prefab for powerup
	// color
	float red = (*readCfg).at("Power1").at("red").as<float>();
	float green = (*readCfg).at("Power1").at("green").as<float>();
	float blue = (*readCfg).at("Power1").at("blue").as<float>();
	// other attributes
	float xscale = (*readCfg).at("Power1").at("xscale").as<float>();
	float yscale = (*readCfg).at("Power1").at("yscale").as<float>();
	float angle = (*readCfg).at("Power1").at("angle").as<float>();
	
	// default projectile orientation & scale
	GW::MATH2D::GMATRIX2F world;
	GW::MATH2D::GMatrix2D::Rotate2F(GW::MATH2D::GIdentityMatrix2F,
		G_DEGREE_TO_RADIAN_F(angle), world);
	GW::MATH2D::GMatrix2D::Scale2F(world,
		GW::MATH2D::GVECTOR2F{ xscale, yscale }, world);
	

	// add prefab to ECS
	auto powerUpPrefab = _game->prefab((*readCfg).at("Power1").at("name").as<std::string>().c_str())
		// .set<> in a prefab means components are shared (instanced)
		.set<Material>({})
		.set<Orientation>({ world })
		// .override<> ensures a component is unique to each entity created from a prefab
		.override<Acceleration>()
		.override<Velocity>()
		.override<Position>()
		.override<PowerUp>() // Tag this prefab as a powerup (for queries/systems)
		.override<Collidable>(); // can be collided with

	// register this prefab by name so other systems can use it
	RegisterPrefab((*readCfg).at("Power1").at("name").as<std::string>().c_str(), powerUpPrefab);

	return true;
}

bool DSC::PowerUpData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all powerups and their prefabs
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, PowerUp&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

	return true;
}
