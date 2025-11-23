#include "Shapes.h"
#include "Interpolation.h"
#include <algorithm>

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
