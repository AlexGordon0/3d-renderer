#pragma once

#include "Colour.h"
#include "ModelTriangle.h"
#include <string>
#include <vector>
#include <map>

std::map<std::string, Colour> readMtl(std::string filename);
std::vector<ModelTriangle> readObj(std::string filename, std::map<std::string, Colour> &colour_map, float scale = 1);
