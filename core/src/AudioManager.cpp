//
// Created by steve on 04/01/2025.
//

#include "AudioManager.hpp"
#include "ResourceManager.hpp"

namespace sage
{
    Music AudioManager::LoadMusic(const std::string& name)
    {
        if (!music.contains(name))
        {
            music.emplace(name);
        }
        return ResourceManager::GetInstance().MusicLoad(name);
    }

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

    void AudioManager::Update() const
    {
        for (const auto& key : music)
        {
            UpdateMusicStream(ResourceManager::GetInstance().MusicLoad(key));
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