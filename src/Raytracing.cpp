#include "Raytracing.h"
#include "glm/gtc/constants.hpp"

RayTriangleIntersection getClosestValidIntersection(glm::vec3 position, glm::vec3 direction,
                                                    std::vector<ModelTriangle> model) {
    RayTriangleIntersection closestIntersection = RayTriangleIntersection({0, 0, 0}, -1, model[0], 0);
    for (int i = 0; i < model.size(); i++) {
        ModelTriangle triangle = model[i];
        glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
        glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
        glm::vec3 SPVector = position - triangle.vertices[0];
        glm::mat3 DEMatrix(-direction, e0, e1);
        glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;

        float t = possibleSolution.x;
        float u = possibleSolution.y;
        float v = possibleSolution.z;

        if (u < 0 || u > 1)
            continue;
        if (v < 0 || v > 1)
            continue;
        if (u + v > 1)
            continue;
        // Strange NaN check for when DEMatrix is singular
        if (t < 0.0001 || t != t)
            continue;

        glm::vec3 r = t * direction;

        RayTriangleIntersection intersection = RayTriangleIntersection(r + position, t, triangle, i);
        if (closestIntersection.distanceFromCamera == -1 ||
            intersection.distanceFromCamera < closestIntersection.distanceFromCamera) {
            closestIntersection = intersection;
        }
    }
    return closestIntersection;
}

void calculateVertexBrightness(Scene &scene) {
    for (ModelTriangle &triangle : scene.model) {
        for (int j = 0; j < 3; j++) {

            glm::vec3 vertexToLight = glm::normalize(scene.lightSource - triangle.vertices[j]);
            float distanceToLight = length(scene.lightSource - triangle.vertices[j]);

            float proximity = 1 / ((2 / 7.0f) * glm::pi<double>() * (glm::pow(distanceToLight, 2)));
            proximity = glm::clamp(proximity, 0.0f, 1.0f);

            glm::vec3 vertexNormal = triangle.vertexNormals[j];
            float diffuse = glm::dot(vertexNormal, vertexToLight);
            diffuse = glm::clamp(diffuse, 0.0f, 1.0f);

            glm::vec3 cameraToVertex =
                scene.camera.orientation * glm::normalize(triangle.vertices[j] - scene.camera.position);
            glm::vec3 reflectionVector =
                glm::normalize(-vertexToLight - 2.0f * vertexNormal * glm::dot(-vertexToLight, vertexNormal));
            float specular = glm::dot(-cameraToVertex, reflectionVector);
            specular = glm::clamp(specular, 0.0f, 1.0f);
            specular = glm::pow(specular, 1);

            float brightness = 0.3 * proximity + 0.6 * diffuse + 1 * specular;
            float minimumThreshold = 0.1f;
            brightness = glm::clamp(brightness, minimumThreshold, 1.0f);
            triangle.vertexBrightness[j] = brightness;
        }
    }
}
