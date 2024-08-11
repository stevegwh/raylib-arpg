#pragma once

#include "components/CombatableActor.hpp"
#include "Cursor.hpp"
#include "particle/RainOfFireVFX.hpp"
#include "TextureTerrainOverlay.hpp"
#include <Timer.hpp>

#include <entt/entt.hpp>

#include <vector>

namespace sage
{

    struct AbilityData
    {
        float cooldownDuration;
        float range;
        int baseDamage;
        AttackElement element = AttackElement::PHYSICAL;
        float animationDelay = 0;
        bool repeatable = false;
    };

    enum class AbilityStateEnum
    {
        IDLE,
        CURSOR_SELECT,
        AWAITING_EXECUTION
    };

    struct AbilityState
    {
        Timer& cooldownTimer;
        Timer& animationDelayTimer;
        virtual ~AbilityState() = default;
        virtual void Update(entt::entity self) {};
        virtual void Draw3D(entt::entity self) {};
        virtual void OnEnter(entt::entity self) {};
        virtual void OnExit(entt::entity self) {};
        AbilityState(Timer& _cooldownTimer, Timer& _animationDelayTimer)
            : cooldownTimer(_cooldownTimer), animationDelayTimer(_animationDelayTimer)
        {
        }
    };

    class Ability
    {
      protected:
        entt::registry* registry;
        Cursor* cursor;

        Timer cooldownTimer{};
        Timer animationDelayTimer{};

        std::unique_ptr<RainOfFireVFX> vfx; // TODO: make a generic VFX class
        std::unique_ptr<TextureTerrainOverlay> spellCursor;
        AbilityData abilityData;

        AbilityState* state;
        std::unordered_map<AbilityStateEnum, std::unique_ptr<AbilityState>> states;

        void ChangeState(entt::entity self, AbilityStateEnum newState);

        class IdleState : public AbilityState
        {
            const bool repeatable;

          public:
            entt::sigh<void(entt::entity)> onRestartTriggered;
            void Update(entt::entity self) override;
            IdleState(
                Timer& _coolDownTimer, Timer& _animationDelayTimer, bool _repeatable)
                : AbilityState(_coolDownTimer, _animationDelayTimer),
                  repeatable(_repeatable)
            {
            }
        };

        class CursorSelectState : public AbilityState
        {

            Cursor* cursor;
            std::unique_ptr<TextureTerrainOverlay> spellCursor;
            bool cursorActive = false;
            void enableCursor();
            void disableCursor();
            void toggleCursor(entt::entity self);

          public:
            entt::sigh<void(entt::entity)> onConfirm;
            void Update(entt::entity self) override;
            void OnEnter(entt::entity self) override;
            void OnExit(entt::entity self) override;
            CursorSelectState(
                Timer& _coolDownTimer,
                Timer& _animationDelayTimer,
                Cursor* _cursor,
                std::unique_ptr<TextureTerrainOverlay> _spellCursor)
                : AbilityState(_coolDownTimer, _animationDelayTimer),
                  cursor(_cursor),
                  spellCursor(std::move(_spellCursor))
            {
            }
        };

        class AwaitingExecutionState : public AbilityState
        {
          public:
            entt::sigh<void(entt::entity)> onExecute;
            void OnEnter(entt::entity self) override;
            void Update(entt::entity self) override;
            AwaitingExecutionState(Timer& _coolDownTimer, Timer& _animationDelayTimer)
                : AbilityState(_coolDownTimer, _animationDelayTimer)
            {
            }
        };

      public:
        // Timer functions
        virtual void ResetCooldown();
        virtual bool IsActive() const;
        float GetRemainingCooldownTime() const;
        float GetCooldownDuration() const;
        bool CooldownReady() const;

        // Ability functions
        virtual void Cancel(entt::entity self);
        virtual void Execute(entt::entity self) = 0;
        virtual void Update(entt::entity self);
        virtual void Draw3D(entt::entity self);
        virtual void Init(entt::entity self);

        virtual ~Ability() = default;
        Ability(const Ability&) = delete;
        Ability& operator=(const Ability&) = delete;
        Ability(
            entt::registry* _registry, const AbilityData& _abilityData, Cursor* _cursor);
    };
} // namespace sage