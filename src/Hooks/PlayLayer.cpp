#include "PlayLayer.hpp"

#include "../Other/RadioManager.hpp"
#include "../UI/VentillaPopup.hpp"
#include "../Other/Utils.hpp"

ProPlayLayer::Fields::~Fields() {
    auto& rm = RadioManager::get();
    rm.update();
}

void ProPlayLayer::onQuit() {
    auto& rm = RadioManager::get();

    rm.m_didQuitPlayLayer = true;

    rm.updateHooks();

    PlayLayer::onQuit();

    rm.update();

    rm.m_didQuitPlayLayer = false;
}

void ProPlayLayer::setupHasCompleted() {
    PlayLayer::setupHasCompleted();

    m_fields.self();

    auto& rm = RadioManager::get();

    rm.update();
}

void ProPlayLayer::togglePracticeMode(bool toggle) {
    PlayLayer::togglePracticeMode(toggle);

    auto& rm = RadioManager::get();
    
    rm.update();
}

void ProPlayLayer::resetLevel() {
    PlayLayer::resetLevel();
    RadioManager::get().update();
}