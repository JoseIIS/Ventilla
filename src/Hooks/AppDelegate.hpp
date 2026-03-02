#ifdef GEODE_IS_DESKTOP

#include <Geode/modify/AppDelegate.hpp>

class $modify(ProAppDelegate, AppDelegate) {

    void applicationDidEnterBackground();

    void applicationWillEnterForeground();

};

#endif