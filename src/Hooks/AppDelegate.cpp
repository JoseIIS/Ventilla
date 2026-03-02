#ifdef GEODE_IS_DESKTOP

#include "../Other/RadioManager.hpp"

#include "AppDelegate.hpp"

void ProAppDelegate::applicationDidEnterBackground() {
    AppDelegate::applicationDidEnterBackground();

    auto& rm = RadioManager::get();

    rm.m_isInBackground = true;
    rm.update();
}

void ProAppDelegate::applicationWillEnterForeground() {
    AppDelegate::applicationWillEnterForeground();

    auto& rm = RadioManager::get();

    rm.m_isInBackground = false;
    rm.update();
}

#endif