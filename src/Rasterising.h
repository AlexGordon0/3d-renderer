#pragma once

#include "CanvasPoint.h"
#include "Scene.h"
#include "glm/detail/type_vec.hpp"
#include "glm/detail/func_geometric.hpp"

CanvasPoint projectVertexOntoCanvasPoint(Scene &scene, glm::vec3 vertexPosition);
void drawModel(Scene &scene);
