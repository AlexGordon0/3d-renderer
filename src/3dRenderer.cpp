#include "glm/detail/type_vec.hpp"
#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>

#define WIDTH 320
#define HEIGHT 240

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

void draw(DrawingWindow &window) {
    window.clearPixels();

    /*
    glm::vec3 topLeft(255, 0, 0);
    glm::vec3 topRight(0, 0, 255);
    glm::vec3 bottomRight(0, 255, 0);
    glm::vec3 bottomLeft(255, 255, 0);

    std::vector<glm::vec3> leftEdge = interpolateThreeElementValues(topLeft, bottomLeft, window.height);
    std::vector<glm::vec3> rightEdge = interpolateThreeElementValues(topRight, bottomRight, window.height);
    */

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
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window.savePPM("output.ppm");
        window.saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
    SDL_Event event;

    // std::vector<glm::vec3> result;
    // result = interpolateThreeElementValues(glm::vec3(1.0, 4.0, 9.2), glm::vec3(4.0, 1.0, 9.8), 4);
    // for(size_t i=0; i<result.size(); i++) {
    // std::cout << "(" << result[i].x << ", " << result[i].y << ", " << result[i].z << ")" << std::endl;
    // }
    // std::cout << std::endl;

    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (window.pollForInputEvents(event))
            handleEvent(event, window);
        draw(window);
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        window.renderFrame();
    }
}
