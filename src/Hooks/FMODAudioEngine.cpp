#include "../Other/RadioManager.hpp"

#include "FMODAudioEngine.hpp"

void ProFMODAudioEngine::playMusic(gd::string file, bool p1, float p2, int p3) {
    FMODAudioEngine::playMusic(file, p1, p2, p3);

    auto& rm = RadioManager::get();

    rm.m_lastMusicPlayed = file;
    
    rm.update();
}