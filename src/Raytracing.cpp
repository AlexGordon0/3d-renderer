#include "Raytracing.h"
#include "CanvasTriangle.h"
#include "Texturer.h"
#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/string_cast.hpp"
#include <limits>

RayTriangleIntersection getClosestValidIntersection(glm::vec3 position, glm::vec3 direction,
                                                    std::vector<std::vector<ModelTriangle>> models) {
    RayTriangleIntersection closestIntersection = RayTriangleIntersection({0, 0, 0}, -1, models[0][0], 0);
    closestIntersection.intersectedTriangle.colour = Colour(0, 0, 0);
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

    // Mirror triangles
    if (intersection.triangleIndex == 26 || intersection.triangleIndex == 31) {
        glm::vec3 reflectionVector = calculateReflection(incidentRay, normal);
        RayTriangleIntersection reflectedRayIntersection =
            getClosestValidIntersection(intersection.intersectionPoint, reflectionVector, scene.models);
        return getPixelColour(scene, reflectedRayIntersection, reflectionVector);
    }

    // Refraction
    if (!strcmp(intersection.intersectedTriangle.colour.name.c_str(), "Red")) {
        float sinCriticalAngle = 1.5; // n2 / n1
        glm::vec3 inverseRay = -incidentRay; // Ray from perspective of triangle
        glm::vec3 tempNormal = intersection.intersectedTriangle.normal;
        if (glm::dot(inverseRay, tempNormal) < 0) {
            // Light ray approaching from inside box
            tempNormal = -tempNormal;
            sinCriticalAngle = 1.0f / sinCriticalAngle;
        }
        float sinIncidence = glm::length(glm::cross(inverseRay, tempNormal));
        if (sinIncidence < sinCriticalAngle) {
            // Refract into box

            // Calculate sideways component of vector
            glm::vec3 t = glm::normalize((incidentRay - glm::dot(incidentRay, -tempNormal) * -tempNormal));
            int sign = glm::dot(incidentRay, -tempNormal) < 0 ? -1 : 1;
            float sinRefractedAngle = 1.0f / sinCriticalAngle * sinIncidence;
            if (sinRefractedAngle > 1) {
                std::cout << sinCriticalAngle << std::endl;
            }
            float c2 = sqrt(1 - sinRefractedAngle * sinRefractedAngle);

            glm::vec3 refractedRay = sinRefractedAngle * t + sign * c2 * -tempNormal;
            //std::cout << glm::to_string(incidentRay) << ", " << glm::to_string(refractedRay) << std::endl;

            RayTriangleIntersection refractedRayIntersection =
                getClosestValidIntersection(intersection.intersectionPoint, refractedRay, scene.models);
            return getPixelColour(scene, refractedRayIntersection, refractedRay);
        }

        if (sinIncidence >= sinCriticalAngle) {
            // Total internal reflection
            glm::vec3 reflectedRay = calculateReflection(incidentRay, tempNormal);
            RayTriangleIntersection reflectedRayIntersection =
                getClosestValidIntersection(intersection.intersectionPoint, reflectedRay, scene.models);
            return getPixelColour(scene, reflectedRayIntersection, reflectedRay);
        }
    }

    float totalBrightness = 0;

    // float lightWidth = 0.05f;
    // int numPoints = 3;
    for (glm::vec3 lightSource : scene.lightSources) {

        glm::vec3 triangleToLight = glm::normalize(lightSource - intersection.intersectionPoint);
        float distanceToLight = glm::length(lightSource - intersection.intersectionPoint);

        /*
        int lightVisible = 0;
        glm::vec3 lightAxis1 = glm::normalize(glm::cross(triangleToLight, {0, 1, 0}));
        glm::vec3 lightAxis2 = glm::normalize(glm::cross({0, 1, 0}, lightAxis1));
        for (float i = -lightWidth / 2; i <= lightWidth / 2; i += lightWidth / 2) {
            for (float j = -lightWidth / 2; j <= lightWidth / 2; j += lightWidth / 2) {
                glm::vec3 triangleToLightNode = lightAxis1 * i + lightAxis2 * j + triangleToLight;
                RayTriangleIntersection lightIntersection =
                    getClosestValidIntersection(intersection.intersectionPoint, triangleToLightNode, scene.models);
                float distanceToLight = glm::length(triangleToLightNode);
                if (lightIntersection.distanceFromCamera == -1 ||
                    lightIntersection.distanceFromCamera > distanceToLight) {
                    lightVisible++;
                }
            }
        }
        float shadowCoefficient = float(lightVisible) / (numPoints * numPoints);
        */
        float shadowCoefficient = 1;
        int size = scene.shadowMap.size();
        glm::vec3 lightToTriangle = -triangleToLight;
        float scale = (size / 2.0f) / -lightToTriangle.y;
        int xIndex = round(lightToTriangle.x * scale) + size / 2;
        int zIndex = round(lightToTriangle.z * scale) + size / 2;
        int softWidth = 10;

        if (xIndex < size - softWidth && zIndex < size - softWidth && xIndex >= softWidth && zIndex >= softWidth) {
            float numShadows = 0;
            for (int i = -softWidth; i <= softWidth; i++) {
                for (int j = -softWidth; j <= softWidth; j++) {
                    if (scene.shadowMap[xIndex + i][zIndex + j] < distanceToLight - 0.03) {
                        numShadows++;
                    }
                }
            }
            shadowCoefficient = 1 - (numShadows / ((softWidth * 2 + 1) * (softWidth * 2 + 1)));
            // std::cout << shadowCoefficient << std::endl;
        }

        // RayTriangleIntersection lightIntersection =
        // getClosestValidIntersection(intersection.intersectionPoint, triangleToLight, scene.models);

        float proximity = 1 / ((2 / 7.0f) * glm::pi<double>() * (glm::pow(distanceToLight, 2)));
        proximity = glm::clamp(proximity, 0.0f, 1.0f);

        // glm::vec3 normal = intersection.intersectedTriangle.normal;
        float diffuse = glm::dot(normal, triangleToLight);
        diffuse = glm::clamp(diffuse, 0.0f, 1.0f);

        glm::vec3 reflectionVector = calculateReflection(-triangleToLight, normal);
        float specular = glm::dot(-incidentRay, reflectionVector);
        specular = glm::clamp(specular, 0.0f, 1.0f);
        specular = glm::pow(specular, 64);

        float brightness = 0.3 * proximity + 0.6 * diffuse + specular;

        totalBrightness += brightness * shadowCoefficient;
    }

    /* Gouraud shading
    float brightness = triangle.vertexBrightness[0] * barycentric.z +
                       triangle.vertexBrightness[1] * barycentric.x +
                       triangle.vertexBrightness[2] * barycentric.y;
    */

    float minimumThreshold = 0.05f;
    totalBrightness = glm::clamp(totalBrightness, minimumThreshold, 1.0f);
    Colour colour = intersection.intersectedTriangle.colour;

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

// Populate shadow map for single light
void populateShadowMap(Scene &scene) {
    int size = scene.shadowMap.size();
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            glm::vec3 lightPosition = scene.lightSources[0];
            glm::vec3 rayDirection = glm::normalize(glm::vec3(i - size / 2, -size / 2, j - size / 2));
            RayTriangleIntersection intersection =
                getClosestValidIntersection(lightPosition, rayDirection, scene.models);
            if (intersection.distanceFromCamera != -1) {
                scene.shadowMap[i][j] = intersection.distanceFromCamera;
            } else {
                scene.shadowMap[i][j] = 1000000;
            }
        }
    }
}
