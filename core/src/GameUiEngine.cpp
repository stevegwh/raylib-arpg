//
// Created by steve on 02/10/2024.
//

#include "GameUiEngine.hpp"

#include "Camera.hpp"
#include "components/CombatableActor.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/Renderable.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "GameUiFactory.hpp"
#include "ResourceManager.hpp"
#include "slib.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/InventorySystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"
#include "UserInput.hpp"

#include <cassert>
#include <ranges>
#include <sstream>

namespace sage
{

#pragma region UIElements
    void UIElement::OnHoverStart()
    {
    }

    void UIElement::OnHoverStop()
    {
    }

    void CellElement::SetVertAlignment(VertAlignment alignment)
    {
        vertAlignment = alignment;
        UpdateDimensions();
    }

    void CellElement::SetHoriAlignment(HoriAlignment alignment)
    {
        horiAlignment = alignment;
        UpdateDimensions();
    }

    void CellElement::OnClick()
    {
        onMouseClicked.publish();
    }

    void CellElement::HoverUpdate()
    {
    }

    void CellElement::OnDragStart()
    {
        beingDragged = true;
    };

    void CellElement::OnDrop(CellElement* receiver)
    {
        beingDragged = false;
        if (receiver && receiver->canReceiveDragDrops)
        {
            receiver->ReceiveDrop(this);
        }
    }

    void CellElement::ReceiveDrop(CellElement* droppedElement)
    {
        if (!canReceiveDragDrops) return;

        std::cout << "Reached here \n";
    }

    void CellElement::ChangeState(std::unique_ptr<UIState> newState)
    {
        if (newState == state || stateLocked) return;

        state->Exit();
        state = std::move(newState);
        state->Enter();
    }

    CellElement::CellElement(GameUIEngine* _engine)
        : engine(_engine), state(std::make_unique<IdleState>(this, engine)){};

    Font TextBox::GetFont() const
    {
        return font;
    }

    void TextBox::UpdateFontScaling()
    {
        float scaleFactor = engine->gameData->settings->GetScreenScaleFactor();
        fontSize = baseFontSize * scaleFactor;
        fontSize = std::clamp(fontSize, minFontSize, maxFontSize);
    }

    void TextBox::SetFont(const Font& _font, float _baseFontSize)
    {
        baseFontSize = _baseFontSize;
        font = _font;
        UpdateFontScaling();
    }

    void TextBox::SetOverflowBehaviour(OverflowBehaviour _behaviour)
    {
        overflowBehaviour = _behaviour;
        UpdateDimensions();
    }

    void TextBox::UpdateDimensions()
    {
        UpdateFontScaling();
        float availableWidth = parent->rec.width - (parent->GetPadding().left + parent->GetPadding().right);

        if (overflowBehaviour == OverflowBehaviour::SHRINK_TO_FIT)
        {
            Vector2 textSize = MeasureTextEx(font, content.c_str(), fontSize, fontSpacing);
            while (textSize.x > availableWidth && fontSize > minFontSize)
            {
                fontSize -= 1;
                textSize = MeasureTextEx(font, content.c_str(), fontSize, fontSpacing);
            }
        }
        else if (overflowBehaviour == OverflowBehaviour::WORD_WRAP)
        {
            std::string wrappedText;
            std::string currentLine;
            std::istringstream words(content);
            std::string word;

            // Process each word
            while (words >> word)
            {
                // Create temporary line with new word
                std::string testLine = currentLine;
                if (!testLine.empty()) testLine += " "; // Add space between words
                testLine += word;

                // Measure the line with the new word
                Vector2 lineSize = MeasureTextEx(font, testLine.c_str(), fontSize, fontSpacing);

                if (lineSize.x <= availableWidth)
                {
                    // Word fits, add it to current line
                    currentLine = testLine;
                }
                else
                {
                    // Word doesn't fit, start new line
                    if (!wrappedText.empty()) wrappedText += "\n";
                    wrappedText += currentLine;
                    currentLine = word;
                }
            }

            // Add the last line
            if (!currentLine.empty())
            {
                if (!wrappedText.empty()) wrappedText += "\n";
                wrappedText += currentLine;
            }

            // Update the content with wrapped text
            content = wrappedText;
        }

        // Measure final text size
        Vector2 textSize = MeasureTextEx(font, content.c_str(), fontSize, fontSpacing);

        float horiOffset = 0;
        float vertOffset = 0;
        float availableHeight = parent->rec.height - (parent->GetPadding().up + parent->GetPadding().down);

        if (vertAlignment == VertAlignment::MIDDLE)
        {
            vertOffset = (availableHeight - textSize.y) / 2;
        }
        else if (vertAlignment == VertAlignment::BOTTOM)
        {
            vertOffset = availableHeight - textSize.y;
        }

        if (horiAlignment == HoriAlignment::RIGHT)
        {
            horiOffset = availableWidth - textSize.x;
        }
        else if (horiAlignment == HoriAlignment::CENTER)
        {
            horiOffset = (availableWidth - textSize.x) / 2;
        }
        else if (horiAlignment == HoriAlignment::WINDOW_CENTER)
        {
            auto window = parent->GetWindow();
            float windowAvailableWidth =
                window->rec.width - (window->GetPadding().left + window->GetPadding().right);
            horiOffset = (windowAvailableWidth - textSize.x) / 2;
        }

        rec = {
            parent->rec.x + parent->GetPadding().left + horiOffset,
            parent->rec.y + parent->GetPadding().up + vertOffset,
            textSize.x,
            textSize.y};
    }

    void TextBox::Draw2D()
    {
        BeginShaderMode(sdfShader);
        DrawTextEx(font, content.c_str(), Vector2{rec.x, rec.y}, fontSize, fontSpacing, BLACK);
        EndShaderMode();
    }

    TextBox::TextBox(GameUIEngine* _engine)
        : CellElement(_engine),
          sdfShader(ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/sdf.fs"))
    {
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
            float offset = 10 * parent->GetWindow()->settings->GetScreenScaleFactor();
            DrawRectangleLines(
                rec.x - offset, rec.y - offset, rec.width + offset * 2, rec.height + offset * 2, BLACK);
        }
    }

    void DialogOption::OnClick()
    {
        auto* conversation = option.parent->parent;
        if (option.nextIndex.has_value())
        {
            conversation->SelectOption(option.nextIndex.value());
        }
        else
        {
            conversation->EndConversation();
        }
    }

    DialogOption::DialogOption(GameUIEngine* _engine, const dialog::Option& _option)
        : TextBox(_engine), option(_option)
    {
        content = option.description;
    }

    void TitleBar::OnDragStart()
    {
        draggedWindow = parent->GetWindow();
        auto mousePos = GetMousePosition();
        dragOffset = {mousePos.x - draggedWindow.value()->rec.x, mousePos.y - draggedWindow.value()->rec.y};
    }

    void TitleBar::DragUpdate()
    {
        auto mousePos = GetMousePosition();
        auto& window = draggedWindow.value();
        auto newPos = Vector2Subtract(mousePos, dragOffset);

        window->SetPosition(newPos.x, newPos.y);
        window->ClampToScreen();
        window->UpdateChildren();
    }

    void TitleBar::OnDrop(CellElement* droppedElement)
    {
        draggedWindow.reset();
        dragOffset = {0, 0};
    }

    TitleBar::TitleBar(GameUIEngine* _engine) : TextBox(_engine)
    {
        draggable = true;
        dragDelayTime = 0.0f;
        dragOffset = {0, 0};
    };

    void ImageBox::SetOverflowBehaviour(OverflowBehaviour _behaviour)
    {
        overflowBehaviour = _behaviour;
        UpdateDimensions();
    }

    void ImageBox::SetHoverShader()
    {
        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/custom/ui_hover.fs");
    }

    void ImageBox::SetGrayscale()
    {
        shader = ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/grayscale.fs");
    }

