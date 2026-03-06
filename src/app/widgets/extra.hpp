#pragma once
#include "imgui.h"
#include <vector>

namespace phys::app
{
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
} // namespace phys::app
