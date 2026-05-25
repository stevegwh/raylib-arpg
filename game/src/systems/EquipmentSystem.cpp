//
// Created by Steve Wheeler on 08/11/2024.
//

#include "EquipmentSystem.hpp"
#include "animation/RpgAnimationIds.hpp"

#include "engine/Camera.hpp"
#include "engine/components/Animation.hpp"
#include "engine/components/DeleteEntityComponent.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/components/UberShaderComponent.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/slib.hpp"

#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/WeaponComponent.hpp"
#include "Systems.hpp"

#include "engine/systems/TransformSystem.hpp"
#include "raylib.h"
#include "raymath.h"

namespace lq
{
    namespace
    {
        constexpr unsigned int PreviewPoseFrame = 0;

        struct PreviewCameraGuard
        {
            sage::Camera& camera;
            Camera3D savedCamera;

            explicit PreviewCameraGuard(sage::Camera& _camera)
                : camera(_camera), savedCamera(*_camera.getRaylibCam())
            {
            }

            ~PreviewCameraGuard()
            {
                *camera.getRaylibCam() = savedCamera;
            }
        };

        void ApplyAnimationPose(
            entt::registry& registry, entt::entity entity, int animationIndex, unsigned int frame)
        {
            auto& animation = registry.get<sage::Animation>(entity);
            auto& renderable = registry.get<sage::Renderable>(entity);
            const ModelAnimation& anim = animation.animations[animationIndex];
            renderable.GetModel()->UpdateAnimation(anim, frame);
            animation.onAnimationUpdated.Publish(entity);
        }

        void ApplyAnimationPose(entt::registry& registry, entt::entity entity, sage::AnimationId animationId)
        {
            auto& animation = registry.get<sage::Animation>(entity);
            assert(animation.animationMap.contains(animationId));
            ApplyAnimationPose(registry, entity, animation.animationMap.at(animationId), PreviewPoseFrame);
        }

        void RestoreAnimationPose(
            entt::registry& registry, entt::entity entity, sage::Animation::AnimData animData)
        {
            auto& animation = registry.get<sage::Animation>(entity);
            animation.current = animData;
            ApplyAnimationPose(registry, entity, animation.current.index, animation.current.currentFrame);
        }

        void RecreateRenderTexture(RenderTexture& texture, float width, float height)
        {
            if (texture.id > 0)
            {
                UnloadRenderTexture(texture);
            }
            texture = LoadRenderTexture(static_cast<int>(width), static_cast<int>(height));
        }

        void DrawRenderablePreview(
            entt::registry& registry, entt::entity entity, Vector3 position, float scale, Color tint)
        {
            auto& renderable = registry.get<sage::Renderable>(entity);

            if (registry.any_of<sage::UberShaderComponent>(entity))
            {
                auto& uber = registry.get<sage::UberShaderComponent>(entity);
                const auto originalFlags = uber.materialMap;

                uber.ClearFlagAll(sage::UberShaderComponent::Lit);
                uber.SetShaderBools();
                renderable.GetModel()->Draw(position, scale, tint);

                uber.materialMap = originalFlags;
                uber.SetShaderBools();
                return;
            }

            renderable.GetModel()->Draw(position, scale, tint);
        }

        Vector3 PreviewPositionForChild(
            const sage::sgTransform& ownerTransform,
            const sage::sgTransform& childTransform,
            Vector3 previewOrigin)
        {
            return Vector3Add(
                previewOrigin, Vector3Subtract(childTransform.GetWorldPos(), ownerTransform.GetWorldPos()));
        }

        void DrawEquipmentPreviewModels(
            entt::registry& registry,
            const EquipmentComponent& equipment,
            const sage::sgTransform& ownerTransform,
            Vector3 previewOrigin)
        {
            if (!equipment.worldModels.contains(EquipmentSlotName::LEFTHAND)) return;

            const entt::entity leftHandEntity = equipment.worldModels.at(EquipmentSlotName::LEFTHAND);
            if (!registry.valid(leftHandEntity)) return;

            auto& leftHandTransform = registry.get<sage::sgTransform>(leftHandEntity);
            const Vector3 previewPosition =
                PreviewPositionForChild(ownerTransform, leftHandTransform, previewOrigin);
            DrawRenderablePreview(
                registry, leftHandEntity, previewPosition, leftHandTransform.GetScale().x, WHITE);
        }

