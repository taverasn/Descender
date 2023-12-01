#include <random>
#include "EnemyLogic.h"
#include "../Components/Identification.h"
#include "../Components/Physics.h"
#include "../Components/Gameplay.h"
#include "../Events/Playevents.h"
#include "../Entities/Prefabs.h"
#include "../Events/Levelevents.h"


using namespace DSC; // descender namespace

// Connects logic to traverse any players and allow a controller to manipulate them
bool DSC::EnemyLogic::Init(std::shared_ptr<flecs::world> _game,
	std::weak_ptr<const GameConfig> _gameConfig,
	GW::CORE::GEventGenerator _eventPusher,
	GW::CORE::GEventGenerator _levelEventGen)
{
	// save a handle to the ECS & game settings
	game = _game;
	gameConfig = _gameConfig;
	eventPusher = _eventPusher;
	levelEventGen = _levelEventGen;


	std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();
	float speed = (*readCfg).at("Enemy1").at("speed").as<float>();
	float bomberspeed = (*readCfg).at("EnemyBomber").at("speed").as<float>();
	float cooldown = (*readCfg).at("Enemy1").at("cooldown").as<float>();

	float distmin = (*readCfg).at("Enemy1").at("distmin").as<float>();
	float distmax = (*readCfg).at("Enemy1").at("distmax").as<float>();
	float worldSizeMinX = (*readCfg).at("World").at("xsizemin").as<float>();
	float worldSizeMaxX = (*readCfg).at("World").at("xsizemax").as<float>();


	// baiter
	float baiterSpeed = (*readCfg).at("EnemyBaiter").at("speed").as<float>();

	// pod
	float podSpeed = (*readCfg).at("EnemyPod").at("speed").as<float>();
	float podDistmin = (*readCfg).at("EnemyPod").at("distmin").as<float>();
	float podDistmax = (*readCfg).at("EnemyPod").at("distmax").as<float>();

	//swarmers
	float swarmerSpeed = (*readCfg).at("EnemySwarmer").at("speed").as<float>();
	float swarmerJitter = (*readCfg).at("EnemySwarmer").at("jitter").as<float>();


	// destroy any enemies who are no longer alive
	auto a = game->system<Enemy, IsAlive>("Enemy Death System")
		.each([this](flecs::entity e, Enemy, IsAlive) {

		// if you have no health left be destroyed
		if (e.get<IsAlive>()->value == false) {
			// play explode sound
			DSC::PLAY_EVENT_DATA x;
			GW::GEvent explode;
			if (e.has<EnemyLander>())
			{
				explode.Write(DSC::PLAY_EVENT::ENEMY_LANDER_DESTROYED, x);
			}
			if (e.has<EnemyMutant>())
			{
				explode.Write(DSC::PLAY_EVENT::ENEMY_MUTANT_DESTROYED, x);
			}
			if (e.has<EnemyBaiter>())
			{
				explode.Write(DSC::PLAY_EVENT::ENEMY_BAITER_DESTROYED, x);
			}
			if (e.has<EnemyBomber>())
			{
				explode.Write(DSC::PLAY_EVENT::ENEMY_BOMBER_DESTROYED, x);
			}
			if (e.has<EnemyPod>())
			{
				explode.Write(DSC::PLAY_EVENT::ENEMY_POD_DESTROYED, x);
				// send info for spawning swarmers somewhere
				SpawnSwarmers(e.get<Position>()->value.x, e.get<Position>()->value.y, 5);
			}
			if (e.has<EnemySwarmer>())
			{
				explode.Write(DSC::PLAY_EVENT::ENEMY_SWARMER_DESTROYED, x);
			}
			eventPusher.Push(explode);
			GW::AUDIO::GSound enemyDeath = *e.get<GW::AUDIO::GSound>();
			enemyDeath.Play();
			e.destruct();
		}
	});
	systems.push_back(a);
	
    auto b = StartEnemyCooldownSystem();
	systems.push_back(b);

	auto c = StartEnemyFiringSystem();
	systems.push_back(c);

	// TODO: Rework with tags and make new systems
	// LanderPosition: cap off Y height at around 0.2f, move left or right, search for humans.
	// MutantPosition: chase player.
	// enemy strafe system	

	auto d = game->system<Enemy, EnemyLander, Position>("Enemy Lander Movement System")
		.each([this, speed, distmin, distmax](flecs::entity e, Enemy, EnemyLander, Position& p) {
		
			float dt = e.delta_time();
			// if human is close, target human
			float closestHumanX = 100.0f;
			Position closestHumanPosition;
			GW::MATH2D::GVECTOR2F enemyPos;
			float magnitude;

			game->filter<EnemyHuman, Position>()
				.each([&closestHumanX, &closestHumanPosition, p, &enemyPos](flecs::entity e, EnemyHuman, Position& hp) {
					if (abs(p.value.x - hp.value.x) < closestHumanX) {
						GW::MATH2D::GVector2D::Subtract2F(p.value, hp.value, enemyPos);
						closestHumanX = hp.value.x;
						closestHumanPosition = hp;
					}
				});

			GW::MATH2D::GVector2D::Magnitude2F(enemyPos, magnitude);

			if (e.has<Captured>()) {
				p.value.y += speed * dt / 2;
				if (p.value.y > 2.2f) {
					// destroy human through event
					DSC::PLAY_EVENT_DATA x;
					GW::GEvent humanAbducted;
					humanAbducted.Write(DSC::PLAY_EVENT::ENEMY_HUMAN_DESTROYED, x);
					eventPusher.Push(humanAbducted);
					// destroy lander no pts
					DSC::PLAY_EVENT_DATA y;
					GW::GEvent landerAbducted;
					landerAbducted.Write(DSC::PLAY_EVENT::ENEMY_LANDER_ESCAPED, x);
					eventPusher.Push(landerAbducted);
					// destroy lander enemy
					e.destruct();
				}
			}
			// target human if close
			else if (magnitude < 0.4f) {
				// move towards human in x/y dir
				if (abs(closestHumanPosition.value.x) - abs(p.value.x) < 0.0f) {
					p.value.x -= speed * dt;
				}
				else {
					p.value.x += speed * dt;
				}
				if (abs(closestHumanPosition.value.y) - abs(p.value.y) < 0.0f) {
					p.value.y -= speed * dt;
				}
				else {
					p.value.y += speed * dt;
				}
			}


			else if (!e.has<HasTarget>() && !e.has<Captured>())
			{
				// if lander has no movement target and is not carrying human, move randomly
				std::random_device rd;  // Will be used to obtain a seed for the random number engine
				std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
				std::uniform_real_distribution<float> y_range(0.1f, 0.6f);
				std::uniform_real_distribution<float> x_range(distmin, distmax);
				// get value between -2, +2 (distance to travel)
				float distance = x_range(gen);
				float height = y_range(gen);
				//float XTarget = p.value.x + (dir < 50 ? -x_range(gen) : x_range(gen));
				e.set_override<TargetPosition>({ distance, height });
				e.set_override<HasTarget>({ true });
			}
		});
	systems.push_back(d);


	auto k = game->system<EnemyHuman, Captured>("Lander Captures Human System")
		.each([this](flecs::entity& humanEntity, EnemyHuman& h, Captured) {
			if (h.heTouchedMeBro.is_alive() && humanEntity.has<Captured>())
			{
				float x = h.heTouchedMeBro.get<Position>()->value.x;
				float y = h.heTouchedMeBro.get<Position>()->value.y;
				humanEntity.set_override<Position>({ x, y - 0.1f });
			}
			else
			{
				humanEntity.remove<Captured>();
				humanEntity.add<Falling>();
				if (humanEntity.get<Position>()->value.y > 2.0f) {
					humanEntity.destruct();
				}
			}
		});
	systems.push_back(k);



	auto e = game->system<Enemy, TargetPosition, Position>("Enemy Target Finder")
		.each([this, speed](flecs::entity e, Enemy, TargetPosition, Position& p) {

			float dt = e.delta_time();
			
			// find random movement target
			if (abs(e.get<TargetPosition>()->value) <= 0.05f) {
				e.remove<HasTarget>();
				e.remove<TargetPosition>();
			}
			// distance is less than 0.05 in the negative direction
			else if (e.get<TargetPosition>()->value <= 0.0f) {
				float move = speed * dt;
				p.value.x -= move;
				e.get_mut<TargetPosition>()->value += move;

			}
			// distance is more than 0.05 in the positive direction
			else {
				float move = speed * dt;
				p.value.x += move;
				e.get_mut<TargetPosition>()->value -= move;
			}

			// move in y direction based on target height
			if (p.value.y <= e.get<TargetPosition>()->height) {
				p.value.y += speed * dt / 2;
			}
			else {
				p.value.y -= speed * dt / 2;
			}
		});
	systems.push_back(e);


	//Go towards player (traverse the x), When near/above player only traverse in the y direction.
	// Todo later: Activation range.

	auto f = StartEnemyMutantMovementSystem(speed);
	systems.push_back(f);

	auto g = StartEnemyBomberMovementSystem(bomberspeed);
	systems.push_back(g);
	auto h = StartEnemyBaiterMovementSystem(baiterSpeed);
	systems.push_back(h);
	auto i = StartEnemyPodMovementSystem(podSpeed, podDistmin, podDistmax);
	systems.push_back(i);
	auto j = StartEnemySwarmerMovementSystem(swarmerSpeed, swarmerJitter);
	systems.push_back(j);
	OnPodDeath(eventPusher);
	return true;
}

