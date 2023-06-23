#ifndef __KRYOS_ENGINE_CORE_APP_HPP__
#define __KRYOS_ENGINE_CORE_APP_HPP__

#include <kryos/kryos.hpp>

class App : public kryos::Application
{
  public:
    App();
    virtual ~App() override = default;
};

#endif
