#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

// Menu Hooks
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelSelectLayer.hpp>
#include <Geode/modify/GJGarageLayer.hpp>
#include <Geode/modify/CreatorLayer.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>

// Special Room Hooks (Shops, Vaults, Tower)
#include <Geode/modify/GJShopLayer.hpp>
#include <Geode/modify/SecretRewardsLayer.hpp>
#include <Geode/modify/SecretLayer.hpp>
#include <Geode/modify/SecretLayer2.hpp>
#include <Geode/modify/SecretLayer3.hpp>
#include <Geode/modify/SecretLayer4.hpp>
#include <Geode/modify/LevelAreaInnerLayer.hpp>

using namespace geode::prelude;

// --- Configuration Constants ---
const float VOLUMEN_MENU = 0.5f;
const float VOLUMEN_JUEGO = 0.0f;
const char* RADIO_URL = "http://radio.5infin.es:8006/radio.mp3";
const char* WEB_URL = "https://radio.5infin.es";

// --- Global Variables ---
FMOD::Channel* g_radioChannel = nullptr;
FMOD::Sound* g_radioStream = nullptr;
bool g_errorShown = false;
float g_reconnectTimer = 0.0f;
bool g_radioEnabled = true;

// Metadata storage
std::string g_currentSongTitle = "Waiting for metadata...";
float g_metadataTimer = 0.0f;

// Forward declaration
void restaurarRadio();

// Metadata Logic

void updateMetadata() {
    // Only proceed if radio components are initialized
    if (!g_radioStream || !g_radioChannel) return;

    FMOD_TAG tag;
    int numTags = 0;
    int numTagsUpdated = 0;

    // Check if new tags are available since last update
    g_radioStream->getNumTags(&numTags, &numTagsUpdated);

    if (numTagsUpdated > 0) {
        for (int i = 0; i < numTags; i++) {
            if (g_radioStream->getTag(nullptr, i, &tag) == FMOD_OK) {
                std::string tagName = (tag.name) ? tag.name : "";

                // Check for standard Icecast/ID3 title tags
                if (tagName == "TITLE" || tagName == "icy-name" || tagName == "ICY-DESCRIPTION") {
                    if (tag.data) {
                        g_currentSongTitle = std::string((char*)tag.data);

                        // Truncate if the title is too long to prevent UI overflow
                        if (g_currentSongTitle.length() > 60) {
                            g_currentSongTitle = g_currentSongTitle.substr(0, 57) + "...";
                        }
                    }
                }
            }
        }
    }
}

// UI Class: Radio Status Popup

