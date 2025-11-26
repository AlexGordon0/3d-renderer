#include "ModelReader.h"
#include "Utils.h"
#include "glm/detail/type_vec3.hpp"
#include "glm/gtx/string_cast.hpp"
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
    // Store which triangle indexes use that vertex index
    std::vector<std::vector<int>> trianglesWithVertex = {};

    std::string file_path = "resources/models/" + filename + ".obj";
    std::ifstream file(file_path);
    std::string line;
    Colour colour = Colour(255, 0, 0);
    while (std::getline(file, line)) {
        if (line[0] == 'u') {
            std::vector<std::string> tokens = split(line, ' ');
            colour = colour_map[tokens[1]];
        }
        if (line[0] == 'v') {
            std::vector<std::string> tokens = split(line, ' ');
            std::array<float, 3> vertex = {std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3])};
            trianglesWithVertex.push_back(std::vector<int>());
            vertices.push_back(glm::vec3(vertex[0] * scale, vertex[1] * scale, vertex[2] * scale));
        }
        if (line[0] == 'f') {
            std::vector<std::string> tokens = split(line.erase(0, 1), '/');
            std::array<int, 3> indexes = {std::stoi(tokens[0]), std::stoi(tokens[1]), std::stoi(tokens[2])};

            glm::vec3 v1 = vertices[indexes[0] - 1];
            glm::vec3 v2 = vertices[indexes[1] - 1];
            glm::vec3 v3 = vertices[indexes[2] - 1];

            trianglesWithVertex[indexes[0] - 1].push_back(triangles.size());
            trianglesWithVertex[indexes[1] - 1].push_back(triangles.size());
            trianglesWithVertex[indexes[2] - 1].push_back(triangles.size());

            glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
            ModelTriangle triangle = ModelTriangle(v1, v2, v3, colour);
            triangle.normal = normal;
            triangles.push_back(triangle);
        }
    }

    file.close();

    // Find vertex normals
    for (int vertexIndex = 0; vertexIndex < trianglesWithVertex.size(); vertexIndex++) {
        glm::vec3 averagedNormal = {0, 0, 0};
        for (int triangleIndex : trianglesWithVertex[vertexIndex]) {
            averagedNormal += triangles[triangleIndex].normal;
        }
        averagedNormal = glm::normalize(averagedNormal);
        for (int triangleIndex : trianglesWithVertex[vertexIndex]) {
            int index = -1;
            for (int i = 0; i < 3; i++) {
                if (glm::operator==(triangles[triangleIndex].vertices[i], vertices[vertexIndex])) {
                    index = i;
                }
            }
            if (index == -1) {
                std::cout << "NO MATCHING VERTEX" << std::endl;
            } else {
                triangles[triangleIndex].vertexNormals[index] = averagedNormal;
            }
        }
    }

    return triangles;
}
