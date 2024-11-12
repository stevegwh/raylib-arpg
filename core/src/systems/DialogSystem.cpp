//
// Created by steve on 11/05/2024.
//

#include "DialogSystem.hpp"

#include "Camera.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"

#include "components/Animation.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "GameUiEngine.hpp"
#include "GameUiFactory.hpp"

namespace sage
{

    void DialogSystem::StartConversation(const sgTransform& cutscenePose, entt::entity npc)
    {
        oldCamPos = gameData->camera->GetPosition();
        oldCamTarget = gameData->camera->getRaylibCam()->target;
        gameData->camera->CutscenePose(cutscenePose);
        gameData->camera->LockInput();
        gameData->cursor->DisableContextSwitching();

        dialogWindow = GameUiFactory::CreateDialogWindow(gameData->uiEngine.get(), npc);
    }

    void DialogSystem::EndConversation()
    {
        gameData->camera->UnlockInput();
        gameData->cursor->EnableContextSwitching();
        gameData->camera->SetCamera(oldCamPos, oldCamTarget);
        oldCamPos = {};
        oldCamTarget = {};
        dialogWindow->Remove();
    }

    DialogSystem::DialogSystem(entt::registry* registry, GameData* _gameData)
        : BaseSystem(registry), gameData(_gameData)
    {
    }
} // namespace sage
