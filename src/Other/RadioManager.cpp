#include "../UI/VentillaPopup.hpp"

#include "RadioManager.hpp"
#include "Utils.hpp"
#include "RadioUpdater.hpp"

RadioManager& RadioManager::get() {
    static RadioManager instance;
    return instance;
}

void RadioManager::start() {
    if (m_channel) {
        m_channel->stop();
        m_channel = nullptr;
    }

    if (m_sound) {
        m_sound->release();
        m_sound = nullptr;
    }

    if (!m_updater) {
        m_updater = new RadioUpdater();
        m_updater->schedule();
    }

    auto res = FMODAudioEngine::get()->m_system->createSound(
        "http://radio.5infin.es:8006/radio.mp3",
        FMOD_CREATESTREAM | FMOD_NONBLOCKING,
        nullptr,
        &m_sound
    );

    if (res != FMOD_OK) {
        m_sound = nullptr;
        log::error("radio sound fail! id: {}", (int)res);
        return;
    }
}

void RadioManager::play() {
    auto system = FMODAudioEngine::get()->m_system;
    auto res = system->playSound(m_sound, nullptr, false, &m_channel);

    if (res != FMOD_OK) {
        m_channel = nullptr;
        log::error("radio channel fail! id: {}", (int)res);
        return;
    }

    system->createDSPByType(FMOD_DSP_TYPE_FFT, &m_dsp);
    m_channel->addDSP(0, m_dsp);

    update();
}

void RadioManager::update() {
    if (!m_sound) {
        start();
    }
    
    updateHooks();
    updateFade(0.f);
    updateState();
    updateVolume();

    if (m_shouldshowNotification && !m_firstMenuLayer) {
        m_shouldshowNotification = false;

        if (m_status == RadioStatus::Connecting || m_status == RadioStatus::Reconnecting) {
            showNotification("Failed to connect to the radio.", NotificationIcon::Error);
        }
    }
}

void RadioManager::updateConnection() {
    FMOD_OPENSTATE state;

    m_sound->getOpenState(&state, nullptr, &m_isStarving, nullptr);

    if (!m_isStarving) {
        m_starvingTime = 0.f;
    }

    if (state == FMOD_OPENSTATE_ERROR || (m_isStarving && m_starvingTime > 0.7f && !m_radioFailed)) {
        if (!m_radioFailed) {
            onNotification(m_didConnect ? "Disconnected from the radio." : "Failed to connect to the radio.", NotificationIcon::Error);
            doFade(3.f);
        }

        m_radioFailed = true;

        start();
        update();

        return;
    }

    if (state != FMOD_OPENSTATE_READY) {
        return;
    }

    if (m_radioFailed)  {
        onNotification(m_didConnect ? "Reconnected to the radio." : "Connected to the radio.", NotificationIcon::Success);
        doFade(3.f);
    }

    m_radioFailed = false;
    m_didConnect = true;

    play();
}

void RadioManager::updateState() {
    auto gm = GameManager::get();
    auto pl = PlayLayer::get();
    auto lel = LevelEditorLayer::get();
    auto inPlayLayer = pl && !m_didQuitPlayLayer;
    auto inEditor = (lel || m_didEnterEditor) && !m_didQuitEditor;
    auto isOutsideMenus = inPlayLayer || inEditor;
    
    auto playOutsideMenus = 
        (pl && inPlayLayer && Utils::getSetting<"play-in-practice-mode", bool>() && pl->m_isPracticeMode && !pl->m_isPaused)
        || (pl && inPlayLayer && Utils::getSetting<"play-in-normal-mode", bool>() && !pl->m_isPracticeMode && !pl->m_isPaused)
        || (pl && inPlayLayer && Utils::getSetting<"play-in-pause-menu", bool>() && pl->m_isPaused)
        || (inEditor && Utils::getSetting<"play-in-editor", bool>() && (!lel || (lel->m_playbackMode != PlaybackMode::Playing && !lel->m_editorUI->m_isPlayingMusic)));

    auto isMenuMusic = m_lastMusicPlayed == gm->getMenuMusicFile() && (!isOutsideMenus || playOutsideMenus);

    if (!isMenuMusic && !isOutsideMenus) {
        isMenuMusic = Mod::get()->hasSetting(m_lastMusicPlayed)
            ? Mod::get()->getSettingValue<bool>(m_lastMusicPlayed)
            : Mod::get()->getSettingValue<bool>("extras");
    }

    auto prevRadioAvailable = m_radioAvailable;

    m_radioAvailable =
        !m_firstMenuLayer
        && (!m_isInBackground || Utils::getSetting<"play-in-background", bool>())
        && Utils::getSetting<"enabled", bool>()
        && isMenuMusic
        && !m_radioFailed
        && !gm->getGameVariable("0122");

    if (!m_firstMenuLayer && m_radioAvailable == prevRadioAvailable) {
        m_isFading = false;
    }

    if (m_updater) {
        m_updater->setPaused(isOutsideMenus && !playOutsideMenus);
    }

    if (Utils::getSetting<"enabled", bool>() && !m_radioFailed && !gm->getGameVariable("0122")) {
        m_status = RadioStatus::Online;
    } else if (!Utils::getSetting<"enabled", bool>() || gm->getGameVariable("0122")) {
        m_status = RadioStatus::Off;
    } else if (!m_didConnect) {
        m_status = RadioStatus::Connecting;
    } else {
        m_status = RadioStatus::Reconnecting;
    }

    if (m_ventillaPopup) {
        m_ventillaPopup->updateStatus();
    }
}

