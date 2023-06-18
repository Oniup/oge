#ifndef __OGE_UTILS_UTILS_HPP__
#define __OGE_UTILS_UTILS_HPP__

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <ogl/ogl.hpp>

namespace oge {

struct ImGuiHelper
{
    static ImVec2 calc_button_size(const std::string& label = "")
    {
        ImVec2 label_size = ImGui::CalcTextSize(label.c_str());

        return ImGui::CalcItemSize(
            ImVec2(ImGui::GetContentRegionAvail().x, 0.0f),
            label_size.x + ImGui::GetStyle().FramePadding.x * 2.0f,
            label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f
        );
    };
};

} // namespace oge

#endif
