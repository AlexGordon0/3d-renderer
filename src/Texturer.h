#pragma once

#include "TextureMap.h"
#include "DrawingWindow.h"
#include "CanvasTriangle.h"

uint32_t getTexturePixelValue(DrawingWindow &window, glm::vec3 relativeCoordinates, CanvasTriangle textureTriangle,
                              TextureMap &texture);
void drawTexturedTriangle(DrawingWindow &window, CanvasTriangle triangle, CanvasTriangle textureTriangle,
                          TextureMap &texture);
