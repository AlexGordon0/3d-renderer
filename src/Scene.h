#pragma once

#include "Camera.h"
#include "TextureMap.h"
#include "DrawingWindow.h"
#include "ModelTriangle.h"

struct Scene {
    DrawingWindow window;
    Camera camera;
    int mode;
    std::vector<ModelTriangle> model;
    glm::vec3 lightSource;
    std::vector<std::vector<float>> zDepth;
    TextureMap texture;

    Scene(DrawingWindow _window, Camera _camera, int _mode, std::vector<ModelTriangle> _model, glm::vec3 _lightSource) {
        window = _window;
        camera = _camera;
        mode = _mode;
        model = _model;
        lightSource = _lightSource;
        for (int i = 0; i < _window.height; i++) {
            std::vector<float> row;
            for (int j = 0; j < _window.width; j++) {
                row.push_back(0.0f);
            }
            zDepth.push_back(row);
        }
    }
};
