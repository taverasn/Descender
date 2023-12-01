#include "EnemyData.h"
#include "../Components/Identification.h"
#include "../Components/visuals.h"
#include "../Components/Physics.h"
#include "../Entities/Prefabs.h"
#include "../Components/Gameplay.h"

bool DSC::EnemyData::Load(	std::shared_ptr<flecs::world> _game,
							std::weak_ptr<const GameConfig> _gameConfig,
							GW::AUDIO::GAudio _audioEngine)
{
	// Grab init settings for players
	std::shared_ptr<const GameConfig> readCfg = _gameConfig.lock();

	// color
	float red = (*readCfg).at("Enemy1").at("red").as<float>();
	float green = (*readCfg).at("Enemy1").at("green").as<float>();
	float blue = (*readCfg).at("Enemy1").at("blue").as<float>();
	// other attributes
	float angle = (*readCfg).at("Enemy1").at("angle").as<float>();
	
	std::string landerDeathFX = (*readCfg).at("Enemy1").at("death").as<std::string>();
	GW::AUDIO::GSound landerDestroyed;
	landerDestroyed.Create(landerDeathFX.c_str(), _audioEngine, 0.15f);
	std::string bomberDeathFX = (*readCfg).at("EnemyBomber").at("death").as<std::string>();
	GW::AUDIO::GSound bomberDestroyed;
	bomberDestroyed.Create(bomberDeathFX.c_str(), _audioEngine, 0.15f);
	std::string mutantDeathFX = (*readCfg).at("EnemyMutant").at("death").as<std::string>();
	GW::AUDIO::GSound mutantDestroyed;
	mutantDestroyed.Create(mutantDeathFX.c_str(), _audioEngine, 0.15f);
	std::string baiterDeathFX = (*readCfg).at("EnemyBaiter").at("death").as<std::string>();
	GW::AUDIO::GSound baiterDestroyed;
	baiterDestroyed.Create(baiterDeathFX.c_str(), _audioEngine, 0.15f);
	std::string podDeathFX = (*readCfg).at("EnemyPod").at("death").as<std::string>();
	GW::AUDIO::GSound podDestroyed;
	podDestroyed.Create(podDeathFX.c_str(), _audioEngine, 0.15f);
	std::string swarmerDeathFX = (*readCfg).at("EnemySwarmer").at("death").as<std::string>();
	GW::AUDIO::GSound swarmerDestroyed;
	swarmerDestroyed.Create(swarmerDeathFX.c_str(), _audioEngine, 0.15f);
	std::string humanDeathFX = (*readCfg).at("EnemyHuman").at("death").as<std::string>();
	GW::AUDIO::GSound humanDestroyed;
	humanDestroyed.Create(humanDeathFX.c_str(), _audioEngine, 0.15f);
	
	std::string landerFX = (*readCfg).at("Enemy1").at("fireFX").as<std::string>();
	GW::AUDIO::GSound shoot;
	shoot.Create(landerFX.c_str(), _audioEngine, 0.15f);
	std::string bombShootFX = (*readCfg).at("EnemyBomber").at("fireFX").as<std::string>();
	GW::AUDIO::GSound bombShoot;
	bombShoot.Create(bombShootFX.c_str(), _audioEngine, 0.15f);
	std::string mutantFX = (*readCfg).at("EnemyMutant").at("fireFX").as<std::string>();
	GW::AUDIO::GSound mutantShoot;
	mutantShoot.Create(mutantFX.c_str(), _audioEngine, 0.15f);
	std::string baiterFX = (*readCfg).at("EnemyBaiter").at("fireFX").as<std::string>();
	GW::AUDIO::GSound baiterShoot;
	baiterShoot.Create(baiterFX.c_str(), _audioEngine, 0.15f);
	std::string swarmerFX = (*readCfg).at("EnemySwarmer").at("fireFX").as<std::string>();
	GW::AUDIO::GSound swarmerShoot;
	swarmerShoot.Create(swarmerFX.c_str(), _audioEngine, 0.15f);
			
	// enemy prefab variables
	// lander
	float enemyLanderFirerate = (*readCfg).at("Enemy1").at("firerate").as<float>();
	float enemyLanderAccuracy = (*readCfg).at("Enemy1").at("accuracy").as<float>();
	float enemyLanderBulletSpeed = (*readCfg).at("Enemy1").at("bulletspeed").as<float>();
	float enemyLanderScaleX = (*readCfg).at("Enemy1").at("xscale").as<float>();
	float enemyLanderScaleY = (*readCfg).at("Enemy1").at("yscale").as<float>();
	float enemyLanderColliderSize = (*readCfg).at("Enemy1").at("colliderscale").as<float>();
	// bomber
	float enemyBomberFirerate = (*readCfg).at("EnemyBomber").at("firerate").as<float>();
	float enemyBomberScaleX = (*readCfg).at("EnemyBomber").at("xscale").as<float>();
	float enemyBomberScaleY = (*readCfg).at("EnemyBomber").at("yscale").as<float>();
	float enemyBomberColliderSize = (*readCfg).at("EnemyBomber").at("colliderscale").as<float>();
	// mutant
	float enemyMutantFirerate = (*readCfg).at("EnemyMutant").at("firerate").as<float>();
	float enemyMutantAccuracy = (*readCfg).at("EnemyMutant").at("accuracy").as<float>();
	float enemyMutantBulletSpeed = (*readCfg).at("EnemyMutant").at("bulletspeed").as<float>();
	float enemyMutantScaleX = (*readCfg).at("EnemyMutant").at("xscale").as<float>();
	float enemyMutantScaleY = (*readCfg).at("EnemyMutant").at("yscale").as<float>();
	float enemyMutantColliderSize = (*readCfg).at("EnemyMutant").at("colliderscale").as<float>();
	// baiter
	float enemyBaiterFirerate = (*readCfg).at("EnemyBaiter").at("firerate").as<float>();
	float enemyBaiterAccuracy = (*readCfg).at("EnemyBaiter").at("accuracy").as<float>();
	float enemyBaiterBulletSpeed = (*readCfg).at("EnemyBaiter").at("bulletspeed").as<float>();
	float enemyBaiterScaleX = (*readCfg).at("EnemyBaiter").at("xscale").as<float>();
	float enemyBaiterScaleY = (*readCfg).at("EnemyBaiter").at("yscale").as<float>();
	float enemyBaiterColliderSize = (*readCfg).at("EnemyBaiter").at("colliderscale").as<float>();
	// pod
	float enemyPodScaleX = (*readCfg).at("EnemyPod").at("xscale").as<float>();
	float enemyPodScaleY = (*readCfg).at("EnemyPod").at("yscale").as<float>();
	float enemyPodColliderSize = (*readCfg).at("EnemyPod").at("colliderscale").as<float>();
	// swarmer
	float enemySwarmerFirerate = (*readCfg).at("EnemySwarmer").at("firerate").as<float>();
	float enemySwarmerAccuracy = (*readCfg).at("EnemySwarmer").at("accuracy").as<float>();
	float enemySwarmerBulletSpeed = (*readCfg).at("EnemySwarmer").at("bulletspeed").as<float>();
	float enemySwarmerScaleX = (*readCfg).at("EnemySwarmer").at("xscale").as<float>();
	float enemySwarmerScaleY = (*readCfg).at("EnemySwarmer").at("yscale").as<float>();
	float enemySwarmerColliderSize = (*readCfg).at("EnemySwarmer").at("colliderscale").as<float>();
	// human
	float enemyHumanScaleX = (*readCfg).at("EnemyHuman").at("xscale").as<float>();
	float enemyHumanScaleY = (*readCfg).at("EnemyHuman").at("yscale").as<float>();
	float enemyHumanColliderSize = (*readCfg).at("EnemyHuman").at("colliderscale").as<float>();
	float enemyHumanDestination = (*readCfg).at("EnemyHuman").at("colliderscale").as<float>();
	// default prefab orientation & scale

	GW::MATH2D::GMATRIX2F world;
	GW::MATH2D::GMatrix2D::Rotate2F(GW::MATH2D::GIdentityMatrix2F, 
		G_DEGREE_TO_RADIAN_F(angle), world);
	GW::MATH2D::GMatrix2D::Scale2F(world,
		GW::MATH2D::GVECTOR2F{ enemyLanderScaleX, enemyLanderScaleY }, world);
	// create lander prefab and register
	auto enemyPrefab = _game->prefab((*readCfg).at("Enemy1").at("name").as<std::string>().c_str())
		// .set<> in a prefab means components are shared (instanced)
		.set<Material>({})
		.set_override<Orientation>({ world })
		.add<EnemyLander>()
		// .override<> ensures a component is unique to each entity created from a prefab
		.set_override<Firerate>({ enemyLanderFirerate, enemyLanderFirerate })
		.set_override<Accuracy>({ enemyLanderAccuracy })
		.set_override<BulletSpeed>({ enemyLanderBulletSpeed })
		.override<Acceleration>()
		.override<Velocity>()
		.override<Position>()
		.override<Enemy>() // Tag this prefab as an enemy (for queries/systems)
		.set_override<IsAlive>({true})
		.override<Collidable>() // can be collided with
		.set_override<Collider>(BuildCollider(enemyLanderColliderSize, enemyLanderColliderSize))
		.set<GW::AUDIO::GSound>(landerDestroyed.Relinquish())
		.set<FiringSound>({ shoot.Relinquish() });
	// register this prefab by name so other systems can use it
	RegisterPrefab((*readCfg).at("Enemy1").at("name").as<std::string>().c_str(), enemyPrefab);

	world = GW::MATH2D::GIdentityMatrix2F;
	GW::MATH2D::GMatrix2D::Scale2F(world,
		GW::MATH2D::GVECTOR2F{ enemyBomberScaleX, enemyBomberScaleY }, world);
	// create bomber prefab and register
	auto enemyBomberPrefab = _game->prefab((*readCfg).at("EnemyBomber").at("name").as<std::string>().c_str())
		.set<Material>({})
		.set_override<Orientation>({ world })
		.add<EnemyBomber>()
		.set_override<Firerate>({ enemyBomberFirerate, enemyBomberFirerate })
		.set_override<Accuracy>({ 0 })
		.set_override<BulletSpeed>({ 0 }) // stationary bullet
		.override<Acceleration>()
		.override<Velocity>()
		.override<Position>()
		.override<Enemy>() // Tag this prefab as an enemy (for queries/systems)
		.set_override<IsAlive>({ true })
		.override<Collidable>() // can be collided with
		.set_override<Collider>(BuildCollider(enemyBomberColliderSize, enemyBomberColliderSize))
		.set<GW::AUDIO::GSound>(bomberDestroyed.Relinquish())
		.set<FiringSound>({ bombShoot.Relinquish() });
	RegisterPrefab((*readCfg).at("EnemyBomber").at("name").as<std::string>().c_str(), enemyBomberPrefab);

	world = GW::MATH2D::GIdentityMatrix2F;
	GW::MATH2D::GMatrix2D::Scale2F(world,
		GW::MATH2D::GVECTOR2F{ enemyMutantScaleX, enemyMutantScaleY }, world);
	// create mutant prefab and register
	auto enemyMutantPrefab = _game->prefab((*readCfg).at("EnemyMutant").at("name").as<std::string>().c_str())
		.set<Material>({})
		.set_override<Orientation>({ world })
		.add<EnemyMutant>()
		.set_override<Firerate>({ enemyMutantFirerate, enemyMutantFirerate })
		.set_override<Accuracy>({ enemyMutantAccuracy })
		.set_override<BulletSpeed>({ enemyMutantBulletSpeed })
		.override<Acceleration>()
		.override<Velocity>()
		.override<Position>()
		.override<Enemy>() // Tag this prefab as an enemy (for queries/systems)
		.set_override<IsAlive>({ true })
		.override<Collidable>() // can be collided with
		.set_override<Collider>(BuildCollider(enemyMutantColliderSize, enemyMutantColliderSize))
		.set<GW::AUDIO::GSound>(mutantDestroyed.Relinquish())
		.set<FiringSound>({ mutantShoot.Relinquish() });
	RegisterPrefab((*readCfg).at("EnemyMutant").at("name").as<std::string>().c_str(), enemyMutantPrefab);

	world = GW::MATH2D::GIdentityMatrix2F;
	GW::MATH2D::GMatrix2D::Scale2F(world,
		GW::MATH2D::GVECTOR2F{ enemyBaiterScaleX, enemyBaiterScaleY }, world);
	// create baiter prefab and register
	auto enemyBaiterPrefab = _game->prefab((*readCfg).at("EnemyBaiter").at("name").as<std::string>().c_str())
		.set<Material>({})
		.set_override<Orientation>({ world })
		.add<EnemyBaiter>()
		.set_override<Firerate>({ enemyBaiterFirerate, enemyBaiterFirerate })
		.set_override<Accuracy>({ enemyBaiterAccuracy })
		.set_override<BulletSpeed>({ enemyBaiterBulletSpeed })
		.override<Acceleration>()
		.override<Velocity>()
		.override<Position>()
		.override<Enemy>() // Tag this prefab as an enemy (for queries/systems)
		.set_override<IsAlive>({ true })
		.override<Collidable>() // can be collided with
		.set_override<Collider>(BuildCollider(enemyBaiterColliderSize, enemyBaiterColliderSize))
		.set<GW::AUDIO::GSound>(baiterDestroyed.Relinquish())
		.set<FiringSound>({ baiterShoot.Relinquish() });
	RegisterPrefab((*readCfg).at("EnemyBaiter").at("name").as<std::string>().c_str(), enemyBaiterPrefab);

	world = GW::MATH2D::GIdentityMatrix2F;
	GW::MATH2D::GMatrix2D::Scale2F(world,
		GW::MATH2D::GVECTOR2F{ enemyPodScaleX, enemyPodScaleY }, world);
	// create pod prefab and register
	auto enemyPodPrefab = _game->prefab((*readCfg).at("EnemyPod").at("name").as<std::string>().c_str())
		.set<Material>({})
		.set_override<Orientation>({ world })
		.add<EnemyPod>()
		// .set_override<Firerate>({ }) // doesn't fire
		// .set_override<Accuracy>({ 0 })
		// .set_override<BulletSpeed>({ 0 })
		.override<Acceleration>()
		.override<Velocity>()
		.override<Position>()
		.override<Enemy>() // Tag this prefab as an enemy (for queries/systems)
		.set_override<IsAlive>({ true })
		.override<Collidable>() // can be collided with
		.set_override<Collider>(BuildCollider(enemyPodColliderSize, enemyPodColliderSize))
		.set<GW::AUDIO::GSound>(podDestroyed.Relinquish());
	RegisterPrefab((*readCfg).at("EnemyPod").at("name").as<std::string>().c_str(), enemyPodPrefab);

	world = GW::MATH2D::GIdentityMatrix2F;
	GW::MATH2D::GMatrix2D::Scale2F(world,
		GW::MATH2D::GVECTOR2F{ enemySwarmerScaleX, enemySwarmerScaleY }, world);
	// create swarmer prefab and register
	auto enemySwarmerPrefab = _game->prefab((*readCfg).at("EnemySwarmer").at("name").as<std::string>().c_str())
		.set<Material>({})
		.set_override<Orientation>({ world })
		.add<EnemySwarmer>()
		.set_override<Firerate>({ enemySwarmerFirerate, enemySwarmerFirerate })
		.set_override<Accuracy>({ enemySwarmerAccuracy })
		.set_override<BulletSpeed>({ enemySwarmerBulletSpeed })
		.override<Acceleration>()
		.override<Velocity>()
		.override<Position>()
		.override<Enemy>() // Tag this prefab as an enemy (for queries/systems)
		.set_override<IsAlive>({ true })
		.override<Collidable>() // can be collided with
		.set_override<Collider>(BuildCollider(enemySwarmerColliderSize, enemySwarmerColliderSize))
		.set<GW::AUDIO::GSound>(swarmerDestroyed.Relinquish())
		.set<FiringSound>({ swarmerShoot.Relinquish() });
	RegisterPrefab((*readCfg).at("EnemySwarmer").at("name").as<std::string>().c_str(), enemySwarmerPrefab);

	world = GW::MATH2D::GIdentityMatrix2F;
	GW::MATH2D::GMatrix2D::Scale2F(world,
		GW::MATH2D::GVECTOR2F{ enemyHumanScaleX, enemyHumanScaleY }, world);
	// create human prefab and register
	auto enemyHumanPrefab = _game->prefab((*readCfg).at("EnemyHuman").at("name").as<std::string>().c_str())
		.set<Material>({})
		.set_override<Orientation>({ world })
		.add<EnemyHuman>() // TODO: need a check for player projectile on this tag
		//.set_override<Firerate>({ }) // doesn't fire
		//.set_override<Accuracy>({ })
		//.set_override<BulletSpeed>({ })
		.set_override<Destination>({ -1.0f })
		//.override<Destination>()
		.override<Acceleration>()
		.override<Velocity>()
		.override<Position>()
		//.override<Enemy>()
		.set_override<IsAlive>({ true })
		.override<Collidable>() // can be collided with
		.set_override<Collider>(BuildCollider(enemyHumanColliderSize, enemyHumanColliderSize))
		.set<GW::AUDIO::GSound>(humanDestroyed.Relinquish());
	RegisterPrefab((*readCfg).at("EnemyHuman").at("name").as<std::string>().c_str(), enemyHumanPrefab);

	prefabs.push_back(enemyPrefab);
	prefabs.push_back(enemyBomberPrefab);
	prefabs.push_back(enemyMutantPrefab);
	prefabs.push_back(enemyBaiterPrefab);
	prefabs.push_back(enemyPodPrefab);
	prefabs.push_back(enemySwarmerPrefab);
	prefabs.push_back(enemyHumanPrefab);

	return true;
}

bool DSC::EnemyData::Unload(std::shared_ptr<flecs::world> _game)
{
	// remove all entities with the Enemy component and their prefabs
	_game->defer_begin(); // required when removing while iterating!
	_game->each([](flecs::entity e, Enemy&) {
		e.destruct(); // destroy this entitiy (happens at frame end)
	});
	_game->each([](flecs::entity e, EnemyHuman&) {
		e.destruct();
	});
	_game->defer_end(); // required when removing while iterating!

	for (auto prefab : prefabs) {
		UnregisterPrefab(prefab.name());
	}

	return true;
}

// {{ -0.1f, -0.1f }, { -0.1f, 0.1f }, { 0.1f, 0.1f }, { 0.1f, -0.1f }}
DSC::Collider DSC::EnemyData::BuildCollider(float scalex, float scaley) {
	DSC::Collider c;
	c.poly[0].x = -scalex;
	c.poly[0].y = -scaley;
	c.poly[1].x = -scalex;
	c.poly[1].y = scaley;
	c.poly[2].x = scalex;
	c.poly[2].y = scaley;
	c.poly[3].x = scalex;
	c.poly[3].y = -scaley;
	return c;
}