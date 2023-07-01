#ifndef __KRYOS_EDITOR_GUI_ASSETS_HPP__
#define __KRYOS_EDITOR_GUI_ASSETS_HPP__

#include "gui/editor.hpp"

namespace workspace {

class KAssets final : public KIWorkspace
{
  public:
    KAssets();
    virtual ~KAssets() override = default;

    virtual void on_imgui_update() override;
};

} // namespace workspace

#endif
