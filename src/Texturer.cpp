#include "Texturer.h"
#include "Interpolation.h"
#include "Shapes.h"
#include <algorithm>

Colour getTexturePixelValue(glm::vec3 relativeCoordinates, CanvasTriangle textureTriangle, TextureMap &texture) {
    CanvasPoint topVertex = textureTriangle.vertices[0];
    CanvasPoint middleVertex = textureTriangle.vertices[1];
    CanvasPoint bottomVertex = textureTriangle.vertices[2];
    TexturePoint texturePixel =
        TexturePoint(middleVertex.x * relativeCoordinates.x + bottomVertex.x * relativeCoordinates.y +
                         topVertex.x * relativeCoordinates.z,
                     middleVertex.y * relativeCoordinates.x + bottomVertex.y * relativeCoordinates.y +
                         topVertex.y * relativeCoordinates.z);
    uint32_t colourValue = texture.pixels[round(texturePixel.y) * texture.width + round(texturePixel.x)];
    return Colour(colourValue >> 16 & 0xff, colourValue >> 8 & 0xff, colourValue & 0xff);
}

void drawTexturedTriangle(Scene &scene, CanvasTriangle triangle, CanvasTriangle textureTriangle, TextureMap &texture) {
    DrawingWindow *window = &scene.window;
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

            Colour pixelValue = getTexturePixelValue(relativeCoordinates, textureTriangle, texture);
            window->setPixelColour(x, y, pixelValue.value());
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

            Colour pixelValue = getTexturePixelValue(relativeCoordinates, textureTriangle, texture);
            window->setPixelColour(x, y, pixelValue.value());
        }
    }

    drawStrokedTriangle(scene, triangle, Colour(255, 255, 255));
}
