#include "../Other/RadioManager.hpp"
#include "../UI/VentillaPopup.hpp"
#include "../Other/Utils.hpp"

#include "PauseLayer.hpp"

ProPauseLayer::Fields::~Fields() {
    RadioManager::get().update();
}

void ProPauseLayer::customSetup() {
    PauseLayer::customSetup();

    m_fields.self();

    auto& rm = RadioManager::get();

    rm.updateWithFade(0.5f);

    if (!Utils::getSetting<"show-button-in-pause-menu", bool>()) {
        return;
    }

    auto menu = getChildByID("left-button-menu");

    if (!menu) {
        return;
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
}

void ProPauseLayer::goEdit() {
    PauseLayer::goEdit();

    auto& rm = RadioManager::get();

    rm.m_didEnterEditor = true;

    rm.update();
}