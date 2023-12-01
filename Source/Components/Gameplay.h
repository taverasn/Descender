// define all ECS components related to gameplay
#ifndef GAMEPLAY_H
#define GAMEPLAY_H

// descender namespace
namespace DSC
{
	
	struct Firerate { float value; float resetValue; };
	struct Cooldown { float value; };
	struct IsAlive { bool value; };
	struct HasTarget { bool value; };
	struct TargetPosition { float value; float height; };
	struct DistanceTraveled { float value; };
	struct Lifetime { float value; };
	struct Accuracy { float value; };
	struct BulletSpeed { float value; };
	struct FacingDirection { bool value; };
	struct LifeCount { int value; };
	struct BombCount { int value; };
	struct Score { int value; };
	struct AwardScore { int value; };
	struct FiringSound { GW::AUDIO::GSound fire; };
	struct NukeSound { GW::AUDIO::GSound sound; };
	struct ThrusterSound { GW::AUDIO::GSound sound; };


	// gameplay tags (states)
	struct Firing {};
	struct Visible {};
	struct Nuke {};
	// powerups
	
};

#endif