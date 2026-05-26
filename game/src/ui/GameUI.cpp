//
// Created by steve on 02/10/2024.
//

#include "GameUI.hpp"

#include "engine/Camera.hpp"
#include "engine/components/Renderable.hpp"
#include "engine/Cursor.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/slib.hpp"
#include "engine/systems/ActorMovementSystem.hpp"

#include "collision/RpgCollisionLayers.hpp"
#include "components/ItemComponent.hpp"
#include "components/QuestComponents.hpp"
#include "QuestManager.hpp"
#include "Systems.hpp"
#include "systems/InventorySystem.hpp"
#include "ui/GameUiFactory.hpp"

#include "raylib.h"

#include <format>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>

namespace lq
{
    namespace
    {
        std::string_view GetPathfindFailureMessage(const sage::PathfindFailureReason reason)
        {
            switch (reason)
            {
            case sage::PathfindFailureReason::DestinationOutOfGrid:
            case sage::PathfindFailureReason::ActorOutOfGrid:
                return "Out of bounds.";
            case sage::PathfindFailureReason::DestinationOutOfRange:
                return "Out of range.";
            case sage::PathfindFailureReason::DestinationUnreachable:
                return "Destination unreachable.";
            }

            return "Destination unreachable.";
        }
    } // namespace

#pragma region UIElements

    void JournalEntryManager::updateQuests()
    {
        SetContent("");
        const auto quests = questManager->GetActiveQuests();
        const auto table = journalEntryRoot->CreateTable();
        for (auto i = 0; i < 12; ++i) // Adjust for spacing
        {
            Quest* quest = nullptr;
            if (i < quests.size())
            {
                quest = quests[i];
            }
            const auto row = table->CreateTableRow();
            auto cell = row->CreateTableCell();
            auto textbox = std::make_unique<JournalEntry>(
                engine,
                cell,
                this->parent,
                quest,
                FontInfo{},
                sage::VertAlignment::MIDDLE,
                sage::HoriAlignment::CENTER);
            cell->element = std::move(textbox);
        }
    }

    JournalEntryManager::JournalEntryManager(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        sage::TableCell* _journalEntryRoot,
        const FontInfo& _fontInfo,
        const sage::VertAlignment _vertAlignment,
        const sage::HoriAlignment _horiAlignment)
        : TextBox(_engine, _parent, _fontInfo, _vertAlignment, _horiAlignment),
          journalEntryRoot(_journalEntryRoot),
          questManager(_engine->sys->questManager.get())
    {
        questManager->onQuestUpdate.Subscribe([this](entt::entity) { updateQuests(); });
    }

    void JournalEntry::OnHoverStart()
    {
        if (!quest) return;
        drawHighlight = true;
        TextBox::OnHoverStart();
    }

    void JournalEntry::OnHoverStop()
    {
        if (!quest) return;
        drawHighlight = false;
        TextBox::OnHoverStop();
    }

    void JournalEntry::Draw2D()
    {
        if (!quest) return;
        TextBox::Draw2D();
        if (drawHighlight)
        {
            float offset = 10 * engine->settings->GetCurrentScaleFactor();
            DrawRectangleLines(
                rec.x - offset, rec.y - offset, rec.width + offset * 2, rec.height + offset * 2, BLACK);
        }
    }

    void JournalEntry::OnClick()
    {
        if (!quest) return;
        const auto text = reinterpret_cast<TextBox*>(descriptionCell->element.value().get());
        text->SetContent(quest->journalDescription);
    }

    JournalEntry::JournalEntry(
        sage::GameUIEngine* _engine,
        sage::TableCell* _parent,
        sage::TableCell* _descriptionCell,
        Quest* _quest,
        const FontInfo& _fontInfo,
        const sage::VertAlignment _vertAlignment,
        const sage::HoriAlignment _horiAlignment)
        : TextBox(_engine, _parent, _fontInfo, _vertAlignment, _horiAlignment),
          quest(_quest),
          descriptionCell(_descriptionCell)
    {
        if (quest)
        {
            SetContent(quest->journalTitle);
        }
    }

