#pragma once

class Timer
{
    bool active = false;
    bool finished = false;
    float remainingTime{};
    
public:
    float maxTime{};

    float RemainingTime() const
    {
        return remainingTime;
    }

    bool HasFinished() const
    {
        return finished;
    }

    void Start()
    {
        Restart();
    }

    void Stop()
    {
        active = false;
        remainingTime = maxTime;
    }

    void Reset()
    {
        active = false;
        finished = false;
        remainingTime = maxTime;
    }

    void Restart()
    {
        Reset();
        active = true;
    }

    void Pause()
    {
        active = false;
    }

    void Update(float dt)
    {
        if (!active)
            return;
        remainingTime -= dt;
        if (remainingTime <= 0)
        {
            finished = true;
            active = false;
        }
    }
};