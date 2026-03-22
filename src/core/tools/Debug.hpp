#pragma once
#include <format>

namespace phys
{

void showDebug(std::string str);

template <typename... Args> void showDebugF(std::format_string<Args...> msg, Args &&...args)
{
    auto msgFormat = std::format(msg, std::forward<Args>(args)...);
    showDebug(msgFormat);
}

void updateDebug();

} // namespace phys
