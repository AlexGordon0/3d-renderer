#include "glm/detail/type_vec.hpp"
#include <CanvasTriangle.h>
#include <Colour.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <vector>

#define WIDTH 960
#define HEIGHT 720

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
    float numSteps = std::max(std::abs(xDiff), std::abs(yDiff));
    float xStep = xDiff / numSteps;
    float yStep = yDiff / numSteps;

    for (int i = 0; i < numSteps; i++) {
        window.setPixelColour(round(from.x + xStep * i), round(from.y + yStep * i), colour.value());
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

    CanvasPoint topVertex = triangle.vertices[0];
    CanvasPoint middleVertex = triangle.vertices[1];
    CanvasPoint bottomVertex = triangle.vertices[2];
    std::vector<float> shortSide =
        interpolateSingleFloats(topVertex.x, middleVertex.x, middleVertex.y - topVertex.y + 1);
    std::vector<float> longSide =
        interpolateSingleFloats(topVertex.x, bottomVertex.x, bottomVertex.y - topVertex.y + 1);

    for (int index = 0, y = topVertex.y; y <= middleVertex.y; index++, y++) {
        drawLine(window, CanvasPoint(shortSide[index], y), CanvasPoint(longSide[index], y), colour);
    }

    shortSide = interpolateSingleFloats(middleVertex.x, bottomVertex.x, bottomVertex.y - middleVertex.y + 1);
    for (int index = 0, y = middleVertex.y; y < bottomVertex.y; index++, y++) {
        drawLine(window, CanvasPoint(shortSide[index], y),
                 CanvasPoint(longSide[index + middleVertex.y - topVertex.y], y), colour);
    }

    drawStrokedTriangle(window, triangle, Colour(255, 255, 255));
}

void draw(DrawingWindow &window) {
    /*
    window.clearPixels();
    CanvasTriangle triangle =
        CanvasTriangle(CanvasPoint(window.width / 2, 0), CanvasPoint(window.width - 1, window.height / 2),
                       CanvasPoint(0, window.height - 1));
    drawFilledTriangle(window, triangle, Colour(255, 0, 0));

    drawLine(window, CanvasPoint(0, 0), CanvasPoint(window.width / 2, window.height / 2), Colour(255, 255, 255));
    drawLine(window, CanvasPoint(window.width - 1, 0), CanvasPoint(window.width / 2, window.height / 2),
             Colour(255, 255, 255));
    drawLine(window, CanvasPoint(window.width / 2, 0), CanvasPoint(window.width / 2, window.height),
             Colour(255, 255, 255));
    drawLine(window, CanvasPoint(window.width / 3, window.height / 2),
             CanvasPoint(window.width * 2 / 3, window.height / 2), Colour(255, 255, 255));

    glm::vec3 topLeft(255, 0, 0);
    glm::vec3 topRight(0, 0, 255);
    glm::vec3 bottomRight(0, 255, 0);
    glm::vec3 bottomLeft(255, 255, 0);

    std::vector<glm::vec3> leftEdge = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
    std::vector<glm::vec3> rightEdge = interpolateThreeElementValues(topRight, bottomRight, window.height);

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
        if (event.key.keysym.sym == SDLK_LEFT)
            std::cout << "LEFT" << std::endl;
        else if (event.key.keysym.sym == SDLK_RIGHT)
            std::cout << "RIGHT" << std::endl;
        else if (event.key.keysym.sym == SDLK_UP)
            std::cout << "UP" << std::endl;
        else if (event.key.keysym.sym == SDLK_DOWN)
            std::cout << "DOWN" << std::endl;
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
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;

    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event))
            handleEvent(event, window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
