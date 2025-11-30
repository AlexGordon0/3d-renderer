#include "Rasterising.h"
#include "CanvasTriangle.h"
#include "Shapes.h"

CanvasPoint projectVertexOntoCanvasPoint(Scene &scene, glm::vec3 vertexPosition) {
    Camera camera = scene.camera;
    glm::vec3 cameraToVertex = vertexPosition - camera.position;
    glm::vec3 transposedPosition = cameraToVertex * camera.orientation;
    float x = camera.focalLength * transposedPosition.x / transposedPosition.z;
    float y = camera.focalLength * transposedPosition.y / transposedPosition.z;

    float scale = 160.0;
    x *= scale;
    y *= scale;

    return CanvasPoint(-x + scene.window.width / 2.0, y + scene.window.height / 2.0, 1.0 / transposedPosition.z);
}

void drawModels(Scene &scene) {
    int mode = scene.mode;
    for (std::vector<ModelTriangle> model : scene.models) {
        for (ModelTriangle modelTriangle : model) {
            CanvasTriangle triangle = CanvasTriangle(projectVertexOntoCanvasPoint(scene, modelTriangle.vertices[0]),
                                                     projectVertexOntoCanvasPoint(scene, modelTriangle.vertices[1]),
                                                     projectVertexOntoCanvasPoint(scene, modelTriangle.vertices[2]));
            if (mode == 0) {
                drawStrokedTriangle(scene, triangle, Colour(255, 255, 255)); // wireframe
            } else if (mode == 1) {
                drawFilledTriangle(scene, triangle, modelTriangle.colour);
                std::cout << "2" << std::endl;
            }
        }
    }
}