    void DialogOption::OnHoverStart()
    {
        drawHighlight = true;
        TextBox::OnHoverStart();
    }

    void DialogOption::OnHoverStop()
    {
        drawHighlight = false;
        TextBox::OnHoverStop();
    }

    void DialogOption::Draw2D()
    {
        TextBox::Draw2D();
        if (drawHighlight)
        {
            const float offset = 10 * parent->GetWindow()->settings->GetCurrentScaleFactor();
            DrawRectangleLines(
                rec.x - offset, rec.y - offset, rec.width + offset * 2, rec.height + offset * 2, BLACK);
        }
    }

    void DialogOption::OnClick()
    {
        auto* conversation = option->parent->parent;
        conversation->SelectOption(option);
    }

    DialogOption::DialogOption(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        dialog::Option* _option,
        unsigned int _index,
        const FontInfo& _fontInfo,
        const sage::VertAlignment _vertAlignment,
        const sage::HoriAlignment _horiAlignment)
        : TextBox(_engine, _parent, _fontInfo, _vertAlignment, _horiAlignment), option(_option), index(_index)
    {
        content = std::format("{}: {}", _index, option->description);
    }

    void CharacterStatText::RetrieveInfo()
    {
        SetContent(contentProvider ? contentProvider() : "");
    }

    CharacterStatText::CharacterStatText(
        LeverUIEngine* _engine, sage::TableCell* _parent, const FontInfo& _fontInfo, StatisticType _statisticType)
        : TextBox(_engine, _parent, _fontInfo), statisticType(_statisticType)
    {
        if (_statisticType == StatisticType::NAME)
        {
            horiAlignment = sage::HoriAlignment::CENTER;
        }
    }

    void ResourceOrb::RetrieveInfo()
    {
        // Get health
        // UpdateDimensions();
    }

    void ResourceOrb::Draw2D()
    {
        DrawTexture(tex, rec.x, rec.y, WHITE);
    }

    ResourceOrb::ResourceOrb(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        sage::VertAlignment _vertAlignment,
        sage::HoriAlignment _horiAlignment)
        : ImageBox(_engine, _parent, OverflowBehaviour::SHRINK_TO_FIT, _vertAlignment, _horiAlignment)
    {
        _engine->sys->selectionSystem->onSelectedActorChange.Subscribe([this](entt::entity, entt::entity) { RetrieveInfo(); });
    }

    void EquipmentCharacterPreview::UpdateDimensions()
    {
        ImageBox::UpdateDimensions();
        if (renderTextureProvider)
        {
            if (auto* renderTexture = renderTextureProvider())
            {
                renderTexture->texture.width = parent->GetRec().width;
                renderTexture->texture.height = parent->GetRec().height;
            }
        }
    }

    void EquipmentCharacterPreview::RetrieveInfo()
    {
        if (previewGenerator)
        {
            previewGenerator(parent->GetRec().width * 4, parent->GetRec().height * 4);
        }
        UpdateDimensions();
    }

    void EquipmentCharacterPreview::Draw2D()
    {
        if (!renderTextureProvider) return;
        const auto* renderTexture = renderTextureProvider();
        if (!renderTexture) return;

        DrawTextureRec(
            renderTexture->texture,
            {0,
             0,
             static_cast<float>(renderTexture->texture.width),
             static_cast<float>(-renderTexture->texture.height)},
            {rec.x, rec.y},
            WHITE);

        //        DrawTextureEx(renderTexture.texture, {rec.x, rec.y}, 0, 0.75f, WHITE);
    }

    EquipmentCharacterPreview::EquipmentCharacterPreview(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        sage::VertAlignment _vertAlignment,
        sage::HoriAlignment _horiAlignment)
        : ImageBox(_engine, _parent, OverflowBehaviour::SHRINK_TO_FIT, _vertAlignment, _horiAlignment)
    {
    }

