#pragma once
#include "physics_functions/PhysicFunctions.hpp"

namespace phys
{

struct Quad
{
    vec3d center_point{0, 0, 0};
    double mass{0};
    std::size_t child_1{0};
    std::size_t child_2{0};
    std::size_t child_3{0};
    std::size_t child_4{0};
};

// Barnes Hut Algorithm (Not reccomended for accurate simulations)
class QuadTree
{
  public:
    QuadTree(EnvironmentBase &env, int pool_size);

    void calcCorners(EnvironmentBase &env);
    void addBody(Body body);

  private:
    vec3d x_corner;
    vec3d y_corner;
    vec3d z_corner;
    std::vector<Quad> pool;
};

} // namespace phys
