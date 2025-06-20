// gui_dropdown.h
#ifndef GUI_DROPDOWN_H
#define GUI_DROPDOWN_H

#include "raylib.h"
#include <stdbool.h>

int Gui_Dropdown(Rectangle bounds, const char **items, int itemCount,
                 int currentIndex, const char *hintText, bool *isOpen, Color color);

#endif // GUI_DROPDOWN_H
