#include "../Other/RadioManager.hpp"

#include "OptionsLayer.hpp"

void ProOptionsLayer::onMenuMusic(CCObject* sender) {
    OptionsLayer::onMenuMusic(sender);
    RadioManager::get().update();
}