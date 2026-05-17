#include "EditorScene.hpp"

#include "EditorGui.hpp"
#include "engine/AudioManager.hpp"
#include "engine/Camera.hpp"
#include "engine/CollisionLayers.hpp"
#include "engine/Cursor.hpp"
#include "engine/EngineSystems.hpp"
#include "engine/GameUiEngine.hpp"
#include "engine/LightManager.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/raylib-cereal.hpp"
#include "engine/UserInput.hpp"
#include "engine/components/Collideable.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/systems/RenderSystem.hpp"
#include "engine/systems/TransformSystem.hpp"

#include "engine/systems/NavigationGridSystem.hpp"
#include "raylib.h"
#include "raymath.h"

#include "cereal/archives/json.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <utility>

namespace sage
{
    namespace
    {
        constexpr float kGridHalfExtent = 50.0f;
        constexpr float kGridPickSurfaceHalfHeight = 0.02f;
        constexpr float kPlacementMarkerHeight = 0.16f;
        constexpr float kPlacementHeightStep = 0.25f;
        constexpr float kPlacementRotationStep = 15.0f;
        constexpr float kPlacementScaleStep = 0.1f;
        constexpr float kPlacementMinScale = 0.1f;
        constexpr Color kPlacementPreviewTint = {255, 255, 255, 150};
        constexpr Color kPlacementPreviewBoundsColor = {37, 99, 235, 210};
        const std::filesystem::path kImportedAssetsDirectory{"resources/Editor/ImportedAssets"};

        BoundingBox TransformBoundingBoxByCorners(const BoundingBox& bounds, const Matrix& transform)
        {
            const std::array<Vector3, 8> corners = {
                Vector3{bounds.min.x, bounds.min.y, bounds.min.z},
                Vector3{bounds.min.x, bounds.min.y, bounds.max.z},
                Vector3{bounds.min.x, bounds.max.y, bounds.min.z},
                Vector3{bounds.min.x, bounds.max.y, bounds.max.z},
                Vector3{bounds.max.x, bounds.min.y, bounds.min.z},
                Vector3{bounds.max.x, bounds.min.y, bounds.max.z},
                Vector3{bounds.max.x, bounds.max.y, bounds.min.z},
                Vector3{bounds.max.x, bounds.max.y, bounds.max.z},
            };

            BoundingBox transformed{};
            transformed.min = transformed.max = Vector3Transform(corners.front(), transform);

            for (const auto& corner : corners)
            {
                const auto worldCorner = Vector3Transform(corner, transform);
                transformed.min.x = std::min(transformed.min.x, worldCorner.x);
                transformed.min.y = std::min(transformed.min.y, worldCorner.y);
                transformed.min.z = std::min(transformed.min.z, worldCorner.z);
                transformed.max.x = std::max(transformed.max.x, worldCorner.x);
                transformed.max.y = std::max(transformed.max.y, worldCorner.y);
                transformed.max.z = std::max(transformed.max.z, worldCorner.z);
            }

            return transformed;
        }

        Matrix BuildPlacementMatrix(const Vector3 position, const float rotationY, const float scale)
        {
            return MatrixMultiply(
                MatrixMultiply(MatrixScale(scale, scale, scale), MatrixRotateY(rotationY * DEG2RAD)),
                MatrixTranslate(position.x, position.y, position.z));
        }

        std::string SanitizeAssetFileStem(const std::string& input)
        {
            std::string result;
            result.reserve(input.size());

            for (const unsigned char ch : input)
            {
                if (std::isalnum(ch) || ch == '-' || ch == '_')
                {
                    result.push_back(static_cast<char>(ch));
                }
                else
                {
                    result.push_back('_');
                }
            }

            return result.empty() ? "asset" : result;
        }
    } // namespace

    const EditorScene::PlaceableMesh& EditorScene::selectedPlaceable() const
    {
        return placeables.at(selectedPlaceableIndex);
    }

    bool EditorScene::isPickState() const
    {
        const auto& state = sys->registry->get<EditorState>(editorStateEntity);
        return std::holds_alternative<EditorPickState>(state.current);
    }

    std::string EditorScene::describeMode() const
    {
        return isPickState() ? "Pick" : "Select";
    }

    std::string EditorScene::describeSelectedAsset() const
    {
        return isPickState() ? selectedPlaceable().displayName : "None";
    }

