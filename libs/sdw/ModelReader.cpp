#include "ModelReader.h"
#include "Utils.h"
#include <fstream>

std::map<std::string, Colour> readMtl(std::string filename) {
    std::map<std::string, Colour> colour_map;

    std::string file_path = "resources/models/" + filename + ".mtl";
    std::ifstream file(file_path);
    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> tokens = split(line, ' ');
        if (tokens[0] == "newmtl") {
            std::getline(file, line);
            std::vector<std::string> colour_tokens = split(line, ' ');
            colour_map.insert(
                {tokens[1], Colour(tokens[1], std::stof(colour_tokens[1]) * 255, std::stof(colour_tokens[2]) * 255,
                                   std::stof(colour_tokens[3]) * 255)});
        }
    }
    return colour_map;
}

std::vector<ModelTriangle> readObj(std::string filename, std::map<std::string, Colour> &colour_map, float scale) {
    std::vector<ModelTriangle> triangles = {};
    std::vector<glm::vec3> vertices = {};

    std::string file_path = "resources/models/" + filename + ".obj";
    std::ifstream file(file_path);
    std::string line;
    Colour colour;
    while (std::getline(file, line)) {
        if (line[0] == 'u') {
            std::vector<std::string> tokens = split(line, ' ');
            colour = colour_map[tokens[1]];
        }
        if (line[0] == 'v') {
            std::vector<std::string> tokens = split(line, ' ');
            std::array<float, 3> vertex = {std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3])};
            vertices.push_back(glm::vec3(vertex[0] * scale, vertex[1] * scale, vertex[2] * scale));
        }
        if (line[0] == 'f') {
            std::vector<std::string> tokens = split(line.erase(0, 1), '/');
            std::array<int, 3> indexes = {std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2])};
            triangles.push_back(ModelTriangle(vertices[indexes[0] - 1], vertices[indexes[1] - 1],
                                              vertices[indexes[2] - 1], colour));
        }
    }

    file.close();
    return triangles;
}

