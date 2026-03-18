#pragma once

#include "../Includes.hpp"

class VentillaPopup : public Popup {

private:

    CCMenuItemToggler* m_enableToggle = nullptr;

    CCLabelBMFont* m_nowPlayingLabel = nullptr;
    CCLabelBMFont* m_songTitleLabel = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCLabelBMFont* m_warningLabel = nullptr;
    CCLabelBMFont* m_linkLabel = nullptr;

    CCSprite* m_statusIcon = nullptr;

    Slider* m_volumeSlider = nullptr;

    TextInput* m_volumeInput = nullptr;

    std::array<CCLayerColor*, 16> m_bars;

    std::string m_lastSongTitle;

    RadioStatus m_previousStatus = static_cast<RadioStatus>(-1);

    int m_currentTitleLetter = -3;

    VentillaPopup();
    
    ~VentillaPopup();

    bool init() override;

    void onSlider(CCObject*);

    void updateSongTitle(float = 0.f);
    void updateSpectrum(float);
    void updateHover(float);

public:

    static VentillaPopup* create();

    void updateStatus();

    void updateSettings();

};