    void ImageBox::RemoveShader()
    {
        shader.reset();
    }

    void ImageBox::OnIdleStart()
    {
        RemoveShader();
    }

    void ImageBox::OnHoverStart()
    {
        hoverTimer = GetTime();
        SetHoverShader();
        CellElement::OnHoverStart();
    }

    void ImageBox::OnHoverStop()
    {
        hoverTimer = 0;
        if (tooltipWindow.has_value())
        {
            tooltipWindow.value()->Remove();
            tooltipWindow.reset();
        }
        // SetGrayscale();
        RemoveShader();
        CellElement::OnHoverStop();
    }

    void ImageBox::OnDragStart()
    {
        CellElement::OnDragStart();
    }

    void ImageBox::DragDraw()
    {
        auto mousePos = GetMousePosition();
        DrawTexture(tex, mousePos.x - rec.width / 2, mousePos.y - rec.height / 2, WHITE);
    }

    void ImageBox::OnDrop(CellElement* droppedElement)
    {
        CellElement::OnDrop(droppedElement);
    }

    Dimensions ImageBox::calculateAvailableSpace() const
    {
        return {
            parent->rec.width - (parent->GetPadding().left + parent->GetPadding().right),
            parent->rec.height - (parent->GetPadding().up + parent->GetPadding().down)};
    }

    float ImageBox::calculateAspectRatio() const
    {
        return static_cast<float>(tex.width) / tex.height;
    }

    Dimensions ImageBox::calculateInitialDimensions(const Dimensions& space) const
    {
        float originalRatio = calculateAspectRatio();

        if (originalRatio > 1.0f)
        { // Wider than tall
            return {space.width, space.width / originalRatio};
        }
        else
        { // Taller than wide
            return {space.height * originalRatio, space.height};
        }
    }

    Vector2 ImageBox::calculateAlignmentOffset(const Dimensions& dimensions, const Dimensions& space) const
    {
        float vertOffset = 0;
        float horiOffset = 0;

        if (vertAlignment == VertAlignment::MIDDLE)
        {
            vertOffset = (space.height - dimensions.height) / 2;
        }
        else if (vertAlignment == VertAlignment::BOTTOM)
        {
            vertOffset = space.height - dimensions.height;
        }

        if (horiAlignment == HoriAlignment::RIGHT)
        {
            horiOffset = space.width - dimensions.width;
        }
        else if (horiAlignment == HoriAlignment::CENTER)
        {
            horiOffset = (space.width - dimensions.width) / 2;
        }

        return {horiOffset, vertOffset};
    }

    void ImageBox::updateRectangle(const Dimensions& dimensions, const Vector2& offset, const Dimensions& space)
    {
        rec = Rectangle{
            parent->rec.x + parent->GetPadding().left + offset.x,
            parent->rec.y + parent->GetPadding().up + offset.y,
            dimensions.width,
            dimensions.height};

        tex.width = dimensions.width;
        tex.height = dimensions.height;
    }

    void ImageBox::shrinkRowToFit() const
    {
        auto& row = parent->parent->children;

        // First pass: calculate the required scale factor for each cell
        float minScaleFactor = 1.0f;

        for (auto& cell : row)
        {
            auto* imageBox = dynamic_cast<ImageBox*>(cell->children.get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            auto dimensions = imageBox->calculateInitialDimensions(space);

            // Calculate scale factor needed for this image
            float widthRatio = space.width / dimensions.width;
            float heightRatio = space.height / dimensions.height;
            float scaleFactor = std::min(widthRatio, heightRatio);

            // Keep track of the smallest scale factor needed
            minScaleFactor = std::min(minScaleFactor, scaleFactor);
        }

        // Second pass: apply the minimum scale factor to all cells
        for (auto& cell : row)
        {
            auto* imageBox = dynamic_cast<ImageBox*>(cell->children.get());
            if (!imageBox) continue;

            auto space = imageBox->calculateAvailableSpace();
            auto dimensions = imageBox->calculateInitialDimensions(space);

            // Apply uniform scaling
            dimensions.width *= minScaleFactor;
            dimensions.height *= minScaleFactor;

            auto offset = imageBox->calculateAlignmentOffset(dimensions, space);
            imageBox->updateRectangle(dimensions, offset, space);
        }
    }

    Dimensions ImageBox::handleOverflow(const Dimensions& dimensions, const Dimensions& space) const
    {
        if (dimensions.width <= space.width && dimensions.height <= space.height)
        {
            return dimensions;
        }

        if (overflowBehaviour == OverflowBehaviour::SHRINK_TO_FIT)
        {
            float widthRatio = space.width / dimensions.width;
            float heightRatio = space.height / dimensions.height;
            float scaleFactor = std::min(widthRatio, heightRatio);

            return {dimensions.width * scaleFactor, dimensions.height * scaleFactor};
        }

        return dimensions; // Return original dimensions if no scaling needed
    }

    void ImageBox::UpdateDimensions()
    {
        if (overflowBehaviour == OverflowBehaviour::SHRINK_ROW_TO_FIT)
        {
            shrinkRowToFit();
            return;
        }
        auto space = calculateAvailableSpace();

        auto dimensions = calculateInitialDimensions(space);
        dimensions = handleOverflow(dimensions, space);
        auto offset = calculateAlignmentOffset(dimensions, space);

        updateRectangle(dimensions, offset, space);
    }

    void ImageBox::Draw2D()
    {
        if (beingDragged) return;
        if (shader.has_value())
        {
            BeginShaderMode(shader.value());
        }
        DrawTexture(tex, rec.x, rec.y, WHITE);
        if (shader.has_value())
        {
            EndShaderMode();
        }
    }

    ImageBox::ImageBox(GameUIEngine* _engine) : CellElement(_engine)
    {
    }

    void EquipmentCharacterPreview::UpdateDimensions()
    {
        ImageBox::UpdateDimensions();
        auto& renderTexture =
            engine->registry
                ->get<EquipmentComponent>(engine->gameData->controllableActorSystem->GetSelectedActor())
                .renderTexture;
        renderTexture.texture.width = parent->rec.width;
        renderTexture.texture.height = parent->rec.height;
    }

    void EquipmentCharacterPreview::RetrieveInfo()
    {
        engine->gameData->equipmentSystem->GenerateRenderTexture(
            engine->gameData->controllableActorSystem->GetSelectedActor(),
            parent->rec.width * 4,
            parent->rec.height * 4);
        UpdateDimensions();
    }

    void EquipmentCharacterPreview::Draw2D()
    {
        auto renderTexture =
            engine->registry
                ->get<EquipmentComponent>(engine->gameData->controllableActorSystem->GetSelectedActor())
                .renderTexture;
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

    EquipmentCharacterPreview::EquipmentCharacterPreview(GameUIEngine* _engine) : ImageBox(_engine)
    {
    }

    void PartyMemberPortrait::RetrieveInfo()
    {
        auto info = partySystem->GetMember(memberNumber);
        tex = ResourceManager::GetInstance().TextureLoad(info.portraitImage);
        UpdateDimensions();
    }

    void PartyMemberPortrait::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            auto receiver = partySystem->GetMember(memberNumber).entity;
            auto sender = controllableActorSystem->GetSelectedActor();
            if (receiver == sender) return;
            auto& receiverInv = engine->registry->get<InventoryComponent>(receiver);
            auto& senderInv = engine->registry->get<InventoryComponent>(sender);
            if (const auto& itemId = senderInv.GetItem(dropped->row, dropped->col); receiverInv.AddItem(itemId))
            {
                senderInv.RemoveItem(dropped->row, dropped->col);
            }
            else
            {
                // inventory full
            }
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            auto receiver = partySystem->GetMember(memberNumber).entity;
            auto sender = controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(receiver);
            auto droppedItemId = engine->gameData->equipmentSystem->GetItem(sender, droppedE->itemType);

            if (inventory.AddItem(droppedItemId))
            {
                engine->gameData->equipmentSystem->DestroyItem(sender, droppedE->itemType);
                droppedE->RetrieveInfo();
                RetrieveInfo();
            }
            else
            {
                // inventory full
            }
        }
    }

