#pragma once

using namespace geode::prelude;

enum class RadioStatus {
    Off = 0,
    Online = 1,
    Reconnecting = 2,
    Connecting = 3,
};