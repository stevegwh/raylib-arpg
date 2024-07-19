//
// Created by Steve Wheeler on 21/03/2024.
//

#include "GameObjectFactory.hpp"
#include "scenes/Scene.hpp"
#include "ResourceManager.hpp"


#include "components/sgTransform.hpp"
#include "components/MovableActor.hpp"
#include "components/Renderable.hpp"
#include "components/Collideable.hpp"
#include "components/WorldObject.hpp"
#include "components/Animation.hpp"
#include "components/HealthBar.hpp"
#include "components/CombatableActor.hpp"
#include "components/states/StateEnemyDefault.hpp"
#include "components/states/StatePlayerDefault.hpp"

#include "raymath.h"
#include <slib.hpp>


namespace sage
{
	BoundingBox createRectangularBoundingBox(float length, float height)
	{
		BoundingBox bb;
		// Calculate half dimensions
		float halfLength = length / 2.0f;
		//float halfHeight = height / 2.0f;

		// Set minimum bounds
		bb.min.x = -halfLength;
		bb.min.y = 0.0f;
		bb.min.z = -halfLength;

		// Set maximum bounds
		bb.max.x = halfLength;
		bb.max.y = height;
		bb.max.z = halfLength;

		return bb;
	}

	entt::entity GameObjectFactory::createEnemy(entt::registry* registry, GameData* game, Vector3 position,
		const char* name)
	{
		entt::entity id = registry->create();

		// Model/Rendering
		auto modelPath = "resources/models/gltf/goblin.glb";
		//sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png"), std::string("resources/models/obj/cube_diffuse.png") };

		auto& transform = registry->emplace<sgTransform>(id);
		GridSquare actorIdx;
		game->navigationGridSystem->WorldToGridSpace(position, actorIdx);
		float height = game->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
		transform.SetPosition({ position.x, height, position.z }, id);
		transform.SetScale(1.0f, id);
		transform.SetRotation({ 0, 0, 0 }, id);
		transform.movementSpeed = 0.05f;
		registry->emplace<MoveableActor>(id);

		auto model = LoadModel(modelPath);

		auto& animation = registry->emplace<Animation>(id, modelPath, &model);
		animation.animationMap[AnimationEnum::IDLE] = 0;
		animation.animationMap[AnimationEnum::DEATH] = 0;
		animation.animationMap[AnimationEnum::MOVE] = 3;
		animation.animationMap[AnimationEnum::AUTOATTACK] = 1;
		animation.ChangeAnimationByEnum(AnimationEnum::MOVE);

		Matrix modelTransform = MatrixScale(0.03f, 0.03f, 0.03f);
		auto& renderable = registry->emplace<Renderable>(id, model, modelTransform);
		renderable.name = name;
		// ---

		// Combat
		auto& healthbar = registry->emplace<HealthBar>(id);
		// TODO: "HealthBar" should be separate from something like CombatData

		auto& combatable = registry->emplace<CombatableActor>(id);
		combatable.actorType = CombatableActorType::WAVEMOB;
		{
			entt::sink sink{ combatable.onHit };
			sink.connect<&WaveMobCombatStateSystem::OnHit>(game->stateSystems->combatSystems->waveMobCombatLogicSubSystem);
		}

		// ---

		// Collision
		BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f);
		auto& collideable = registry->emplace<Collideable>(id, bb);
		//collideable.debugDraw = true;
		collideable.collisionLayer = CollisionLayer::ENEMY;
		game->collisionSystem->UpdateWorldBoundingBox(id, transform.GetMatrix());
		{
			entt::sink sink{ transform.onPositionUpdate };
			sink.connect < [](CollisionSystem& collisionSystem, entt::entity entity)
				{
					collisionSystem.OnTransformUpdate(entity);
				} > (*game->collisionSystem);
		}
		// ---

		auto& worldObject = registry->emplace<WorldObject>(id);

