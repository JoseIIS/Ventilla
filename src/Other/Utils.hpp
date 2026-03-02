#pragma once

#include "../Includes.hpp"

class Utils {

public:

    static void setHookEnabled(std::string_view, bool);

    static void setHooksEnabled(const std::vector<std::string_view>&, bool);

    static ccColor3B getColorForStatus(RadioStatus); 

    static ZStringView getNameForStatus(RadioStatus);

    static float getRadioVolume();

    template <string::ConstexprString S, typename T>
    static const T& getSetting() {
        static T value = (
            listenForSettingChanges<T>(S.data(), [](T val) {
                value = val;
                
                auto& rm = RadioManager::get();

                rm.update();

                if (rm.m_ventillaPopup) {
                    rm.m_ventillaPopup->updateSettings();
                }
            }),
            Mod::get()->getSettingValue<T>(S.data())
        );

        return value;
    }

};