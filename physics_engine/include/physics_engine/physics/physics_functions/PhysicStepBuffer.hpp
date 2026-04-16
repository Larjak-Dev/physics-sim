#pragma once
#include "../../core/Units_basic.hpp"
#include <vector>

namespace phys
{

struct Derivates
{
    vec3d l{};
    vec3d k{};
};

// Buffer for memory during RK4 algorithm
struct StepBuffer
{
    std::vector<Derivates> der_1;
    std::vector<Derivates> der_2;
    std::vector<Derivates> der_3;
    std::vector<Derivates> der_4;
    std::size_t size{0};
    void buffer(std::size_t size);
};

} // namespace phys