    void PartyMemberPortrait::HoverUpdate()
    {
        // Hacky solution.
        // The size of the columm of PartyMemberPortrait is calculated beforehand.
        // However, normally OnHover disables the cursor interacting with the environment when focused on the UI.
        // Therefore, to avoid this, if there is no party member in this slot then we reenable the cursor.
        const auto entity = GetMember();
        if (entity == entt::null)
        {
            if (onEmptyHovered) onEmptyHovered();
        }
    }

    void PartyMemberPortrait::UpdateDimensions()
    {
        ImageBox::UpdateDimensions();
        portraitBgTex.width = tex.width + engine->settings->ScaleValueWidth(10);
        portraitBgTex.height = tex.height + engine->settings->ScaleValueHeight(10);
    }

    void PartyMemberPortrait::RetrieveInfo()
    {
        if (GetMember() != entt::null && portraitProvider)
        {
            portraitProvider(tex);
        }
        UpdateDimensions();
    }

    void PartyMemberPortrait::ReceiveDrop(CellElement* droppedElement)
    {
        if (GetMember() == entt::null) return;
        onDroppedOnPortrait.Publish(this, droppedElement);
    }

    void PartyMemberPortrait::OnClick()
    {
        if (GetMember() == entt::null) return;
        onPortraitClicked.Publish(this);
    }

    void PartyMemberPortrait::Draw2D()
    {
        const auto entity = GetMember();
        if (entity == entt::null) return;
        if (isSelectedProvider && isSelectedProvider())
        {
            SetHoverShader();
        }

        DrawTextureRec(
            tex, {0, 0, static_cast<float>(tex.width), static_cast<float>(-tex.height)}, {rec.x, rec.y}, WHITE);

        if (shader.has_value())
        {
            BeginShaderMode(shader.value());
        }
        DrawTexture(
            portraitBgTex,
            rec.x - engine->settings->ScaleValueWidth(5),
            rec.y - engine->settings->ScaleValueHeight(5),
            WHITE);
        if (shader.has_value())
        {
            EndShaderMode();
        }
    }

    PartyMemberPortrait::PartyMemberPortrait(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        const unsigned int _memberNumber,
        const int _width,
        const int _height)
        : ImageBox(
              _engine,
              _parent,
              OverflowBehaviour::ALLOW_OVERFLOW,
              sage::VertAlignment::MIDDLE,
              sage::HoriAlignment::CENTER),
          memberNumber(_memberNumber),
          width(_width),
          height(_height)
    {

        portraitBgTex =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/avatar_border_set.png");
        portraitBgTex.width = width;
        portraitBgTex.height = height;
        tex.width = width;
        tex.height = height;
        canReceiveDragDrops = true;
    }

    unsigned int PartyMemberPortrait::GetMemberNumber() const
    {
        return memberNumber;
    }

    entt::entity PartyMemberPortrait::GetMember() const
    {
        if (!memberProvider) return entt::null;
        return memberProvider();
    }

    void DialogPortrait::Draw2D()
    {
        DrawTextureRec(
            tex, {0, 0, static_cast<float>(tex.width), static_cast<float>(-tex.height)}, {rec.x, rec.y}, WHITE);
    }

    DialogPortrait::DialogPortrait(LeverUIEngine* _engine, sage::TableCell* _parent, const Texture& _tex)
        : ImageBox(
              _engine,
              _parent,
              _tex,
              OverflowBehaviour::SHRINK_TO_FIT,
              sage::VertAlignment::MIDDLE,
              sage::HoriAlignment::CENTER)
    {
    }

    void AbilitySlot::RetrieveInfo()
    {
        if (iconProvider) tex = iconProvider();
        if (isInteractiveProvider) stateLocked = !isInteractiveProvider();
        UpdateDimensions();
    }