    std::string EditorScene::describeHoveredGrid() const
    {
        if (!hoveredGridSquare.has_value()) return "None";
        return std::format("{}, {}", hoveredGridSquare->row, hoveredGridSquare->col);
    }

    std::string EditorScene::describePlacementHeight() const
    {
        return std::format("{:.2f}", placementHeightOffset);
    }

    std::string EditorScene::describePlacementRotation() const
    {
        return std::format("{:.0f}", placementRotationY);
    }

    std::string EditorScene::describePlacementScale() const
    {
        return std::format("{:.2f}", placementScale);
    }

    std::string EditorScene::describeSelectedModelDefaultHeight() const
    {
        return isPickState() ? std::format("{:.2f}", selectedPlaceable().modelDefaultHeightOffset) : "0.00";
    }

    std::string EditorScene::describeSelectedModelDefaultRotation() const
    {
        return isPickState() ? std::format("{:.0f}", selectedPlaceable().modelDefaultRotationY) : "0";
    }

    std::string EditorScene::describeSelectedModelDefaultScale() const
    {
        return isPickState() ? std::format("{:.2f}", selectedPlaceable().modelDefaultScale) : "1.00";
    }

    std::string EditorScene::describeEntity(const entt::entity entity) const
    {
        return std::format("entity_{}", entt::to_integral(entity));
    }

    std::string EditorScene::describeSelectedSceneEntity() const
    {
        if (!selectedSceneEntity.has_value()) return "None";
        if (!sys->registry->valid(*selectedSceneEntity) || !sys->registry->any_of<sgTransform>(*selectedSceneEntity))
        {
            return "None";
        }
        return describeEntity(*selectedSceneEntity);
    }

    std::string EditorScene::describeSelectedPositionX() const
    {
        if (!selectedSceneEntity.has_value()) return "0.00";
        const auto& transform = sys->registry->get<sgTransform>(*selectedSceneEntity);
        return std::format("{:.2f}", transform.GetWorldPos().x);
    }

    std::string EditorScene::describeSelectedPositionY() const
    {
        if (!selectedSceneEntity.has_value()) return "0.00";
        const auto& transform = sys->registry->get<sgTransform>(*selectedSceneEntity);
        return std::format("{:.2f}", transform.GetWorldPos().y);
    }

    std::string EditorScene::describeSelectedPositionZ() const
    {
        if (!selectedSceneEntity.has_value()) return "0.00";
        const auto& transform = sys->registry->get<sgTransform>(*selectedSceneEntity);
        return std::format("{:.2f}", transform.GetWorldPos().z);
    }

    std::string EditorScene::describeSelectedRotationX() const
    {
        if (!selectedSceneEntity.has_value()) return "0";
        const auto& transform = sys->registry->get<sgTransform>(*selectedSceneEntity);
        return std::format("{:.0f}", transform.GetWorldRot().x);
    }

    std::string EditorScene::describeSelectedRotationY() const
    {
        if (!selectedSceneEntity.has_value()) return "0";
        const auto& transform = sys->registry->get<sgTransform>(*selectedSceneEntity);
        return std::format("{:.0f}", transform.GetWorldRot().y);
    }

    std::string EditorScene::describeSelectedRotationZ() const
    {
        if (!selectedSceneEntity.has_value()) return "0";
        const auto& transform = sys->registry->get<sgTransform>(*selectedSceneEntity);
        return std::format("{:.0f}", transform.GetWorldRot().z);
    }

    std::string EditorScene::describeSelectedScaleX() const
    {
        if (!selectedSceneEntity.has_value()) return "1.00";
        const auto& transform = sys->registry->get<sgTransform>(*selectedSceneEntity);
        return std::format("{:.2f}", transform.GetScale().x);
    }

    std::string EditorScene::describeSelectedScaleY() const
    {
        if (!selectedSceneEntity.has_value()) return "1.00";
        const auto& transform = sys->registry->get<sgTransform>(*selectedSceneEntity);
        return std::format("{:.2f}", transform.GetScale().y);
    }

    std::string EditorScene::describeSelectedScaleZ() const
    {
        if (!selectedSceneEntity.has_value()) return "1.00";
        const auto& transform = sys->registry->get<sgTransform>(*selectedSceneEntity);
        return std::format("{:.2f}", transform.GetScale().z);
    }

