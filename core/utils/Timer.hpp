#pragma once

class Timer
{
    bool active = false;
    bool finished = true;
    bool autoFinish = true; // New boolean to control automatic finishing
    float counter{};
    float maxTime{};

  public:
    void SetMaxTime(float _maxTime)
    {
        maxTime = _maxTime;
    }

    void SetAutoFinish(bool _autoFinish)
    {
        autoFinish = _autoFinish;
    }

    [[nodiscard]] bool GetAutoFinish() const
    {
        return autoFinish;
    }

    [[nodiscard]] float GetMaxTime() const
    {
        return maxTime;
    }

    [[nodiscard]] bool IsRunning() const
    {
        return active;
    }

    [[nodiscard]] float GetRemainingTime() const
    {
        return maxTime - counter;
    }

    [[nodiscard]] float GetCurrentTime() const
    {
        return counter;
    }

    [[nodiscard]] bool HasFinished() const
    {
        return finished;
    }

    [[nodiscard]] bool HasExceededMaxTime() const
    {
        return counter >= maxTime;
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
        if (counter >= maxTime && autoFinish)
        {
            finished = true;
            active = false;
        }
    }
};