    void AbilitySlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<AbilitySlot*>(droppedElement))
        {
            onSwapRequested.Publish(dropped);
        }
    }

    void AbilitySlot::HoverUpdate()
    {
        if (!shader.has_value())
        {
            SetHoverShader();
        }
        ImageBox::HoverUpdate();
        if (tooltipWindow.has_value() || GetTime() < hoverTimer + hoverTimerThreshold) return;
        if (tooltipFactory)
        {
            if (auto* tt = tooltipFactory({rec.x, rec.y}))
            {
                tooltipWindow = tt;
                const auto _rec = tt->GetRec();
                tt->SetPos(_rec.x, _rec.y - _rec.height);
                tt->InitLayout();
            }
        }
    }

    void AbilitySlot::Draw2D()
    {
        const bool ready = !cooldownReadyProvider || cooldownReadyProvider();
        if (ready)
        {
            ImageBox::Draw2D();
        }
        else
        {
            SetGrayscale();
            ImageBox::Draw2D();
            RemoveShader();
        }
    }

    void AbilitySlot::OnClick()
    {
        onClicked.Publish();
        CellElement::OnClick();
    }

    AbilitySlot::AbilitySlot(LeverUIEngine* _engine, sage::TableCell* _parent, const unsigned int _slotNumber)
        : ImageBox(
              _engine,
              _parent,
              OverflowBehaviour::SHRINK_ROW_TO_FIT,
              sage::VertAlignment::MIDDLE,
              sage::HoriAlignment::CENTER),
          slotNumber(_slotNumber)
    {
        draggable = true;
        canReceiveDragDrops = true;
        // Providers and event handlers are wired by the factory (see
        // GameUiFactory::CreateAbilityRow). Until they are, RetrieveInfo, Draw2D,
        // HoverUpdate are all defensive (null-checked) and OnClick / ReceiveDrop
        // simply fan out to subscribers — no game-system access happens here.
    }

    Texture ItemSlot::getEmptyTex() const
    {
        if (emptyTextureProvider)
        {
            return emptyTextureProvider();
        }
        return backgroundTex;
    }

    void ItemSlot::dropItemInWorld()
    {
        if (GetItemId() == entt::null) return;
        onDroppedToWorld.Publish(this);
        RetrieveInfo();
    }

    void ItemSlot::updateRectangle(
        const sage::Dimensions& dimensions, const Vector2& offset, const sage::Dimensions& space)
    {
        ImageBox::updateRectangle(dimensions, offset, space);
        backgroundTex.width = dimensions.width;
        backgroundTex.height = dimensions.height;
    }

    void ItemSlot::HoverUpdate()
    {
        ImageBox::HoverUpdate();
        if (tooltipWindow.has_value() || GetTime() < hoverTimer + hoverTimerThreshold) return;
        if (GetItemId() != entt::null && tooltipFactory)
        {
            tooltipWindow = tooltipFactory({rec.x + rec.width, rec.y - rec.height}, parent->GetWindow());
        }
    }

    void ItemSlot::RetrieveInfo()
    {
        if (GetItemId() != entt::null)
        {
            if (iconProvider) tex = iconProvider();
            stateLocked = false;
        }
        else
        {
            stateLocked = true;
            tex = getEmptyTex();
        }
        UpdateDimensions();
    }

    entt::entity ItemSlot::GetItemId() const
    {
        if (!itemProvider) return entt::null;
        return itemProvider();
    }

    void ItemSlot::Draw2D()
    {
        DrawTexture(backgroundTex, rec.x, rec.y, WHITE);
        ImageBox::Draw2D();
    }

    void ItemSlot::OnDrop(CellElement* receiver)
    {
        beingDragged = false;

        if (receiver && receiver->canReceiveDragDrops)
        {
            receiver->ReceiveDrop(this);
        }
        else
        {
            if (const auto* characterWindow = parent->GetWindow();
                !sage::PointInsideRect(characterWindow->GetRec(), engine->ViewportMousePosition()))
            {
                dropItemInWorld();
            }
        }
    }

    void ItemSlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<ItemSlot*>(droppedElement))
        {
            onDroppedOnSlot.Publish(dropped, this);
        }
    }

    ItemSlot::ItemSlot(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        const sage::VertAlignment _vertAlignment,
        const sage::HoriAlignment _horiAlignment)
        : ImageBox(_engine, _parent, OverflowBehaviour::SHRINK_ROW_TO_FIT, _vertAlignment, _horiAlignment)
    {
        draggable = true;
        canReceiveDragDrops = true;
        backgroundTex =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/empty-inv_slot.png");
    }

    EquipmentSlot::EquipmentSlot(
        LeverUIEngine* _engine, sage::TableCell* _parent, const EquipmentSlotName _itemType)
        : ItemSlot(_engine, _parent, sage::VertAlignment::MIDDLE, sage::HoriAlignment::CENTER), itemType(_itemType)
    {
    }

    entt::entity InventorySlot::GetOwner() const
    {
        return owner;
    }

    void InventorySlot::SetOwner(const entt::entity _owner)
    {
        owner = _owner;
        RetrieveInfo();
    }

    InventorySlot::InventorySlot(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        const entt::entity _owner,
        const unsigned int _row,
        const unsigned int _col)
        : ItemSlot(_engine, _parent, sage::VertAlignment::MIDDLE, sage::HoriAlignment::CENTER),
          owner(_owner),
          row(_row),
          col(_col)
    {
    }

