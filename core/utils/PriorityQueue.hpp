//
// Created by Steve Wheeler on 18/06/2024.
//

#pragma once

#include <utility>
#include <queue>

template<typename T, typename priority_t>
struct PriorityQueue {
    typedef std::pair<priority_t, T> PQElement;
    std::priority_queue<PQElement, std::vector<PQElement>,
    std::greater<PQElement>> elements;

    [[nodiscard]] inline bool empty() const 
    {
        return elements.empty();
    }

    inline void put(T item, priority_t priority) 
    {
        elements.emplace(priority, item);
    }

    T get() 
    {
        T best_item = elements.top().second;
        elements.pop();
        return best_item;
    }
};
