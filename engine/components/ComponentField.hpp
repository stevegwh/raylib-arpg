//
// Created by Steve Wheeler on 24/05/2026.
//

#pragma once

#include <utility>

namespace sage
{

    template <typename T, typename Component>
    struct ComponentField
    {
        T value{};
        Component& owner;

        explicit ComponentField(Component& newOwner) : owner(newOwner)
        {
        }
        ComponentField(Component& newOwner, const T& v) : value(v), owner(newOwner)
        {
        }
        ComponentField(Component& newOwner, T&& v) : value(std::move(v)), owner(newOwner)
        {
        }
        ComponentField(Component& newOwner, const ComponentField& rhs) : value(rhs.value), owner(newOwner)
        {
        }
        ComponentField(Component& newOwner, ComponentField&& rhs) noexcept
            : value(std::move(rhs.value)), owner(newOwner)
        {
        }

        ComponentField(const ComponentField& rhs) = delete;
        ComponentField(ComponentField&& rhs) noexcept = delete;

        ComponentField& operator=(const ComponentField& rhs)
        {
            if (this == &rhs) return *this;
            value = rhs.value;
            markDirty();
            return *this;
        }
        ComponentField& operator=(ComponentField&& rhs) noexcept
        {
            if (this == &rhs) return *this;
            value = std::move(rhs.value);
            markDirty();
            return *this;
        }

        void markDirty()
        {
            owner.markDirty();
        }

        ComponentField& operator=(const T& v)
        {
            value = v;
            markDirty();
            return *this;
        }
        ComponentField& operator=(T&& v)
        {
            value = std::move(v);
            markDirty();
            return *this;
        }
        ComponentField& operator+=(const T& v)
        {
            value += v;
            markDirty();
            return *this;
        }

        operator T() const
        {
            return value;
        }
        T& operator*()
        {
            return value;
        }
        const T& operator*() const
        {
            return value;
        }
    };
} // namespace sage
