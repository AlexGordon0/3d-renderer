#include "CanvasTriangle.h"
#include "Colour.h"
#include "DrawingWindow.h"
#include "ModelReader.h"
#include "RayTriangleIntersection.h"
#include "TextureMap.h"
#include "Utils.h"
#include "glm/detail/func_exponential.hpp"
#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_mat.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/string_cast.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

#define WIDTH 960
#define HEIGHT 720

struct Camera {
    glm::vec3 position;
    glm::mat3 orientation;
    float focalLength;
    bool orbit;
};

Camera camera;
double cameraAngle = 0;

int mode;
glm::vec3 lightSource = {0, 0.9, 0};

std::vector<ModelTriangle> cornellBox;
const float scale = 160.0;
std::array<std::array<float, WIDTH>, HEIGHT> zDepth = {{{0}}};

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
    std::vector<float> interpolated = {};
    float step = (to - from) / (numberOfValues - 1);
    for (int i = 0; i < numberOfValues; i++) {
        interpolated.push_back(from + step * i);
    }
    return interpolated;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
    std::vector<glm::vec3> interpolated = {};

    float step1 = (to.x - from.x) / (numberOfValues - 1);
    float step2 = (to.y - from.y) / (numberOfValues - 1);
    float step3 = (to.z - from.z) / (numberOfValues - 1);
    glm::vec3 steps = glm::vec3(step1, step2, step3);

    for (float i = 0; i < numberOfValues; i++) {
        interpolated.push_back(glm::vec3(from + steps * i));
    }
    return interpolated;
}

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) {
    float xDiff = to.x - from.x;
    float yDiff = to.y - from.y;
    float zDiff = to.depth - from.depth;
    float numSteps = std::max(std::abs(xDiff), std::abs(yDiff));
    float xStep = xDiff / numSteps;
    float yStep = yDiff / numSteps;
    float zStep = zDiff / numSteps;

    for (int i = 0; i < numSteps; i++) {
        int x = round(from.x + xStep * i);
        int y = round(from.y + yStep * i);
        float z = from.depth + zStep * i;
        if (x < 0 || x >= window.width || y < 0 || y >= window.height)
            continue;
        if (-z >= zDepth[y][x]) {
            window.setPixelColour(x, y, colour.value());
            zDepth[y][x] = -z;
        }
    }
}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
    drawLine(window, triangle.v0(), triangle.v1(), colour);
    drawLine(window, triangle.v1(), triangle.v2(), colour);
    drawLine(window, triangle.v2(), triangle.v0(), colour);
}

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
    std::sort(triangle.vertices.begin(), triangle.vertices.end(),
              [](CanvasPoint a, CanvasPoint b) { return a.y < b.y; });

    CanvasPoint topVertex =
        CanvasPoint(round(triangle.vertices[0].x), round(triangle.vertices[0].y), triangle.vertices[0].depth);
    CanvasPoint middleVertex =
        CanvasPoint(round(triangle.vertices[1].x), round(triangle.vertices[1].y), triangle.vertices[1].depth);
    CanvasPoint bottomVertex =
        CanvasPoint(round(triangle.vertices[2].x), round(triangle.vertices[2].y), triangle.vertices[2].depth);

    std::vector<float> shortSideX =
        interpolateSingleFloats(topVertex.x, middleVertex.x, middleVertex.y - topVertex.y + 1);
    std::vector<float> shortSideZ =
        interpolateSingleFloats(topVertex.depth, middleVertex.depth, middleVertex.y - topVertex.y + 1);

    std::vector<float> longSideX =
        interpolateSingleFloats(topVertex.x, bottomVertex.x, bottomVertex.y - topVertex.y + 1);
    std::vector<float> longSideZ =
        interpolateSingleFloats(topVertex.depth, bottomVertex.depth, bottomVertex.y - topVertex.y + 1);

    // Upper triangle
    for (int index = 0, y = topVertex.y; y < middleVertex.y; index++, y++) {
        drawLine(window, CanvasPoint(shortSideX[index], y, shortSideZ[index]),
                 CanvasPoint(longSideX[index], y, longSideZ[index]), colour);
    }

    // Lower triangle
    shortSideX = interpolateSingleFloats(middleVertex.x, bottomVertex.x, bottomVertex.y - middleVertex.y + 1);
    shortSideZ = interpolateSingleFloats(middleVertex.depth, bottomVertex.depth, bottomVertex.y - middleVertex.y + 1);
    for (int index = 0, y = middleVertex.y; y < bottomVertex.y; index++, y++) {
        drawLine(window, CanvasPoint(shortSideX[index], y, shortSideZ[index]),
                 CanvasPoint(longSideX[index + middleVertex.y - topVertex.y], y,
                             longSideZ[index + middleVertex.y - topVertex.y]),
                 colour);
    }

    // Outline triangle
    // drawStrokedTriangle(window, triangle, Colour(255, 255, 255));
}