// Free any resources used to run this system
bool DSC::EnemyLogic::Shutdown()
{
	game->defer_begin(); // is this needed for systems? (see enemydata.cpp)
	for (auto system : systems) {
		system.destruct();
	}
	game->defer_end(); // is this needed for systems? (see enemydata.cpp)
	// invalidate the shared pointers
	game.reset();
	gameConfig.reset();
	return true;
}

// Toggle if a system's Logic is actively running
bool DSC::EnemyLogic::Activate(bool runSystem)
{
	if (runSystem) {
		for (auto system : systems) {
			system.enable();
		}
	}
	else {
		for (auto system : systems) {
			system.disable();
		}
	}
	
	return false;
}

bool DSC::EnemyLogic::EnemyFireWeapon(flecs::world& stage, flecs::entity &e, Position origin) {
	// declare a bullet entity
	flecs::entity bullet;
	
	Velocity v;

	if (e.has<EnemyBomber>()) {
		RetreivePrefab("Enemy Bomb", bullet);
	}
	else {
		RetreivePrefab("Enemy Bullet", bullet);
	}

	// aim at player position with bullet velocity v
	GetAimTarget(stage, e, origin, v);
	
	// create bullet in game world at position with velocity
	auto laser = stage.entity().is_a(bullet)
		.set<Position>(origin)
		.set<Velocity>({ v });

	// play the sound of the Lazer prefab
	GW::AUDIO::GSound shoot = e.get<FiringSound>()->fire;
	shoot.Play();

	return true;
}

