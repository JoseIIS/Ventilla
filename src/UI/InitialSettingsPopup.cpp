#include "InitialSettingsPopup.hpp"

InitialSettingsPopup* InitialSettingsPopup::create() {
    auto ret = new InitialSettingsPopup();

    if (ret->init()) {
        ret->autorelease();
        return ret;
    }

    delete ret;
    return nullptr;
}

bool InitialSettingsPopup::init() {
    Popup::init(262, 255, "square01_001.png");

    m_closeBtn->setVisible(false);

    setTitle("Ventilla", "goldFont.fnt", 0.7f, 23.5f);

    auto textArea = MDTextArea::create(
        "By default, <cl>Ventilla</c> only plays the radio where the Main Menu Music would play, but there is <cg>more settings</c> to choose from!\n\n* You can also play the radio in:",
        {220, 163}
    );
    textArea->setPosition({m_size.width / 2.f, 136});

    m_mainLayer->addChild(textArea);

    auto menu = textArea->getScrollLayer()->m_contentLayer->getChildByType<CCMenu>(0);

    if (!menu) {
        return false;
    }

    auto createToggle = [menu](ZStringView text, auto&& callback, float y) {
        auto toggle = CCMenuItemExt::createToggler(
            CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
            CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
            std::forward<decltype(callback)>(callback)
        );
        toggle->setPosition({20, y});
        toggle->setScale(0.5f);

        menu->addChild(toggle);

        auto lbl = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        lbl->setAnchorPoint({0, 0.5f});
        lbl->setPosition({34, y});
        lbl->setScale(0.375f);

        menu->addChild(lbl);
    };

    auto createToggleQuick = [&createToggle](ZStringView text, std::string_view setting, float y) {
        createToggle(text, [setting](CCMenuItemToggler* toggle) {
            Mod::get()->setSettingValue(setting, !toggle->isToggled());
        }, y);
    };
    
    createToggleQuick("Editor", "play-in-editor", 97);
    createToggleQuick("Pause Menu", "play-in-pause-menu", 77);
    createToggleQuick("Practice Mode", "play-in-practice-mode", 57);
    createToggleQuick("Normal Mode", "play-in-normal-mode", 37);
    createToggle("All Menus", [](CCMenuItemToggler* toggle) {
        Mod::get()->setSettingValue("shop.mp3", !toggle->isToggled());
        Mod::get()->setSettingValue("tower01.mp3", !toggle->isToggled());
        Mod::get()->setSettingValue("secretLoop02.mp3", !toggle->isToggled());
        Mod::get()->setSettingValue("secretLoop03.mp3", !toggle->isToggled());
        Mod::get()->setSettingValue("secretLoop04.mp3", !toggle->isToggled());
        Mod::get()->setSettingValue("secretShop.mp3", !toggle->isToggled());
        Mod::get()->setSettingValue("shop3.mp3", !toggle->isToggled());
        Mod::get()->setSettingValue("shop4.mp3", !toggle->isToggled());
        Mod::get()->setSettingValue("shop5.mp3", !toggle->isToggled());
        Mod::get()->setSettingValue("secretLoop.mp3", !toggle->isToggled());
    }, 17);

    auto spr = ButtonSprite::create("Ok");
    spr->setScale(0.8f);

    auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(InitialSettingsPopup::onClose)); 
    btn->setPosition({m_size.width / 2.f, 36});

    m_buttonMenu->addChild(btn);

    return true;
}