    void PartyMemberPortrait::OnClick()
    {
        controllableActorSystem->SetSelectedActor(partySystem->GetMember(memberNumber).entity);
    }

    void PartyMemberPortrait::Draw2D()
    {
        if (controllableActorSystem->GetSelectedActor() == partySystem->GetMember(memberNumber).entity)
        {
            SetGrayscale();
        }
        ImageBox::Draw2D();
    }

    PartyMemberPortrait::PartyMemberPortrait(GameUIEngine* _engine) : ImageBox(_engine){};

    void AbilitySlot::RetrieveInfo()
    {
        if (const Ability* ability = playerAbilitySystem->GetAbility(slotNumber))
        {
            tex = ResourceManager::GetInstance().TextureLoad(ability->icon);
            stateLocked = false;
        }
        else
        {
            stateLocked = true;
            tex = ResourceManager::GetInstance().TextureLoad("resources/icons/ui/empty.png");
        }
        UpdateDimensions();
    }

    void AbilitySlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<AbilitySlot*>(droppedElement))
        {
            playerAbilitySystem->SwapAbility(slotNumber, dropped->slotNumber);
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
        if (auto* ability = playerAbilitySystem->GetAbility(slotNumber))
        {
            tooltipWindow = GameUiFactory::CreateAbilityToolTip(engine, *ability, {rec.x, rec.y});
            tooltipWindow.value()->rec.y = tooltipWindow.value()->rec.y - tooltipWindow.value()->rec.height;
            tooltipWindow.value()->UpdateChildren();
        }
    }

    void AbilitySlot::Draw2D()
    {
        auto ability = playerAbilitySystem->GetAbility(slotNumber);
        if (!ability) return;
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
        playerAbilitySystem->PressAbility(slotNumber);
        CellElement::OnClick();
    }

    AbilitySlot::AbilitySlot(GameUIEngine* _engine) : ImageBox(_engine)
    {
    }

    void ItemSlot::dropItemInWorld()
    {
        const auto itemId = getItemId();
        const auto cursorPos = engine->gameData->cursor->getFirstNaviCollision();
        const auto playerPos =
            engine->registry->get<sgTransform>(engine->gameData->controllableActorSystem->GetSelectedActor())
                .GetWorldPos();
        const auto dist = Vector3Distance(playerPos, cursorPos.point);

        if (const bool outOfRange = dist > ItemComponent::MAX_ITEM_DROP_RANGE; cursorPos.hit && !outOfRange)
        {
            if (GameObjectFactory::spawnInventoryItem(engine->registry, engine->gameData, itemId, cursorPos.point))
            {
                onItemDroppedToWorld();
                RetrieveInfo();
            }
        }
        else
        {
            // TODO: Report to the user that you can't drop it here.
            std::cout << "Out of range \n";
        }
    }

    void ItemSlot::updateRectangle(const Dimensions& dimensions, const Vector2& offset, const Dimensions& space)
    {
        ImageBox::updateRectangle(dimensions, offset, space);
        emptyTex.width = dimensions.width;
        emptyTex.height = dimensions.height;
    }

    void ItemSlot::HoverUpdate()
    {
        ImageBox::HoverUpdate();
        if (tooltipWindow.has_value() || GetTime() < hoverTimer + hoverTimerThreshold) return;
        auto itemId = getItemId();
        if (itemId != entt::null)
        {
            auto& item = engine->registry->get<ItemComponent>(itemId);
            tooltipWindow =
                GameUiFactory::CreateItemTooltip(engine, item, {rec.x + rec.width, rec.y - rec.height});
        }
    }

    void ItemSlot::RetrieveInfo()
    {
        auto itemId = getItemId();
        if (itemId != entt::null)
        {
            auto& item = engine->registry->get<ItemComponent>(itemId);
            tex = ResourceManager::GetInstance().TextureLoad(item.icon);
            stateLocked = false;
        }
        else
        {
            stateLocked = true;
            tex = emptyTex;
        }
        UpdateDimensions();
    }

    void ItemSlot::Draw2D()
    {
        DrawTexture(emptyTex, rec.x, rec.y, WHITE);
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
                !PointInsideRect(characterWindow->rec, GetMousePosition()))
            {
                dropItemInWorld();
            }
        }
    }

    ItemSlot::ItemSlot(sage::GameUIEngine* _engine) : ImageBox(_engine)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/icons/ui/empty.png");
        emptyTex = ResourceManager::GetInstance().TextureLoad("resources/icons/ui/empty.png");
    }

    void EquipmentSlot::onItemDroppedToWorld()
    {
        engine->gameData->equipmentSystem->DestroyItem(
            engine->gameData->controllableActorSystem->GetSelectedActor(), itemType);
    }

    bool EquipmentSlot::validateDrop(const ItemComponent& item) const
    {
        if (item.HasFlag(ItemFlags::WEAPON))
        {
            if (itemType == EquipmentSlotName::LEFTHAND)
            {
                return true;
            }
            if (itemType == EquipmentSlotName::RIGHTHAND &&
                !item.HasAnyFlag(ItemFlags::RIGHT_HAND_RESTRICTED_FLAGS))
            {
                return true;
            }
        }
        else if (item.HasFlag(ItemFlags::ARMOR))
        {
            if (itemType == EquipmentSlotName::HELM && item.HasFlag(ItemFlags::HELMET))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::AMULET && item.HasFlag(ItemFlags::AMULET))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::CHEST && item.HasFlag(ItemFlags::CHEST))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::BELT && item.HasFlag(ItemFlags::BELT))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::ARMS && item.HasFlag(ItemFlags::ARMS))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::LEGS && item.HasFlag(ItemFlags::LEGS))
            {
                return true;
            }
            if (itemType == EquipmentSlotName::BOOTS && item.HasFlag(ItemFlags::BOOTS))
            {
                return true;
            }
            if ((itemType == EquipmentSlotName::RING1 || itemType == EquipmentSlotName::RING2) &&
                item.HasFlag(ItemFlags::RING))
            {
                return true;
            }
        }

        return false;
    }

    entt::entity EquipmentSlot::getItemId()
    {
        return engine->gameData->equipmentSystem->GetItem(
            engine->gameData->controllableActorSystem->GetSelectedActor(), itemType);
    }

    void EquipmentSlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            const auto actor = engine->gameData->controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(actor);
            const auto itemId = inventory.GetItem(dropped->row, dropped->col);
            auto& item = engine->registry->get<ItemComponent>(itemId);
            if (!validateDrop(item)) return;
            inventory.RemoveItem(dropped->row, dropped->col);
            engine->gameData->equipmentSystem->MoveItemToInventory(actor, itemType);
            engine->gameData->equipmentSystem->EquipItem(actor, itemId, itemType);
            dropped->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            const auto actor = engine->gameData->controllableActorSystem->GetSelectedActor();
            if (!engine->gameData->equipmentSystem->SwapItems(actor, itemType, droppedE->itemType))
            {
                // handle swap fail?
            }
        }
    }

    EquipmentSlot::EquipmentSlot(GameUIEngine* _engine, EquipmentSlotName _itemType)
        : ItemSlot(_engine), itemType(_itemType)
    {
        ResourceManager::GetInstance().ImageLoadFromFile("resources/icons/ui/empty.png");
        emptyTex = ResourceManager::GetInstance().TextureLoad("resources/icons/ui/empty.png");
    }

    void InventorySlot::onItemDroppedToWorld()
    {
        auto& inventory = engine->registry->get<InventoryComponent>(
            engine->gameData->controllableActorSystem->GetSelectedActor());
        inventory.RemoveItem(row, col);
    }

    entt::entity InventorySlot::getItemId()
    {
        auto& inventory = engine->registry->get<InventoryComponent>(
            engine->gameData->controllableActorSystem->GetSelectedActor());
        return inventory.GetItem(row, col);
    }

    void InventorySlot::ReceiveDrop(CellElement* droppedElement)
    {
        if (auto* dropped = dynamic_cast<InventorySlot*>(droppedElement))
        {
            auto& inventory = engine->registry->get<InventoryComponent>(
                engine->gameData->controllableActorSystem->GetSelectedActor());
            inventory.SwapItems(row, col, dropped->row, dropped->col);
            dropped->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
        else if (auto* droppedE = dynamic_cast<EquipmentSlot*>(droppedElement))
        {
            const auto actor = engine->gameData->controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(actor);
            const auto droppedItemId = engine->gameData->equipmentSystem->GetItem(actor, droppedE->itemType);
            engine->gameData->equipmentSystem->DestroyItem(actor, droppedE->itemType);

            if (const auto inventoryItemId = inventory.GetItem(row, col); inventoryItemId != entt::null)
            {
                engine->gameData->equipmentSystem->EquipItem(actor, inventoryItemId, droppedE->itemType);
            }

            inventory.AddItem(droppedItemId, row, col);
            droppedE->RetrieveInfo();
            RetrieveInfo();
            engine->BringClickedWindowToFront(parent->GetWindow());
        }
    }

    InventorySlot::InventorySlot(GameUIEngine* _engine) : ItemSlot(_engine){};

    void CloseButton::OnClick()
    {
        parent->GetWindow()->Hide();
    }

    CloseButton::CloseButton(GameUIEngine* _engine) : ImageBox(_engine){};

    void WindowDocked::OnScreenSizeChange()
    {
        rec = {GetOffset().x, GetOffset().y, GetDimensions().width, GetDimensions().height};
        SetAlignment(vertAlignment, horiAlignment);
        UpdateChildren();
    }

    Vector2 WindowDocked::GetOffset() const
    {
        return Vector2{settings->screenWidth * xOffsetPercent, settings->screenHeight * yOffsetPercent};
    }

    /**
     *
     * @param _xOffsetPercent x offset from docked position in percent of screen (1-100)
     * @param _yOffsetPercent y offset from docked position in percent of screen (1-100)
     */
    void WindowDocked::SetOffsetPercent(float _xOffsetPercent, float _yOffsetPercent)
    {
        xOffsetPercent = _xOffsetPercent / 100;
        yOffsetPercent = _yOffsetPercent / 100;
        UpdateChildren();
    }

    void WindowDocked::SetAlignment(VertAlignment vert, HoriAlignment hori)
    {
        vertAlignment = vert;
        horiAlignment = hori;
        float originalXOffset = GetOffset().x;
        float originalYOffset = GetOffset().y;

        float xOffset = 0;
        float yOffset = 0;

        // Calculate horizontal position
        switch (hori)
        {
        case HoriAlignment::LEFT:
            xOffset = 0;
            break;

        case HoriAlignment::CENTER:
            xOffset = (settings->screenWidth - rec.width) / 2;
            break;

        case HoriAlignment::RIGHT:
            xOffset = settings->screenWidth - rec.width;
            break;
        default:;
        }

        // Calculate vertical position
        switch (vert)
        {
        case VertAlignment::TOP:
            yOffset = 0;
            break;

        case VertAlignment::MIDDLE:
            yOffset = (settings->screenHeight - rec.height) / 2;
            break;

        case VertAlignment::BOTTOM:
            yOffset = settings->screenHeight - rec.height;
            break;
        }

        rec.x = xOffset + originalXOffset;
        rec.y = yOffset + originalYOffset;

        UpdateChildren();
    }

    Panel* Window::CreatePanel()
    {
        children.push_back(std::make_unique<Panel>());
        const auto& panel = children.back();
        panel->parent = this;
        UpdateChildren();
        return panel.get();
    }

    Panel* Window::CreatePanel(float _requestedHeight)
    {
        auto panel = CreatePanel();
        panel->requestedHeight = _requestedHeight;
        panel->autoSize = false;
        UpdateChildren();
        return panel;
    }

    void Window::ToggleHide()
    {
        hidden = !hidden;
    }

    void Window::Show()
    {
        hidden = false;
    }

    void Window::Hide()
    {
        hidden = true;
    }

    bool Window::IsHidden() const
    {
        return hidden;
    }

    bool Window::IsMarkedForRemoval() const
    {
        return markForRemoval;
    }

    void Window::Remove()
    {
        hidden = true;
        markForRemoval = true;
    }

    void Window::OnScreenSizeChange()
    {
        if (markForRemoval) return;
        // TODO: Could do something fancier with changing the x/y based on the new width/height
        // This would be "easy" to do as OnScreenSizeChange is reacting to an event that passes width/height as the
        // parameters (which we currently ignore). Alternatively, we have a pointer to settings, anyway.
        rec = {GetPosition().x, GetPosition().y, GetDimensions().width, GetDimensions().height};
        UpdateChildren();
    }

    Dimensions Window::GetDimensions() const
    {
        return Dimensions{settings->ScaleValue(referenceWidth), settings->ScaleValue(referenceHeight)};
    }

    void Window::SetDimensions(float _width, float _height)
    {
        referenceWidth = _width;
        referenceHeight = _height;

        // TODO: clamp to bounds?
    }

    void Window::SetPosition(float x, float y)
    {
        //        float scaleFactor = settings->GetScreenScaleFactor();
        //        rec.x = x * scaleFactor;
        //        rec.y = y * scaleFactor;
        rec.x = x;
        rec.y = y;
    }

    Vector2 Window::GetPosition() const
    {
        return {rec.x, rec.y};
    }

    void Window::ClampToScreen()
    {
        if (rec.x + rec.width > settings->screenWidth)
        {
            rec.x = settings->screenWidth - rec.width;
        }
        if (rec.y + rec.height > settings->screenHeight)
        {
            rec.y = settings->screenHeight - rec.height;
        }
        if (rec.x < 0)
        {
            rec.x = 0;
        }
        if (rec.y < 0)
        {
            rec.y = 0;
        }
    }

    void Window::OnHoverStart()
    {
        UIElement::OnHoverStart();
    }

    void Window::OnHoverStop()
    {
        UIElement::OnHoverStop();
        for (auto& panel : children)
        {
            for (auto& table : panel->children)
            {
                for (auto& row : table->children)
                {
                    for (auto& cell : row->children)
                    {
                        auto& element = cell->children;
                        // Allow elements being dragged to continue being active outside of window's bounds
                        if (element->beingDragged) continue;
                        element->ChangeState(std::make_unique<IdleState>(element.get(), element->engine));
                    }
                }
            }
        }
    }

    void Window::DrawDebug2D()
    {
        std::vector colors = {RED, BLUE, YELLOW, WHITE, PINK};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& panel = children[i];
            Color col = colors[i];
            col.a = 150;
            DrawRectangle(panel->rec.x, panel->rec.y, panel->rec.width, panel->rec.height, col);
            // panel->DrawDebug2D();
        }
    }

    void Window::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& child : children)
        {
            child->Draw2D();
        }
    }

    // In Window class - always stacks panels vertically
    void Window::UpdateChildren()
    {
        if (children.empty()) return;

        float availableWidth = rec.width - (GetPadding().left + GetPadding().right);
        float availableHeight = rec.height - (GetPadding().up + GetPadding().down);
        float startX = rec.x + GetPadding().left;
        float startY = rec.y + GetPadding().up;

        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based heights
        for (const auto& panel : children)
        {
            if (panel->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += panel->requestedHeight;
            }
        }

        totalRequestedPercent = std::min(totalRequestedPercent, 100.0f);
        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each panel
        float currentY = startY;
        for (const auto& panel : children)
        {
            panel->parent = this;
            panel->rec = rec;

            float panelHeight;
            if (panel->autoSize)
            {
                panelHeight = std::ceil(availableHeight * (autoSizePercent / 100.0f));
            }
            else
            {
                panelHeight = std::ceil(availableHeight * (panel->requestedHeight / 100.0f));
            }

            panel->rec.height = panelHeight;
            panel->rec.y = currentY;
            panel->rec.width = availableWidth;
            panel->rec.x = startX;

            if (!panel->children.empty()) panel->UpdateChildren();

            currentY += panelHeight;
        }
    }

    Window::~Window()
    {
        windowUpdateCnx.release();
    }

    void Panel::DrawDebug2D()
    {
        std::vector colors = {PINK, RED, BLUE, YELLOW, WHITE};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& table = children[i];
            Color col = colors[i];
            col.a = 150;
            DrawRectangle(table->rec.x, table->rec.y, table->rec.width, table->rec.height, col);
            table->DrawDebug2D();
        }
    }

    void Panel::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& table : children)
        {
            table->Draw2D();
        }
    }

    TableGrid* Panel::CreateTableGrid(int rows, int cols, float cellSpacing)
    {
        children.push_back(std::make_unique<TableGrid>());
        const auto& table = dynamic_cast<TableGrid*>(children.back().get());
        table->parent = this;
        table->cellSpacing = cellSpacing;
        // Create rows and cells with initial autoSize = true
        for (int i = 0; i < rows; ++i)
        {
            TableRow* row = table->CreateTableRow();
            for (int j = 0; j < cols; ++j)
            {
                row->CreateTableCell();
            }
        }
        UpdateChildren();
        return table;
    }

    Table* Panel::CreateTable()
    {
        children.push_back(std::make_unique<Table>());
        const auto& table = children.back();
        table->parent = this;
        UpdateChildren();
        return table.get();
    }

    Table* Panel::CreateTable(float _requestedWidth)
    {
        auto table = CreateTable();
        table->autoSize = false;
        table->requestedWidth = _requestedWidth;
        UpdateChildren();
        return table;
    }

    void Panel::UpdateChildren()
    {
        if (children.empty()) return;

        float availableWidth = rec.width - (GetPadding().left + GetPadding().right);
        float availableHeight = rec.height - (GetPadding().up + GetPadding().down);
        float startX = rec.x + GetPadding().left;
        float startY = rec.y + GetPadding().up;

        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based widths
        for (const auto& table : children)
        {
            if (table->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += table->requestedWidth;
            }
        }

        totalRequestedPercent = std::min(totalRequestedPercent, 100.0f);
        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each table
        float currentX = startX;
        for (const auto& table : children)
        {
            table->parent = this;
            table->rec = rec;

            float tableWidth;
            if (table->autoSize)
            {
                tableWidth = std::ceil(availableWidth * (autoSizePercent / 100.0f));
            }
            else
            {
                tableWidth = std::ceil(availableWidth * (table->requestedWidth / 100.0f));
            }

            table->rec.width = tableWidth;
            table->rec.x = currentX;
            table->rec.height = availableHeight;
            table->rec.y = startY;

            if (!table->children.empty()) table->UpdateChildren();

            currentX += tableWidth;
        }
    }

    void TableGrid::UpdateChildren()
    {
        if (children.empty()) return;
        // 1. Get number of columns
        unsigned int cols = children[0]->children.size();

        // 2. Calculate available space
        float availableWidth = rec.width - (GetPadding().left + GetPadding().right);
        float availableHeight = rec.height - (GetPadding().up + GetPadding().down);

        // 3. Find the maximum power-of-two cell size that fits
        int maxCellSize = 1 << static_cast<int>(std::floor(
                              std::log2(std::min(availableWidth / cols, availableHeight / children.size()))));

        // 4. Calculate actual cell size with spacing
        float cellSize = (std::min(availableWidth / cols, availableHeight / children.size()) - cellSpacing) /
                         maxCellSize * maxCellSize;

        // 5. Calculate total grid dimensions
        float gridWidth = cellSize * cols + cellSpacing * (cols - 1); // Account for spacing between columns
        float gridHeight =
            cellSize * children.size() + cellSpacing * (children.size() - 1); // Account for spacing between rows

        // 6. Calculate starting position to center the grid
        float startX = rec.x + (availableWidth - gridWidth) / 2.0f;
        float startY = rec.y + (availableHeight - gridHeight) / 2.0f;

        // 7. Apply dimensions to rows and cells
        float currentY = startY;
        for (const auto& row : children)
        {
            row->rec.height = cellSize;
            row->rec.y = currentY;
            row->rec.x = startX;
            row->rec.width = gridWidth;

            float currentX = row->rec.x;
            for (const auto& cell : row->children)
            {
                cell->rec.width = cellSize;
                cell->rec.x = currentX;
                cell->rec.y = currentY;
                cell->rec.height = cellSize;
                currentX += cellSize + cellSpacing; // Add spacing after each cell
            }
            currentY += cellSize + cellSpacing; // Add spacing after each row
        }

        for (auto& row : children)
        {
            for (auto& cell : row->children)
            {
                if (cell->children)
                {
                    cell->children->UpdateDimensions();
                }
            }
        }
    }

    void Table::DrawDebug2D()
    {
        // std::vector colors = {PINK, RED, BLUE, YELLOW, WHITE};
        // for (int i = 0; i < children.size(); ++i)
        // {
        //     const auto& row = children[i];
        //     Color col = colors[i];
        //     col.a = 150;
        //     // DrawRectangle(row->rec.x, row->rec.y, row->rec.width, row->rec.height, col);
        //     row->DrawDebug2D();
        // }
    }

    void Table::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& row : children)
        {
            row->Draw2D();
        }
    }

    void Table::UpdateChildren()
    {
        // Account for table padding
        float availableHeight = rec.height - (GetPadding().up + GetPadding().down);
        float startY = rec.y + GetPadding().up;
        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based heights
        for (const auto& row : children)
        {
            if (row->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += row->requestedHeight;
            }
        }

        if (totalRequestedPercent > 100.0f)
        {
            totalRequestedPercent = 100.0f;
        }

        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each row
        float currentY = startY;
        for (const auto& row : children)
        {
            row->parent = this;
            row->rec = rec;

            float rowHeight;
            if (row->autoSize)
            {
                rowHeight = std::ceil(availableHeight * (autoSizePercent / 100.0f));
            }
            else
            {
                rowHeight = std::ceil(availableHeight * (row->requestedHeight / 100.0f));
            }

            // Set row dimensions accounting for table padding
            row->rec.height = rowHeight;
            row->rec.y = currentY;
            row->rec.x = rec.x + GetPadding().left;
            row->rec.width = rec.width - (GetPadding().left + GetPadding().right);

            if (!row->children.empty())
            {
                row->UpdateChildren();
            }

            currentY += rowHeight;
        }
    }

    TableRow* Table::CreateTableRow()
    {
        children.push_back(std::make_unique<TableRow>());
        const auto& row = children.back();
        row->parent = this;
        UpdateChildren();
        return row.get();
    }

    /**
     *
     * @param _requestedHeight The desired height of the cell as a percent (0-100)
     * @return
     */
    TableRow* Table::CreateTableRow(float _requestedHeight)
    {
        assert(_requestedHeight <= 100 && _requestedHeight >= 0);
        children.push_back(std::make_unique<TableRow>());
        const auto& row = children.back();
        row->parent = this;
        row->autoSize = false;
        row->requestedHeight = _requestedHeight;
        UpdateChildren();
        return row.get();
    }

    TableCell* TableRow::CreateTableCell()
    {
        children.push_back(std::make_unique<TableCell>());
        const auto& cell = children.back();
        cell->parent = this;
        UpdateChildren();
        return cell.get();
    }

    /**
     *
     * @param requestedWidth The desired width of the cell as a percent (0-100)
     * @return
     */
    TableCell* TableRow::CreateTableCell(float requestedWidth)
    {
        assert(requestedWidth <= 100 && requestedWidth >= 0);
        children.push_back(std::make_unique<TableCell>());
        const auto& cell = children.back();
        cell->parent = this;
        cell->autoSize = false;
        cell->requestedWidth = requestedWidth;
        UpdateChildren();
        return cell.get();
    }

    void TableRow::DrawDebug2D()
    {
        std::vector colors = {RED, BLUE, YELLOW, WHITE, PINK};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& cell = children[i];
            Color col = colors[i];
            col.a = 100;
            DrawRectangle(cell->rec.x, cell->rec.y, cell->rec.width, cell->rec.height, col);
            cell->DrawDebug2D();
        }
    }

    void TableRow::Draw2D()
    {
        TableElement::Draw2D();
        for (const auto& cell : children)
        {
            cell->Draw2D();
        }
    }

    void TableRow::UpdateChildren()
    {
        // Account for row padding
        float availableWidth = rec.width - (GetPadding().left + GetPadding().right);
        float startX = rec.x + GetPadding().left;
        float totalRequestedPercent = 0.0f;
        int autoSizeCount = 0;

        // First pass: Calculate total of percentage-based widths
        for (const auto& cell : children)
        {
            if (cell->autoSize)
            {
                autoSizeCount++;
            }
            else
            {
                totalRequestedPercent += cell->requestedWidth;
            }
        }

        if (totalRequestedPercent > 100.0f)
        {
            totalRequestedPercent = 100.0f;
        }

        float remainingPercent = 100.0f - totalRequestedPercent;
        float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

        // Second pass: Update each cell
        float currentX = startX;
        for (const auto& cell : children)
        {
            cell->parent = this;
            cell->rec = rec;

            float cellWidth;
            if (cell->autoSize)
            {
                cellWidth = std::ceil(availableWidth * (autoSizePercent / 100.0f));
            }
            else
            {
                cellWidth = std::ceil(availableWidth * (cell->requestedWidth / 100.0f));
            }

            // Set cell dimensions accounting for row padding
            cell->rec.width = cellWidth;
            cell->rec.x = currentX;
            cell->rec.y = rec.y + GetPadding().up;
            cell->rec.height = rec.height - (GetPadding().up + GetPadding().down);

            cell->UpdateChildren();

            currentX += cellWidth;
        }
    }

    PartyMemberPortrait* TableCell::CreatePartyMemberPortrait(
        GameUIEngine* engine,
        PartySystem* _partySystem,
        ControllableActorSystem* _controllableActorSystem,
        unsigned int _memberNumber)
    {
        children = std::make_unique<PartyMemberPortrait>(engine);
        auto* portrait = dynamic_cast<PartyMemberPortrait*>(children.get());
        portrait->parent = this;
        portrait->partySystem = _partySystem;
        portrait->controllableActorSystem = _controllableActorSystem;
        portrait->draggable = true;
        portrait->canReceiveDragDrops = true;
        portrait->memberNumber = _memberNumber;
        portrait->RetrieveInfo();
        {
            entt::sink sink{_controllableActorSystem->onSelectedActorChange};
            sink.connect<&PartyMemberPortrait::RetrieveInfo>(portrait);
        }
        UpdateChildren();
        return portrait;
    }

    AbilitySlot* TableCell::CreateAbilitySlot(
        GameUIEngine* engine,
        PlayerAbilitySystem* _playerAbilitySystem,
        ControllableActorSystem* _controllableActorSystem,
        unsigned int _slotNumber)
    {
        children = std::make_unique<AbilitySlot>(engine);
        auto* abilitySlot = dynamic_cast<AbilitySlot*>(children.get());
        abilitySlot->parent = this;
        abilitySlot->playerAbilitySystem = _playerAbilitySystem;
        abilitySlot->controllableActorSystem = _controllableActorSystem;
        abilitySlot->draggable = true;
        abilitySlot->canReceiveDragDrops = true;
        abilitySlot->slotNumber = _slotNumber;
        abilitySlot->RetrieveInfo();
        {
            entt::sink sink{_controllableActorSystem->onSelectedActorChange};
            sink.connect<&AbilitySlot::RetrieveInfo>(abilitySlot);
        }
        UpdateChildren();
        return abilitySlot;
    }

    EquipmentSlot* TableCell::CreateEquipmentSlot(
        GameUIEngine* engine, ControllableActorSystem* _controllableActorSystem, EquipmentSlotName _itemType)
    {
        children = std::make_unique<EquipmentSlot>(engine, _itemType);
        auto* slot = dynamic_cast<EquipmentSlot*>(children.get());
        slot->parent = this;
        slot->controllableActorSystem = _controllableActorSystem;
        // slot->SetGrayscale();
        slot->draggable = true;
        slot->canReceiveDragDrops = true;
        slot->RetrieveInfo();
        UpdateChildren();
        {
            entt::sink sink{_controllableActorSystem->onSelectedActorChange};
            sink.connect<&EquipmentSlot::RetrieveInfo>(slot);
        }
        return slot;
    }

    InventorySlot* TableCell::CreateInventorySlot(
        GameUIEngine* engine,
        ControllableActorSystem* _controllableActorSystem,
        unsigned int row,
        unsigned int col)
    {
        children = std::make_unique<InventorySlot>(engine);
        auto* slot = dynamic_cast<InventorySlot*>(children.get());
        slot->parent = this;
        slot->controllableActorSystem = _controllableActorSystem;
        // slot->SetGrayscale();
        slot->draggable = true;
        slot->canReceiveDragDrops = true;
        slot->row = row;
        slot->col = col;
        slot->RetrieveInfo();
        UpdateChildren();
        {
            entt::sink sink{_controllableActorSystem->onSelectedActorChange};
            sink.connect<&InventorySlot::RetrieveInfo>(slot);
        }
        return slot;
    }

    TitleBar* TableCell::CreateTitleBar(GameUIEngine* engine, const std::string& _title, float fontSize)
    {
        children = std::make_unique<TitleBar>(engine);
        auto* titleBar = dynamic_cast<TitleBar*>(children.get());
        titleBar->parent = this;
        titleBar->SetFont(GetFontDefault(), fontSize);
        titleBar->overflowBehaviour = TextBox::OverflowBehaviour::SHRINK_TO_FIT;
        titleBar->content = _title;
        UpdateChildren();
        return titleBar;
    }

    CloseButton* TableCell::CreateCloseButton(GameUIEngine* engine, const Texture& _tex)
    {
        children = std::make_unique<CloseButton>(engine);
        auto* closeButton = dynamic_cast<CloseButton*>(children.get());
        closeButton->parent = this;
        // closeButton->SetGrayscale();
        closeButton->tex = _tex;
        UpdateChildren();
        return closeButton;
    }

    TextBox* TableCell::CreateTextbox(
        GameUIEngine* engine,
        const std::string& _content,
        float fontSize,
        TextBox::OverflowBehaviour overflowBehaviour)
    {
        children = std::make_unique<TextBox>(engine);
        auto* textbox = dynamic_cast<TextBox*>(children.get());
        textbox->parent = this;
        textbox->SetFont(GetFontDefault(), fontSize);
        SetTextureFilter(textbox->GetFont().texture, TEXTURE_FILTER_BILINEAR);
        textbox->overflowBehaviour = overflowBehaviour;
        textbox->content = _content;
        UpdateChildren();
        return textbox;
    }

    DialogOption* TableCell::CreateDialogOption(
        GameUIEngine* engine,
        const dialog::Option& option,
        float fontSize,
        TextBox::OverflowBehaviour overflowBehaviour)
    {
        children = std::make_unique<DialogOption>(engine, option);
        auto* textbox = dynamic_cast<DialogOption*>(children.get());
        textbox->parent = this;
        textbox->SetFont(GetFontDefault(), fontSize);
        SetTextureFilter(textbox->GetFont().texture, TEXTURE_FILTER_BILINEAR);
        textbox->overflowBehaviour = overflowBehaviour;
        UpdateChildren();
        return textbox;
    }

    ImageBox* TableCell::CreateImagebox(GameUIEngine* engine, const Texture& _tex)
    {
        children = std::make_unique<ImageBox>(engine);
        auto* image = dynamic_cast<ImageBox*>(children.get());
        image->parent = this;
        image->draggable = true;
        image->tex = _tex;
        UpdateChildren();
        return image;
    }

    EquipmentCharacterPreview* TableCell::CreateEquipmentCharacterPreview(GameUIEngine* engine)
    {
        children = std::make_unique<EquipmentCharacterPreview>(engine);
        auto* image = dynamic_cast<EquipmentCharacterPreview*>(children.get());
        image->parent = this;
        image->draggable = false;

        {
            entt::sink sink{engine->gameData->equipmentSystem->onEquipmentUpdated};
            sink.connect<&EquipmentCharacterPreview::RetrieveInfo>(image);
        }
        UpdateChildren();
        image->RetrieveInfo();
        return image;
    }

    void TableCell::UpdateChildren()
    {
        if (auto& element = children)
        {
            element->parent = this;
            element->rec = rec;
            element->UpdateDimensions();
        }
    }

    void TableCell::Draw2D()
    {
        TableElement::Draw2D();
        if (auto& element = children) // hide if dragged
        {
            element->Draw2D();
        }
    }

    void TableCell::DrawDebug2D()
    {
        auto col = BLACK;
        col.a = 50;
        // DrawRectangle(children->rec.x, children->rec.y, children->rec.width, children->rec.height, col);
    }
