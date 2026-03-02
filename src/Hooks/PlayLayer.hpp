#include <Geode/modify/PlayLayer.hpp>

#include "../Includes.hpp"

class $modify(ProPlayLayer, PlayLayer) {

    struct Fields {
        ~Fields();
    };

    void setupHasCompleted();

    void onQuit();

    void togglePracticeMode(bool);

    void resetLevel();

};