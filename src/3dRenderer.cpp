#include "Camera.h"
#include "CanvasTriangle.h"
#include "Colour.h"
#include "DrawingWindow.h"
#include "ModelReader.h"
#include "Rasterising.h"
#include "Raytracing.h"
#include "Scene.h"
#include "Shapes.h"
#include "glm/detail/func_geometric.hpp"
#include "glm/detail/type_mat.hpp"
#include "glm/detail/type_vec.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtx/string_cast.hpp"
#include <cstring>
#include <iomanip>
#include <sstream>

#define WINDOW_SCALE 2
#define WIDTH 320 * WINDOW_SCALE
#define HEIGHT 240 * WINDOW_SCALE

double cameraAngle = 0;
int frame = 0;

void lookAt(Camera &camera, glm::vec3 focusPoint) {
    glm::vec3 forwardVector = glm::normalize(camera.position - focusPoint);
    glm::vec3 rightVector = glm::cross({0, 1, 0}, forwardVector);
    glm::vec3 upVector = glm::cross(forwardVector, rightVector);
    camera.orientation = {rightVector, upVector, forwardVector};
}

void drawRaytracedScene(Scene &scene) {
    DrawingWindow window = scene.window;
    Camera camera = scene.camera;
    scene.window.clearPixels();
    float scale = 160.0;
    for (int y = 0; y < window.height; y++) {
        for (int x = 0; x < window.width; x++) {

            glm::vec3 endPoint = {x - window.width / 2.0, -(y - window.height / 2.0), -camera.focalLength * scale};
            glm::vec3 cameraToTriangle = camera.orientation * glm::normalize(endPoint - camera.position);
            RayTriangleIntersection intersection =
                getClosestValidIntersection(camera.position, cameraToTriangle, scene.models);

            if (intersection.distanceFromCamera != -1) {

                uint32_t colour = getPixelColour(scene, intersection, cameraToTriangle);
                scene.window.setPixelColour(x, y, colour);
            }
        }
    }
}