		registry->emplace<StateEnemyDefault>(id);
		// Always set state last to ensure everything is initialised properly before.
		return id;
	}

	entt::entity GameObjectFactory::createKnight(entt::registry* registry, GameData* game, Vector3 position,
		const char* name)
	{
		entt::entity id = registry->create();
		auto modelPath = "resources/models/gltf/arissa.glb";
		//sage::Material mat = { LoadTexture("resources/models/obj/cube_diffuse.png"), std::string("resources/models/obj/cube_diffuse.png") };

		auto& transform = registry->emplace<sgTransform>(id);
		GridSquare actorIdx;
		game->navigationGridSystem->WorldToGridSpace(position, actorIdx);
		float height = game->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
		transform.SetPosition({ position.x, height, position.z }, id);
		transform.SetScale(1.0f, id);
		transform.SetRotation({ 0, 0, 0 }, id);

		auto model = LoadModel(modelPath);
		auto& animation = registry->emplace<Animation>(id, modelPath, &model);
		animation.ChangeAnimation(0);

		Matrix modelTransform = MatrixScale(0.045f, 0.045f, 0.045f);
		auto& renderable = registry->emplace<Renderable>(id, model, modelTransform);
		renderable.name = name;

		//auto& combat = registry->emplace<HealthBar>(id);

		BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
		auto& collideable = registry->emplace<Collideable>(id, bb);
		collideable.collisionLayer = CollisionLayer::NPC;
		game->collisionSystem->UpdateWorldBoundingBox(id, transform.GetMatrix());
		{
			entt::sink sink{ transform.onPositionUpdate };
			sink.connect < [](CollisionSystem& collisionSystem, entt::entity entity)
				{
					collisionSystem.OnTransformUpdate(entity);
				} > (*game->collisionSystem);
		}

		auto& dialogue = registry->emplace<Dialogue>(id);
		dialogue.sentence = "Hello, this is a test sentence.";
		dialogue.conversationPos = Vector3Add(transform.position(),
			Vector3Multiply(transform.forward(), { 10.0f, 1, 10.0f }));
		auto& worldObject = registry->emplace<WorldObject>(id);
		return id;
	}

	entt::entity GameObjectFactory::createPlayer(entt::registry* registry, GameData* game, Vector3 position,
		const char* name)
	{
		entt::entity id = registry->create();
		auto modelPath = "resources/models/gltf/hero.glb";

		auto& transform = registry->emplace<sgTransform>(id);
		GridSquare actorIdx;
		game->navigationGridSystem->WorldToGridSpace(position, actorIdx);
		float height = game->navigationGridSystem->GetGridSquare(actorIdx.row, actorIdx.col)->terrainHeight;
		transform.SetPosition({ position.x, height, position.z }, id);
		transform.SetScale(1.0f, id);
		transform.SetRotation({ 0, 0, 0 }, id);
		registry->emplace<MoveableActor>(id);

		auto model = LoadModel(modelPath);

		// Set animation hooks
		auto& animation = registry->emplace<Animation>(id, modelPath, &model);

		animation.animationMap[AnimationEnum::IDLE] = 2;
		animation.animationMap[AnimationEnum::MOVE] = 5;
		animation.animationMap[AnimationEnum::TALK] = 3;
		animation.animationMap[AnimationEnum::AUTOATTACK] = 1;
		animation.ChangeAnimationByEnum(AnimationEnum::IDLE);

		{
			entt::sink sink{ transform.onFinishMovement };
			sink.connect < [](Animation& animation, entt::entity entity)
				{
					animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
				} > (animation);
		}
		{
			entt::sink sink{ transform.onStartMovement };
			sink.connect < [](Animation& animation, entt::entity entity)
				{
					animation.ChangeAnimationByEnum(AnimationEnum::MOVE);
				} > (animation);
		}
		{
			entt::sink sink{ game->userInput->keyIPressed };
			sink.connect < [](Animation& animation)
				{
					// TODO: Just to test animations on demand
					if (animation.animIndex == 0)
					{
						animation.ChangeAnimationByEnum(AnimationEnum::TALK);
					}
					else if (animation.animIndex == 2)
					{
						animation.ChangeAnimationByEnum(AnimationEnum::IDLE);
					}
				} > (animation);
		}

		// Combat
		auto& combatable = registry->emplace<CombatableActor>(id);
		combatable.self = id;
		combatable.actorType = CombatableActorType::PLAYER;
		// Links the cursor's events to the combatable actor which passes on the entity id to the system
		{
			entt::sink sink{ game->cursor->onEnemyClick };
			sink.connect<&CombatableActor::EnemyClicked>(combatable);
		}
		{
			entt::sink sink{ game->cursor->onFloorClick };
			sink.connect<&CombatableActor::AttackCancelled>(combatable);
		}
		// ---

		Matrix modelTransform = MatrixScale(0.035f, 0.035f, 0.035f);
		auto& renderable = registry->emplace<Renderable>(id, model, modelTransform);
		renderable.name = name;

		BoundingBox bb = createRectangularBoundingBox(3.0f, 7.0f); // Manually set bounding box dimensions
		auto& collideable = registry->emplace<Collideable>(id, bb);
		collideable.collisionLayer = CollisionLayer::PLAYER;
		game->collisionSystem->UpdateWorldBoundingBox(id, transform.GetMatrix());
		{
			entt::sink sink{ transform.onPositionUpdate };
			sink.connect<&CollisionSystem::OnTransformUpdate>(*game->collisionSystem);
		}
		auto& worldObject = registry->emplace<WorldObject>(id);

		auto& actor = registry->emplace<ControllableActor>(id);
		actor.pathfindingBounds = 100;
		game->controllableActorSystem->SetControlledActor(id);
		registry->emplace<StatePlayerDefault>(id);
		// Always set state last to ensure everything is initialised properly before.
		return id;
	}

	void GameObjectFactory::createBuilding(entt::registry* registry, GameData* data, Vector3 position, const char* name,
		const char* modelPath, const char* texturePath)
	{
		auto id = registry->create();
		auto& transform = registry->emplace<sgTransform>(id);
		transform.SetPosition(position, id);
		transform.SetScale(2.0f, id);
		transform.SetRotation({ 0, 0, 0 }, id);
		auto model = LoadModel(modelPath);
		MaterialPaths matPaths;
		matPaths.diffuse = texturePath;
		model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(ResourceManager::LoadTexture(matPaths.diffuse));
		auto& renderable = registry->emplace<Renderable>(id, model, matPaths, MatrixIdentity());
		renderable.name = name;
		auto& collideable = registry->emplace<Collideable>(
			id, CalculateModelBoundingBox(renderable.model));
		collideable.collisionLayer = CollisionLayer::BUILDING;
		data->collisionSystem->UpdateWorldBoundingBox(id, transform.GetMatrix());
		{
			entt::sink sink{ transform.onPositionUpdate };
			sink.connect < [](CollisionSystem& collisionSystem, entt::entity entity)
				{
					collisionSystem.OnTransformUpdate(entity);
				} > (*data->collisionSystem);
		}
		registry->emplace<WorldObject>(id);
	}

	BoundingBox calculateFloorSize(const std::vector<Collideable*>& floorMeshes)
	{
		// TODO: Below doesn't seem to work always, depending on the map.
		BoundingBox mapBB{ Vector3{0, 0, 0}, Vector3{0, 0, 0} }; // min, max
		for (const auto& col : floorMeshes)
		{
			if (col->worldBoundingBox.min.x <= mapBB.min.x && col->worldBoundingBox.min.z <= mapBB.min.z)
			{
				mapBB.min = col->worldBoundingBox.min;
			}
			if (col->worldBoundingBox.max.x >= mapBB.max.x && col->worldBoundingBox.max.z >= mapBB.max.z)
			{
				mapBB.max = col->worldBoundingBox.max;
			}
		}
		mapBB.min.y = 0.1f;
		mapBB.max.y = 0.1f;
		return mapBB;
	}

	void GameObjectFactory::loadMap(entt::registry* registry, Scene* scene, float& slices, const std::string& _mapPath)
	{

		MaterialPaths matPaths;
		matPaths.diffuse = "resources/models/obj/PolyAdventureTexture_01.png";
		Model parent = LoadModel(_mapPath.c_str());
		std::vector<Collideable*> floorMeshes;

		for (int i = 0; i < parent.meshCount; ++i)
		{
			entt::entity id = registry->create();
			auto& transform = registry->emplace<sgTransform>(id);
			transform.SetPosition({ 0, 0, 0 }, id);
			transform.SetScale(1.0f, id);
			transform.SetRotation({ 0, 0, 0 }, id);

			Matrix modelTransform = MatrixScale(5.0f, 5.0f, 5.0f);
			Model model = LoadModelFromMesh(parent.meshes[i]);
			//Matrix modelTransform = MatrixScale(1,1,1);
			auto& renderable = registry->emplace<Renderable>(id, model, matPaths, modelTransform);
			renderable.name = parent.meshes[i].name;
			renderable.serializable = false;

			scene->lightSubSystem->LinkRenderableToLight(&renderable);
			model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTextureFromImage(ResourceManager::LoadTexture(matPaths.diffuse));
			auto& collideable = registry->emplace<Collideable>(id, CalculateModelBoundingBox(renderable.model));
			if (renderable.name.find("SM_Bld") != std::string::npos)
			{
				collideable.collisionLayer = CollisionLayer::BUILDING;
			}
			else if (renderable.name.find("SM_Env_NoWalk") != std::string::npos)
			{
				collideable.collisionLayer = CollisionLayer::TERRAIN;
			}
			else if (renderable.name.find("SM_Env") != std::string::npos)
			{
				collideable.collisionLayer = CollisionLayer::FLOOR;
				floorMeshes.push_back(&collideable);
			}
			else if (renderable.name.find("SM_Prop") != std::string::npos)
			{
				collideable.collisionLayer = CollisionLayer::BUILDING;
			}
			else
			{
				collideable.collisionLayer = CollisionLayer::DEFAULT;
			}
			scene->data->collisionSystem->UpdateWorldBoundingBox(id, transform.GetMatrix());
		}

		// Calculate grid based on walkable area
		BoundingBox mapBB{ Vector3{-500, 0, -500}, Vector3{500, 0, 500} }; // min, max
		//BoundingBox mapBB = calculateFloorSize(floorMeshes);

		slices = mapBB.max.x - mapBB.min.x;
		// Create floor
		createFloor(registry, scene, mapBB);
	}

	void GameObjectFactory::createFloor(entt::registry* registry, Scene* scene, BoundingBox bb)
	{
		entt::entity floor = registry->create();
		auto& floorCollidable = registry->emplace<Collideable>(floor, bb);
		floorCollidable.collisionLayer = CollisionLayer::FLOOR;
	}
} // sage
