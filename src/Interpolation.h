#pragma once

#include <vector>
#include "glm/detail/type_vec.hpp"
#include "glm/detail/func_geometric.hpp"

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues);
std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues);
