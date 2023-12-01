// define all ECS components related to movement & collision
#ifndef PHYSICS_H
#define PHYSICS_H

// descender namespace
namespace DSC 
{
	// ECS component types should be *strongly* typed for proper queries
	// typedef is tempting but it does not help templates/functions resolve type
	struct Position { GW::MATH2D::GVECTOR2F value; };
	struct Velocity { GW::MATH2D::GVECTOR2F value; };
	struct Destination { float value; };
	struct Orientation {
		GW::MATH2D::GMATRIX2F value;
		float lastX;
	};
	struct Acceleration { GW::MATH2D::GVECTOR2F value; };
	struct Collider { GW::MATH2D::GVECTOR2F poly[4]; };

	// Individual TAGs
	struct Collidable {}; 
	
	// ECS Relationship tags
	struct CollidedWith {};
};

#endif