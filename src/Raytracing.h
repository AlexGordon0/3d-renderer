#pragma once

#include "RayTriangleIntersection.h"
#include "Scene.h"
#include <vector>

RayTriangleIntersection getClosestValidIntersection(glm::vec3 position, glm::vec3 direction,
                                                    std::vector<std::vector<ModelTriangle>> models);
void calculateVertexBrightness(Scene &scene);