uint32_t getTexturePixelValue(DrawingWindow &window, glm::vec3 relativeCoordinates, CanvasTriangle textureTriangle,
                              TextureMap &texture) {
    CanvasPoint topVertex = textureTriangle.vertices[0];
    CanvasPoint middleVertex = textureTriangle.vertices[1];
    CanvasPoint bottomVertex = textureTriangle.vertices[2];
    TexturePoint texturePixel =
        TexturePoint(middleVertex.x * relativeCoordinates.x + bottomVertex.x * relativeCoordinates.y +
                         topVertex.x * relativeCoordinates.z,
                     middleVertex.y * relativeCoordinates.x + bottomVertex.y * relativeCoordinates.y +
                         topVertex.y * relativeCoordinates.z);
    return texture.pixels[round(texturePixel.y) * texture.width + round(texturePixel.x)];
}

void drawTexturedTriangle(DrawingWindow &window, CanvasTriangle triangle, CanvasTriangle textureTriangle,
                          TextureMap &texture) {
    std::sort(triangle.vertices.begin(), triangle.vertices.end(),
              [](CanvasPoint a, CanvasPoint b) { return a.y < b.y; });
    std::sort(textureTriangle.vertices.begin(), textureTriangle.vertices.end(),
              [](CanvasPoint a, CanvasPoint b) { return a.y < b.y; });

    CanvasPoint topVertex = triangle.vertices[0];
    CanvasPoint middleVertex = triangle.vertices[1];
    CanvasPoint bottomVertex = triangle.vertices[2];
    std::vector<float> shortSideX =
        interpolateSingleFloats(topVertex.x, middleVertex.x, middleVertex.y - topVertex.y + 1);
    std::vector<float> longSideX =
        interpolateSingleFloats(topVertex.x, bottomVertex.x, bottomVertex.y - topVertex.y + 1);

    for (int index = 0, y = topVertex.y; y < middleVertex.y; index++, y++) {
        int start = std::min(shortSideX[index], longSideX[index]);
        int limit = std::max(shortSideX[index], longSideX[index]);
        for (int x = start; x <= limit; x++) {
            glm::vec3 relativeCoordinates = convertToBarycentricCoordinates(
                glm::vec2(topVertex.x, topVertex.y), glm::vec2(middleVertex.x, middleVertex.y),
                glm::vec2(bottomVertex.x, bottomVertex.y), glm::vec2(x, y));

            uint32_t pixelValue = getTexturePixelValue(window, relativeCoordinates, textureTriangle, texture);
            window.setPixelColour(x, y, pixelValue);
        }
    }

    shortSideX = interpolateSingleFloats(middleVertex.x, bottomVertex.x, bottomVertex.y - middleVertex.y + 1);
    for (int index = 0, y = middleVertex.y; y < bottomVertex.y; index++, y++) {
        int start = std::min(shortSideX[index], longSideX[index + middleVertex.y - topVertex.y]);
        int limit = std::max(shortSideX[index], longSideX[index + middleVertex.y - topVertex.y]);
        for (int x = start; x <= limit; x++) {
            glm::vec3 relativeCoordinates = convertToBarycentricCoordinates(
                glm::vec2(topVertex.x, topVertex.y), glm::vec2(middleVertex.x, middleVertex.y),
                glm::vec2(bottomVertex.x, bottomVertex.y), glm::vec2(x, y));

            uint32_t pixelValue = getTexturePixelValue(window, relativeCoordinates, textureTriangle, texture);
            window.setPixelColour(x, y, pixelValue);
        }
    }

    drawStrokedTriangle(window, triangle, Colour(255, 255, 255));
}

CanvasPoint projectVertexOntoCanvasPoint(glm::vec3 cameraPosition, float focalLength, glm::vec3 vertexPosition) {
    glm::vec3 cameraToVertex = vertexPosition - cameraPosition;
    glm::vec3 transposedPosition = cameraToVertex * camera.orientation;
    float x = focalLength * transposedPosition.x / transposedPosition.z;
    float y = focalLength * transposedPosition.y / transposedPosition.z;

    x *= scale;
    y *= scale;

    return CanvasPoint(-x + WIDTH / 2.0, y + HEIGHT / 2.0, 1.0 / transposedPosition.z);
}

