#pragma once

#include "RayTriangleIntersection.h"
#include "Scene.h"
#include "glm/detail/type_vec.hpp"
#include <vector>

RayTriangleIntersection getClosestValidIntersection(glm::vec3 position, glm::vec3 direction,
                                                    std::vector<std::vector<ModelTriangle>> models);
void calculateVertexBrightness(Scene &scene);
glm::vec3 calculateReflection(glm::vec3 incidentRay, glm::vec3 normal);
uint32_t getPixelColour(Scene &scene, RayTriangleIntersection intersection, glm::vec3 incidentRay);
