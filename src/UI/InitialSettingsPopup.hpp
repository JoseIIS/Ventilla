#include "../Includes.hpp"

class InitialSettingsPopup : public Popup {
  
private:

    bool init() override;

public:

    static InitialSettingsPopup* create();
    
};