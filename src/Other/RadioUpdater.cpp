#include "RadioUpdater.hpp"
#include "RadioManager.hpp"

RadioUpdater::RadioUpdater() {
    retain();
}

void RadioUpdater::update(float dt) {
    auto& rm = RadioManager::get();

    m_connectionCheckCooldown += dt;

    if (m_connectionCheckCooldown > 0.5f) {
        rm.updateConnection();
        m_connectionCheckCooldown = 0.f;
    }

    rm.updateFade(dt);
}

void RadioUpdater::schedule() {
    CCScheduler::get()->scheduleSelector(schedule_selector(RadioUpdater::update), this, 1.f / 30.f, kCCRepeatForever, 0.f, false);
}

void RadioUpdater::setPaused(bool paused) {
    if (paused) {
        CCScheduler::get()->pauseTarget(this);
    } else {
        CCScheduler::get()->resumeTarget(this);
    }
}