#include <Geode/modify/EditorPauseLayer.hpp>

#include "../Includes.hpp"

class $modify(ProEditorPauseLayer, EditorPauseLayer) {

    bool init(LevelEditorLayer*);

    void onExitEditor(CCObject*);

};