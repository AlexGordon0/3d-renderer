#pragma once

#include "CanvasTriangle.h"
#include "DrawingWindow.h"
#include "Colour.h"
#include "TextureMap.h"

Colour getTexturePixelValue(glm::vec3 relativeCoordinates, CanvasTriangle textureTriangle, TextureMap &texture);
void drawTexturedTriangle(DrawingWindow &window, CanvasTriangle triangle, CanvasTriangle textureTriangle,
                          TextureMap &texture);
