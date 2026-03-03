#pragma once
#include <format>

namespace phys
{

void showMessage(const char *msg);

template <typename... Args> void showMessageF(std::format_string<Args...> msg, Args &&...args)
{
    auto msgFormat = std::format(msg, std::forward<Args>(args)...);
    auto msgFull = std::format("PhysicsSim MSG: {}", msgFormat);
    showMessage(msgFull.c_str());
}

} // namespace phys
