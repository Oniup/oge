#ifndef __KRYOS_ENGINE_GUI_PREFERENCES_HPP__
#define __KRYOS_ENGINE_GUI_PREFERENCES_HPP__

#include "utils/utils.hpp"
#include <kryos/kryos.hpp>
#include <portable-file-dialogs/portable-file-dialogs.h>
#include <yaml/yaml.hpp>

// TODO: Implement Preferences Editor window
class Preferences
{
  public:
    inline static const std::string get_preferences_path()
    {
#ifndef _WIN32
        return pfd::path::home() + "/.config/kryos/preferences.yaml";
#else
        // TODO: ...
#endif
    }

    inline static yaml::Node get_preferences() { return yaml::open(get_preferences_path()); }

  public:
    Preferences();
};

#endif