    Matrix EditorScene::modelDefaultTransform(const PlaceableMesh& placeable) const
    {
        const Matrix editableTransform = MatrixMultiply(
            MatrixMultiply(
                MatrixScale(placeable.modelDefaultScale, placeable.modelDefaultScale, placeable.modelDefaultScale),
                MatrixRotateY(placeable.modelDefaultRotationY * DEG2RAD)),
            MatrixTranslate(0.0f, placeable.modelDefaultHeightOffset, 0.0f));

        return MatrixMultiply(editableTransform, placeable.appliedModelDefaultTransform);
    }

    Matrix EditorScene::selectedModelDefaultTransform() const
    {
        return modelDefaultTransform(selectedPlaceable());
    }

    EditorScene::SerializedAssetDefaults EditorScene::serializeAssetDefaults(const PlaceableMesh& placeable) const
    {
        return {
            .displayName = placeable.displayName,
            .modelKey = placeable.modelKey,
            .modelDefaultHeightOffset = placeable.modelDefaultHeightOffset,
            .modelDefaultRotationY = placeable.modelDefaultRotationY,
            .modelDefaultScale = placeable.modelDefaultScale,
            .appliedModelDefaultTransform = placeable.appliedModelDefaultTransform,
        };
    }

    std::string EditorScene::assetDefaultsPath(const PlaceableMesh& placeable) const
    {
        return (kImportedAssetsDirectory / (SanitizeAssetFileStem(placeable.modelKey) + ".json")).string();
    }

    std::string EditorScene::makePlacedLabel(const entt::entity entity) const
    {
        return describeEntity(entity);
    }

    void EditorScene::appendSceneObjectEntry(
        std::vector<editor::EditorGui::SceneObjectEntry>& entries,
        const entt::entity entity,
        const int depth) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        entries.push_back({.entity = entity, .displayName = describeEntity(entity), .depth = depth});

        auto children = sys->registry->get<sgTransform>(entity).GetChildren();
        std::ranges::sort(children, [](const entt::entity lhs, const entt::entity rhs) {
            return entt::to_integral(lhs) < entt::to_integral(rhs);
        });

