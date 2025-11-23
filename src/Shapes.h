#pragma once

#include "DrawingWindow.h"
#include "CanvasPoint.h"
#include "CanvasTriangle.h"
#include "Colour.h"

void drawLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour);
void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour);
void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour);
