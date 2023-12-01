#ifndef GAMEPLAYOBJS_H
#define GAMEPLAYOBJS_H

#include "../Components/Gameplay.h"
#include "../Components/Visuals.h"
#include "../Entities/Prefabs.h"

namespace DSC {
	// defines logic used to manipulate the models in the level
	class Gameplay
	{
		std::shared_ptr<flecs::world> world;
		std::weak_ptr<const GameConfig> gameConfig;

	public:
		// constructor assumes game objects have been loaded by level loader
		Gameplay(const Level_Data& import, GW::SYSTEM::GLog log, std::weak_ptr<const GameConfig> _gameConfig, std::shared_ptr<flecs::world> _world) {
			// create flecs world
			world = _world;
			gameConfig = _gameConfig;
			std::shared_ptr<const GameConfig> readCfg = gameConfig.lock();

			// add a named entity for each object in blender
			for (auto& i : import.blenderObjects) {
					Material m;
					flecs::entity ent = world->entity(i.blendername);
					m.transform = import.levelTransforms[i.transformIndex];
					m.transformStart = i.transformIndex;
					m.indexStart = import.levelModels[i.modelIndex].indexStart;
					m.vertexStart = import.levelModels[i.modelIndex].vertexStart;
					m.name = (std::string)i.blendername;
					m.transformCount = 1;
					for (int j = 0; j < import.levelModels[i.modelIndex].meshCount; ++j)
					{
						unsigned int actualMeshIndex = j + import.levelModels[i.modelIndex].meshStart;
						const H2B::MESH* mesh = &import.levelMeshes[actualMeshIndex];
						m.modelMeshes.push_back(*mesh);
						m.modelMaterials.push_back(import.levelMaterials[mesh->materialIndex + import.levelModels[i.modelIndex].materialStart].attrib);
					}
					ent.set<Material>(m);
			}
			
		};
		// TODO: Write more gameplay logic/systems to manpulate specific objects in the level 
	};
};

#endif