        for (const auto child : children)
        {
            appendSceneObjectEntry(entries, child, depth + 1);
        }
    }

    std::vector<editor::EditorGui::SceneObjectEntry> EditorScene::collectSceneObjectEntries() const
    {
        std::vector<entt::entity> roots;
        auto view = sys->registry->view<sgTransform>();
        for (const auto entity : view)
        {
            const auto parent = view.get<sgTransform>(entity).GetParent();
            if (parent == entt::null || !sys->registry->valid(parent) || !sys->registry->any_of<sgTransform>(parent))
            {
                roots.push_back(entity);
            }
        }

        std::ranges::sort(roots, [](const entt::entity lhs, const entt::entity rhs) {
            return entt::to_integral(lhs) < entt::to_integral(rhs);
        });

        std::vector<editor::EditorGui::SceneObjectEntry> entries;
        entries.reserve(roots.size());
        for (const auto root : roots)
        {
            appendSceneObjectEntry(entries, root, 0);
        }
        return entries;
    }

    void EditorScene::createGridPickSurface() const
    {
        const auto entity = sys->registry->create();
        const BoundingBox gridBounds = {
            {-kGridHalfExtent, -kGridPickSurfaceHalfHeight, -kGridHalfExtent},
            {kGridHalfExtent, kGridPickSurfaceHalfHeight, kGridHalfExtent}};
        auto& collideable = sys->registry->emplace<Collideable>(entity, gridBounds, MatrixIdentity());
        collideable.SetCollisionLayer(collision_layers::GeometrySimple);
        sys->registry->emplace<StaticCollideable>(entity);
    }

    void EditorScene::refreshPlacementTarget()
    {
        hoveredGridSquare.reset();
        snappedPlacementPosition.reset();

        const auto& collision = sys->cursor->getFirstNaviCollision();
        if (!collision.hit) return;

        GridSquare square{};
        if (!sys->navigationGridSystem->WorldToGridSpace(collision.point, square)) return;

        const auto* gridSquare = sys->navigationGridSystem->GetGridSquare(square.row, square.col);
        if (!gridSquare) return;

        hoveredGridSquare = square;
        snappedPlacementPosition = {
            gridSquare->worldPosCentre.x,
            collision.point.y + placementHeightOffset,
            gridSquare->worldPosCentre.z};
    }

    void EditorScene::refreshOverlay() const
    {
        gui->SetPlacementStatus(
            describeMode(),
            describeSelectedAsset(),
            describeHoveredGrid(),
            describePlacementHeight(),
            describePlacementRotation(),
            describePlacementScale(),
            describeSelectedModelDefaultHeight(),
            describeSelectedModelDefaultRotation(),
            describeSelectedModelDefaultScale(),
            lastPlacedLabel);
        gui->SetSelectedAsset(isPickState() ? std::optional<std::size_t>{selectedPlaceableIndex} : std::nullopt);
    }

    void EditorScene::refreshSceneWindows() const
    {
        const auto activeEntity =
            selectedSceneEntity.has_value() && sys->registry->valid(*selectedSceneEntity) &&
                    sys->registry->any_of<sgTransform>(*selectedSceneEntity)
                ? selectedSceneEntity
                : std::nullopt;

        gui->SetHierarchy(collectSceneObjectEntries(), activeEntity);
        gui->SetInspector(
            describeSelectedSceneEntity(),
            describeSelectedPositionX(),
            describeSelectedPositionY(),
            describeSelectedPositionZ(),
            describeSelectedRotationX(),
            describeSelectedRotationY(),
            describeSelectedRotationZ(),
            describeSelectedScaleX(),
            describeSelectedScaleY(),
            describeSelectedScaleZ());
    }

    void EditorScene::resetPlacementTransform()
    {
        placementHeightOffset = 0.0f;
        placementRotationY = 0.0f;
        placementScale = 1.0f;
        refreshPlacementTarget();
    }

    void EditorScene::selectPlaceable(const std::size_t index)
    {
        if (index >= placeables.size()) return;
        changeState(EditorPickState{.placeableIndex = index});
    }

    void EditorScene::selectSceneEntity(const entt::entity entity)
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;
        selectedSceneEntity = entity;
        changeState(EditorSelectState{});
        gui->HideDeleteConfirmation();
        refreshSceneWindows();
    }

    bool EditorScene::selectSceneEntityUnderCursor()
    {
        const auto& hitInfo = sys->cursor->getMouseHitInfo();
        if (hitInfo.collidedEntityId == entt::null ||
            !sys->registry->valid(hitInfo.collidedEntityId) ||
            !sys->registry->any_of<sgTransform>(hitInfo.collidedEntityId))
        {
            return false;
        }

        selectSceneEntity(hitInfo.collidedEntityId);
        return true;
    }

    void EditorScene::clearSceneEntitySelection()
    {
        selectedSceneEntity.reset();
        gui->HideDeleteConfirmation();
        refreshSceneWindows();
    }

    void EditorScene::requestDeleteSelectedEntity()
    {
        if (!selectedSceneEntity.has_value()) return;
        if (!sys->registry->valid(*selectedSceneEntity) || !sys->registry->any_of<sgTransform>(*selectedSceneEntity))
        {
            clearSceneEntitySelection();
            return;
        }

        gui->ShowDeleteConfirmation(describeSelectedSceneEntity());
    }

    void EditorScene::cancelDeleteSelectedEntity()
    {
        gui->HideDeleteConfirmation();
    }

    void EditorScene::releaseNavigationOccupation(const entt::entity entity) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->any_of<Collideable>(entity)) return;

        const auto& collideable = sys->registry->get<Collideable>(entity);
        if (collideable.blocksNavigation)
        {
            sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, false, entity);
        }
    }

    void EditorScene::deleteEntityAndChildren(const entt::entity entity)
    {
        if (!sys->registry->valid(entity)) return;

        std::vector<entt::entity> children;
        if (sys->registry->any_of<sgTransform>(entity))
        {
            children = sys->registry->get<sgTransform>(entity).GetChildren();
        }

        for (const auto child : children)
        {
            deleteEntityAndChildren(child);
        }

        if (sys->registry->valid(entity) && sys->registry->any_of<sgTransform>(entity))
        {
            sys->transformSystem->SetParent(entity, entt::null);
        }

        releaseNavigationOccupation(entity);

        if (sys->registry->valid(entity))
        {
            sys->registry->destroy(entity);
        }
    }

    void EditorScene::confirmDeleteSelectedEntity()
    {
        if (!selectedSceneEntity.has_value())
        {
            gui->HideDeleteConfirmation();
            return;
        }

        const auto entity = *selectedSceneEntity;
        selectedSceneEntity.reset();
        gui->HideDeleteConfirmation();
        deleteEntityAndChildren(entity);
        refreshSceneWindows();
        refreshOverlay();
    }

    void EditorScene::cyclePlaceable()
    {
        if (placeables.empty()) return;
        const std::size_t nextIndex = isPickState() ? (selectedPlaceableIndex + 1) % placeables.size() : 0;
        selectPlaceable(nextIndex);
    }

    void EditorScene::adjustPlacementHeight(const float amount)
    {
        placementHeightOffset += amount;
        refreshPlacementTarget();
        refreshOverlay();
    }

    void EditorScene::adjustPlacementRotation(const float amount)
    {
        placementRotationY += amount;
        if (placementRotationY >= 360.0f) placementRotationY -= 360.0f;
        if (placementRotationY < 0.0f) placementRotationY += 360.0f;
        refreshOverlay();
    }

    void EditorScene::adjustPlacementScale(const float amount)
    {
        placementScale = std::max(kPlacementMinScale, placementScale + amount);
        refreshOverlay();
    }

    void EditorScene::adjustSelectedTransform(const editor::EditorGui::TransformField field, const float amount)
    {
        if (!selectedSceneEntity.has_value()) return;
        const auto entity = *selectedSceneEntity;
        if (!sys->registry->valid(entity) || !sys->registry->any_of<sgTransform>(entity)) return;

        const auto& transform = sys->registry->get<sgTransform>(entity);
        auto position = transform.GetWorldPos();
        auto rotation = transform.GetWorldRot();
        auto scale = transform.GetScale();

        switch (field)
        {
        case editor::EditorGui::TransformField::PositionX:
            position.x += amount;
            sys->transformSystem->SetPosition(entity, position);
            break;
        case editor::EditorGui::TransformField::PositionY:
            position.y += amount;
            sys->transformSystem->SetPosition(entity, position);
            break;
        case editor::EditorGui::TransformField::PositionZ:
            position.z += amount;
            sys->transformSystem->SetPosition(entity, position);
            break;
        case editor::EditorGui::TransformField::RotationX:
            rotation.x += amount;
            sys->transformSystem->SetRotation(entity, rotation);
            break;
        case editor::EditorGui::TransformField::RotationY:
            rotation.y += amount;
            sys->transformSystem->SetRotation(entity, rotation);
            break;
        case editor::EditorGui::TransformField::RotationZ:
            rotation.z += amount;
            sys->transformSystem->SetRotation(entity, rotation);
            break;
        case editor::EditorGui::TransformField::ScaleX:
            scale.x = std::max(kPlacementMinScale, scale.x + amount);
            sys->transformSystem->SetScale(entity, scale);
            break;
        case editor::EditorGui::TransformField::ScaleY:
            scale.y = std::max(kPlacementMinScale, scale.y + amount);
            sys->transformSystem->SetScale(entity, scale);
            break;
        case editor::EditorGui::TransformField::ScaleZ:
            scale.z = std::max(kPlacementMinScale, scale.z + amount);
            sys->transformSystem->SetScale(entity, scale);
            break;
        }

        updateEntityCollisionBounds(entity);
        refreshSceneWindows();
    }

    void EditorScene::adjustSelectedModelDefaultHeight(const float amount)
    {
        if (!isPickState()) return;
        placeables.at(selectedPlaceableIndex).modelDefaultHeightOffset += amount;
        refreshOverlay();
    }

    void EditorScene::adjustSelectedModelDefaultRotation(const float amount)
    {
        if (!isPickState()) return;
        auto& rotationY = placeables.at(selectedPlaceableIndex).modelDefaultRotationY;
        rotationY += amount;
        if (rotationY >= 360.0f) rotationY -= 360.0f;
        if (rotationY < 0.0f) rotationY += 360.0f;
        refreshOverlay();
    }

    void EditorScene::adjustSelectedModelDefaultScale(const float amount)
    {
        if (!isPickState()) return;
        auto& scale = placeables.at(selectedPlaceableIndex).modelDefaultScale;
        scale = std::max(kPlacementMinScale, scale + amount);
        refreshOverlay();
    }

    void EditorScene::applySelectedModelDefaults()
    {
        if (!isPickState()) return;
        auto& placeable = placeables.at(selectedPlaceableIndex);
        placeable.appliedModelDefaultTransform = modelDefaultTransform(placeable);
        placeable.modelDefaultHeightOffset = 0.0f;
        placeable.modelDefaultRotationY = 0.0f;
        placeable.modelDefaultScale = 1.0f;
        saveAssetDefaults(placeable);
        refreshOverlay();
    }

    void EditorScene::resetSelectedModelDefaults()
    {
        if (!isPickState()) return;
        auto& placeable = placeables.at(selectedPlaceableIndex);
        placeable.appliedModelDefaultTransform = MatrixIdentity();
        placeable.modelDefaultHeightOffset = 0.0f;
        placeable.modelDefaultRotationY = 0.0f;
        placeable.modelDefaultScale = 1.0f;
        saveAssetDefaults(placeable);
        refreshOverlay();
    }

    void EditorScene::loadAssetDefaults()
    {
        std::filesystem::create_directories(kImportedAssetsDirectory);
        for (auto& placeable : placeables)
        {
            loadAssetDefaults(placeable);
        }
    }

    void EditorScene::loadAssetDefaults(PlaceableMesh& placeable) const
    {
        const auto path = assetDefaultsPath(placeable);
        if (!std::filesystem::exists(path))
        {
            saveAssetDefaults(placeable);
            return;
        }

        try
        {
            std::ifstream inputFile(path);
            cereal::JSONInputArchive input(inputFile);
            SerializedAssetDefaults defaults{};
            input(cereal::make_nvp("assetDefaults", defaults));

            if (!defaults.modelKey.empty() && defaults.modelKey != placeable.modelKey)
            {
                std::cerr << "WARN: Asset defaults model key mismatch in " << path << std::endl;
                return;
            }

            placeable.displayName = defaults.displayName.empty() ? placeable.displayName : defaults.displayName;
            placeable.modelDefaultHeightOffset = defaults.modelDefaultHeightOffset;
            placeable.modelDefaultRotationY = defaults.modelDefaultRotationY;
            placeable.modelDefaultScale = defaults.modelDefaultScale;
            placeable.appliedModelDefaultTransform = defaults.appliedModelDefaultTransform;
        }
        catch (const cereal::Exception& e)
        {
            std::cerr << "WARN: Failed to load asset defaults from " << path << ": " << e.what() << std::endl;
        }
    }

    void EditorScene::saveAssetDefaults(const PlaceableMesh& placeable) const
    {
        std::filesystem::create_directories(kImportedAssetsDirectory);

        const auto path = assetDefaultsPath(placeable);
        std::ofstream outputFile(path);
        if (!outputFile.is_open())
        {
            std::cerr << "ERROR: Unable to save asset defaults to " << path << std::endl;
            return;
        }

        cereal::JSONOutputArchive output(outputFile);
        auto defaults = serializeAssetDefaults(placeable);
        output(cereal::make_nvp("assetDefaults", defaults));
    }

    void EditorScene::placeSelectedMesh()
    {
        if (!isPickState()) return;
        if (!snappedPlacementPosition.has_value()) return;

        const auto& placeable = selectedPlaceable();
        const auto entity = sys->registry->create();
        sys->registry->emplace<sgTransform>(entity);
        sys->transformSystem->SetPosition(entity, *snappedPlacementPosition);
        sys->transformSystem->SetScale(entity, placementScale);
        sys->transformSystem->SetRotation(entity, {0.0f, placementRotationY, 0.0f});

        auto model = ResourceManager::GetInstance().GetModelView(placeable.modelKey);
        auto& renderable =
            sys->registry->emplace<Renderable>(entity, std::move(model), modelDefaultTransform(placeable));
        lastPlacedLabel = makePlacedLabel(entity);
        renderable.SetName(lastPlacedLabel);

        const auto localBounds = renderable.GetModel()->CalcLocalBoundingBox();
        const auto placementMatrix = BuildPlacementMatrix(*snappedPlacementPosition, placementRotationY, placementScale);
        auto& collideable = sys->registry->emplace<Collideable>(
            entity, localBounds, sys->registry->get<sgTransform>(entity).GetMatrixNoRot());
        collideable.worldBoundingBox = TransformBoundingBoxByCorners(localBounds, placementMatrix);
        collideable.SetCollisionLayer(collision_layers::Obstacle);
        collideable.blocksNavigation = true;
        sys->registry->emplace<StaticCollideable>(entity);
        sys->navigationGridSystem->MarkSquareAreaOccupied(collideable.worldBoundingBox, true, entity);

        ++placedMeshCount;
        selectedSceneEntity = entity;
        refreshSceneWindows();
        refreshOverlay();
    }

    void EditorScene::updateEntityCollisionBounds(const entt::entity entity) const
    {
        if (!sys->registry->valid(entity) || !sys->registry->all_of<sgTransform, Collideable>(entity)) return;

        const auto& transform = sys->registry->get<sgTransform>(entity);
        auto& collideable = sys->registry->get<Collideable>(entity);
        collideable.worldBoundingBox = TransformBoundingBoxByCorners(collideable.localBoundingBox, transform.GetMatrix());
    }

    void EditorScene::changeState(EditorSelectState newState)
    {
        auto& state = sys->registry->get<EditorState>(editorStateEntity);
        std::visit([this](auto& current) { exitState(current); }, state.current);
        state.RemoveAllSubscriptions();
        state.current = std::move(newState);
        enterState(std::get<EditorSelectState>(state.current));
    }

    void EditorScene::changeState(EditorPickState newState)
    {
        auto& state = sys->registry->get<EditorState>(editorStateEntity);
        std::visit([this](auto& current) { exitState(current); }, state.current);
        state.RemoveAllSubscriptions();
        state.current = std::move(newState);
        enterState(std::get<EditorPickState>(state.current));
    }

    void EditorScene::enterState(EditorSelectState&)
    {
        refreshOverlay();
    }

    void EditorScene::exitState(EditorSelectState&)
    {
    }

    void EditorScene::updateState(EditorSelectState&)
    {
        if (IsKeyPressed(KEY_TAB))
        {
            cyclePlaceable();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !sys->UI().GetCellUnderCursor())
        {
            if (!selectSceneEntityUnderCursor())
            {
                clearSceneEntitySelection();
            }
        }
    }

    void EditorScene::drawState3D(const EditorSelectState&) const
    {
    }

    void EditorScene::enterState(EditorPickState& state)
    {
        selectedPlaceableIndex = state.placeableIndex;
        resetPlacementTransform();
        refreshOverlay();
    }

    void EditorScene::exitState(EditorPickState&)
    {
    }

    void EditorScene::updateState(EditorPickState&)
    {
        if (IsKeyPressed(KEY_TAB))
        {
            cyclePlaceable();
        }
        if (IsKeyPressed(KEY_EQUAL))
        {
            adjustPlacementHeight(kPlacementHeightStep);
        }
        if (IsKeyPressed(KEY_MINUS))
        {
            adjustPlacementHeight(-kPlacementHeightStep);
        }
        if (IsKeyPressed(KEY_LEFT_BRACKET))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                adjustPlacementScale(-kPlacementScaleStep);
            }
            else
            {
                adjustPlacementRotation(-kPlacementRotationStep);
            }
        }
        if (IsKeyPressed(KEY_RIGHT_BRACKET))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))
            {
                adjustPlacementScale(kPlacementScaleStep);
            }
            else
            {
                adjustPlacementRotation(kPlacementRotationStep);
            }
        }
        if (IsKeyPressed(KEY_P))
        {
            placeSelectedMesh();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !sys->UI().GetCellUnderCursor())
        {
            if (!selectSceneEntityUnderCursor())
            {
                placeSelectedMesh();
            }
        }
    }

    void EditorScene::drawState3D(const EditorPickState&) const
    {
        if (!snappedPlacementPosition.has_value()) return;

        drawPlacementPreview();

        const Vector3 marker = {
            snappedPlacementPosition->x,
            snappedPlacementPosition->y + kPlacementMarkerHeight,
            snappedPlacementPosition->z};
        DrawCubeWires(marker, 1.0f, kPlacementMarkerHeight, 1.0f, GOLD);
        DrawSphere(marker, 0.08f, GOLD);
    }

    void EditorScene::drawPlacementPreview() const
    {
        if (!snappedPlacementPosition.has_value()) return;

        const auto& placeable = selectedPlaceable();
        auto previewModel = ResourceManager::GetInstance().GetModelView(placeable.modelKey);
        previewModel.SetTransform(selectedModelDefaultTransform());
        previewModel.Draw(
            *snappedPlacementPosition,
            {0.0f, 1.0f, 0.0f},
            placementRotationY,
            {placementScale, placementScale, placementScale},
            kPlacementPreviewTint);

        const auto previewBounds = TransformBoundingBoxByCorners(
            previewModel.CalcLocalBoundingBox(),
            BuildPlacementMatrix(*snappedPlacementPosition, placementRotationY, placementScale));
        DrawBoundingBox(previewBounds, kPlacementPreviewBoundsColor);
    }

    void EditorScene::Update()
    {
        sys->audioManager->Update();
        sys->userInput->ListenForInput();
        sys->camera->Update();
        sys->cursor->Update();
        refreshPlacementTarget();

        if (IsKeyPressed(KEY_DELETE) && !gui->IsDeleteConfirmationVisible())
        {
            requestDeleteSelectedEntity();
        }

        auto& state = sys->registry->get<EditorState>(editorStateEntity);
        std::visit([this](auto& current) { updateState(current); }, state.current);

        refreshOverlay();
        refreshSceneWindows();
        sys->UI().Update();
    }

    void EditorScene::Draw3D() const
    {
        sys->renderSystem->Draw();
        DrawGrid(120, 1.0f);
        DrawLine3D({0, 0.02f, 0}, {8, 0.02f, 0}, RED);
        DrawLine3D({0, 0.02f, 0}, {0, 8, 0}, GREEN);
        DrawLine3D({0, 0.02f, 0}, {0, 0.02f, 8}, BLUE);

        const auto& state = sys->registry->get<EditorState>(editorStateEntity);
        std::visit([this](const auto& current) { drawState3D(current); }, state.current);

        if (selectedSceneEntity.has_value() && sys->registry->valid(*selectedSceneEntity) &&
            sys->registry->any_of<Collideable>(*selectedSceneEntity))
        {
            DrawBoundingBox(sys->registry->get<Collideable>(*selectedSceneEntity).worldBoundingBox, ORANGE);
        }
    }

    void EditorScene::Draw2D() const
    {
        sys->UI().Draw2D();
    }

    EditorScene::EditorScene(EngineSystems* _sys) : sys(_sys)
    {
        sys->navigationGridSystem->Init(100, 1.0f);
        createGridPickSurface();
        placeables = {
            PlaceableMesh{"Sphere", "vfx_sphere", "SPHERE"},
            PlaceableMesh{"Flat Torus", "vfx_flattorus", "FLAT_TORUS"},
            PlaceableMesh{"Sword", "mdl_sword", "SWORD"},
        };
        loadAssetDefaults();

        editorStateEntity = sys->registry->create();
        sys->registry->emplace<EditorState>(editorStateEntity);

        std::vector<editor::EditorGui::AssetEntry> assetEntries;
        assetEntries.reserve(placeables.size());
        for (const auto& placeable : placeables)
        {
            assetEntries.push_back({placeable.displayName, placeable.modelKey});
        }

        gui = std::make_unique<editor::EditorGui>(
            &sys->UI(),
            sys->settings,
            assetEntries,
            [this](const std::size_t index) { selectPlaceable(index); },
            [this](const entt::entity entity) { selectSceneEntity(entity); },
            editor::EditorGui::ModelDefaultCallbacks{
                .heightDown = [this]() { adjustSelectedModelDefaultHeight(-kPlacementHeightStep); },
                .heightUp = [this]() { adjustSelectedModelDefaultHeight(kPlacementHeightStep); },
                .rotationDown = [this]() { adjustSelectedModelDefaultRotation(-kPlacementRotationStep); },
                .rotationUp = [this]() { adjustSelectedModelDefaultRotation(kPlacementRotationStep); },
                .scaleDown = [this]() { adjustSelectedModelDefaultScale(-kPlacementScaleStep); },
                .scaleUp = [this]() { adjustSelectedModelDefaultScale(kPlacementScaleStep); },
                .apply = [this]() { applySelectedModelDefaults(); },
                .reset = [this]() { resetSelectedModelDefaults(); }},
            editor::EditorGui::InspectorCallbacks{
                .adjustTransform =
                    [this](const editor::EditorGui::TransformField field, const float amount) {
                        adjustSelectedTransform(field, amount);
                    }},
            editor::EditorGui::DeleteConfirmationCallbacks{
                .confirm = [this]() { confirmDeleteSelectedEntity(); },
                .cancel = [this]() { cancelDeleteSelectedEntity(); }});
        refreshOverlay();
        refreshSceneWindows();
    }

    EditorScene::~EditorScene() = default;
} // namespace sage