#pragma endregion

    void LeverUIEngine::onWorldItemHover(entt::entity entity)
    {
        const auto& col = registry->get<sage::Collideable>(entity);
        if (col.collisionLayer != lq::collision_layers::Item) return;
        if (!sys->inventorySystem->CheckWorldItemRange(true) || tooltipWindow) return;
        auto& item = registry->get<ItemComponent>(entity);
        auto viewport = sys->engine.settings->GetViewPort();
        Vector2 pos = GetWorldToScreenEx(
            cursor->getMouseHitInfo().rlCollision.point, *sys->engine.camera->getRaylibCam(), viewport.x, viewport.y);
        pos.x += sys->engine.settings->ScaleValueWidth(20); // TODO: magic number
        GameUiFactory::CreateWorldTooltip(this, item.localizedName, pos);
    }

    void LeverUIEngine::onNPCHover(entt::entity entity)
    {
        const auto& col = registry->get<sage::Collideable>(entity);
        if (col.collisionLayer != lq::collision_layers::Npc) return;
        if (tooltipWindow) return;
        auto& renderable = registry->get<sage::Renderable>(entity);
        auto viewport = sys->engine.settings->GetViewPort();
        Vector2 pos = GetWorldToScreenEx(
            cursor->getMouseHitInfo().rlCollision.point, *sys->engine.camera->getRaylibCam(), viewport.x, viewport.y);
        pos.x += sys->engine.settings->ScaleValueWidth(20); // TODO: magic number
        GameUiFactory::CreateWorldTooltip(this, renderable.GetVanityName(), pos);
    }

    void LeverUIEngine::onStopWorldHover() const
    {
        if (tooltipWindow)
        {
            tooltipWindow->Remove();
        }
    }

    void LeverUIEngine::onPathfindFailed(
        const entt::entity entity, Vector3, const sage::PathfindFailureReason reason)
    {
        if (entity != sys->selectionSystem->GetSelectedActor()) return;
        CreateErrorMessage(std::string{GetPathfindFailureMessage(reason)});
    }

    LeverUIEngine::LeverUIEngine(entt::registry* _registry, Systems* _sys)
        : GameUIEngine(_registry, _sys->Engine()), sys(_sys)
    {
        _sys->engine.cursor->onHover.Subscribe(
            [this](entt::entity entity, sage::CollisionLayer) { onWorldItemHover(entity); });
        _sys->engine.cursor->onHover.Subscribe(
            [this](entt::entity entity, sage::CollisionLayer) { onNPCHover(entity); });
        _sys->engine.cursor->onStopHover.Subscribe([this]() { onStopWorldHover(); });
        _sys->engine.actorMovementSystem->onPathfindFailed.Subscribe(
            [this](
                const entt::entity entity, const Vector3 destination, const sage::PathfindFailureReason reason) {
                onPathfindFailed(entity, destination, reason);
            });
    }

} // namespace lq
