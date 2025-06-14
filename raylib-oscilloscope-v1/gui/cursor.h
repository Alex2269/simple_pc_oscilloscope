// cursor.h
#ifndef CURSOR_H
#define CURSOR_H

#include "raylib.h"
#include <stdbool.h>

#define DEFAULT_CURSOR_WIDTH 10.0f
#define DEFAULT_CURSOR_HEIGHT 20.0f
#define DEFAULT_CURSOR_TOP_Y 5
#define ARROW_SIZE 10
#define THIN_LINE 100

typedef struct {
    float x;
    float y;
    float topY;
    float width;
    float height;
    Color color;
    bool isDragging;
    int value;
    int minValue;
    int maxValue;
} Cursor;

Cursor InitCursor(float startX, float topY, float width, float height, Color color, int minValue, int maxValue);

void UpdateAndHandleCursors(Cursor *cursors, int count, Vector2 mousePos, bool mouseButtonPressed, bool mouseButtonDown, bool mouseButtonReleased);

void DrawCursorsAndDistance(Cursor *cursors, int count, Font font, int fontSize);

#endif // CURSOR_H
