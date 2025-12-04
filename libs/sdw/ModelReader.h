#pragma once

#include "Colour.h"
#include "ModelTriangle.h"
#include <map>
#include <string>
#include <vector>

std::map<std::string, Colour> readMtl(std::string filename);
std::vector<ModelTriangle> readObj(std::string filename, std::map<std::string, Colour> &colour_map, float scale = 1,
                                   glm::vec3 translation = glm::vec3(0, 0, 0));
