//
// Created by steve on 02/10/2024.
//

#pragma once

#include "components/DialogComponent.hpp"

#include "engine/Event.hpp"
#include "engine/GameUiEngine.hpp"
#include "engine/ResourceManager.hpp"
#include "engine/Settings.hpp"
#include "engine/Timer.hpp"

#include "entt/entt.hpp"
#include "raylib.h"

// #include <memory>
#include <optional>
// #include <utility>
#include <vector>

namespace lq
{
    class LeverUIEngine;
    class Systems;
    struct ItemComponent;
    class EquipmentComponent;
    enum class EquipmentSlotName;
    class PartySystem;
    class PlayerAbilitySystem;
    class ControllableActorSystem;
    class QuestManager;

    // Also displays description
    class JournalEntryManager : public sage::TextBox
    {
        sage::TableCell* journalEntryRoot;
        QuestManager* questManager;

        void updateQuests();

      public:
        JournalEntryManager(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            sage::TableCell* _journalEntryRoot,
            const FontInfo& _fontInfo,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class JournalEntry : public sage::TextBox
    {
        entt::entity questId{};
        Quest* quest;
        sage::TableCell* descriptionCell;
        bool drawHighlight = false;

      public:
        void OnHoverStart() override;
        void OnHoverStop() override;
        void Draw2D() override;
        void OnClick() override;
        JournalEntry(
            sage::GameUIEngine* _engine,
            sage::TableCell* _parent,
            sage::TableCell* _descriptionCell,
            Quest* _quest,
            const FontInfo& _fontInfo,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class DialogOption : public sage::TextBox
    {
        // Systems* sys;
        dialog::Option* option;
        unsigned int index{};
        bool drawHighlight = false;

      public:
        void OnHoverStart() override;
        void OnHoverStop() override;
        void Draw2D() override;
        void OnClick() override;
        DialogOption(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            dialog::Option* _option,
            unsigned int _index,
            const FontInfo& _fontInfo,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class CharacterStatText final : public sage::TextBox
    {
        // Systems* sys;

      public:
        enum class StatisticType
        {
            NAME,
            STRENGTH,
            AGILITY,
            INTELLIGENCE,
            CONSTITUTION,
            WITS,
            MEMORY,
            COUNT // must be last
        };
        StatisticType statisticType;
        void RetrieveInfo() override;
        ~CharacterStatText() override = default;
        CharacterStatText(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            const FontInfo& _fontInfo,
            StatisticType _statisticType);
    };

    class ResourceOrb : public sage::ImageBox // Health, mana, etc.
    {

      public:
        void RetrieveInfo() override;
        void Draw2D() override;
        ResourceOrb(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class EquipmentCharacterPreview : public sage::ImageBox
    {
        EquipmentSystem* equipmentSystem;

      public:
        void UpdateDimensions() override;
        void RetrieveInfo() override;
        void Draw2D() override;
        EquipmentCharacterPreview(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
    };

    class PartyMemberPortrait : public sage::ImageBox
    {
        PartySystem* partySystem;
        EquipmentSystem* equipmentSystem;
        unsigned int memberNumber{};
        Texture portraitBgTex{};
        int width;
        int height;

      public:
        void HoverUpdate() override;
        void UpdateDimensions() override;
        void RetrieveInfo() override;
        void ReceiveDrop(CellElement* droppedElement) override;
        void OnClick() override;
        void Draw2D() override;
        PartyMemberPortrait(
            LeverUIEngine* _engine, sage::TableCell* _parent, unsigned int _memberNumber, int _width, int _height);
        friend class sage::TableCell;
    };

    class DialogPortrait : public sage::ImageBox
    {
      public:
        void Draw2D() override;
        DialogPortrait(LeverUIEngine* _engine, sage::TableCell* _parent, const Texture& _tex);
        friend class sage::TableCell;
    };

    class AbilitySlot : public sage::ImageBox
    {
        PlayerAbilitySystem* playerAbilitySystem;
        unsigned int slotNumber{};

      public:
        void RetrieveInfo() override;
        void ReceiveDrop(CellElement* droppedElement) override;
        void HoverUpdate() override;
        void Draw2D() override;
        void OnClick() override;
        AbilitySlot(LeverUIEngine* _engine, sage::TableCell* _parent, unsigned int _slotNumber);
        friend class sage::TableCell;
    };

    class ItemSlot : public sage::ImageBox
    {
      protected:
        EquipmentSystem* equipmentSystem;
        Texture backgroundTex{};
        void dropItemInWorld();
        virtual void onItemDroppedToWorld() = 0;
        void updateRectangle(
            const sage::Dimensions& dimensions, const Vector2& offset, const sage::Dimensions& space) override;

        [[nodiscard]] virtual entt::entity getItemId() = 0;
        [[nodiscard]] virtual Texture getEmptyTex();

      public:
        void Draw2D() override;
        void RetrieveInfo() override;
        void OnDrop(CellElement* receiver) override;
        void HoverUpdate() override;
        ItemSlot(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            sage::VertAlignment _vertAlignment = sage::VertAlignment::TOP,
            sage::HoriAlignment _horiAlignment = sage::HoriAlignment::LEFT);
        friend class sage::TableCell;
    };

    class EquipmentSlot : public ItemSlot
    {
        [[nodiscard]] bool validateDrop(const ItemComponent& item) const;

      protected:
        Texture getEmptyTex() override;
        void onItemDroppedToWorld() override;
        [[nodiscard]] entt::entity getItemId() override;

      public:
        EquipmentSlotName itemType;
        void ReceiveDrop(CellElement* droppedElement) override;
        EquipmentSlot(LeverUIEngine* _engine, sage::TableCell* _parent, EquipmentSlotName _itemType);
        friend class sage::TableCell;
    };

    class InventorySlot : public ItemSlot
    {
      protected:
        entt::entity owner{};
        void onItemDroppedToWorld() override;
        [[nodiscard]] entt::entity getItemId() override;

      public:
        unsigned int row{};
        unsigned int col{};
        [[nodiscard]] entt::entity GetOwner() const;
        void SetOwner(entt::entity _owner);
        void ReceiveDrop(CellElement* droppedElement) override;
        InventorySlot(
            LeverUIEngine* _engine,
            sage::TableCell* _parent,
            entt::entity _owner,
            unsigned int _row,
            unsigned int _col);
        friend class sage::TableCell;
    };

    class LeverUIEngine : public sage::GameUIEngine
    {
        void onWorldItemHover(entt::entity entity);
        void onNPCHover(entt::entity entity);
        void onStopWorldHover() const;

      public:
        Systems* sys{};
        static DialogOption* CreateDialogOption(
            sage::TableCell* cell, std::unique_ptr<DialogOption> _dialogOption);
        static CharacterStatText* CreateCharacterStatText(
            sage::TableCell* cell, std::unique_ptr<CharacterStatText> _statText);
        static AbilitySlot* CreateAbilitySlot(sage::TableCell* cell, std::unique_ptr<AbilitySlot> _slot);
        static EquipmentCharacterPreview* CreateEquipmentCharacterPreview(
            sage::TableCell* cell, std::unique_ptr<EquipmentCharacterPreview> _preview);
        static PartyMemberPortrait* CreatePartyMemberPortrait(
            sage::TableCell* cell, std::unique_ptr<PartyMemberPortrait> _portrait);
        static EquipmentSlot* CreateEquipmentSlot(sage::TableCell* cell, std::unique_ptr<EquipmentSlot> _slot);
        static InventorySlot* CreateInventorySlot(sage::TableCell* cell, std::unique_ptr<InventorySlot> _slot);
        static ResourceOrb* CreateResourceOrb(sage::TableCell* cell, std::unique_ptr<ResourceOrb> _orb);
        LeverUIEngine(entt::registry* _registry, Systems* _sys);
    };

} // namespace lq
