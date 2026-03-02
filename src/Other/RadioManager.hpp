#pragma once

#include "../Includes.hpp"

class RadioUpdater;
class VentillaPopup;

class RadioManager {

public:

    FMOD::Channel* m_channel = nullptr;
    FMOD::Sound* m_sound = nullptr;
    FMOD::DSP* m_dsp = nullptr;

    RadioUpdater* m_updater = nullptr;
    VentillaPopup* m_ventillaPopup = nullptr;

    std::string m_lastMusicPlayed;

    RadioStatus m_status = RadioStatus::Connecting;

    bool m_firstMenuLayer = true;
    bool m_radioFailed = false;
    bool m_radioAvailable = false;
    bool m_isInBackground = false;
    bool m_isStarving = false;
    bool m_isFirstFading = true;
    bool m_isFading = false;
    bool m_shouldshowNotification = false;
    bool m_didConnect = false;
    bool m_didQuitPlayLayer = false;
    bool m_didQuitEditor = false;
    bool m_didEnterEditor = false;
    bool m_didEnterTreasureRoom = false;
    bool m_didEnterTheVault = false;

    float m_starvingTime = 0.0f;
    float m_firstFadeTime = 0.f;
    float m_fadeRequiredTime = 0.f;
    float m_fadeTime = 0.f;
    float m_fadeProgress = 1.f;

    static RadioManager& get();

    void start();
    void play();

    void update();
    void updateConnection();
    void updateState();
    void updateVolume();
    void updateWithFade(float);
    void updateFade(float);
    void updateHooks();

    std::string getSongTitle();

    void showNotification(ZStringView, NotificationIcon);
    void onNotification(ZStringView, NotificationIcon);

    void doFade(float);

};