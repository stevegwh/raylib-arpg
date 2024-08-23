#pragma once

class Timer
{
    bool active = false;
    bool finished = true;
    float counter{};
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
        return maxTime - counter;
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
        counter = 0;
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
        counter += dt;
        if (counter >= maxTime)
        {
            finished = true;
            active = false;
        }
    }
};