#pragma once
#include <string>

namespace phys::tools
{

enum class Key
{
    Simulator,
};

const char *translate(Key key);
template <typename... Args> std::string getID(const char *id, const char *formatStr, Args &&...args);

} // namespace phys::tools