bool DSC::EnemyLogic::GetAimTarget(flecs::world& stage, flecs::entity &e, const Position& origin, Velocity& v) {
	// get player position, enemy accuracy value, enemy bullet speed
	Position p = *stage.entity("Player One").get<Position>();
	auto accuracy = e.get<Accuracy>()->value;
	float bulletSpeed = e.get<BulletSpeed>()->value;

	// compute random variance of target x/y location
	std::random_device rd;  // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
	std::uniform_real_distribution<float> x_range(-accuracy, +accuracy);
	std::uniform_real_distribution<float> y_range(-accuracy, +accuracy);
	float x_offset = x_range(gen); // normal rand() doesn't work great multi-threaded
	float y_offset = y_range(gen);

	// apply random x/y offset to player position
	p.value.x += x_offset;
	p.value.y += y_offset;

	// get vector between target and enemy origin
	GW::MATH2D::GVECTOR2F normalizedVelocity;
	GW::MATH2D::GVector2D::Subtract2F(p.value, origin.value, normalizedVelocity);
	
	// get magnitude of vector between target and enemy origin
	float magnitude;
	GW::MATH2D::GVector2D::Magnitude2F(normalizedVelocity, magnitude);

	// normalize the vector which is 'aiming' at the target location
	GW::MATH2D::GVector2D::Normalize2F(normalizedVelocity, normalizedVelocity);
	
	// multiply velocity by magnitude to arrive at target in bulletspeed seconds
	// 1 bulletspeed = bullet arrives at target in 1 second
	// 2 bulletspeed = bullet arrives at target in 0.5 seconds
	// 0 bulletspeed = bullet is stationary
	v.value.x = normalizedVelocity.x * magnitude * bulletSpeed;
	v.value.y = normalizedVelocity.y * magnitude * bulletSpeed;

	return true;
}

