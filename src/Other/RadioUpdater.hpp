#include "../Includes.hpp"

class RadioUpdater : public CCObject {

public:

    float m_connectionCheckCooldown = 0.f;

    RadioUpdater();

    void update(float);

    void schedule();
    
    void setPaused(bool);

};