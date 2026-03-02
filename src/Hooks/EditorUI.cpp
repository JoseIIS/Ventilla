#include "../Other/RadioManager.hpp"

#include "EditorUI.hpp"

void ProEditorUI::onPlayback(CCObject* sender) {
    EditorUI::onPlayback(sender);
    RadioManager::get().updateWithFade(0.5f);
}
    
void ProEditorUI::onPlaytest(CCObject* sender) {
    EditorUI::onPlaytest(sender);
    RadioManager::get().updateWithFade(0.5f);
}