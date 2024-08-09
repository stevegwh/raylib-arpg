#pragma once

class Timer
{
    bool active = false;
    bool finished = true;
    float remainingTime{};
    float maxTime{};

  public:
    void SetMaxTime(float _maxTime)
    {
        maxTime = _maxTime;
    }

    float GetMaxTime() const
    {
        return maxTime;
    }

    bool IsRunning() const
    {
        return active;
    }

    float GetRemainingTime() const
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
        Reset();
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
        if (!active) return;
        if (finished) finished = false; // Reset finished flag when active
        remainingTime -= dt;
        if (remainingTime <= 0)
        {
            finished = true;
            active = false;
        }
    }
};