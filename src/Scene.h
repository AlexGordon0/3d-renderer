#pragma once

#include "Camera.h"
#include "DrawingWindow.h"
#include "ModelTriangle.h"
#include "TextureMap.h"

struct Scene {
    DrawingWindow window;
    Camera camera;
    int mode;
    std::vector<ModelTriangle> models;
    std::vector<glm::vec3> lightSources;
    std::vector<std::vector<float>> zDepth;
    std::array<std::array<float, 1024>, 1024> shadowMap;
    TextureMap texture;

    Scene(DrawingWindow _window, Camera _camera, int _mode, std::vector<ModelTriangle> _models,
          std::vector<glm::vec3> _lightSources) {
        window = _window;
        camera = _camera;
        mode = _mode;
        models = _models;
        lightSources = _lightSources;
        for (int i = 0; i < _window.height; i++) {
            std::vector<float> row;
            for (int j = 0; j < _window.width; j++) {
                row.push_back(0.0f);
            }
            zDepth.push_back(row);
        }
    }
};