void drawModel(DrawingWindow &window, std::vector<ModelTriangle> &model) {
    for (ModelTriangle modelTriangle : model) {
        CanvasTriangle triangle = CanvasTriangle(
            projectVertexOntoCanvasPoint(camera.position, camera.focalLength, modelTriangle.vertices[0]),
            projectVertexOntoCanvasPoint(camera.position, camera.focalLength, modelTriangle.vertices[1]),
            projectVertexOntoCanvasPoint(camera.position, camera.focalLength, modelTriangle.vertices[2]));
        if (mode == 0) {
            drawStrokedTriangle(window, triangle, Colour(255, 255, 255)); // wireframe
        } else if (mode == 1) {
            drawFilledTriangle(window, triangle, modelTriangle.colour);
        }
    }
}

void lookAt(glm::vec3 focusPoint) {
    glm::vec3 forwardVector = glm::normalize(camera.position - focusPoint);
    glm::vec3 rightVector = glm::cross({0, 1, 0}, forwardVector);
    glm::vec3 upVector = glm::cross(forwardVector, rightVector);
    camera.orientation = {rightVector, upVector, forwardVector};
}

RayTriangleIntersection getClosestValidIntersection(glm::vec3 position, glm::vec3 direction) {
    RayTriangleIntersection closestIntersection = RayTriangleIntersection({0, 0, 0}, -1, cornellBox[0], 0);
    for (int i = 0; i < cornellBox.size(); i++) {
        ModelTriangle triangle = cornellBox[i];
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

void drawRaytracedScene(DrawingWindow &window) {
    window.clearPixels();
    for (int y = 0; y < window.height; y++) {
        for (int x = 0; x < window.width; x++) {

            glm::vec3 endPoint = {x - window.width / 2.0, -(y - window.height / 2.0), -camera.focalLength * scale};
            glm::vec3 cameraToTriangle = camera.orientation * glm::normalize(endPoint - camera.position);
            RayTriangleIntersection intersection = getClosestValidIntersection(camera.position, cameraToTriangle);

            if (intersection.distanceFromCamera != -1) {

                glm::vec3 triangleToLight = glm::normalize(lightSource - intersection.intersectionPoint);
                float distanceToLight = length(lightSource - intersection.intersectionPoint);
                RayTriangleIntersection lightIntersection =
                    getClosestValidIntersection(intersection.intersectionPoint, triangleToLight);

                float ambient = 0.1;

                float proximity = 1 / ((2 / 7.0f) * glm::pi<double>() * (glm::pow(distanceToLight, 2)));
                proximity = glm::clamp(proximity, 0.0f, 1.0f);

                glm::vec3 normal = intersection.intersectedTriangle.normal;
                float diffuse = glm::dot(normal, triangleToLight);
                diffuse = glm::clamp(diffuse, 0.0f, 1.0f);

                glm::vec3 reflectionVector =
                    glm::normalize(-triangleToLight - 2.0f * normal * glm::dot(-triangleToLight, normal));
                float specular = glm::dot(-cameraToTriangle, reflectionVector);
                specular = glm::clamp(specular, 0.0f, 1.0f);
                specular = glm::pow(specular, 256);

                float brightness = 0.3 * proximity + 0.6 * diffuse + 0.1 * specular;
                brightness = glm::clamp(brightness, 0.0f, 1.0f);

                // Shadow implemetation
                //if (lightIntersection.distanceFromCamera == -1 ||
                    //lightIntersection.distanceFromCamera > distanceToLight) {
                    window.setPixelColour(x, y, intersection.intersectedTriangle.colour.value(brightness));
                //} else {
                    //window.setPixelColour(x, y, intersection.intersectedTriangle.colour.value(0.1));
                //}
            }
        }
    }
}

void drawRasterisedScene(DrawingWindow &window) {
    window.clearPixels();
    zDepth = {{{0}}};
    drawModel(window, cornellBox);

    if (camera.orbit) {
        const double turnSpeed = glm::pi<double>() / 240;
        cameraAngle += turnSpeed;
        camera.position = {4 * sin(cameraAngle), 0, 4 * cos(cameraAngle)};
        lookAt({0, 0, 0});
    }

    /*
    CanvasTriangle triangle = CanvasTriangle(CanvasPoint(320, 20), CanvasPoint(600, 460), CanvasPoint(20, 300));
    CanvasTriangle textureTriangle = CanvasTriangle(CanvasPoint(195, 5), CanvasPoint(395, 380), CanvasPoint(65,
    330)); TextureMap texture = TextureMap("resources/texture.ppm"); drawTexturedTriangle(window, triangle,
    textureTriangle, texture);

    for (size_t y = 0; y < window.height; y++) {
        for (size_t x = 0; x < window.width; x++) {
            glm::vec3 pixelValues =
                convertToBarycentricCoordinates(glm::vec2(0, window.height), glm::vec2(window.width / 2, 0),
                                                glm::vec2(window.width, window.height), glm::vec2(x, y));
            if (pixelValues.x < 0 || pixelValues.y < 0 || pixelValues.z < 0)
                continue;
            float red = pixelValues.z * 255;
            float green = pixelValues.x * 255;
            float blue = pixelValues.y * 255;
            uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
            window.setPixelColour(x, y, colour);
        }
    }
    */
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
    if (event.type == SDL_KEYDOWN) {

        const double angle = glm::pi<double>() / 8;
        const float moveDistance = 0.5;

        if (event.key.keysym.sym == SDLK_LEFT) {
            glm::mat3 rotationMatrix = {{cos(angle), 0, -sin(angle)}, {0, 1, 0}, {sin(angle), 0, cos(angle)}};
            camera.orientation = camera.orientation * rotationMatrix;
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
            glm::mat3 rotationMatrix = {{cos(angle), 0, sin(angle)}, {0, 1, 0}, {-sin(angle), 0, cos(angle)}};
            camera.orientation = camera.orientation * rotationMatrix;
        } else if (event.key.keysym.sym == SDLK_UP) {
            glm::mat3 rotationMatrix = {{1, 0, 0}, {0, cos(angle), sin(angle)}, {0, -sin(angle), cos(angle)}};
            camera.orientation = camera.orientation * rotationMatrix;
        } else if (event.key.keysym.sym == SDLK_DOWN) {
            glm::mat3 rotationMatrix = {{1, 0, 0}, {0, cos(angle), -sin(angle)}, {0, sin(angle), cos(angle)}};
            camera.orientation = camera.orientation * rotationMatrix;
        }

        else if (event.key.keysym.sym == SDLK_w) {
            camera.position += camera.orientation * glm::vec3(0, 0, -moveDistance);
        } else if (event.key.keysym.sym == SDLK_a) {
            camera.position += camera.orientation * glm::vec3(-moveDistance, 0, 0);
        } else if (event.key.keysym.sym == SDLK_s) {
            camera.position += camera.orientation * glm::vec3(0, 0, moveDistance);
        } else if (event.key.keysym.sym == SDLK_d) {
            camera.position += camera.orientation * glm::vec3(moveDistance, 0, 0);
        } else if (event.key.keysym.sym == SDLK_c) {
            camera.position += camera.orientation * glm::vec3(0, moveDistance, 0);
        } else if (event.key.keysym.sym == SDLK_x) {
            camera.position += camera.orientation * glm::vec3(0, -moveDistance, 0);
        } else if (event.key.keysym.sym == SDLK_o) {
            camera.orbit = !camera.orbit;
        }

        else if (event.key.keysym.sym == SDLK_u) {
            CanvasPoint v0 = CanvasPoint(rand() % window.width, rand() % window.height);
            CanvasPoint v1 = CanvasPoint(rand() % window.width, rand() % window.height);
            CanvasPoint v2 = CanvasPoint(rand() % window.width, rand() % window.height);
            CanvasTriangle triangle = CanvasTriangle(v0, v1, v2);
            Colour colour = Colour(rand() % 256, rand() % 256, rand() % 256);
            drawStrokedTriangle(window, triangle, colour);
        } else if (event.key.keysym.sym == SDLK_f) {
            CanvasPoint v0 = CanvasPoint(rand() % window.width, rand() % window.height);
            CanvasPoint v1 = CanvasPoint(rand() % window.width, rand() % window.height);
            CanvasPoint v2 = CanvasPoint(rand() % window.width, rand() % window.height);
            CanvasTriangle triangle = CanvasTriangle(v0, v1, v2);
            Colour colour = Colour(rand() % 256, rand() % 256, rand() % 256);
            drawFilledTriangle(window, triangle, colour);
        }

        else if (event.key.keysym.sym == SDLK_m) {
            mode = (mode + 1) % 3;
        }

        else if (event.key.keysym.sym == SDLK_r) {
            // Reset camera position
            camera.position = {0, 0, 4};
            camera.orientation = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
        }

        else if (event.key.keysym.sym == SDLK_l) {
            // implement light movement
        }
    }

    else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

    camera.position = {0.0, 0.0, 4.0};
    camera.orientation = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    camera.focalLength = 2.0;
    camera.orbit = false;

    mode = 1;

    std::map<std::string, Colour> colourMap = readMtl("cornell-box");
    cornellBox = readObj("cornell-box", colourMap, 0.35);

    // RayTriangleIntersection intersection = getClosestValidIntersection(camera.position, {0, 0, -1});
    // if (intersection.distanceFromCamera != -1) {
    // std::cout << "FOUND" << std::endl;
    //}

    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event))
            handleEvent(event, window);
        if (mode == 0 || mode == 1) {
            drawRasterisedScene(window);
        } else {
            drawRaytracedScene(window);
        }
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
