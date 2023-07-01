#ifndef __KRYOS_ENGINE_GUI_APP_HPP__
#define __KRYOS_ENGINE_GUI_APP_HPP__

#include <kryos/utils/utils.hpp>
#include <kryos/core/application.hpp>

class KEditorApp : public KIApplication
{
  public:
    KEditorApp();
    virtual ~KEditorApp() override = default;
};

#endif
