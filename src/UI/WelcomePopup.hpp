#include "../Includes.hpp"

class WelcomePopup : public Popup {
  
private:

    bool init() override;

    void onClose(CCObject*) override;

public:

    static WelcomePopup* create();
    
};