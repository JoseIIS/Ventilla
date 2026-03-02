#include "../Other/RadioManager.hpp"
#include "../UI/VentillaPopup.hpp"

#include "Utils.hpp"

void Utils::setHookEnabled(std::string_view name, bool enabled) {
    for (auto hook : Mod::get()->getHooks()) {
        if (hook->getDisplayName() == name) {
            (void)(enabled ? hook->enable() : hook->disable());
            break;
        }
    }
}

void Utils::setHooksEnabled(const std::vector<std::string_view>& names, bool enabled) {
    for (const auto& name : names) {
        setHookEnabled(name, enabled);
    }
}

ccColor3B Utils::getColorForStatus(RadioStatus status) {
    switch (status) {
        default:
        case RadioStatus::Off: return ccColor3B{ 127, 127, 127 };
        case RadioStatus::Online: return ccColor3B{ 69, 255, 69 };
        case RadioStatus::Reconnecting: return ccColor3B{ 255, 147, 69 };
        case RadioStatus::Connecting: return ccColor3B{ 255, 246, 69 };
    }
}

ZStringView Utils::getNameForStatus(RadioStatus status) {
    switch (status) {
        default: return "Unknown";
        case RadioStatus::Off: return "Off";
        case RadioStatus::Online: return "Online";
        case RadioStatus::Reconnecting: return "Reconnecting";
        case RadioStatus::Connecting: return "Connecting";
    }
}

float Utils::getRadioVolume() {
    return Utils::getSetting<"volume", int>() / 100.f;
}