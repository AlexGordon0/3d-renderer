#pragma once

#include "glm/detail/type_vec.hpp"
#include "glm/mat3x3.hpp"
#include "glm/detail/func_geometric.hpp"

struct Camera {
    glm::vec3 position;
    glm::mat3 orientation;
    float focalLength;
    bool orbit;
};