void RadioManager::updateVolume() {
    auto fmod = FMODAudioEngine::get();
    auto progress = m_isFading && m_radioAvailable ? m_fadeProgress : 1.f;

    if (
        (m_isFirstFading && GameManager::get()->getGameVariable("0122"))
        || !Utils::getSetting<"fade-in", bool>()
    ) {
        m_isFirstFading = false;
    }

    if (m_isStarving && m_radioAvailable) {
        progress *= 1.f - clampf(m_starvingTime / 0.6f, 0.f, 1.f);
    }

    if (m_isFirstFading && m_radioAvailable) {
        float t = clampf(m_firstFadeTime / 1.5f, 0.f, 1.f);
        progress *= t * t;
    }

    auto radioVolume = (Utils::getRadioVolume() * fmod->m_musicVolume) * (m_radioAvailable ? progress : (1.f - progress));
    auto musicVolume = fmod->m_musicVolume * (m_radioAvailable ? (1.f - progress) : progress);

    fmod->m_backgroundMusicChannel->setVolume(musicVolume);

    if (m_channel) {
        m_channel->setVolume(radioVolume);
    }
}

void RadioManager::updateWithFade(float time) {
    doFade(time);
    update();
}

void RadioManager::updateFade(float dt) {
    if (
        (!m_isStarving && !m_isFirstFading && !m_isFading)
        || m_firstMenuLayer
    ) {
        return;
    }

    if (m_isStarving) {
        // m_starvingTime += dt;
    }

    if (m_isFirstFading) {
        m_firstFadeTime += 0.033f;

        if (m_firstFadeTime > 1.5f) {
            m_isFirstFading = false;
        }
    }

    if (m_isFading) {     
        m_fadeTime += dt;
        m_fadeProgress = clampf(m_fadeTime / m_fadeRequiredTime, 0.f, 1.f);

        if (m_fadeProgress >= 1.f) {
            m_isFading = false;
            m_fadeTime = 0.f;
        }
    }

    updateVolume();
}

void RadioManager::updateHooks() {
    if (PlayLayer::get() && !m_didQuitPlayLayer) {
        Utils::setHookEnabled("FMODAudioEngine::playMusic", false);

        Utils::setHooksEnabled(
            { "FMOD::ChannelControl::setVolume", "PlayLayer::resetLevel" },
            (
                Utils::getSetting<"play-in-practice-mode", bool>()
                || Utils::getSetting<"play-in-normal-mode", bool>()
                || Utils::getSetting<"play-in-pause-menu", bool>()
            )
        );

        return;
    }

    if ((LevelEditorLayer::get() || m_didEnterEditor) && !m_didQuitEditor) {
        Utils::setHookEnabled("FMODAudioEngine::playMusic", false);

        Utils::setHooksEnabled(
            { "FMOD::ChannelControl::setVolume", "LevelEditorLayer::onStopPlaytest" },
            Utils::getSetting<"play-in-editor", bool>()
        );
        
        return;
    }

    Utils::setHookEnabled("FMODAudioEngine::playMusic", true);
    Utils::setHookEnabled("FMOD::ChannelControl::setVolume", true);
}

std::string RadioManager::getSongTitle() {
    if (!Utils::getSetting<"enabled", bool>() || GameManager::get()->getGameVariable("0122")) {
        return "The Radio is Off";
    }

    if (!m_sound) {
        return "N/A";
    }

    FMOD_TAG tag;
    std::string ret = "N/A";
    int numTags = 0;
    int numTagsUpdated = 0;

    m_sound->getNumTags(&numTags, &numTagsUpdated);

    for (int i = 0; i < numTags; ++i) {
        if (m_sound->getTag(nullptr, i, &tag) != FMOD_OK) {
            continue;
        }

        if (!tag.name || !tag.data) {
            continue;
        }

        std::string name = tag.name;

        if (name == "StreamTitle" || name == "TITLE") {
            ret = {
                reinterpret_cast<const char*>(tag.data),
                tag.datalen
            };
            
            while (!ret.empty() && ret.back() == '\0') {
                ret.pop_back();
            }
        }
    }

    if (ret.empty()) {
        return "N/A";
    }

    return ret;
}

void RadioManager::showNotification(ZStringView str, NotificationIcon icon) {
    auto notif = Notification::create(str, icon);
    notif->setTime(1.35f);
    notif->show();
}

void RadioManager::onNotification(ZStringView str, NotificationIcon icon) {
    if (m_shouldshowNotification && m_firstMenuLayer) {
        return;
    }

    if (m_didConnect && !m_isFading) {
        m_fadeTime = 0.f;
        m_isFading = true;
    }

    if (m_firstMenuLayer) {
        m_shouldshowNotification = true;
        return;
    }

    showNotification(str, icon);
}

void RadioManager::doFade(float time) {
    if (!Utils::getSetting<"fade-in", bool>()) {
        return;
    }

    m_fadeRequiredTime = time;
    m_fadeTime = 0.f;
    m_isFading = true;
}