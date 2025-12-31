//
// Created by steve on 04/01/2025.
//

#include "AudioManager.hpp"
#include "ResourceManager.hpp"

namespace sage
{
    Music AudioManager::PlayMusic(const std::string& name)
    {
        if (!musicPlaying.contains(name))
        {
            musicPlaying.emplace(name);
        }
        auto m = ResourceManager::GetInstance().GetMusic(name);
        PlayMusicStream(m);
        return m;
    }

    Sound AudioManager::PlaySFX(const std::string& name)
    {
        if (!sfxPlaying.contains(name))
        {
            sfxPlaying.emplace(name);
        }
        auto s = ResourceManager::GetInstance().GetSFX(name);
        PlaySound(s);
        return s;
    }

    void AudioManager::Update() const
    {
        for (const auto& key : musicPlaying)
        {
            UpdateMusicStream(ResourceManager::GetInstance().GetMusic(key));
        }
        for (const auto& key : sfxPlaying)
        {
            // UpdateSound(ResourceManager::GetInstance().GetSFX(key));
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