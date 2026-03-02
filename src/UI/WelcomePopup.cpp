#include "WelcomePopup.hpp"
#include "InitialSettingsPopup.hpp"

WelcomePopup* WelcomePopup::create() {
    auto ret = new WelcomePopup();

    ret->init();
    ret->autorelease();

    return ret;
}

void WelcomePopup::onClose(CCObject* sender) {
    Popup::onClose(sender);

    queueInMainThread([] {
        if (auto popup = InitialSettingsPopup::create()) {
            popup->show();
        }
    });
}

bool WelcomePopup::init() {
    Popup::init(242, 263, "square01_001.png");

    m_closeBtn->setVisible(false);

    setTitle("Ventilla", "goldFont.fnt", 0.7f, 26.5f);

    auto textArea = MDTextArea::create(
        "### Thank you for installing <cl>Ventilla</c>!\n<cl>Ventilla</c> is a community-driven, non-profit radio station that streams songs loved by the community 24/7.\n\n\n<cy>Important:</c> Not all songs are copyright-free. If you're a content creator, please <cr>be cautious</c> when using this mod.\n\n\nIf you're an artist and would like your song removed, please email <cg>joseii@5infin.es</c> or send a DM to <cg>@Analogias_Esp</c> on X (formerly Twitter).",
        {200, 166}
    );
    textArea->setPosition({m_size.width / 2.f, 136});

    m_mainLayer->addChild(textArea);

    auto spr = ButtonSprite::create("Ok");
    spr->setScale(0.8f);

    auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(WelcomePopup::onClose)); 
    btn->setPosition({m_size.width / 2.f, 30});

    m_buttonMenu->addChild(btn);

    return true;
}