#pragma endregion

#pragma region UIStates

    void IdleState::Enter()
    {
        element->OnIdleStart();
    }

    void IdleState::Update()
    {
        auto mousePos = GetMousePosition();
        if (PointInsideRect(element->parent->rec, mousePos))
        {
            element->ChangeState(std::make_unique<HoverState>(element, engine));
        }
    }

    void IdleState::Exit()
    {
        element->OnIdleStop();
    }

    IdleState::IdleState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void HoverState::Enter()
    {
        element->OnHoverStart();
    }

    void HoverState::Exit()
    {
        // if we swap to predrag then this will be called...
        element->OnHoverStop();
    }

    void HoverState::Update()
    {
        engine->gameData->cursor->DisableContextSwitching();
        engine->gameData->cursor->Disable();

        auto mousePos = GetMousePosition();
        if (!PointInsideRect(element->parent->rec, mousePos))
        {
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            element->OnClick();
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && element->draggable)
        {
            element->ChangeState(std::make_unique<DragDelayState>(element, engine));
            return;
        }

        element->HoverUpdate();
    }

    HoverState::HoverState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DragDelayState::Enter()
    {
        element->OnHoverStart();
        dragTimer.SetMaxTime(element->dragDelayTime);
        dragTimer.SetAutoFinish(false);
    }

    void DragDelayState::Exit()
    {
        engine->hoveredDraggableCellElement.reset();
        element->OnHoverStop();
    }

    void DragDelayState::Update()
    {
        dragTimer.Update(GetFrameTime());
        if (!engine->ObjectBeingDragged() && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            if (dragTimer.HasExceededMaxTime())
            {
                element->ChangeState(std::make_unique<DragState>(element, engine));
                return;
            }
            if (!dragTimer.IsRunning())
            {
                engine->hoveredDraggableCellElement = element;
                dragTimer.Start();
            }
            if (engine->hoveredDraggableCellElement.has_value() &&
                engine->hoveredDraggableCellElement.value() != element)
            {
                element->ChangeState(std::make_unique<IdleState>(element, engine));
            }
        }
        else
        {
            element->OnClick();
            element->ChangeState(std::make_unique<IdleState>(element, engine));
        }
    }

    DragDelayState::DragDelayState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    void DragState::Enter()
    {
        engine->draggedObject = element;
        element->OnDragStart();
    }

    void DragState::Exit()
    {
        auto cell = engine->GetCellUnderCursor();
        element->OnDrop(cell);
        engine->draggedObject.reset();
    }

    void DragState::Update()
    {
        // Determine if object is still being dragged
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
        {
            // Drop item
            element->ChangeState(std::make_unique<IdleState>(element, engine));
            return;
        }
        element->DragUpdate(); // Update drag
    }

    void DragState::Draw()
    {
        element->DragDraw();
    }

    DragState::DragState(CellElement* _element, GameUIEngine* _engine) : UIState(_element, _engine)
    {
    }

    UIState::UIState(CellElement* _element, GameUIEngine* _engine) : element(_element), engine(_engine)
    {
    }
