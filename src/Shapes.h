#pragma once

#include "CanvasTriangle.h"
#include "Scene.h"
#include "Colour.h"

void drawLine(Scene &scene, CanvasPoint from, CanvasPoint to, Colour colour);
void drawStrokedTriangle(Scene &scene, CanvasTriangle triangle, Colour colour);
void drawFilledTriangle(Scene &scene, CanvasTriangle triangle, Colour colour);
