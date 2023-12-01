// define all ECS components related to drawing
#ifndef VISUALS_H
#define VISUALS_H
#include "../Utils/h2bParser.h"

// descender namespace
namespace DSC
{
	struct Color { GW::MATH2D::GVECTOR3F value; };

	struct Material {
		std::string name;
		unsigned int indexStart, transformStart , vertexStart= 0;
		unsigned int transformCount = 1;
		GW::MATH::GMATRIXF transform;
		std::vector<H2B::VERTEX> modelVertices;
		std::vector<H2B::ATTRIBUTES> modelMaterials;
		std::vector<H2B::MESH> modelMeshes;
	};
};

#endif