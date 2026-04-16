#pragma once
#include "../../core/Units.hpp"
#include "imgui.h"
#include <glm/common.hpp>
#include <vector>

namespace phys::app
{

inline Color hueToRGB(float f)
{
    vec3f rgb = glm::clamp(glm::abs(glm::mod(f * 6.0f + vec3f(0.0f, 4.0f, 2.0), 6.0f) - 3.0f) - 1.0f, 0.0f, 1.0f);
    return Color(rgb.r, rgb.g, rgb.b, 1.0f);
}

template <typename T>
bool EnumCombo(const char *label, T &value, const std::vector<std::pair<T, const char *>> &options)
{
    bool changed = false;

    const char *preview = "Unkown";
    for (const auto &[val, name] : options)
    {
        if (val == value)
        {
            preview = name;
            break;
        }
    }

    if (ImGui::BeginCombo(label, preview))
    {
        for (const auto &[val, name] : options)
        {
            const bool is_selected = (value == val);
            if (ImGui::Selectable(name, is_selected))
            {
                value = val;
                changed = true;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

inline void drawTableLabel(const char *label)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(label);
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-1);
}

inline void drawTableInputD(const char *label, double *value, ImGuiInputTextFlags flags = 0,
                            const char *format = "%.2e")
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(label);
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputDouble("##x", value, 0.0, 0.0, format, flags);
}

inline bool CheckboxInverted(const char *label, bool *v)
{
    bool temp = !(*v);
    bool pressed = ImGui::Checkbox(label, &temp);
    if (pressed)
        *v = !temp;
    return pressed;
}

} // namespace phys::app