void drawRasterisedScene(Scene &scene) {
    scene.window.clearPixels();
    for (int i = 0; i < scene.zDepth.size(); i++) {
        for (int j = 0; j < scene.zDepth[i].size(); j++) {
            scene.zDepth[i][j] = 0;
        }
    }
    drawModels(scene);

    if (scene.camera.orbit) {
        const double turnSpeed = glm::pi<double>() / 240;
        cameraAngle += turnSpeed;
        scene.camera.position = {4 * sin(cameraAngle), 0, 4 * cos(cameraAngle)};
        lookAt(scene.camera, {0, 0, 0});
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

void handleEvent(SDL_Event event, Scene &scene) {
    Camera *camera = &scene.camera;
    DrawingWindow *window = &scene.window;
    if (event.type == SDL_KEYDOWN) {

        const double angle = glm::pi<double>() / 8;
        const float moveDistance = 0.5;

        // Rotate camera
        if (event.key.keysym.sym == SDLK_LEFT) {
            glm::mat3 rotationMatrix = {{cos(angle), 0, -sin(angle)}, {0, 1, 0}, {sin(angle), 0, cos(angle)}};
            camera->orientation = camera->orientation * rotationMatrix;
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
            glm::mat3 rotationMatrix = {{cos(angle), 0, sin(angle)}, {0, 1, 0}, {-sin(angle), 0, cos(angle)}};
            camera->orientation = camera->orientation * rotationMatrix;
        } else if (event.key.keysym.sym == SDLK_UP) {
            glm::mat3 rotationMatrix = {{1, 0, 0}, {0, cos(angle), sin(angle)}, {0, -sin(angle), cos(angle)}};
            camera->orientation = camera->orientation * rotationMatrix;
        } else if (event.key.keysym.sym == SDLK_DOWN) {
            glm::mat3 rotationMatrix = {{1, 0, 0}, {0, cos(angle), -sin(angle)}, {0, sin(angle), cos(angle)}};
            camera->orientation = camera->orientation * rotationMatrix;
        }

        // Move camera
        else if (event.key.keysym.sym == SDLK_w) {
            camera->position += camera->orientation * glm::vec3(0, 0, -moveDistance);
        } else if (event.key.keysym.sym == SDLK_a) {
            camera->position += camera->orientation * glm::vec3(-moveDistance, 0, 0);
        } else if (event.key.keysym.sym == SDLK_s) {
            camera->position += camera->orientation * glm::vec3(0, 0, moveDistance);
        } else if (event.key.keysym.sym == SDLK_d) {
            camera->position += camera->orientation * glm::vec3(moveDistance, 0, 0);
        } else if (event.key.keysym.sym == SDLK_c) {
            camera->position += camera->orientation * glm::vec3(0, moveDistance, 0);
        } else if (event.key.keysym.sym == SDLK_x) {
            camera->position += camera->orientation * glm::vec3(0, -moveDistance, 0);
        } else if (event.key.keysym.sym == SDLK_o) {
            camera->orbit = !camera->orbit;
        }

        // Draw triangles
        else if (event.key.keysym.sym == SDLK_u) {
            CanvasPoint v0 = CanvasPoint(rand() % window->width, rand() % window->height);
            CanvasPoint v1 = CanvasPoint(rand() % window->width, rand() % window->height);
            CanvasPoint v2 = CanvasPoint(rand() % window->width, rand() % window->height);
            CanvasTriangle triangle = CanvasTriangle(v0, v1, v2);
            Colour colour = Colour(rand() % 256, rand() % 256, rand() % 256);
            drawStrokedTriangle(scene, triangle, colour);
        } else if (event.key.keysym.sym == SDLK_f) {
            CanvasPoint v0 = CanvasPoint(rand() % window->width, rand() % window->height);
            CanvasPoint v1 = CanvasPoint(rand() % window->width, rand() % window->height);
            CanvasPoint v2 = CanvasPoint(rand() % window->width, rand() % window->height);
            CanvasTriangle triangle = CanvasTriangle(v0, v1, v2);
            Colour colour = Colour(rand() % 256, rand() % 256, rand() % 256);
            drawFilledTriangle(scene, triangle, colour);
        }

        // Change rendering mode
        else if (event.key.keysym.sym == SDLK_m) {
            scene.mode = (scene.mode + 1) % 3;
        }

        // Reset camera position
        else if (event.key.keysym.sym == SDLK_r) {
            camera->position = {0, 0, 4};
            camera->orientation = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
        }

    }

    else if (event.type == SDL_MOUSEBUTTONDOWN) {
        window->savePPM("output.ppm");
        window->saveBMP("output.bmp");
    }
}

int main(int argc, char *argv[]) {
    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

    Camera camera;
    camera.position = {0.0, 0.0, 4.0};
    camera.orientation = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    camera.focalLength = 2.0;
    camera.orbit = false;

    int mode = 1;

    std::map<std::string, Colour> colourMap = readMtl("cornell-box");
    std::vector<ModelTriangle> models = {};
    models = readObj("cornell-box", colourMap, 0.35);
    std::vector<ModelTriangle> sphere = readObj("sphere", colourMap, 0.2, glm::vec3(-0.5, -1.05, 0.6));
    models.insert(std::end(models), std::begin(sphere), std::end(sphere));
    colourMap = readMtl("bunny");
    std::vector<ModelTriangle> bunny = readObj("bunny", colourMap, 0.004, glm::vec3(0.24, -0.38, 0.37));
    models.insert(std::end(models), std::begin(bunny), std::end(bunny));

    std::vector<glm::vec3> lightSources = {};
    lightSources.push_back(glm::vec3(0, 0.9, 0));
    // lightSources.push_back(glm::vec3(10, 2, 20));
    Scene scene = Scene(window, camera, mode, models, lightSources);
    scene.texture = TextureMap("resources/texture.ppm");
    // Calculate vertex brightnesses before scene drawn
    // calculateVertexBrightness(scene);
    populateShadowMap(scene);

    // RayTriangleIntersection intersection = getClosestValidIntersection(camera.position, {0, 0, -1});
    // if (intersection.distanceFromCamera != -1) {
    // std::cout << "FOUND" << std::endl;
    //}

    SDL_Event event;
    while (true) {
        // We MUST poll for events - otherwise the window will freeze !
        if (scene.window.pollForInputEvents(event))
            handleEvent(event, scene);
        if (scene.mode == 0 || scene.mode == 1) {
            drawRasterisedScene(scene);
        } else {
            drawRaytracedScene(scene);
        }
        // Need to render the frame at the end, or nothing actually gets shown on the screen !
        scene.window.renderFrame();

        /* Animation code
        std::ostringstream oss;
        oss << "output" << std::setw(3) << std::setfill('0') << frame << ".ppm";
        std::string s = oss.str();
        scene.window.savePPM(s);

        if (frame < 125) {
            scene.camera.position.z -= 0.064;
            if (scene.camera.position.z < -0.181289) {
                float angle = glm::pi<float>() / 60;
                glm::mat3 rotationMatrix = {{cos(angle), 0, sin(angle)}, {0, 1, 0}, {-sin(angle), 0, cos(angle)}};
                scene.camera.orientation = scene.camera.orientation * rotationMatrix;
            }
        }
        if (frame >= 126 && frame < 225) {
            const float turnSpeed = glm::pi<float>() / 100;
            cameraAngle += turnSpeed;
            scene.camera.position = {4 * sin(cameraAngle), 0, 4 * cos(cameraAngle + glm::pi<float>())};
            lookAt(scene.camera, {0, 0, 0});
        }
        if (frame > 225) {
            return 0;
        }

        frame++;
        std::cout << frame << std::endl;
        */
    }
}
