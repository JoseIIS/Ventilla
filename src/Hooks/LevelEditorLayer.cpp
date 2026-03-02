#include "../Other/RadioManager.hpp"

#include "LevelEditorLayer.hpp"

ProLevelEditorLayer::Fields::~Fields() {
    auto& rm = RadioManager::get();
    rm.update();
    rm.m_didEnterEditor = false;
}

bool ProLevelEditorLayer::init(GJGameLevel* p0, bool p1) {
    auto& rm = RadioManager::get();

    rm.m_didEnterEditor = false;

    if (!LevelEditorLayer::init(p0, p1)) {
        return false;
    }

    m_fields.self();

    rm.update();

    return true;
}

void ProLevelEditorLayer::onStopPlaytest() {
    LevelEditorLayer::onStopPlaytest();
    RadioManager::get().updateWithFade(0.5f);
}