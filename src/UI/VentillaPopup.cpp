#include <Geode/ui/GeodeUI.hpp>
#include <Geode/ui/Button.hpp>

#include "../Other/RadioManager.hpp"

#include "VentillaPopup.hpp" 

#include "../Other/Utils.hpp"

VentillaPopup::VentillaPopup() {
    RadioManager::get().m_ventillaPopup = this;
}

VentillaPopup::~VentillaPopup() {
    RadioManager::get().m_ventillaPopup = nullptr;
}

VentillaPopup* VentillaPopup::create() {
    auto ret = new VentillaPopup();

    ret->init();
    ret->autorelease();

    return ret;
}

void VentillaPopup::onSlider(CCObject*) {
    auto& rm = RadioManager::get();
    auto value = static_cast<int>(m_volumeSlider->getValue() * 100);

    Mod::get()->setSettingValue("volume", value);

    m_volumeInput->setString(numToString(value));

    Mod::get()->setSettingValue("volume", m_volumeSlider->getValue());
    
    updateSongTitle();
}

void VentillaPopup::updateSongTitle(float dt) {
    auto& rm = RadioManager::get();
    auto title = rm.getSongTitle();

    if (title != m_lastSongTitle) {
        m_currentTitleLetter = -4;
    }

    m_lastSongTitle = title;

    m_songTitleLabel->setString(title.c_str());
    m_songTitleLabel->setOpacity(rm.m_radioAvailable && title != "N/A" ? 255 : 140);

    auto width = m_songTitleLabel->getScaledContentWidth();

    if (width <= 120.f) {
        return;
    }

    auto ogTitleLength = title.size();
    
    title += " | " + title;
    
    m_songTitleLabel->setString(title.c_str());
    
    auto children = m_songTitleLabel->getChildrenExt();
    auto count = 0;
    
    for (auto letter : children) {
        auto pos = letter->getPositionX() + letter->getContentWidth() / 2.f;

        if (m_currentTitleLetter > 0) {
            pos -= children[m_currentTitleLetter - 1]->getPositionX();
        }

        if (pos * m_songTitleLabel->getScale() > 120.f) {
            break;
        }
        
        count++;
    }

    auto letter = std::max(0, m_currentTitleLetter);
    auto newTitle = title.substr(letter, count - letter);

    m_songTitleLabel->setString(newTitle.c_str());

    if (dt <= 0.f) {
        return;
    }

    if (letter >= ogTitleLength + 3) {
        m_currentTitleLetter = -4;
    } else {
        m_currentTitleLetter++;
    }
}

void VentillaPopup::updateSpectrum(float dt) {
    auto& rm = RadioManager::get();

    if (!rm.m_radioAvailable || Utils::getRadioVolume() * FMODAudioEngine::get()->m_musicVolume <= 0.f) {
        for (auto bar : m_bars) {
            bar->setContentHeight(std::max(
                bar->getContentHeight() - (bar->getContentHeight() - 2) / 10.f,
                2.f
            ));
        }

        return;
    }

    FMOD_DSP_PARAMETER_FFT* data = nullptr;

    rm.m_dsp->getParameterData(
        FMOD_DSP_FFT_SPECTRUMDATA,
        (void**)&data,
        nullptr,
        nullptr,
        0
    );

    if (!data) {
        return;
    }

    int binsPerBar = data->length / 16;
    float* spectrum = data->spectrum[0];

    float realHeights[8];

    for (int i = 0; i < 8; i++) {
        float sum = 0;

        for (int j = 0; j < binsPerBar; j++) {
            sum += spectrum[i * binsPerBar + j];
        }

        realHeights[i] = 2 + 36 * std::min(1.f, log10f(1.f + (sum / binsPerBar) * 100.f) * 3.f);
    }

    for (int i = 0; i < 8; i++) {
        int bar = i * 2;
        m_bars[bar]->setContentHeight(realHeights[i]);

        if (i < 7) {
            float midpoint = (realHeights[i] + realHeights[i + 1]) * 0.5f;
            m_bars[bar + 1]->setContentHeight(midpoint);
        } else {
            m_bars[bar + 1]->setContentHeight(realHeights[i]);
        }
    }
}