class RadioStatusPopup : public geode::Popup<> {
protected:
    bool setup() override {
        auto winSize = m_mainLayer->getContentSize();

        // 1. Setup Title
        this->setTitle("Ventilla Status");
        if (auto titleNode = m_mainLayer->getChildByID("title-label")) {
            titleNode->setPositionY(winSize.height - 22.f);
        }

        // 2. Setup Status Text
        std::string statusText = "Unknown";
        ccColor3B statusColor = {255, 255, 255};

        // Determine status string and color based on connection state
        if (!g_radioEnabled) {
            statusText = "Disabled";
            statusColor = {200, 100, 100};
        } else if (g_radioChannel) {
            bool isPlaying = false;
            g_radioChannel->isPlaying(&isPlaying);

            if (isPlaying) {
                statusText = "Online";
                statusColor = {100, 255, 100};
            } else {
                statusText = "Reconnecting...";
                statusColor = {255, 200, 100};
            }
        } else {
            statusText = "Idle";
            statusColor = {255, 200, 100};
        }

        auto statusLabel = CCLabelBMFont::create(fmt::format("Status: {}", statusText).c_str(), "bigFont.fnt");
        statusLabel->setScale(0.5f);
        statusLabel->setPosition({winSize.width / 2, winSize.height - 50.f});
        statusLabel->setColor(statusColor);
        m_mainLayer->addChild(statusLabel);

        // 3. Setup "Now Playing" Section (Only if enabled)
        if (g_radioEnabled && g_radioChannel) {

            // A) Create "NOW PLAYING" header
            auto nowPlayingHeader = CCLabelBMFont::create("NOW PLAYING", "goldFont.fnt");
            nowPlayingHeader->setScale(0.6f);
            nowPlayingHeader->setPosition({winSize.width / 2, winSize.height - 80.f});
            m_mainLayer->addChild(nowPlayingHeader);

            // Create pulsing animation (heartbeat effect)
            auto scaleUp = CCScaleTo::create(0.5f, 0.65f);
            auto scaleDown = CCScaleTo::create(0.5f, 0.6f);
            auto seq = CCSequence::create(scaleUp, scaleDown, nullptr);
            auto repeat = CCRepeatForever::create(seq);
            nowPlayingHeader->runAction(repeat);

            // B) Create Background Box for song title
            auto songBg = CCScale9Sprite::create("square02_small.png");
            songBg->setContentSize({260.f, 40.f});
            songBg->setOpacity(100);
            songBg->setPosition({winSize.width / 2, winSize.height - 110.f});
            m_mainLayer->addChild(songBg);

            // Create Song Title Label
            auto songLabel = CCLabelBMFont::create(g_currentSongTitle.c_str(), "chatFont.fnt");
            songLabel->setPosition(songBg->getPosition());
            songLabel->setColor({255, 255, 200});

            // Auto-scale text if it exceeds box width
            float maxWidth = 240.f;
            if (songLabel->getContentSize().width > maxWidth) {
                songLabel->setScale(maxWidth / songLabel->getContentSize().width);
            }
            m_mainLayer->addChild(songLabel);
        }

        // 4. Setup Buttons

        // Website Button
        auto webBtnSprite = CCScale9Sprite::create("GJ_button_01.png");
        webBtnSprite->setContentSize({140.f, 30.f});

        auto webLabel = CCLabelBMFont::create("Website", "bigFont.fnt");
        webLabel->setScale(0.5f);
        webLabel->setPosition(webBtnSprite->getContentSize() / 2);
        webBtnSprite->addChild(webLabel);

        auto webBtn = CCMenuItemSpriteExtra::create(
            webBtnSprite, this, menu_selector(RadioStatusPopup::onOpenWeb)
        );
        m_buttonMenu->addChild(webBtn);

        // Toggle Button (Enable/Disable)
        const char* btnSpriteImg = g_radioEnabled ? "GJ_button_02.png" : "GJ_button_01.png";
        const char* btnText = g_radioEnabled ? "Disable" : "Enable";

        auto toggleBtnSprite = CCScale9Sprite::create(btnSpriteImg);
        toggleBtnSprite->setContentSize({140.f, 30.f});

        auto toggleLabel = CCLabelBMFont::create(btnText, "bigFont.fnt");
        toggleLabel->setScale(0.5f);
        toggleLabel->setPosition(toggleBtnSprite->getContentSize() / 2);
        toggleBtnSprite->addChild(toggleLabel);

        auto toggleBtn = CCMenuItemSpriteExtra::create(
            toggleBtnSprite, this, menu_selector(RadioStatusPopup::onToggleRadio)
        );
        m_buttonMenu->addChild(toggleBtn);

        // Configure Layout (Vertical stack)
        m_buttonMenu->setLayout(
            ColumnLayout::create()->setGap(8.f)->setAxisAlignment(AxisAlignment::Center)
        );
        m_buttonMenu->setPosition({winSize.width / 2, 55.f});
        m_buttonMenu->updateLayout();

        return true;
    }

public:
    static RadioStatusPopup* create() {
        auto ret = new RadioStatusPopup();
        if (ret && ret->initAnchored(300.f, 290.f)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void onToggleRadio(CCObject*) {
        g_radioEnabled = !g_radioEnabled;

        // Update user state and show notification
        if (!g_radioEnabled) {
            Notification::create("Radio Disabled", NotificationIcon::Error)->show();
            g_currentSongTitle = "Radio Disabled";
        } else {
            Notification::create("Radio Enabled", NotificationIcon::Success)->show();
            g_currentSongTitle = "Connecting...";
        }

        // Apply changes immediately
        restaurarRadio();
        this->onClose(nullptr);
    }

    void onOpenWeb(CCObject*) {
        geode::utils::web::openLinkInBrowser(WEB_URL);
    }
};

// Core Logic: Radio Management

void restaurarRadio() {
    auto engine = FMODAudioEngine::sharedEngine();
    auto system = engine->m_system;

    // 1. If disabled by user, cleanup resources and exit
    if (!g_radioEnabled) {
        if (g_radioChannel) { g_radioChannel->stop(); g_radioChannel = nullptr; }
        if (g_radioStream) { g_radioStream->release(); g_radioStream = nullptr; }
        return;
    }

    // 2. Stop any existing game music to prevent overlapping
    engine->stopAllMusic(true);

    bool isPlaying = false;
    if (g_radioChannel) g_radioChannel->isPlaying(&isPlaying);

    // 3. If radio is already active, ensure it's unpaused and volume is correct
    if (g_radioChannel && isPlaying) {
        g_radioChannel->setPaused(false);
        g_radioChannel->setVolume(VOLUMEN_MENU);
        g_errorShown = false;
        g_reconnectTimer = 0.0f;
    }
    // 4. Otherwise, attempt to connect
    else {
        // Release old stream handle if exists
        if (g_radioStream) { g_radioStream->release(); g_radioStream = nullptr; }
        g_radioChannel = nullptr;

        // Prevent spamming connection attempts if we recently showed an error
        if (!g_errorShown) {
            log::info("Attempting to connect radio...");
            g_currentSongTitle = "Connecting...";
            Notification::create("Connecting Radio...", NotificationIcon::Loading)->show();

            // Create stream ignoring tags initially to speed up start
            FMOD_RESULT res = system->createSound(
                RADIO_URL, FMOD_CREATESTREAM | FMOD_LOOP_NORMAL | FMOD_IGNORETAGS, nullptr, &g_radioStream
            );

            if (res == FMOD_OK) {
                system->playSound(g_radioStream, nullptr, false, &g_radioChannel);
                g_radioChannel->setVolume(VOLUMEN_MENU);
                Notification::create("Radio Connected!", NotificationIcon::Success)->show();
                g_currentSongTitle = "Waiting for info...";
            } else {
                log::error("Radio Connection Failed: {}", static_cast<int>(res));
                g_errorShown = true;
                g_currentSongTitle = "Connection Error";
            }
        }
    }
}

// Helper to pause radio when entering special rooms (Shops, Vaults, etc.)
void pausarRadioParaSalaEspecial() {
    if (g_radioChannel && g_radioEnabled) {
        g_radioChannel->setPaused(true);
    }
}

// GameManager Hook (The Watchdog)

class $modify(SilenceGameManager, GameManager) {
    // Override playMenuMusic to prevent default menu loop if radio is enabled
    void playMenuMusic() {
        if(g_radioEnabled) {}
        else { GameManager::playMenuMusic(); }
    }

    // Override fadeIn to prevent music overlap when exiting levels
    void fadeInMenuMusic() {
        if(g_radioEnabled) {}
        else { GameManager::fadeInMenuMusic(); }
    }

    void update(float dt) {
        GameManager::update(dt);

        if (g_radioEnabled && g_radioChannel) {
            // Check current channel state
            bool isPlaying = false;
            bool isPaused = false;
            g_radioChannel->isPlaying(&isPlaying);
            g_radioChannel->getPaused(&isPaused);

            // 1. Reconnection Logic (Watchdog)
            // If not playing and not paused by us, connection dropped or stream ended
            if (!isPlaying && !isPaused) {
                g_reconnectTimer += dt;

                // Wait 3 seconds before retrying to avoid spam
                if (g_reconnectTimer > 3.0f) {
                    g_errorShown = false;
                    restaurarRadio();
                    g_reconnectTimer = 0.0f;
                }
            } else {
                g_reconnectTimer = 0.0f;

                // 2. Metadata Update Logic
                // Only check tags every 1.0 second to save performance
                g_metadataTimer += dt;
                if (g_metadataTimer > 1.0f) {
                    updateMetadata();
                    g_metadataTimer = 0.0f;
                }
            }
        }
    }
};

// Menu Hooks

class $modify(RadioMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        // Reset error flag and start radio on menu load
        g_errorShown = false;
        restaurarRadio();

        // Add Radio Button to Bottom Menu
        auto bottomMenu = this->getChildByID("bottom-menu");
        if (bottomMenu) {
            auto radioBtnSprite = CCSprite::createWithSpriteFrameName("GJ_playMusicBtn_001.png");
            auto radioBtn = CCMenuItemSpriteExtra::create(
                radioBtnSprite, this, menu_selector(RadioMenuLayer::onRadioPopupButton)
            );
            bottomMenu->addChild(radioBtn);
            bottomMenu->updateLayout();
        }
        return true;
    }

    void onRadioPopupButton(CCObject*) {
        // Refresh metadata before showing popup
        updateMetadata();
        RadioStatusPopup::create()->show();
    }
};

class $modify(RadioPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // When entering a level, keep radio running but mute volume (0%)
        // This ensures instant resume when exiting the level
        if (g_radioEnabled && g_radioChannel) {
            g_radioChannel->setPaused(false);
            g_radioChannel->setVolume(VOLUMEN_JUEGO);
        }
        return true;
    }
};

