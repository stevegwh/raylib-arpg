#pragma once
#include <functional>
#include <vector>
#include "raylib.h"

namespace sage
{
    class TimerManager
    {
    private:
        struct Timer
        {
            std::function<void()> func;
			float maxTime;
            float remainingTime;
            int id;
			bool repeating = true;
        };

        std::vector<Timer> timers;
        int nextId = 0;

    public:

        template <typename Func, typename... Args>
        [[nodiscard]] int AddTimer(float duration, Func&& func, Args&&... args)
        {
            int id = nextId++;
            timers.push_back({
                [f = std::forward<Func>(func), ... capturedArgs = std::forward<Args>(args)]() mutable {
                    std::invoke(f, std::forward<Args>(capturedArgs)...);
                },
				duration,
                duration,
                id
            });
            return id;
        }

		template <typename Func, typename... Args>
        [[nodiscard]] int AddTimerOneshot(float duration, Func&& func, Args&&... args)
        {
            int id = nextId++;
            timers.push_back({
                [f = std::forward<Func>(func), ... capturedArgs = std::forward<Args>(args)]() mutable {
                    std::invoke(f, std::forward<Args>(capturedArgs)...);
                },
				duration,
                duration,
                id
            });
			timers.back().repeating = false;
            return id;
        }

        // Overload for member functions
        template <typename C, typename R, typename... Args>
        [[nodiscard]] int AddTimer(float duration, R(C::*memFunc)(Args...), C* instance, Args... args)
        {
            return AddTimer(duration, [instance, memFunc, ... capturedArgs = std::move(args)]() mutable {
                (instance->*memFunc)(std::move(capturedArgs)...);
            });
        }

		template <typename C, typename R, typename... Args>
        [[nodiscard]] int AddTimerOneshot(float duration, R(C::*memFunc)(Args...), C* instance, Args... args)
        {
            int id = AddTimer(duration, [instance, memFunc, ... capturedArgs = std::move(args)]() mutable {
                (instance->*memFunc)(std::move(capturedArgs)...);
            });
			timers.back().repeating = false;
			return id;
        }

		[[nodiscard]] int AddTimer(float duration)
        {
            // Adding a dummy function as callback
            return AddTimer(duration, []() {});
        }

		[[nodiscard]] int AddTimerOneshot(float duration)
        {
            // Adding a dummy function as callback
            int id = AddTimer(duration, []() {});
			timers.back().repeating = false;
			return id;
        }

        void Update()
        {
            float dt = GetFrameTime();
            
            for (auto it = timers.begin(); it != timers.end(); )
            {
                it->remainingTime -= dt;
                
                if (it->remainingTime <= 0)
                {
                    it->func();  // Execute the function
					if (it->repeating)
					{
						it->remainingTime = it->maxTime;
						++it;
					}
					else
					{
						it = timers.erase(it);  // Remove the timer
					}
                }
				else
				{
					++it;
				}
            }
        }

		float GetRemainingTime(int id)
		{
			for (auto& timer : timers)
			{
				if (timer.id == id)
				{
					return timer.remainingTime;
				}
			}
			return -1;
		}

        void RemoveTimer(int id)
        {
            timers.erase(
                std::remove_if(timers.begin(), timers.end(),
                    [id](const Timer& timer) { return timer.id == id; }),
                timers.end());
        }
    };
}  // namespace sage