        void RenderCharacterPreview(
            entt::registry& registry,
            sage::Camera& camera,
            entt::entity entity,
            RenderTexture& target,
            float width,
            float height,
            sage::AnimationId animationId,
            Vector3 cameraPosition,
            Vector3 cameraTarget,
            Color background,
            bool drawEquipment)
        {
            auto& animation = registry.get<sage::Animation>(entity);
            const auto current = animation.current;

            constexpr Vector3 previewOrigin = {0, -999, 0};
            PreviewCameraGuard cameraGuard(camera);
            camera.SetCamera(cameraPosition, cameraTarget);
            ApplyAnimationPose(registry, entity, animationId);

            RecreateRenderTexture(target, width, height);
            BeginTextureMode(target);
            ClearBackground(background);
            BeginMode3D(*camera.getRaylibCam());

            const auto& transform = registry.get<sage::sgTransform>(entity);
            DrawRenderablePreview(registry, entity, previewOrigin, transform.GetScale().x, WHITE);

            if (drawEquipment)
            {
                const auto& equipment = registry.get<EquipmentComponent>(entity);
                DrawEquipmentPreviewModels(registry, equipment, transform, previewOrigin);
            }

            EndMode3D();
            EndTextureMode();

            RestoreAnimationPose(registry, entity, current);
        }
    } // namespace

    void EquipmentSystem::updateCharacterWeaponPosition(entt::entity owner) const
    {
        const auto& equipment = registry->get<EquipmentComponent>(owner);
        for (auto [k, entity] : equipment.worldModels)
        {
            if (registry->valid(entity) && registry->any_of<WeaponComponent>(entity))
            {
                auto& weapon = registry->get<WeaponComponent>(entity);
                auto& weaponRend = registry->get<sage::Renderable>(entity);
                auto& model = registry->get<sage::Renderable>(weapon.owner).GetModel()->GetRlModel();
                auto boneId = sage::GetBoneIdByName(model.bones, model.boneCount, weapon.parentBoneName.c_str());
                assert(boneId >= 0);
                auto* matrices = model.meshes[0].boneMatrices;
                auto mat = matrices[boneId];
                mat = MatrixMultiply(weapon.parentSocket, mat);
                mat = MatrixMultiply(mat, weaponRend.initialTransform);
                weaponRend.GetModel()->SetTransform(mat);
            }
        }
    }

    void EquipmentSystem::instantiateWeapon(
        entt::entity owner, entt::entity itemId, EquipmentSlotName itemType) const
    {
        auto weaponEntity = registry->create();
        auto& equipment = registry->get<EquipmentComponent>(owner);

        if (equipment.worldModels.contains(itemType) && registry->valid(equipment.worldModels[itemType]))
        {
            DestroyItem(owner, itemType);
        }
        equipment.worldModels[itemType] = weaponEntity;

        Matrix weaponMat;
        {
            // Hard coded location of the "socket" for the weapon
            // TODO: Export sockets as txt and store their transform in weaponSocket
            auto translation = Vector3{-86.803f, 159.62f, 6.0585f};
            Quaternion rotation{0.021f, -0.090f, 0.059f, 0.994f};
            auto scale = Vector3{1, 1, 1};
            weaponMat = sage::ComposeMatrix(translation, rotation, scale);
        }

        auto& weapon = registry->emplace<WeaponComponent>(weaponEntity);
        weapon.parentSocket = weaponMat;
        weapon.parentBoneName = "mixamorig:RightHand";
        weapon.owner = owner;
        const auto& renderable = registry->get<sage::Renderable>(owner);
        const auto& item = registry->get<ItemComponent>(itemId);
        registry->emplace<sage::Renderable>(
            weaponEntity,
            sage::ResourceManager::GetInstance().GetModelView(item.model),
            renderable.initialTransform);
        auto& uber =
            registry->emplace<sage::UberShaderComponent>(weaponEntity, renderable.GetModel()->GetMaterialCount());
        uber.SetFlagAll(sage::UberShaderComponent::Flags::Lit);

        registry->emplace<sage::sgTransform>(weaponEntity);
        sys->engine.transformSystem->SetParent(weaponEntity, owner);
        sys->engine.transformSystem->SetLocalPos(weaponEntity, Vector3Zero());
        sys->engine.transformSystem->SetLocalRot(weaponEntity, {0, 0, 0, 0});
        auto& animation = registry->get<sage::Animation>(owner);
        weapon.animationFollowSub = animation.onAnimationUpdated.Subscribe(
            [this](entt::entity _entity) { updateCharacterWeaponPosition(_entity); });
    }

