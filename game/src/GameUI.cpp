//
// Created by steve on 02/10/2024.
//

#include "GameUI.hpp"

#include "../../engine/Cursor.hpp"
#include "components/Ability.hpp"
#include "components/CombatableActor.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/QuestComponents.hpp"
#include "engine/Camera.hpp"
#include "GameObjectFactory.hpp"
#include "GameUiFactory.hpp"
#include "QuestManager.hpp"
#include "Systems.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include "engine/components/Renderable.hpp"
#include "engine/components/sgTransform.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/slib.hpp"
#include "engine/UserInput.hpp"
#include "systems/ControllableActorSystem.hpp"

#include "raylib.h"

#include "engine/Cursor.hpp"
#include "magic_enum.hpp"

#include <cassert>
#include <format>
#include <queue>
#include <ranges>
#include <sstream>
#include <unordered_map>

namespace lq
{

#pragma region UIElements

    void JournalEntryManager::updateQuests()
    {
        SetContent("");
        const auto quests = engine->sys->questManager->GetActiveQuests();
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
        QuestManager* _questManager,
        const FontInfo& _fontInfo,
        const sage::VertAlignment _vertAlignment,
        const sage::HoriAlignment _horiAlignment)
        : TextBox(_engine, _parent, _fontInfo, _vertAlignment, _horiAlignment),
          engine(_engine),
          journalEntryRoot(_journalEntryRoot),
          questManager(_questManager)
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
            float offset = 10 * parent->GetWindow()->settings->GetCurrentScaleFactor();
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
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        sage::TableCell* _descriptionCell,
        Quest* _quest,
        const FontInfo& _fontInfo,
        const sage::VertAlignment _vertAlignment,
        const sage::HoriAlignment _horiAlignment)
        : TextBox(_engine, _parent, _fontInfo, _vertAlignment, _horiAlignment),
          engine(_engine),
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
        : TextBox(_engine, _parent, _fontInfo, _vertAlignment, _horiAlignment),
          engine(_engine),
          option(_option),
          index(_index)
    {
        content = std::format("{}: {}", _index, option->description);
    }

    void CharacterStatText::RetrieveInfo()
    {
        auto& combatable = engine->registry->get<CombatableActor>(engine->sys->cursor->GetSelectedActor());
        if (statisticType == StatisticType::NAME)
        {
            const auto& renderable =
                engine->registry->get<sage::Renderable>(engine->sys->cursor->GetSelectedActor());
            SetContent(std::format("{}", renderable.GetVanityName()));
        }
        else if (statisticType == StatisticType::STRENGTH)
        {
            SetContent(std::format("Strength: {}", combatable.baseStatistics.strength));
        }
        else if (statisticType == StatisticType::AGILITY)
        {
            SetContent(std::format("Agility: {}", combatable.baseStatistics.agility));
        }
        else if (statisticType == StatisticType::INTELLIGENCE)
        {
            SetContent(std::format("Intelligence: {}", combatable.baseStatistics.intelligence));
        }
        else if (statisticType == StatisticType::CONSTITUTION)
        {
            SetContent(std::format("Constitution: {}", combatable.baseStatistics.constitution));
        }
        else if (statisticType == StatisticType::WITS)
        {
            SetContent(std::format("Wits: {}", combatable.baseStatistics.wits));
        }
        else if (statisticType == StatisticType::MEMORY)
        {
            SetContent(std::format("Memory: {}", combatable.baseStatistics.memory));
        }
    }

    CharacterStatText::CharacterStatText(
        LeverUIEngine* _engine, sage::TableCell* _parent, const FontInfo& _fontInfo, StatisticType _statisticType)
        : TextBox(_engine, _parent, _fontInfo), statisticType(_statisticType), engine(_engine)
    {
        _engine->sys->cursor->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });

        _engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe([this](entt::entity) { RetrieveInfo(); });

        if (_statisticType == StatisticType::NAME)
        {
            horiAlignment = sage::HoriAlignment::CENTER;
        }

        RetrieveInfo();
    }

    void ResourceOrb::RetrieveInfo()
    {
        // Get health
        // UpdateDimensions();
    }

    void ResourceOrb::Draw2D()
    {
        // auto renderTexture =
        //     engine->registry->get<EquipmentComponent>(engine->sys->controllableActorSystem->GetSelectedActor())
        //         .renderTexture;
        // DrawTextureRec(
        //     renderTexture.texture,
        //     {0,
        //      0,
        //      static_cast<float>(renderTexture.texture.width),
        //      static_cast<float>(-renderTexture.texture.height)},
        //     {rec.x, rec.y},
        //     WHITE);
        //        DrawTextureEx(renderTexture.texture, {rec.x, rec.y}, 0, 0.75f, WHITE);

        // float center_x = rec.x + (rec.width / 2);
        // float center_y = rec.y + (rec.height / 2);
        DrawTexture(tex, rec.x, rec.y, WHITE);
    }

    ResourceOrb::ResourceOrb(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        sage::VertAlignment _vertAlignment,
        sage::HoriAlignment _horiAlignment)
        : ImageBox(_engine, _parent, OverflowBehaviour::SHRINK_TO_FIT, _vertAlignment, _horiAlignment)
    {
        _engine->sys->cursor->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });
    }

    void EquipmentCharacterPreview::UpdateDimensions()
    {
        ImageBox::UpdateDimensions();
        auto& renderTexture =
            engine->registry->get<EquipmentComponent>(engine->sys->cursor->GetSelectedActor()).renderTexture;
        renderTexture.texture.width = parent->GetRec().width;
        renderTexture.texture.height = parent->GetRec().height;
    }

    void EquipmentCharacterPreview::RetrieveInfo()
    {
        engine->sys->equipmentSystem->GenerateRenderTexture(
            engine->sys->cursor->GetSelectedActor(), parent->GetRec().width * 4, parent->GetRec().height * 4);
        UpdateDimensions();
    }

    void EquipmentCharacterPreview::Draw2D()
    {
        auto renderTexture =
            engine->registry->get<EquipmentComponent>(engine->sys->cursor->GetSelectedActor()).renderTexture;
        DrawTextureRec(
            renderTexture.texture,
            {0,
             0,
             static_cast<float>(renderTexture.texture.width),
             static_cast<float>(-renderTexture.texture.height)},
            {rec.x, rec.y},
            WHITE);

        //        DrawTextureEx(renderTexture.texture, {rec.x, rec.y}, 0, 0.75f, WHITE);
    }

    EquipmentCharacterPreview::EquipmentCharacterPreview(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        sage::VertAlignment _vertAlignment,
        sage::HoriAlignment _horiAlignment)
        : ImageBox(_engine, _parent, OverflowBehaviour::SHRINK_TO_FIT, _vertAlignment, _horiAlignment),
          engine(_engine)
    {
        _engine->sys->cursor->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });

        _engine->sys->equipmentSystem->onEquipmentUpdated.Subscribe([this](entt::entity) { RetrieveInfo(); });
    }

    void PartyMemberPortrait::HoverUpdate()
    {
        // Hacky solution.
        // The size of the columm of PartyMemberPortrait is calculated beforehand.
        // However, normally OnHover disables the cursor interacting with the environment when focused on the UI.
        // Therefore, to avoid this, if there is no party member in this slot then we reenable the cursor.
        const auto entity = engine->sys->partySystem->GetMember(memberNumber);
        if (entity == entt::null)
        {
            engine->sys->cursor->EnableContextSwitching();
            engine->sys->cursor->Enable();
        }
    }

    void PartyMemberPortrait::UpdateDimensions()
    {
        ImageBox::UpdateDimensions();
        portraitBgTex.width = tex.width + engine->sys->settings->ScaleValueWidth(10);
        portraitBgTex.height = tex.height + engine->sys->settings->ScaleValueHeight(10);
    }

    void PartyMemberPortrait::RetrieveInfo()
    {
        if (const auto entity = engine->sys->partySystem->GetMember(memberNumber); entity != entt::null)
        {
            const auto& info = engine->registry->get<PartyMemberComponent>(entity);

            engine->sys->equipmentSystem->GeneratePortraitRenderTexture(entity, tex.width * 4, tex.height * 4);

            tex.id = info.portraitImg.texture.id;
        }
        UpdateDimensions();
    }

    void PartyMemberPortrait::ReceiveDrop(CellElement* droppedElement)
    {
        if (const auto entity = engine->sys->partySystem->GetMember(memberNumber); entity == entt::null) return;
        if (const auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            const auto receiver = engine->sys->partySystem->GetMember(memberNumber);
            const auto sender = engine->sys->cursor->GetSelectedActor();
            if (receiver == sender) return;
            auto& receiverInv = engine->registry->get<InventoryComponent>(receiver);
            auto& senderInv = engine->registry->get<InventoryComponent>(sender);
            if (const auto& itemId = senderInv.GetItem(dropped->row, dropped->col); receiverInv.AddItem(itemId))
            {
                senderInv.RemoveItem(dropped->row, dropped->col);
            }
            else
            {
                engine->CreateErrorMessage("Inventory full.");
            }
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            const auto receiver = engine->sys->partySystem->GetMember(memberNumber);
            const auto sender = engine->sys->cursor->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(receiver);

            if (auto droppedItemId = engine->sys->equipmentSystem->GetItem(sender, droppedE->itemType);
                inventory.AddItem(droppedItemId))
            {
                engine->sys->equipmentSystem->DestroyItem(sender, droppedE->itemType);
                droppedE->RetrieveInfo();
                RetrieveInfo();
            }
            else
            {
                engine->CreateErrorMessage("Inventory full.");
            }
        }
    }

    void PartyMemberPortrait::OnClick()
    {
        const auto entity = engine->sys->partySystem->GetMember(memberNumber);
        if (entity == entt::null) return;
        engine->sys->cursor->SetSelectedActor(entity);
    }

    void PartyMemberPortrait::Draw2D()
    {
        const auto entity = engine->sys->partySystem->GetMember(memberNumber);
        if (entity == entt::null) return;
        if (engine->sys->cursor->GetSelectedActor() == entity)
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
            rec.x - engine->sys->settings->ScaleValueWidth(5),
            rec.y - engine->sys->settings->ScaleValueHeight(5),
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
          engine(_engine),
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
        _engine->sys->cursor->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });
        _engine->sys->partySystem->onPartyChange.Subscribe([this]() { RetrieveInfo(); });
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
              sage::HoriAlignment::CENTER),
          engine(_engine)
    {
    }

    void AbilitySlot::RetrieveInfo()
    {
        if (const Ability* ability = engine->sys->playerAbilitySystem->GetAbility(slotNumber))
        {
            tex = sage::ResourceManager::GetInstance().TextureLoad(ability->icon);
            stateLocked = false;
        }
        else
        {
            stateLocked = true;
            tex = sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/empty-inv_slot.png");
        }
        UpdateDimensions();
    }

    void AbilitySlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<AbilitySlot*>(droppedElement))
        {
            engine->sys->playerAbilitySystem->SwapAbility(slotNumber, dropped->slotNumber);
            dropped->RetrieveInfo();
            RetrieveInfo();
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
        if (const auto* ability = engine->sys->playerAbilitySystem->GetAbility(slotNumber))
        {
            tooltipWindow = GameUiFactory::CreateAbilityToolTip(engine, *ability, {rec.x, rec.y});
            const auto _rec = tooltipWindow.value()->GetRec();
            tooltipWindow.value()->SetPos(_rec.x, _rec.y - _rec.height);
            tooltipWindow.value()->InitLayout();
        }
    }

    void AbilitySlot::Draw2D()
    {
        const auto ability = engine->sys->playerAbilitySystem->GetAbility(slotNumber);
        if (!ability)
        {
            ImageBox::Draw2D();
            return;
        }
        if (ability->CooldownReady())
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
        engine->sys->playerAbilitySystem->PressAbility(slotNumber);
        CellElement::OnClick();
    }

    AbilitySlot::AbilitySlot(LeverUIEngine* _engine, sage::TableCell* _parent, const unsigned int _slotNumber)
        : ImageBox(
              _engine,
              _parent,
              OverflowBehaviour::SHRINK_ROW_TO_FIT,
              sage::VertAlignment::MIDDLE,
              sage::HoriAlignment::CENTER),
          engine(_engine),
          slotNumber(_slotNumber)
    {
        draggable = true;
        canReceiveDragDrops = true;
        engine->sys->cursor->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });
    }

    Texture ItemSlot::getEmptyTex()
    {
        return backgroundTex; // TODO: Replace with a better solution (drawing two background textures with this
                              // solution).
    }

    void ItemSlot::dropItemInWorld()
    {
        // TODO: If you're going to have it so that an item can only be picked up if you can pathfind there, you
        // should do the same for this.
        const auto itemId = getItemId();
        const auto cursorPos = engine->sys->cursor->getFirstNaviCollision();
        const auto playerPos =
            engine->registry->get<sage::sgTransform>(engine->sys->cursor->GetSelectedActor()).GetWorldPos();
        const auto dist = Vector3Distance(playerPos, cursorPos.point);

        if (const bool outOfRange = dist > ItemComponent::MAX_ITEM_DROP_RANGE; cursorPos.hit && !outOfRange)
        {
            if (GameObjectFactory::spawnItemInWorld(engine->registry, engine->sys, itemId, cursorPos.point))
            {
                onItemDroppedToWorld();
                RetrieveInfo();
            }
            else
            {
                engine->CreateErrorMessage("Item cannot be dropped.");
            }
        }
        else
        {
            engine->CreateErrorMessage("Out of range.");
        }
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
        if (const auto itemId = getItemId(); itemId != entt::null)
        {
            auto& item = engine->registry->get<ItemComponent>(itemId);
            tooltipWindow = GameUiFactory::CreateItemTooltip(
                engine, item, parent->GetWindow(), {rec.x + rec.width, rec.y - rec.height});
        }
    }

    void ItemSlot::RetrieveInfo()
    {
        if (const auto itemId = getItemId(); itemId != entt::null)
        {
            if (engine->registry->any_of<sage::Renderable>(itemId))
            {
                const auto& renderable = engine->registry->get<sage::Renderable>(itemId);
                std::cout << renderable.GetModel()->GetKey() << std::endl;
            }
            const auto& item = engine->registry->get<ItemComponent>(itemId);
            tex = sage::ResourceManager::GetInstance().TextureLoad(item.icon);
            stateLocked = false;
        }
        else
        {
            stateLocked = true;
            tex = getEmptyTex();
        }
        UpdateDimensions();
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
                !sage::PointInsideRect(characterWindow->GetRec(), GetMousePosition()))
            {
                dropItemInWorld();
            }
        }
    }

    ItemSlot::ItemSlot(
        LeverUIEngine* _engine,
        sage::TableCell* _parent,
        const sage::VertAlignment _vertAlignment,
        const sage::HoriAlignment _horiAlignment)
        : ImageBox(_engine, _parent, OverflowBehaviour::SHRINK_ROW_TO_FIT, _vertAlignment, _horiAlignment),
          engine(_engine)
    {
        draggable = true;
        canReceiveDragDrops = true;
        backgroundTex =
            sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/empty-inv_slot.png");
    }

    Texture EquipmentSlot::getEmptyTex()
    {
        if (itemType == EquipmentSlotName::HELM)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/helm.png");
        }
        if (itemType == EquipmentSlotName::ARMS)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/arms.png");
        }
        if (itemType == EquipmentSlotName::CHEST)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/chest.png");
        }
        if (itemType == EquipmentSlotName::BELT)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/belt.png");
        }
        if (itemType == EquipmentSlotName::BOOTS)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/boots.png");
        }
        if (itemType == EquipmentSlotName::LEGS)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/legs.png");
        }
        if (itemType == EquipmentSlotName::LEFTHAND)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/mainhand.png");
        }
        if (itemType == EquipmentSlotName::RIGHTHAND)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/offhand.png");
        }
        if (itemType == EquipmentSlotName::RING1)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/ring.png");
        }
        if (itemType == EquipmentSlotName::RING2)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/ring.png");
        }
        if (itemType == EquipmentSlotName::AMULET)
        {
            return sage::ResourceManager::GetInstance().TextureLoad("resources/textures/ui/amulet.png");
        }
        return backgroundTex;
    }

    void EquipmentSlot::onItemDroppedToWorld()
    {
        engine->sys->equipmentSystem->DestroyItem(engine->sys->cursor->GetSelectedActor(), itemType);
    }

    bool EquipmentSlot::validateDrop(const ItemComponent& item) const
    {
        if (item.HasFlag(ItemFlags::WEAPON))
        {
            if (itemType == EquipmentSlotName::LEFTHAND)
            {
                return true;
            }
            const ItemFlags RIGHT_HAND_RESTRICTED_FLAGS =
                ItemFlags::MAIN_HAND_ONLY | ItemFlags::TWO_HANDED | ItemFlags::BOW | ItemFlags::CROSSBOW;
            if (itemType == EquipmentSlotName::RIGHTHAND && !item.HasAnyFlag(RIGHT_HAND_RESTRICTED_FLAGS))
            {
                return true;
            }
        }
        else if (item.HasFlag(ItemFlags::ARMOR))
        {
            static const std::unordered_map<EquipmentSlotName, ItemFlags> map{
                {EquipmentSlotName::HELM, ItemFlags::HELMET},
                {EquipmentSlotName::AMULET, ItemFlags::AMULET},
                {EquipmentSlotName::CHEST, ItemFlags::CHEST},
                {EquipmentSlotName::BELT, ItemFlags::BELT},
                {EquipmentSlotName::ARMS, ItemFlags::ARMS},
                {EquipmentSlotName::LEGS, ItemFlags::LEGS},
                {EquipmentSlotName::BOOTS, ItemFlags::BOOTS},
                {EquipmentSlotName::RING1, ItemFlags::RING},
                {EquipmentSlotName::RING2, ItemFlags::RING},
            };
            assert(map.size() == static_cast<int>(EquipmentSlotName::COUNT));
            return item.HasFlag(map.at(itemType));
        }
        return false;
    }

    entt::entity EquipmentSlot::getItemId()
    {
        return engine->sys->equipmentSystem->GetItem(engine->sys->cursor->GetSelectedActor(), itemType);
    }

    void EquipmentSlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            const auto actor = engine->sys->cursor->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(dropped->GetOwner());
            const auto itemId = inventory.GetItem(dropped->row, dropped->col);
            if (const auto& item = engine->registry->get<ItemComponent>(itemId); !validateDrop(item)) return;
            inventory.RemoveItem(dropped->row, dropped->col);
            engine->sys->equipmentSystem->MoveItemToInventory(actor, itemType); // ?
            engine->sys->equipmentSystem->EquipItem(actor, itemId, itemType);
            dropped->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
        else if (const auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            // TODO: BUG: Can swap main hand only to offhand here
            if (const auto actor = engine->sys->cursor->GetSelectedActor();
                !engine->sys->equipmentSystem->SwapItems(actor, itemType, droppedE->itemType))
            {
                // handle swap fail?
            }
        }
    }

    EquipmentSlot::EquipmentSlot(
        LeverUIEngine* _engine, sage::TableCell* _parent, const EquipmentSlotName _itemType)
        : ItemSlot(_engine, _parent, sage::VertAlignment::MIDDLE, sage::HoriAlignment::CENTER), itemType(_itemType)
    {
        engine->sys->cursor->onSelectedActorChange.Subscribe(
            [this](entt::entity, entt::entity) { RetrieveInfo(); });
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

    void InventorySlot::onItemDroppedToWorld()
    {
        auto& inventory = engine->registry->get<InventoryComponent>(owner);
        inventory.RemoveItem(row, col);
        RetrieveInfo();
    }

    entt::entity InventorySlot::getItemId()
    {
        const auto& inventory = engine->registry->get<InventoryComponent>(owner);
        return inventory.GetItem(row, col);
    }

    void InventorySlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            if (dropped->GetOwner() == owner)
            {
                auto& inventory = engine->registry->get<InventoryComponent>(owner);
                inventory.SwapItems(row, col, dropped->row, dropped->col);
            }
            else
            {
                auto& inventory = engine->registry->get<InventoryComponent>(owner);
                auto& droppedInv = engine->registry->get<InventoryComponent>(dropped->GetOwner());
                auto itemId = droppedInv.GetItem(dropped->row, dropped->col);
                if (!inventory.AddItem(itemId, row, col))
                {
                    engine->CreateErrorMessage("Inventory Full.");
                    return;
                }
                droppedInv.RemoveItem(itemId);
            }

            dropped->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            auto& inventory = engine->registry->get<InventoryComponent>(owner);
            const auto droppedItemId = engine->sys->equipmentSystem->GetItem(owner, droppedE->itemType);
            engine->sys->equipmentSystem->DestroyItem(owner, droppedE->itemType);

            if (const auto inventoryItemId = inventory.GetItem(row, col); inventoryItemId != entt::null)
            {
                engine->sys->equipmentSystem->EquipItem(owner, inventoryItemId, droppedE->itemType);
            }

            inventory.AddItem(droppedItemId, row, col);
            droppedE->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
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

    PartyMemberPortrait* LeverUIEngine::CreatePartyMemberPortrait(
        sage::TableCell* cell, std::unique_ptr<PartyMemberPortrait> _portrait)
    {
        cell->element = std::move(_portrait);

        auto* portrait = dynamic_cast<PartyMemberPortrait*>(cell->element.value().get());
        portrait->RetrieveInfo();
        cell->InitLayout();
        return portrait;
    }

    AbilitySlot* LeverUIEngine::CreateAbilitySlot(sage::TableCell* cell, std::unique_ptr<AbilitySlot> _slot)
    {
        cell->element = std::move(_slot);
        auto* abilitySlot = dynamic_cast<AbilitySlot*>(cell->element.value().get());
        abilitySlot->RetrieveInfo();
        cell->InitLayout();
        return abilitySlot;
    }

    EquipmentSlot* LeverUIEngine::CreateEquipmentSlot(sage::TableCell* cell, std::unique_ptr<EquipmentSlot> _slot)
    {
        cell->element = std::move(_slot);
        auto* slot = dynamic_cast<EquipmentSlot*>(cell->element.value().get());
        slot->RetrieveInfo();
        cell->InitLayout();
        return slot;
    }

    InventorySlot* LeverUIEngine::CreateInventorySlot(sage::TableCell* cell, std::unique_ptr<InventorySlot> _slot)
    {
        cell->element = std::move(_slot);
        auto* slot = dynamic_cast<InventorySlot*>(cell->element.value().get());
        slot->RetrieveInfo();
        cell->InitLayout();
        return slot;
    }

    ResourceOrb* LeverUIEngine::CreateResourceOrb(sage::TableCell* cell, std::unique_ptr<ResourceOrb> _orb)
    {
        cell->element = std::move(_orb);
        auto* orb = dynamic_cast<ResourceOrb*>(cell->element.value().get());
        orb->RetrieveInfo();
        cell->InitLayout();
        return orb;
    }

    CharacterStatText* LeverUIEngine::CreateCharacterStatText(
        sage::TableCell* cell, std::unique_ptr<CharacterStatText> _statText)
    {
        cell->element = std::move(_statText);
        auto* charStatText = dynamic_cast<CharacterStatText*>(cell->element.value().get());
        cell->InitLayout();
        return charStatText;
    }

    DialogOption* LeverUIEngine::CreateDialogOption(
        sage::TableCell* cell, std::unique_ptr<DialogOption> _dialogOption)
    {
        cell->element = std::move(_dialogOption);
        auto* textbox = dynamic_cast<DialogOption*>(cell->element.value().get());
        cell->InitLayout();
        return textbox;
    }

    EquipmentCharacterPreview* LeverUIEngine::CreateEquipmentCharacterPreview(
        sage::TableCell* cell, std::unique_ptr<EquipmentCharacterPreview> _preview)
    {
        cell->element = std::move(_preview);
        auto* image = dynamic_cast<EquipmentCharacterPreview*>(cell->element.value().get());
        image->draggable = false;
        cell->InitLayout();
        image->RetrieveInfo();
        return image;
    }

#pragma endregion

    void LeverUIEngine::onWorldItemHover(entt::entity entity)
    {
        if (!sys->inventorySystem->CheckWorldItemRange(true) || tooltipWindow) return;
        auto& item = registry->get<ItemComponent>(entity);
        auto viewport = sys->settings->GetViewPort();
        Vector2 pos = GetWorldToScreenEx(
            sys->cursor->getMouseHitInfo().rlCollision.point,
            *sys->camera->getRaylibCam(),
            viewport.x,
            viewport.y);
        pos.x += sys->settings->ScaleValueWidth(20); // TODO: magic number
        GameUiFactory::CreateWorldTooltip(this, item.localizedName, pos);
    }

    void LeverUIEngine::onNPCHover(entt::entity entity)
    {
        if (tooltipWindow) return;
        auto& renderable = registry->get<sage::Renderable>(entity);
        auto viewport = sys->settings->GetViewPort();
        Vector2 pos = GetWorldToScreenEx(
            sys->cursor->getMouseHitInfo().rlCollision.point,
            *sys->camera->getRaylibCam(),
            viewport.x,
            viewport.y);
        pos.x += sys->settings->ScaleValueWidth(20); // TODO: magic number
        GameUiFactory::CreateWorldTooltip(this, renderable.GetVanityName(), pos);
    }

    void LeverUIEngine::onStopWorldHover() const
    {
        if (tooltipWindow)
        {
            tooltipWindow->Remove();
        }
    }

    LeverUIEngine::LeverUIEngine(entt::registry* _registry, Systems* _sys)
        : GameUIEngine(_registry, _sys), sys(_sys)
    {
        _sys->cursor->onItemHover.Subscribe([this](const entt::entity entity) { onWorldItemHover(entity); });
        _sys->cursor->onStopHover.Subscribe([this]() { onStopWorldHover(); });
        _sys->cursor->onNPCHover.Subscribe([this](const entt::entity entity) { onNPCHover(entity); });
    }

} // namespace lq