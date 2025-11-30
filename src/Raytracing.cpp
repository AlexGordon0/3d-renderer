#include "Raytracing.h"
#include "CanvasTriangle.h"
#include "Texturer.h"
#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtc/constants.hpp"

RayTriangleIntersection getClosestValidIntersection(glm::vec3 position, glm::vec3 direction,
                                                    std::vector<std::vector<ModelTriangle>> models) {
    RayTriangleIntersection closestIntersection = RayTriangleIntersection({0, 0, 0}, -1, models[0][0], 0);
    for (std::vector<ModelTriangle> model : models) {
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
    }
    return closestIntersection;
}

void calculateVertexBrightness(Scene &scene) {
    for (std::vector<ModelTriangle> &model : scene.models) {
        for (ModelTriangle &triangle : model) {
            for (int j = 0; j < 3; j++) {

                float totalBrightness = 0;

                for (glm::vec3 lightSource : scene.lightSources) {

                    glm::vec3 vertexToLight = glm::normalize(lightSource - triangle.vertices[j]);
                    float distanceToLight = length(lightSource - triangle.vertices[j]);

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
                    totalBrightness += brightness;
                }

                float minimumThreshold = 0.1f;
                totalBrightness = glm::clamp(totalBrightness, minimumThreshold, 1.0f);
                triangle.vertexBrightness[j] = totalBrightness;
            }
        }
    }
}

glm::vec3 calculateReflection(glm::vec3 incidentRay, glm::vec3 normal) {
    return glm::normalize(incidentRay - 2.0f * normal * glm::dot(incidentRay, normal));
}

uint32_t getPixelColour(Scene &scene, RayTriangleIntersection intersection, glm::vec3 incidentRay) {
    ModelTriangle triangle = intersection.intersectedTriangle;
    glm::vec3 barycentric = convertToBarycentricCoordinates(triangle.vertices[0], triangle.vertices[1],
                                                            triangle.vertices[2], intersection.intersectionPoint);

    glm::vec3 normal = triangle.vertexNormals[0] * barycentric.z + triangle.vertexNormals[1] * barycentric.x +
                       triangle.vertexNormals[2] * barycentric.y;
    normal = glm::normalize(normal);

    float totalBrightness = 0;

    for (glm::vec3 lightSource : scene.lightSources) {
        glm::vec3 triangleToLight = glm::normalize(lightSource - intersection.intersectionPoint);
        RayTriangleIntersection lightIntersection =
            getClosestValidIntersection(intersection.intersectionPoint, triangleToLight, scene.models);
        float distanceToLight = length(lightSource - intersection.intersectionPoint);

        float proximity = 1 / ((2 / 7.0f) * glm::pi<double>() * (glm::pow(distanceToLight, 2)));
        proximity = glm::clamp(proximity, 0.0f, 1.0f);

        // glm::vec3 normal = intersection.intersectedTriangle.normal;
        float diffuse = glm::dot(normal, triangleToLight);
        diffuse = glm::clamp(diffuse, 0.0f, 1.0f);

        glm::vec3 reflectionVector = calculateReflection(-triangleToLight, normal);
        float specular = glm::dot(-incidentRay, reflectionVector);
        specular = glm::clamp(specular, 0.0f, 1.0f);
        specular = glm::pow(specular, 16);

        float brightness = 0.3 * proximity + 0.6 * diffuse + specular;

        if (lightIntersection.distanceFromCamera == -1 || lightIntersection.distanceFromCamera > distanceToLight) {
            totalBrightness += brightness;
        }
    }

    /* Gouraud shading
    float brightness = triangle.vertexBrightness[0] * barycentric.z +
                       triangle.vertexBrightness[1] * barycentric.x +
                       triangle.vertexBrightness[2] * barycentric.y;
    */

    float minimumThreshold = 0.1f;
    totalBrightness = glm::clamp(totalBrightness, minimumThreshold, 1.0f);
    Colour colour = intersection.intersectedTriangle.colour;

    // Mirror triangles
    if (intersection.triangleIndex == 10 || intersection.triangleIndex == 11) {
        glm::vec3 reflectionVector = calculateReflection(incidentRay, normal);
        RayTriangleIntersection reflectedRayIntersection =
            getClosestValidIntersection(intersection.intersectionPoint, reflectionVector, scene.models);
        if (reflectedRayIntersection.distanceFromCamera != -1) {
            return getPixelColour(scene, reflectedRayIntersection, reflectionVector);
        }
    }

    // Floor texture mapping
    if (intersection.triangleIndex == 6) {
        CanvasTriangle textureTriangle = CanvasTriangle(CanvasPoint(scene.texture.width, scene.texture.height),
                                                        CanvasPoint(0, 0), CanvasPoint(0, scene.texture.height));
        colour = getTexturePixelValue(barycentric, textureTriangle, scene.texture);
    }
    if (intersection.triangleIndex == 7) {
        CanvasTriangle textureTriangle = CanvasTriangle(CanvasPoint(scene.texture.width, scene.texture.height),
                                                        CanvasPoint(scene.texture.width, 0), CanvasPoint(0, 0));
        colour = getTexturePixelValue(barycentric, textureTriangle, scene.texture);
    }

    return colour.value(totalBrightness);
}
