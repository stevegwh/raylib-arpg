//
// Created by steve on 04/01/2025.
//

#include "AudioManager.hpp"
#include "ResourceManager.hpp"

namespace sage
{
    Music AudioManager::PlayMusic(const std::string& name)
    {
        if (!music.contains(name))
        {
            music.emplace(name);
        }
        auto m = ResourceManager::GetInstance().MusicLoad(name);
        PlayMusicStream(m);
        return m;
    }

    Sound AudioManager::PlaySFX(const std::string& name)
    {
        if (!sfx.contains(name))
        {
            sfx.emplace(name);
        }
        auto s = ResourceManager::GetInstance().SFXLoad(name);
        PlaySound(s);
        return s;
    }

    void AudioManager::Update() const
    {
        for (const auto& key : music)
        {
            UpdateMusicStream(ResourceManager::GetInstance().MusicLoad(key));
        }
        for (const auto& key : sfx)
        {
            // UpdateSound(ResourceManager::GetInstance().SFXLoad(key));
        }
    }

    AudioManager::~AudioManager()
    {
        CloseAudioDevice();
    }

    AudioManager::AudioManager()
    {
        InitAudioDevice();
    }
} // namespace sage