// starts the enemy firing system and returns that system for handling
flecs::system DSC::EnemyLogic::StartEnemyFiringSystem() {
	return game->system<Firing, Enemy, Position>("Enemy Firing System")
		.iter([this](flecs::iter it, Firing*, Enemy*, Position* origin) {
		auto player = game->entity("Player One");
			if (player.has<IsAlive>()) {
				for (auto i : it) {
					EnemyFireWeapon(it.world(), it.entity(i), *origin);
					it.entity(i).remove<Firing>();
				}
			}
		});
}

// begin weapon cooldown for every active enemy
flecs::system DSC::EnemyLogic::StartEnemyCooldownSystem() {
	return game->system<Enemy, IsActive, Firerate>("Enemy Cooldown System")
		.iter([this](flecs::iter it, Enemy*, IsActive*, Firerate* f) {
			for (auto i : it) {
				// count down the enemy fire rate
				float cooldown = f->value;
				cooldown -= game->delta_time();
				// if cooldown is zero, enable enemy to fire and reset timer
				if (cooldown <= 0.0f) {
					it.entity(i).add<Firing>();
					f->value = f->resetValue;
				}
				else {
					f->value = cooldown;
				}
			}
		});
}

flecs::system DSC::EnemyLogic::StartEnemyMutantMovementSystem(float speed) {
	return game->system<EnemyMutant, Position>("Enemy Mutant Movement System")
		.iter([this, speed](flecs::iter iter, EnemyMutant*, Position* p) {
			auto dt = iter.delta_time();
			//Sin waves for jittery movement as the mutants move left and right.
			float x_offset = sin(iter.world().time() * 5.0) * 0.5;
			float y_offset = sin(iter.world().time() * 5.0) * 0.5;


			if (iter.world().entity("Player One").has<IsAlive>())
			{
				GW::MATH2D::GVECTOR2F playerPosition = iter.world().entity("Player One").get<Position>()->value;
				for (auto i : iter)
				{
					if (p[i].value.x < playerPosition.x)
					{
						p[i].value.x += speed * dt;
						//Erratic Strafing movement while moving towards the player on the X axis
						p[i].value.y += y_offset * dt;
					}
					if (p[i].value.x > playerPosition.x)
					{
						p[i].value.x -= speed * dt;
						p[i].value.y += y_offset * dt;
					}
					
					if (std::abs(playerPosition.x - p[i].value.x) < 0.1f)
					{
						if (p[i].value.y < playerPosition.y)
						{
							p[i].value.y += speed * dt;
							//Erratic Strafing movement while moving towards the player on the Y axis
							p[i].value.x += x_offset * dt;
						}
						if (p[i].value.y > playerPosition.y)
						{
							p[i].value.y -= speed * dt;
							p[i].value.x += x_offset * dt;
						}
					}
				}
			}
		});
}

flecs::system DSC::EnemyLogic::StartEnemyBomberMovementSystem(float speed) {
	return game->system<EnemyBomber, Position>("Enemy Bomber Movement System")
		.iter([this, speed](flecs::iter iter, EnemyBomber*, Position* p) {
		auto dt = iter.delta_time();
		

		if (iter.world().entity("Player One").has<IsAlive>())
		{
			GW::MATH2D::GVECTOR2F playerPosition = iter.world().entity("Player One").get<Position>()->value;
			for (auto i : iter)
			{
				GW::MATH2D::GVECTOR2F direction;
				GW::MATH2D::GVector2D::Subtract2F(playerPosition, p[i].value, direction);
				GW::MATH2D::GVector2D::Normalize2F(direction, direction);

				GW::MATH2D::GVECTOR2F targetPos;
				GW::MATH2D::GVector2D::Scale2F(direction, (speed * dt), targetPos);
				GW::MATH2D::GVector2D::Add2F(p[i].value, targetPos, targetPos);
				//This is essentially the p[i].value + direection * (speed * dt). Just really convoluted because gateware itself is convoluted.
				float lerp = 0.2f;
				GW::MATH2D::GVector2D::Lerp2F(p[i].value, targetPos, lerp, p[i].value);
			}
		}
		});
}

