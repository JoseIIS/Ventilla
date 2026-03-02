#include "../Other/RadioManager.hpp"
#include "../UI/VentillaPopup.hpp"
#include "../Other/Utils.hpp"

#include "EditorPauseLayer.hpp"

bool ProEditorPauseLayer::init(LevelEditorLayer* p0) {
    if (!EditorPauseLayer::init(p0)) {
        return false;
    }

    if (!Utils::getSetting<"show-button-in-editor-pause", bool>()) {
        return true;
    }

    auto menu = getChildByID("guidelines-menu");

    if (!menu) {
        return true;
    }

    auto spr1 = CCSprite::create("button-icon.png"_spr);
    auto spr2 = CircleButtonSprite::create(spr1, CircleBaseColor::Green, CircleBaseSize::Medium);
    spr1->setPosition(spr1->getPosition() + ccp(1.25f, -1.f));
    spr2->setScale(0.82f);

    auto btn = CCMenuItemExt::createSpriteExtra(spr2, [](CCMenuItemSpriteExtra*) {
        VentillaPopup::create()->show();
    });
    btn->setID("button"_spr);

    menu->addChild(btn);
    menu->updateLayout();

    return true;
}

void ProEditorPauseLayer::onExitEditor(CCObject* sender) {
    auto& rm = RadioManager::get();

    rm.m_didQuitEditor = true;

    rm.updateHooks();

    EditorPauseLayer::onExitEditor(sender);

    rm.update();

    rm.m_didQuitEditor = false;
}