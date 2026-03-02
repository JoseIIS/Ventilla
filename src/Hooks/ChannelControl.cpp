#include "../Other/RadioManager.hpp"

#include "ChannelControl.hpp"

FMOD_RESULT ProChannelControl::setVolume(float volume) {
    if (
        this != static_cast<FMOD::ChannelControl*>(FMODAudioEngine::get()->m_backgroundMusicChannel)
    ) {
        return FMOD::ChannelControl::setVolume(volume);
    }

    auto& rm = RadioManager::get();

    rm.updateVolume();

    if (rm.m_radioAvailable) {
        volume = 0.f;
    }
    
    return FMOD::ChannelControl::setVolume(volume);
}