flecs::system DSC::EnemyLogic::StartEnemyBaiterMovementSystem(float baiterSpeed) {
	return game->system<EnemyBaiter, Position>()
		.each([this, baiterSpeed](flecs::entity& e, EnemyBaiter, Position& p) {
			// ensure player is alive before trying for player location
			if (game->entity("Player One").has<IsAlive>()) {
				// get player position
				// move in a close position around player but not through player
				GW::MATH2D::GVECTOR2F playerPosition = game->entity("Player One").get<Position>()->value;
				GW::MATH2D::GVECTOR2F targetPosition = playerPosition;

				// modify player position/target by circular movement
				float x_offset = 1.0f;
				float y_offset = 0.33f;
				float d_offset = 0.0f;

				// if x is pos, baiter is left
				// if x is neg, baiter is right
				// if y is pos, baiter is below
				// if y is neg, baiter is above
				float baiterLocationX = playerPosition.x - p.value.x;
				float baiterLocationY = playerPosition.y - p.value.y;

				// baiter is left, below, Q3
				if (baiterLocationX > d_offset && baiterLocationY > d_offset) {
					targetPosition.x = playerPosition.x + x_offset;
					targetPosition.y = playerPosition.y - y_offset; // Q4
				}
				// baiter is right, below, Q4
				else if (baiterLocationX < d_offset && baiterLocationY > d_offset) {
					targetPosition.x = playerPosition.x + x_offset;
					targetPosition.y = playerPosition.y + y_offset; // Q1
				}
				// baiter is right, above, Q1
				else if (baiterLocationX < d_offset && baiterLocationY < d_offset) {
					targetPosition.x = playerPosition.x - x_offset;
					targetPosition.y = playerPosition.y + y_offset; // Q2
				}
				// baiter is right, above, Q2
				else if (baiterLocationX > d_offset && baiterLocationY < d_offset) {
					targetPosition.x = playerPosition.x - x_offset;
					targetPosition.y = playerPosition.y - y_offset; // Q3
				}				

				// get vector pointing towards targetPosition from current baiter position
				GW::MATH2D::GVECTOR2F direction;
				GW::MATH2D::GVector2D::Subtract2F(targetPosition, p.value, direction);

				// probably don't need magnitude
				float magnitude;
				GW::MATH2D::GVector2D::Magnitude2F(direction, magnitude);

				// normalize vector
				GW::MATH2D::GVector2D::Normalize2F(direction, direction);

				// move baiter position in direction per frame times speed
				p.value.x += direction.x * game->delta_time() * baiterSpeed;
				p.value.y += direction.y * game->delta_time() * baiterSpeed;
			}
		});
}

flecs::system DSC::EnemyLogic::StartEnemyPodMovementSystem(float podSpeed, float podDistmin, float podDistmax) {
	return game->system<EnemyPod, Position>()
		.each([this, podSpeed, podDistmin, podDistmax](flecs::entity e, EnemyPod, Position p) {
			auto dt = e.delta_time();
			// its gonna have a similar movement system to the Lander, just slower and without a constrained Y.
			std::random_device rd;  // Will be used to obtain a seed for the random number engine
			std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
			// get random value between 0.25 and 1.75 (world y is 0-2)
			std::uniform_real_distribution<float> x_range(podDistmin, podDistmax);
			std::uniform_real_distribution<float> y_range(podDistmin, podDistmax);
			std::uniform_real_distribution<float> direction(1, 100);
			int dir = direction(gen);
		
			if (!e.get<HasTarget>())//|| e.get<HasTarget>()->value == false)
			{
				//float XTarget = x_range(gen) + p.value.x;
				//p.value.y = max(-1.4f,min(p.value.y,1.4f));
				//float XTarget = p.value.x + (dir < 50 ? -x_range(gen) : x_range(gen));
				float x_offset = x_range(gen);
				float y_offset = y_range(gen);
				float x_target;
				float y_target;
				if (dir < 50) {
					x_target = p.value.x + x_offset;
				}
				else {
					x_target = p.value.x - x_offset;
				}
				//float YTarget = p.value.y + (dir < 50 ? -y_range(gen) : y_range(gen));
				// no need to randomize further as it's 0.25 - 1.75 up/down
				y_target = y_offset;

				e.set_override<TargetPosition>({ x_target, y_target });
				e.set_override<HasTarget>({ true });
			}

		// Updates the pod's position for use in the swarmer code.
		// podPosition = p.value;
		});
}

