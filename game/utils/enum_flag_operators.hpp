//
// Created by Steve Wheeler on 01/12/2024.
//
#pragma once

#include <type_traits>

namespace lq
{

    template <typename E>
    struct EnableBitMaskOperators
    {
        static const bool enable = false;
    };

    // Bitwise operators
    template <typename E>
    typename std::enable_if<EnableBitMaskOperators<E>::enable, E>::type operator|(E lhs, E rhs)
    {
        using underlying = typename std::underlying_type<E>::type;
        return static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
    }

    template <typename E>
    typename std::enable_if<EnableBitMaskOperators<E>::enable, E>::type operator&(E lhs, E rhs)
    {
        using underlying = typename std::underlying_type<E>::type;
        return static_cast<E>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
    }

    template <typename E>
    typename std::enable_if<EnableBitMaskOperators<E>::enable, E&>::type operator|=(E& lhs, E rhs)
    {
        lhs = lhs | rhs;
        return lhs;
    }
} // namespace sage