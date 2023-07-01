#include "gui/assets.hpp"

#include <imgui/imgui.h>

namespace workspace {

KAssets::KAssets() : KIWorkspace("Assets") {}

void KAssets::on_imgui_update()
{
    ImGui::Begin(get_name().c_str(), &get_enabled());
    ImGui::End();
}

} // namespace workspace
