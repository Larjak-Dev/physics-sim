#include "Debug.hpp"
#include <imgui.h>
#include <queue>

using namespace phys;

std::queue<std::string> debugQueue;

void phys::showDebug(std::string str)
{
    debugQueue.push(str);
}

void phys::updateDebug()
{
    while (debugQueue.size() > 0)
    {
        auto str = debugQueue.front();
        ImGui::Text(str.c_str());
        debugQueue.pop();
    }
}
