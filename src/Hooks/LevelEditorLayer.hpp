#include <Geode/modify/LevelEditorLayer.hpp>

class $modify(ProLevelEditorLayer, LevelEditorLayer) {

    struct Fields {
        ~Fields();
    };

    bool init(GJGameLevel*, bool);

    void onStopPlaytest();

};