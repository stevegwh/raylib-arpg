#pragma once
#include <functional>
#include <utility>

namespace sage
{
struct EventCallback
{
    std::function<void()> callback;
    explicit EventCallback(std::function<void()> func): callback(std::move(func)){}
};
}