void DSC::EnemyLogic::SpawnSwarmers(float podX, float podY, int numSwarmers) {

		//gameLock.LockSyncWrite();
		const float spawnRadius = 0.05f;
		GW::MATH2D::GVECTOR2F targetPosition;
		
		//Calculate random acceleration
		//std::random_device rd;  // Will be used to obtain a seed for the random number engine
		//std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
		//std::uniform_real_distribution<float> x_range(0.1, 2);
		//std::uniform_real_distribution<float> y_range(0.1, 2);
		//std::uniform_real_distribution<float> direction(1, 100);
		//int dir = direction(gen);

		for (int i = 0; i < numSwarmers; i++) {
			// Calculate random position around the pod
			float angle = (float)i / 5 * 2 * 3.141f;
			float xOffset = cos(angle) * spawnRadius;
			float yOffset = sin(angle) * spawnRadius;

			targetPosition.x = podX + xOffset;
			targetPosition.y = podY + yOffset;

			//float yAccel = (dir < 50 ? -y_range(gen) : y_range(gen));
			//float xAccel = (dir < 50 ? -x_range(gen) : x_range(gen));

			//Ideally, it would just move away from its spawn point, then start to home in on the player.
			//In the movement segment, I'll need to make the movements more varied and prevent a huge cluster from forming.

			GW::GEvent spawnSwarmers;
			DSC::LEVEL_EVENT_SWARMER_DATA x;
			x.count = 1;
			x.xpos = targetPosition.x;
			x.ypos = targetPosition.y;
			spawnSwarmers.Write(DSC::LEVEL_EVENT::SPAWN_SWARMERS, x);
			levelEventGen.Push(spawnSwarmers);
		}

		// Spawn swarmer entity
		//flecs::entity swarmer;
		//RetreivePrefab("Enemy Type Swarmer", swarmer);
	
		////Add an enemy
		//game->entity().is_a(swarmer)
		//	.set<Velocity>({ 0,0 })
		//	.set<Acceleration>({ 0, 0 })
		//	.set<Position>({ targetPosition.x, targetPosition.y })
		//	.set<IsAlive>({ true });
		////to debug//
		//std::cout << "Spawned Pod at : " << targetPosition.x << " , " << targetPosition.y << std::endl;

}

void DSC::EnemyLogic::OnPodDeath(GW::CORE::GEventGenerator _eventPusher) {

	//This is a really ugly function.
	/*	onExplode.Create([this](const GW::GEvent& e) {
			DSC::PLAY_EVENT event;
			DSC::PLAY_EVENT_DATA eventData;
			if (+e.Read(event, eventData)) {
				if (event == DSC::PLAY_EVENT::ENEMY_POD_DESTROYED)
				{
					for (size_t i = 0; i < 5; i++)
					{
						SpawnSwarmers(podPosition.x, podPosition.y,i);
					}
				}
			}
		});
	_eventPusher.Register(onExplode);*/
}

flecs::system DSC::EnemyLogic::StartEnemySwarmerMovementSystem(float swarmerSpeed, float jitterAmount) {
	return game->system<EnemySwarmer, Position>()
		.iter([this, swarmerSpeed,jitterAmount](flecs::iter iter, EnemySwarmer*, Position* p) {
		auto dt = iter.delta_time();
		if (iter.world().entity("Player One").has<IsAlive>())
		{
			// Copying the Bomber movement for the moment, because think smarter not harder.
			GW::MATH2D::GVECTOR2F playerPosition = iter.world().entity("Player One").get<Position>()->value;
			for (auto i : iter)
			{

				GW::MATH2D::GVECTOR2F direction;
				GW::MATH2D::GVector2D::Subtract2F(playerPosition, p[i].value, direction);
				GW::MATH2D::GVector2D::Normalize2F(direction, direction);

				GW::MATH2D::GVECTOR2F targetPos;
				GW::MATH2D::GVector2D::Scale2F(direction, (swarmerSpeed * dt), targetPos);
				GW::MATH2D::GVector2D::Add2F(p[i].value, targetPos, targetPos);
				//This is essentially the p[i].value + direection * (speed * dt). Just really convoluted because gateware itself is convoluted.
				float lerp = 0.5f;
				//Gives the enemies a more random jitter, hopefully.
				float jitterX = (rand() % 100 / 100.0f) * jitterAmount - (jitterAmount / 2);
				float jitterY = (rand() % 100 / 100.0f) * jitterAmount - (jitterAmount / 2);

				targetPos.x += jitterX;
				targetPos.y += jitterY;

				GW::MATH2D::GVector2D::Lerp2F(p[i].value, targetPos, lerp, p[i].value);
			}
		}
		});
}