// General UI Hooks (Restore Radio)

// Restore radio when entering Level Info (Play button screen)
class $modify(RadioLevelInfo, LevelInfoLayer) {
    bool init(GJGameLevel* l, bool c) {
        if(!LevelInfoLayer::init(l,c)) return false;
        restaurarRadio();
        return true;
    }
    void onEnter() {
        LevelInfoLayer::onEnter();
        restaurarRadio();
    }
};

// Restore radio on Level Select (Stereo Madness, etc.)
class $modify(RadioLevelSelect, LevelSelectLayer) {
    bool init(int p) {
        if(!LevelSelectLayer::init(p)) return false;
        restaurarRadio();
        return true;
    }
};

// Restore radio in Icon Kit
class $modify(RadioGarage, GJGarageLayer) {
    bool init() {
        if(!GJGarageLayer::init()) return false;
        restaurarRadio();
        return true;
    }
};

// Restore radio in Creator Menu
class $modify(RadioCreator, CreatorLayer) {
    bool init() {
        if(!CreatorLayer::init()) return false;
        restaurarRadio();
        return true;
    }
};

// Restore radio in Level Browser (Saved/Online levels)
class $modify(RadioLevelBrowser, LevelBrowserLayer) {
    bool init(GJSearchObject* s) {
        if(!LevelBrowserLayer::init(s)) return false;
        restaurarRadio();
        return true;
    }
};

