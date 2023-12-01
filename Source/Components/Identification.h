// define all ECS components related to identification
#ifndef IDENTIFICATION_H
#define IDENTIFICATION_H

// descender namespace
namespace DSC
{
	// player identification
	struct Player {};
	struct PlayerBullet {};
	struct Lives {};
	struct PowerUp {};
	struct Award { GW::AUDIO::GSound award; };
	struct ControllerID {
		unsigned index = 0;
	};

	// enemy identification
	struct Enemy {};
	struct IsActive {};
	struct EnemyBullet {};
	struct EnemyBomb { GW::AUDIO::GSound explosion; };
	struct EnemyLander {};
    struct EnemyMutant {};
	struct EnemyBomber {};
	struct EnemyBaiter {};
	struct EnemyPod {};
	struct EnemySwarmer {};
	struct EnemyHuman { flecs::entity heTouchedMeBro; };
	struct TargetingHuman {};

	// human states 
	struct Falling {};
	struct Grounded {};
	struct Captured {};
	struct PlayerCaptured {};
};

#endif