void VentillaPopup::updateStatus() {
    auto& rm = RadioManager::get();
    auto color = Utils::getColorForStatus(rm.m_status);
    auto i = 0;

    m_warningLabel->setVisible(Utils::getSetting<"enabled", bool>() && GameManager::get()->getGameVariable("0122"));
    m_statusIcon->setColor(color);
    m_statusLabel->setString(Utils::getNameForStatus(rm.m_status).c_str());
    m_statusLabel->setColor(color);

    if (rm.m_status == RadioStatus::Online) {
        m_nowPlayingLabel->stopAllActions();
        m_nowPlayingLabel->setScale(0.575f);
        m_nowPlayingLabel->runAction(
            CCRepeatForever::create(
                CCSequence::create(
                    CCEaseSineInOut::create(CCScaleTo::create(1.2f, 0.59f)),
                    CCEaseSineInOut::create(CCScaleTo::create(1.2f, 0.575f)),
                    nullptr
                )
            )
        );
    } else {
        m_nowPlayingLabel->stopAllActions();
        m_nowPlayingLabel->runAction(CCEaseSineInOut::create(CCScaleTo::create(1.2f, 0.575f)));
    }

    if (
        rm.m_status != m_previousStatus
        && (rm.m_status == RadioStatus::Connecting || rm.m_status == RadioStatus::Reconnecting)
    ) {
        m_statusLabel->stopAllActions();
        m_statusLabel->setOpacity(255);
        m_statusLabel->runAction(
            CCRepeatForever::create(
                CCSequence::create(
                    CCEaseSineInOut::create(CCFadeTo::create(0.59f, 170)),
                    CCEaseSineInOut::create(CCFadeTo::create(0.59f, 255)),
                    nullptr
                )
            )
        );
    } else if (rm.m_status != m_previousStatus) {
        m_statusLabel->stopAllActions();
        m_statusLabel->runAction(CCEaseSineInOut::create(CCFadeTo::create(0.59f, 255)));
    }

    m_previousStatus = rm.m_status;
}

void VentillaPopup::updateSettings() {
    auto& rm = RadioManager::get();
    auto volume = Utils::getRadioVolume();

    m_volumeSlider->setValue(volume);
    m_volumeInput->setString(numToString(static_cast<int>(volume * 100)));

    geode::queueInMainThread([toggle = Ref(m_enableToggle)] {
        toggle->toggle(Utils::getSetting<"enabled", bool>());
    });

    updateSongTitle();
}