// Special Room Hooks (Pause Radio)

// Shops
class $modify(OShop, GJShopLayer) {
    bool init(ShopType p){
        if(!GJShopLayer::init(p)) return false;
        pausarRadioParaSalaEspecial();
        return true;
    }
};

// Treasure Room
class $modify(OTreasure, SecretRewardsLayer) {
    bool init(bool p){
        if(!SecretRewardsLayer::init(p)) return false;
        pausarRadioParaSalaEspecial();
        return true;
    }
};

// The Vault (Spooky)
class $modify(OV1, SecretLayer) {
    bool init(){
        if(!SecretLayer::init()) return false;
        pausarRadioParaSalaEspecial();
        return true;
    }
};

// Vault of Secrets
class $modify(OV2, SecretLayer2) {
    bool init(){
        if(!SecretLayer2::init()) return false;
        pausarRadioParaSalaEspecial();
        return true;
    }
};

// Chamber of Time
class $modify(OV3, SecretLayer3) {
    bool init(){
        if(!SecretLayer3::init()) return false;
        pausarRadioParaSalaEspecial();
        return true;
    }
};

// The Tower Map
class $modify(OTower, LevelAreaInnerLayer) {
    bool init(bool p){
        if(!LevelAreaInnerLayer::init(p)) return false;
        pausarRadioParaSalaEspecial();
        return true;
    }
};

// Fallback/Event Secret Layer
class $modify(OV4, SecretLayer4) {
    bool init(){
        if(!SecretLayer4::init()) return false;
        pausarRadioParaSalaEspecial();
        return true;
    }
};