#pragma endregion

#pragma region GameUIEngine
    void GameUIEngine::pruneWindows()
    {
        {
            std::vector<unsigned int> toRemove;
            for (unsigned int i = 0; i < windows.size(); ++i)
            {
                const auto& window = windows[i];
                if (window->IsMarkedForRemoval())
                {
                    toRemove.push_back(i);
                }
            }

            for (auto& i : toRemove)
            {
                windows.erase(windows.begin() + i);
            }
        }

        if (tooltipWindow && tooltipWindow->IsMarkedForRemoval())
        {
            tooltipWindow.reset();
        }
    }

    Window* GameUIEngine::CreateTooltipWindow(
        const Texture& _nPatchTexture, const float x, const float y, const float _width, const float _height)
    {
        return CreateWindow(_nPatchTexture, x, y, _width, _height, true);
    }

    Window* GameUIEngine::CreateWindow(
        Texture _nPatchTexture,
        const float x,
        const float y,
        const float _width,
        const float _height,
        const bool tooltip)
    {

        auto window = std::make_unique<Window>(gameData->settings);
        window->SetPosition(x, y);
        window->SetDimensions(_width, _height);
        window->tex = _nPatchTexture;
        // TODO: Shouldn't SetPosition/SetDimensions already do below?
        window->rec = {
            window->GetPosition().x,
            window->GetPosition().y,
            window->GetDimensions().width,
            window->GetDimensions().height};

        // PlaceWindow(window.get(), window->GetPosition());

        entt::sink sink{gameData->userInput->onWindowUpdate};
        window->windowUpdateCnx = sink.connect<&Window::OnScreenSizeChange>(window.get());
        if (tooltip)
        {
            tooltipWindow = std::move(window);
            return tooltipWindow.get();
        }
        windows.push_back(std::move(window));
        return windows.back().get();
    }

    WindowDocked* GameUIEngine::CreateWindowDocked(
        float _xOffsetPercent, float _yOffsetPercent, float _width, float _height)
    {
        windows.push_back(std::make_unique<WindowDocked>(gameData->settings));
        auto* window = dynamic_cast<WindowDocked*>(windows.back().get());
        window->SetOffsetPercent(_xOffsetPercent, _yOffsetPercent);
        window->SetDimensions(_width, _height);
        window->rec = {
            window->GetOffset().x,
            window->GetOffset().y,
            window->GetDimensions().width,
            window->GetDimensions().height};

        entt::sink sink{gameData->userInput->onWindowUpdate};
        window->windowUpdateCnx = sink.connect<&WindowDocked::OnScreenSizeChange>(window);

        return window;
    }

    WindowDocked* GameUIEngine::CreateWindowDocked(
        Texture _nPatchTexture,
        const float _xOffsetPercent,
        const float _yOffsetPercent,
        const float _width,
        const float _height)
    {
        auto* window = CreateWindowDocked(_xOffsetPercent, _yOffsetPercent, _width, _height);
        window->tex = _nPatchTexture;
        return window;
    }

    void GameUIEngine::PlaceWindow(Window* window, Vector2 requestedPos) const
    {
        window->SetPosition(requestedPos.x, requestedPos.y);
        window->ClampToScreen();
        window->UpdateChildren();

        if (auto collision = GetWindowCollision(window))
        {
            window->rec.x = collision->rec.x - window->rec.width;
            window->ClampToScreen();
            collision = GetWindowCollision(window);
            if (collision)
            {
                window->rec.x = collision->rec.x + collision->rec.width;
                window->ClampToScreen();
                collision = GetWindowCollision(window);
                if (collision)
                {
                    assert(0);
                }
            }
        }
    }

    bool GameUIEngine::ObjectBeingDragged() const
    {
        return draggedObject.has_value();
    }

    Window* GameUIEngine::GetWindowCollision(const Window* toCheck) const
    {
        for (auto& window : windows)
        {
            if (window.get() == toCheck || window->IsHidden()) continue;
            if (CheckCollisionRecs(window->rec, toCheck->rec))
            {
                return window.get();
            }
        }
        return nullptr;
    }

    CellElement* GameUIEngine::GetCellUnderCursor() const
    {
        Window* windowUnderCursor = nullptr;
        const auto mousePos = GetMousePosition();
        for (auto& window : windows)
        {
            if (window->IsHidden()) continue;
            if (PointInsideRect(window->rec, mousePos) && mouseInNonObscuredWindowRegion(window.get(), mousePos))
            {
                windowUnderCursor = window.get();
            }
        }
        if (windowUnderCursor == nullptr) return nullptr;
        for (const auto& panel : windowUnderCursor->children)
        {
            for (const auto& table : panel->children)
            {
                for (const auto& row : table->children)
                {
                    for (const auto& cell : row->children)
                    {
                        const auto element = cell->children.get();
                        if (PointInsideRect(cell->rec, mousePos))
                        {
                            return element;
                        }
                    }
                }
            }
        }
        return nullptr;
    }

    Rectangle GameUIEngine::GetOverlap(Rectangle rec1, Rectangle rec2)
    {
        float x1 = std::max(rec1.x, rec2.x);
        float y1 = std::max(rec1.y, rec2.y);
        float x2 = std::min(rec1.x + rec1.width, rec2.x + rec2.width);
        float y2 = std::min(rec1.y + rec1.height, rec2.y + rec2.height);

        if (x1 < x2 && y1 < y2)
        {
            Rectangle overlap;
            overlap.x = x1;
            overlap.y = y1;
            overlap.width = x2 - x1;
            overlap.height = y2 - y1;
            return overlap;
        }
        return Rectangle{0, 0, 0, 0};
    }

    void GameUIEngine::BringClickedWindowToFront(Window* clicked)
    {
        const auto it = std::ranges::find_if(
            windows, [clicked](const std::unique_ptr<Window>& ptr) { return ptr.get() == clicked; });
        std::rotate(it, it + 1, windows.end());
    }

    void GameUIEngine::DrawDebug2D() const
    {
        for (const auto& window : windows)
        {
            if (window->IsHidden()) continue;
            window->DrawDebug2D();
        }
    }

    void GameUIEngine::Draw2D() const
    {
        for (const auto& window : windows)
        {
            if (window->IsHidden()) continue;
            window->Draw2D();
        }

        if (tooltipWindow)
        {
            tooltipWindow->Draw2D();
        }

        if (draggedObject.has_value())
        {
            draggedObject.value()->state->Draw();
        }
    }

    /**
     *
     * @return Whether the window region is not obscured by another window
     */
    bool GameUIEngine::mouseInNonObscuredWindowRegion(Window* window, Vector2 mousePos) const
    {
        if (auto collision = GetWindowCollision(window))
        {
            // check if window is lower
            auto windowIt = std::ranges::find_if(
                windows, [window](const std::unique_ptr<Window>& ptr) { return ptr.get() == window; });

            const auto colIt = std::ranges::find_if(
                windows, [collision](const std::unique_ptr<Window>& ptr) { return ptr.get() == collision; });

            const auto windowDist = std::distance(windows.begin(), windowIt);

            if (auto colDist = std::distance(windows.begin(), colIt); windowDist < colDist)
            {
                auto rec = GetOverlap(window->rec, collision->rec);
                if (PointInsideRect(rec, mousePos))
                {
                    // this part of the window is being obscured by another
                    return false;
                }
            }
        }
        return true;
    }

    void GameUIEngine::processWindows()
    {
        const auto mousePos = GetMousePosition();

        for (auto& window : std::ranges::reverse_view(windows))
        {
            if (!window || window->IsHidden()) continue;

            if (!PointInsideRect(window->rec, mousePos) || !mouseInNonObscuredWindowRegion(window.get(), mousePos))
            {
                window->OnHoverStop();
                continue;
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                BringClickedWindowToFront(window.get());
            }

            gameData->cursor->Disable();
            gameData->cursor->DisableContextSwitching();

            window->OnHoverStart(); // TODO: Need to check if it was already being hovered?
            for (const auto& panel : window->children)
            {
                for (const auto& table : panel->children)
                {
                    for (const auto& row : table->children)
                    {
                        for (const auto& cell : row->children)
                        {
                            const auto element = cell->children.get();
                            element->state->Update();
                        }
                    }
                }
            }
        }
    }

    void GameUIEngine::onWorldItemHover(entt::entity entity) const
    {
        if (!gameData->inventorySystem->CheckWorldItemRange()) return;
        auto& item = registry->get<ItemComponent>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        GameUiFactory::CreateWorldTooltip(gameData->uiEngine.get(), item.name, pos);
    }

    void GameUIEngine::onWorldCombatableHover(entt::entity entity) const
    {
        auto& renderable = registry->get<Renderable>(entity);
        auto& combatable = registry->get<CombatableActor>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        // Create a name tooltip
        GameUiFactory::CreateCombatableTooltip(gameData->uiEngine.get(), renderable.name, combatable, pos);
    }

    void GameUIEngine::onNPCHover(entt::entity entity) const
    {
        auto& renderable = registry->get<Renderable>(entity);
        Vector2 pos = GetWorldToScreen(
            gameData->cursor->getMouseHitInfo().rlCollision.point, *gameData->camera->getRaylibCam());
        // Create a name tooltip
        GameUiFactory::CreateWorldTooltip(gameData->uiEngine.get(), renderable.name, pos);
    }

    void GameUIEngine::onStopWorldHover() const
    {
        if (tooltipWindow)
        {
            tooltipWindow->Remove();
        }
    }

    void GameUIEngine::Update()
    {
        if (draggedObject.has_value())
        {
            draggedObject.value()->state->Update();
        }
        else
        {
            gameData->cursor->Enable();
            gameData->cursor->EnableContextSwitching();
            pruneWindows();
            processWindows();
        }
    }

    GameUIEngine::GameUIEngine(entt::registry* _registry, GameData* _gameData)
        : registry(_registry), gameData(_gameData)
    {
        entt::sink sink{_gameData->cursor->onCombatableHover};
        sink.connect<&GameUIEngine::onWorldCombatableHover>(this);
        entt::sink sink2{_gameData->cursor->onItemHover};
        sink2.connect<&GameUIEngine::onWorldItemHover>(this);
        entt::sink sink3{_gameData->cursor->onStopHover};
        sink3.connect<&GameUIEngine::onStopWorldHover>(this);
        entt::sink sink4{_gameData->cursor->onNPCHover};
        sink4.connect<&GameUIEngine::onNPCHover>(this);
    }
#pragma endregion
} // namespace sage