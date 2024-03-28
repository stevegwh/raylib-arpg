#pragma once
#include <functional>
#include <utility>
#include <typeinfo>
#include <cstdint>
#include <sstream>
#include <string>

namespace sage
{
struct EventCallback
{
    std::string signature;
    std::function<void()> callback;
    explicit EventCallback(std::function<void()> func): callback(std::move(func)) 
    {
        std::ostringstream oss;
        oss << typeid(*this).name() << "@" << reinterpret_cast<uintptr_t>(this);
        signature = oss.str();
    }
};
}


