//
// Created by steve on 11/05/2024.
//

#include "DialogueSystem.hpp"
#include "components/Collideable.hpp"
#include "components/Animation.hpp"

#include <iostream>

namespace sage
{
	void DialogueSystem::changeControlledActor(entt::entity entity)
	{
		controlledActor = entity;
	}

	void DialogueSystem::startConversation(entt::entity entity)
	{
		onConversationStart.publish();

		const auto& npcDiag = registry->get<Dialogue>(clickedNPC);
		std::cout << npcDiag.sentence << std::endl;
		registry->get<Animation>(clickedNPC).ChangeAnimation(1); // TODO: Change to an enum

		// Rotate to look at NPC
		auto& actorTrans = registry->get<Transform>(controlledActor);
		auto& npcTrans = registry->get<Transform>(clickedNPC);
		Vector3 direction = Vector3Subtract(npcTrans.position(), actorTrans.position());
		direction = Vector3Normalize(direction);
		float angle = atan2f(direction.x, direction.z);
		actorTrans.SetRotation({actorTrans.rotation().x, RAD2DEG * angle, actorTrans.rotation().z}, controlledActor);

		{
			entt::sink sink{cursor->onAnyClick};
			sink.connect<&DialogueSystem::endConversation>(this);
		}

		oldCamPos = camera->getRaylibCam()->position;
		oldCamTarget = camera->getRaylibCam()->target;

		camera->CutscenePose(npcTrans);
		camera->LockInput();

		cursor->LockCursor();
		controllableActorSystem->Disable();
		active = true;

		{
			entt::sink sink{actorTrans.onFinishMovement};
			sink.disconnect<&DialogueSystem::startConversation>(this);
			entt::sink sink2{actorTrans.onMovementCancel};
			sink2.disconnect<&DialogueSystem::cancelConversation>(this);
		}
	}

	void DialogueSystem::endConversation(entt::entity actor)
	{
		onConversationEnd.publish();
		{
			entt::sink sink3{cursor->onAnyClick};
			sink3.disconnect<&DialogueSystem::endConversation>(this);
		}

		camera->UnlockInput();
		cursor->UnlockCursor();
		controllableActorSystem->Enable();
		active = false;
		registry->get<Animation>(clickedNPC).ChangeAnimation(0); // TODO: Change to an enum
		clickedNPC = entt::null;

		camera->SetCamera(oldCamPos, oldCamTarget);
		oldCamPos = {};
		oldCamTarget = {};
	}

	void DialogueSystem::cancelConversation(entt::entity entity) // Not the best name (isn't on stopping a conversation)
	{
		{
			auto& actorTrans = registry->get<Transform>(entity);
			entt::sink sink{actorTrans.onFinishMovement};
			sink.disconnect<&DialogueSystem::startConversation>(this);
			entt::sink sink2{actorTrans.onMovementCancel};
			sink2.disconnect<&DialogueSystem::cancelConversation>(this);
		}
		clickedNPC = entt::null;
	}

	void DialogueSystem::NPCClicked(entt::entity _clickedNPC)
	{
		if (clickedNPC != entt::null) return;
		clickedNPC = _clickedNPC;
		const auto& npc = registry->get<Dialogue>(_clickedNPC);
		const auto& actorCol = registry->get<Collideable>(controlledActor);
		const auto& npcCol = registry->get<Collideable>(_clickedNPC);
		controllableActorSystem->PathfindToLocation(controlledActor, npc.conversationPos);
		auto& actorTrans = registry->get<Transform>(controlledActor);
		{
			entt::sink sink{actorTrans.onFinishMovement};
			sink.connect<&DialogueSystem::startConversation>(this);
			entt::sink sink2{actorTrans.onMovementCancel};
			sink2.connect<&DialogueSystem::cancelConversation>(this);
		}
	}

	void DialogueSystem::Update()
	{
		if (!active) return;
	}

	void DialogueSystem::Draw2D()
	{
		if (!active) return;
		window->Draw();
	}

	DialogueSystem::DialogueSystem(entt::registry* registry,
	                               Cursor* _cursor,
	                               Camera* _camera,
	                               Settings* _settings,
	                               ControllableActorSystem* _controllableActorSystem) :
		BaseSystem(registry),
		clickedNPC(entt::null),
		controllableActorSystem(_controllableActorSystem),
		cursor(_cursor),
		camera(_camera),
		window(std::make_unique<DialogueWindow>(_settings))
	{
		{
			entt::sink sink{_controllableActorSystem->onControlledActorChange};
			sink.connect<&DialogueSystem::changeControlledActor>(this);
			controlledActor = _controllableActorSystem->GetControlledActor();
		}
		{
			entt::sink sink{_cursor->onNPCClick};
			sink.connect<&DialogueSystem::NPCClicked>(this);
		}
	}
} // sage
