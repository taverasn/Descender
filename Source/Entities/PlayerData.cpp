#include "PlayerData.h"
#include "../Components/Identification.h"
#include "../Components/Visuals.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "Prefabs.h"

bool DSC::PlayerData::Load(  std::shared_ptr<flecs::world> _game, 
                            std::weak_ptr<const GameConfig> _gameConfig,
							GW::AUDIO::GAudio _audioEngine)
{
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();
	// color
	float red = (*readCfg).at("Player1").at("red").as<float>();
	float green = (*readCfg).at("Player1").at("green").as<float>();
	float blue = (*readCfg).at("Player1").at("blue").as<float>();
	// start position
	float xstart = (*readCfg).at("Player1").at("xstart").as<float>();
	float ystart = (*readCfg).at("Player1").at("ystart").as<float>();
	float scale = (*readCfg).at("Player1").at("scale").as<float>();
	// amount of starting lives
	int lives = (*readCfg).at("Player1").at("lives").as<int>();
	// amount of starting bombs
	int bombs = (*readCfg).at("Player1").at("bombs").as<int>();
	
	float playerColliderSize = (*readCfg).at("Player1").at("colliderscale").as<float>();
	DSC::Collider playerCollider = { {{-playerColliderSize, -playerColliderSize},
		                              {-playerColliderSize, playerColliderSize},
		                              {playerColliderSize, playerColliderSize},
		                              {playerColliderSize, -playerColliderSize}} };
	
	std::string deathFX = (*readCfg).at("Player1").at("explode").as<std::string>();
	GW::AUDIO::GSound explosion;
	explosion.Create(deathFX.c_str(), _audioEngine, 0.075f);
	std::string awardFX = (*readCfg).at("Player1").at("award").as<std::string>();
	GW::AUDIO::GSound awardSound;
	awardSound.Create(awardFX.c_str(), _audioEngine, 0.15f);
	std::string nukeFX = (*readCfg).at("Player1").at("nuke").as<std::string>();
	GW::AUDIO::GSound nukeSound;
	nukeSound.Create(nukeFX.c_str(), _audioEngine, 0.10f); // TODO: we need a global music & sfx volumes
	std::string thrusterFX = (*readCfg).at("Player1").at("thruster").as<std::string>();
	GW::AUDIO::GSound thrusterSound;
	thrusterSound.Create(thrusterFX.c_str(), _audioEngine, 0.15f);

	// Create Player One
	_game->entity("Player One")
		.set([&](IsAlive& h, FacingDirection& f, Position& p, Orientation& o, Material& m, ControllerID& c, LifeCount& l, BombCount& b, Collider& d, Score& s) {
		h = { true };
		f = { false };
		c = { 0 };
		p = { xstart, ystart };
		m = {};
		o = { GW::MATH2D::GIdentityMatrix2F };
		l = { lives };
		b = { bombs };
		s = { 0 };
		d = { playerCollider };
		GW::MATH2D::GMatrix2D::Scale2F(o.value, GW::MATH2D::GVECTOR2F{ scale, scale }, o.value);
			})
		.add<Player>() // Tag this entity as a Player
		.add<Collidable>() // Make player collidable
		.set<GW::AUDIO::GSound>(explosion.Relinquish())
		.set<NukeSound>({nukeSound.Relinquish()})
		.set<ThrusterSound>({ thrusterSound.Relinquish() })
		.set<Award>({ awardSound.Relinquish() });

	return true;
}

bool DSC::PlayerData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all players
	_game->defer_begin(); // required when removing while iterating!
		_game->each([](flecs::entity e, Player&) {
			e.destruct(); // destroy this entitiy (happens at frame end)
		});
	_game->defer_end(); // required when removing while iterating!

    return true;
}
