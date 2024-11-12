//
// Created by steve on 02/10/2024.
//

#include "GameUiElements.hpp"
#include "components/EquipmentComponent.hpp"
#include "components/InventoryComponent.hpp"
#include "components/ItemComponent.hpp"
#include "components/PartyMemberComponent.hpp"
#include "components/sgTransform.hpp"
#include "Cursor.hpp"
#include "GameData.hpp"
#include "GameObjectFactory.hpp"
#include "GameUiEngine.hpp"
#include "GameUiFactory.hpp"
#include "ResourceManager.hpp"
#include "rlgl.h"
#include "slib.hpp"
#include "systems/ControllableActorSystem.hpp"
#include "systems/EquipmentSystem.hpp"
#include "systems/PartySystem.hpp"
#include "systems/PlayerAbilitySystem.hpp"

#include <cassert>
#include <sstream>

namespace sage
{

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
          sdfShader(ResourceManager::GetInstance().ShaderLoad(nullptr, "resources/shaders/glsl330/sdf.fs")){};

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
        auto mousePos = GetMousePosition();
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

    ImageBox::ImageBox(GameUIEngine* _engine) : CellElement(_engine){};

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
        auto& inventory = engine->registry->get<InventoryComponent>(
            engine->gameData->controllableActorSystem->GetSelectedActor());
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
        const auto& inventory = engine->registry->get<InventoryComponent>(
            engine->gameData->controllableActorSystem->GetSelectedActor());
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

    bool EquipmentSlot::validateDrop(ItemComponent& item) const
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
            auto actor = engine->gameData->controllableActorSystem->GetSelectedActor();
            auto& inventory = engine->registry->get<InventoryComponent>(actor);
            auto droppedItemId = engine->gameData->equipmentSystem->GetItem(actor, droppedE->itemType);
            engine->gameData->equipmentSystem->DestroyItem(actor, droppedE->itemType);

            if (auto inventoryItemId = inventory.GetItem(row, col); inventoryItemId != entt::null)
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
        parent->GetWindow()->hidden = true;
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

    void Window::Remove()
    {
        hidden = true;
        markForRemoval = true;
        windowUpdateCnx.release();
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
        return Dimensions{settings->screenWidth * widthPercent, settings->screenHeight * heightPercent};
    }

    /**
     *
     * @param _widthPercent Width as percent of screen (1-100)
     * @param _heightPercent Height as percent of screen (1-100)
     */
    void Window::SetDimensionsPercent(float _widthPercent, float _heightPercent)
    {
        widthPercent = _widthPercent / 100;
        heightPercent = _heightPercent / 100;
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
        for (auto& table : children)
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

    void Window::DrawDebug2D()
    {
        for (auto& child : children)
        {
            child->DrawDebug2D();
        }
    }

    void Window::Draw2D()
    {
        TableElement::Draw2D();
        for (auto& child : children)
        {
            child->Draw2D();
        }
    }

    TableGrid* Window::CreateTableGrid(int rows, int cols, float cellSpacing)
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

    Table* Window::CreateTable()
    {
        children.push_back(std::make_unique<Table>());
        const auto& table = children.back();
        table->parent = this;
        UpdateChildren();
        return table.get();
    }

    Table* Window::CreateTable(float requestedWidthOrHeight)
    {
        children.push_back(std::make_unique<Table>());
        const auto& table = children.back();
        table->parent = this;
        table->autoSize = false;
        if (tableAlignment == WindowTableAlignment::STACK_VERTICAL)
        {
            table->requestedHeight = requestedWidthOrHeight;
        }
        else if (tableAlignment == WindowTableAlignment::STACK_HORIZONTAL)
        {
            table->requestedWidth = requestedWidthOrHeight;
        }
        UpdateChildren();
        return table.get();
    }

    void Window::UpdateChildren()
    {
        if (children.empty()) return;

        // Account for window padding
        float availableWidth = rec.width - (GetPadding().left + GetPadding().right);
        float availableHeight = rec.height - (GetPadding().up + GetPadding().down);
        float startX = rec.x + GetPadding().left;
        float startY = rec.y + GetPadding().up;

        switch (tableAlignment)
        {
        case WindowTableAlignment::STACK_HORIZONTAL: {
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

            if (totalRequestedPercent > 100.0f)
            {
                totalRequestedPercent = 100.0f;
            }

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
            break;
        }

        case WindowTableAlignment::STACK_VERTICAL: {
            float totalRequestedPercent = 0.0f;
            int autoSizeCount = 0;

            // First pass: Calculate total of percentage-based heights
            for (const auto& table : children)
            {
                if (table->autoSize)
                {
                    autoSizeCount++;
                }
                else
                {
                    totalRequestedPercent += table->requestedHeight;
                }
            }

            if (totalRequestedPercent > 100.0f)
            {
                totalRequestedPercent = 100.0f;
            }

            float remainingPercent = 100.0f - totalRequestedPercent;
            float autoSizePercent = autoSizeCount > 0 ? (remainingPercent / autoSizeCount) : 0.0f;

            // Second pass: Update each table
            float currentY = startY;
            for (const auto& table : children)
            {
                table->parent = this;
                table->rec = rec;

                float tableHeight;
                if (table->autoSize)
                {
                    tableHeight = std::ceil(availableHeight * (autoSizePercent / 100.0f));
                }
                else
                {
                    tableHeight = std::ceil(availableHeight * (table->requestedHeight / 100.0f));
                }

                table->rec.height = tableHeight;
                table->rec.y = currentY;
                table->rec.width = availableWidth;
                table->rec.x = startX;

                if (!table->children.empty()) table->UpdateChildren();

                currentY += tableHeight;
            }
            break;
        }
        }
    }

    void TableGrid::UpdateChildren()
    {
        if (children.empty()) return;
        // 1. Get number of columns
        int cols = children[0]->children.size();

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
        std::vector colors = {PINK, RED, BLUE, YELLOW, WHITE};
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& row = children[i];
            Color col = colors[i];
            col.a = 150;
            // DrawRectangle(row->rec.x, row->rec.y, row->rec.width, row->rec.height, col);
            row->DrawDebug2D();
        }
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

    CloseButton* TableCell::CreateCloseButton(GameUIEngine* engine, Texture _tex)
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

    ImageBox* TableCell::CreateImagebox(GameUIEngine* engine, Texture _tex)
    {
        children = std::make_unique<ImageBox>(engine);
        auto* image = dynamic_cast<ImageBox*>(children.get());
        image->parent = this;
        image->draggable = true;
        image->tex = _tex;
        UpdateChildren();
        return image;
    }

    void TableCell::UpdateChildren()
    {
        auto& element = children;
        if (element)
        {
            element->parent = this;
            element->rec = rec;
            element->UpdateDimensions();
        }
    }

    void TableCell::Draw2D()
    {
        TableElement::Draw2D();
        auto& element = children;
        if (element) // hide if dragged
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
} // namespace sage