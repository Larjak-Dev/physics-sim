#include "Error.hpp"
#include <cstdlib>
#include <format>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace phys;

void phys::showMessage(const char *msg)
{
    std::cout << msg << '\n';

#if defined(__linux__)
    auto cmd = std::format("notify-send -u critical \"PhysicsSim: {}\"", msg);
    std::system(cmd.c_str());

#elif defined(_WIN32)
    MessageBoxA(nullptr, msg, "PhysicsSim", MB_ICONERROR | MB_SYSTEMMODAL | MB_OK);

#elif defined(__APPLE__)
    auto cmd = std::format("osascript -e 'display dialog \"{}\" with title \"PhysicsSim\" buttons {{\"OK\"}} default "
                           "button \"OK\" with icon stop'",
                           msg);
    std::system(cmd.c_str());

#else
    std::cerr << "CRITICAL ALERT [PhysicsSim]: " << msg << '\n';
#endif
}
