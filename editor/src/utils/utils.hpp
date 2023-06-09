#ifndef __KRYOS_EDITOR_UTILS_UTILS_HPP__
#define __KRYOS_EDITOR_UTILS_UTILS_HPP__

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <string>

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

#endif