bool VentillaPopup::init() {
    Popup::init(255, 259);

    setTitle("Ventilla", "goldFont.fnt", 0.8f);

    auto btn = Button::createWithSpriteFrameName("GJ_optionsBtn_001.png", [](Button*) {
        openSettingsPopup(Mod::get(), false);
    });
    btn->setScale(0.726f);
    btn->setPosition(m_size - ccp(3, 3));

    m_mainLayer->addChild(btn);

    auto line = CCSprite::createWithSpriteFrameName("floorLine_001.png");
    line->setScaleX(0.445f);
    line->setScaleY(0.8f);
    line->setOpacity(89);
    line->setPosition({m_size.width / 2.f, 219});

    m_mainLayer->addChild(line);

    m_nowPlayingLabel = CCLabelBMFont::create("Now Playing:", "goldFont.fnt");
    m_nowPlayingLabel->setScale(0.575f);
    m_nowPlayingLabel->setPosition({66.7f, 203});

    m_mainLayer->addChild(m_nowPlayingLabel);

    m_songTitleLabel = CCLabelBMFont::create("N/A", "bigFont.fnt");
    m_songTitleLabel->setScale(0.368f);
    m_songTitleLabel->setAnchorPoint({0, 0.5f});
    m_songTitleLabel->setPosition({122.45f, 201.7f});

    m_mainLayer->addChild(m_songTitleLabel);

    auto bg = NineSlice::create("square02b_001.png");
    bg->setPosition({m_size.width / 2.f, 164});
    bg->setContentSize({228, 47});
    bg->setColor({0, 0, 0});
    bg->setOpacity(49);

    m_mainLayer->addChild(bg);

    bg = NineSlice::create("square02b_001.png");
    bg->setPosition({m_size.width / 2.f, 73});
    bg->setContentSize({228, 115});
    bg->setColor({0, 0, 0});
    bg->setOpacity(49);

    m_mainLayer->addChild(bg);

    m_warningLabel = CCLabelBMFont::create("* Radio is disabled along with menu music in GD options", "chatFont.fnt");
    m_warningLabel->setScale(0.425f);
    m_warningLabel->setAnchorPoint({0, 0.5f});
    m_warningLabel->setPosition({25, 122});
    m_warningLabel->setOpacity(67);

    m_mainLayer->addChild(m_warningLabel);

    auto lbl = CCLabelBMFont::create("Radio Status:", "bigFont.fnt");
    lbl->setScale(0.35f);
    lbl->setAnchorPoint({0, 0.5f});
    lbl->setPosition({25, 111});
    lbl->setOpacity(210);

    m_mainLayer->addChild(lbl);

    m_statusIcon = CCSprite::create("status-icon.png"_spr);
    m_statusIcon->setScale(0.37f);
    m_statusIcon->setPosition({118, 110.5f});

    m_mainLayer->addChild(m_statusIcon);

    m_statusLabel = CCLabelBMFont::create("Online", "bigFont.fnt");
    m_statusLabel->setScale(0.35f);
    m_statusLabel->setAnchorPoint({0, 0.5f});
    m_statusLabel->setPosition({126, 111});

    m_mainLayer->addChild(m_statusLabel);

    lbl = CCLabelBMFont::create("Volume:", "bigFont.fnt");
    lbl->setScale(0.35f);
    lbl->setAnchorPoint({0, 0.5f});
    lbl->setPosition({25, 87.5f});
    lbl->setOpacity(210);

    m_mainLayer->addChild(lbl);

    m_volumeSlider = Slider::create(this, menu_selector(VentillaPopup::onSlider), 0.55125f);
    m_volumeSlider->setPosition({136, 86.75f});
    m_volumeSlider->setValue(Utils::getRadioVolume());
    
    m_mainLayer->addChild(m_volumeSlider);

    m_volumeInput = TextInput::create(54.5f, "0");
    m_volumeInput->setScale(0.5f);
    m_volumeInput->setPosition({217.5f, 86.75f});
    m_volumeInput->setString(numToString(Utils::getSetting<"volume", int>()));
    m_volumeInput->setFilter("1234567890");
    m_volumeInput->setCallback([this](const std::string& str) {
        if (auto res = numFromString<int>(str); res.isOk()) {
            Mod::get()->setSettingValue("volume", static_cast<int>(clampf(res.unwrap(), 0, 100)));
        }
    });

    m_mainLayer->addChild(m_volumeInput);

    lbl = CCLabelBMFont::create("Enable Radio:", "bigFont.fnt");
    lbl->setScale(0.35f);
    lbl->setAnchorPoint({0, 0.5f});
    lbl->setPosition({25, 62});
    lbl->setOpacity(210);

    m_mainLayer->addChild(lbl);

    m_enableToggle = CCMenuItemExt::createToggler(
        CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png"),
        CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png"),
        [this](CCObject* sender) {
            Mod::get()->setSettingValue<bool>(
                "enabled",
                !static_cast<CCMenuItemToggler*>(sender)->isToggled()
            );
        }
    );  
    m_enableToggle->setPosition({123, 62});
    m_enableToggle->setScale(0.475f);
    m_enableToggle->toggle(Utils::getSetting<"enabled", bool>());

    m_buttonMenu->addChild(m_enableToggle);

    lbl = CCLabelBMFont::create("Website:", "bigFont.fnt");
    lbl->setScale(0.35f);
    lbl->setAnchorPoint({0, 0.5f});
    lbl->setPosition({25, 38});
    lbl->setOpacity(210);

    m_mainLayer->addChild(lbl);

    auto spr = CCSprite::create("GJ_button_05.png");

    auto spr2 = CCSprite::create("website-icon.png"_spr);

    spr->addChildAtPosition(spr2, Anchor::Center, {0.5f, 0.5f});

    spr2->setPosition(spr2->getPosition() - ccp(0.5f, 0.5f));

    btn = Button::createWithNode(spr, [](Button*) {
        web::openLinkInBrowser("https://radio.5infin.es");
    });
    btn->setPosition({91, 37});
    btn->setScale(0.516925f);
    
    m_mainLayer->addChild(btn);

    lbl = CCLabelBMFont::create("  Request your\nfavorite songs!", "bigFont.fnt");
    lbl->setScale(0.25f);
    lbl->setPosition({180, 29.5f});
    lbl->setOpacity(131);

    m_mainLayer->addChild(lbl);

    btn = Button::createWithSpriteFrameName("accountBtn_messages_001.png", [](Button*) {
        web::openLinkInBrowser("https://forms.cloud.microsoft/e/E8PNCNQE65");
    });
    btn->setScale(0.525f);
    btn->setPosition({227, 29});
    
    m_mainLayer->addChild(btn);

    btn = Button::createWithSpriteFrameName("gj_discordIcon_001.png", [](Button*) {
        web::openLinkInBrowser("https://discord.gg/MBGX5BTEU7");
    });
    btn->setScale(0.85f);
    btn->setPosition({272, 14});
    
    m_mainLayer->addChild(btn);

    btn = Button::createWithSpriteFrameName("gj_twIcon_001.png", [](Button*) {
        web::openLinkInBrowser("https://x.com/Analogias_Esp");
    });
    btn->setScale(0.85f);
    btn->setPosition({272, 43});
    
    m_mainLayer->addChild(btn);

    for (int i = 0; i < 16; i++) {
        auto bar = CCLayerColor::create({255, 255, 255, 67}, 10, 2);
        bar->setContentSize({10, 2});
        bar->setOpacity(67);
        bar->setPosition({31 + (12.9f * i), 164});
        bar->ignoreAnchorPointForPosition(false);

        m_mainLayer->addChild(bar);

        m_bars[i] = bar;
    }
    
    updateStatus();
    updateSongTitle();

    schedule(schedule_selector(VentillaPopup::updateSongTitle), 0.17f, kCCRepeatForever, 1.f);
    schedule(schedule_selector(VentillaPopup::updateSpectrum), 1.f/ 60.f, kCCRepeatForever, 0.f);

    return true;
}