    void EquipmentSystem::GeneratePortraitRenderTexture(entt::entity entity, float width, float height)
    {
        auto& info = registry->get<PartyMemberComponent>(entity);
        RenderCharacterPreview(
            *registry,
            *sys->engine.camera,
            entity,
            info.portraitImg,
            width,
            height,
            lq::animation_ids::Idle2,
            {-1.5, -992.5, 1.5},
            {0.5, -992.5, 0},
            BLACK,
            false);
    }

    void EquipmentSystem::GenerateRenderTexture(entt::entity entity, float width, float height)
    {

        auto& equipment = registry->get<EquipmentComponent>(entity);
        RenderCharacterPreview(
            *registry,
            *sys->engine.camera,
            entity,
            equipment.renderTexture,
            width,
            height,
            lq::animation_ids::Idle,
            {6, -996, 12},
            {0, -996, 0},
            BLANK,
            true);
    }

    entt::entity EquipmentSystem::GetItem(entt::entity owner, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (!equipment.slots.contains(itemType)) return entt::null;
        return equipment.slots[itemType];
    }

    void EquipmentSystem::EquipItem(entt::entity owner, entt::entity item, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        equipment.slots[itemType] = item;
        if (itemType == EquipmentSlotName::LEFTHAND)
        {
            instantiateWeapon(owner, item, itemType);
        }
        onEquipmentUpdated.Publish(owner);
    }

    void EquipmentSystem::MoveItemToInventory(entt::entity owner, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (equipment.slots[itemType] != entt::null)
        {
            auto& inventory = registry->get<InventoryComponent>(owner);
            if (!inventory.AddItem(equipment.slots[itemType]))
            {
                inventory.onInventoryFull.Publish();
                return;
            }
            DestroyItem(owner, itemType);
        }
    }

    void EquipmentSystem::DestroyItem(entt::entity owner, EquipmentSlotName itemType) const
    {
        auto& equipment = registry->get<EquipmentComponent>(owner);
        if (equipment.slots[itemType] != entt::null)
        {
            if (equipment.worldModels.contains(itemType) && equipment.worldModels[itemType] != entt::null)
            {
                sys->engine.transformSystem->SetParent(equipment.worldModels[itemType], entt::null);
                registry->emplace<sage::DeleteEntityComponent>(equipment.worldModels[itemType]);
                auto& weapon = registry->get<WeaponComponent>(equipment.worldModels[itemType]);
                weapon.animationFollowSub.UnSubscribe();
                equipment.worldModels[itemType] = entt::null;
            }
            equipment.slots[itemType] = entt::null;
            onEquipmentUpdated.Publish(owner);
        }
    }

    bool EquipmentSystem::SwapItems(entt::entity owner, EquipmentSlotName itemType1, EquipmentSlotName itemType2)
    {
        bool bothRings = (itemType1 == EquipmentSlotName::RING1 || itemType1 == EquipmentSlotName::RING2) &&
                         (itemType2 == EquipmentSlotName::RING1 || itemType2 == EquipmentSlotName::RING2);

        bool bothHands = (itemType1 == EquipmentSlotName::LEFTHAND || itemType1 == EquipmentSlotName::RIGHTHAND) &&
                         (itemType2 == EquipmentSlotName::LEFTHAND || itemType2 == EquipmentSlotName::RIGHTHAND);

        if (!bothRings && !bothHands)
        {
            return false;
        }

        auto& equipment = registry->get<EquipmentComponent>(owner);

        entt::entity item1 = entt::null;
        entt::entity item2 = entt::null;

        if (equipment.slots.contains(itemType1) && equipment.slots[itemType1] != entt::null)
        {
            item1 = equipment.slots[itemType1];
        }

        if (equipment.slots.contains(itemType2) && equipment.slots[itemType2] != entt::null)
        {
            item2 = equipment.slots[itemType2];
        }

        if (item1 != entt::null)
        {
            DestroyItem(owner, itemType1);
        }
        if (item2 != entt::null)
        {
            DestroyItem(owner, itemType2);
        }

        if (item1 != entt::null)
        {
            EquipItem(owner, item1, itemType2);
        }
        if (item2 != entt::null)
        {
            EquipItem(owner, item2, itemType1);
        }

        return true;
    }

    void EquipmentSystem::onComponentAdded(entt::entity addedEntity)
    {
    }

    void EquipmentSystem::onComponentRemoved(entt::entity removedEntity)
    {
    }

    EquipmentSystem::EquipmentSystem(entt::registry* _registry, Systems* _sys) : registry(_registry), sys(_sys)
    {
        registry->on_construct<EquipmentComponent>().connect<&EquipmentSystem::onComponentAdded>(this);
        registry->on_destroy<EquipmentComponent>().connect<&EquipmentSystem::onComponentRemoved>(this);
    }
} // namespace lq
