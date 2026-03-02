#include "../UI/VentillaPopup.hpp"
#include "../UI/WelcomePopup.hpp"
#include "../Other/RadioManager.hpp"
#include "../Other/Utils.hpp"

#include "MenuLayer.hpp"

bool ProMenuLayer::init() {
    if (!MenuLayer::init()) {
        return false;
    }

    if (!Mod::get()->setSavedValue("firt-time3", true)) {
        geode::queueInMainThread([] {
            WelcomePopup::create()->show();
        });
    }

    auto& rm = RadioManager::get();

    if (rm.m_firstMenuLayer) {
        rm.m_firstMenuLayer = false;
        rm.update();
    }

    if (!Utils::getSetting<"show-button-in-main-menu", bool>()) {
        return true;
    }

    auto menu = getChildByID("bottom-menu");

    if (!menu) {
        return true;
    }

    auto spr1 = CCSprite::create("button-icon.png"_spr);
    auto spr2 = CircleButtonSprite::create(spr1, CircleBaseColor::Green, CircleBaseSize::MediumAlt);
    spr1->setPosition(spr1->getPosition() + ccp(1.25f, -1.f));

    auto btn = CCMenuItemExt::createSpriteExtra(spr2, [](CCMenuItemSpriteExtra*) {
        VentillaPopup::create()->show();
    });
    btn->setID("button"_spr);

    menu->addChild(btn);
    menu->updateLayout();

    return true;
}