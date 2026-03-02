#include "Includes.hpp"

#include "Other/RadioManager.hpp"

$on_mod(Loaded) {
    RadioManager::get().update();
}