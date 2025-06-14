// file draw_signal.h

#ifndef DRAW_SIGNAL_H
#define DRAW_SIGNAL_H

#include "main.h"

void draw_channel_signal(float *history, int history_index, int trigger_index,
                         float offset_y, float scale_y, Color color,
                         float osc_width, float lineThickness, int history_size,
                         int points_left, int points_right, float trigger_x_pos, bool reverse);

void draw_signal(OscData *oscData, float osc_width, float lineThickness);

#endif // DRAW_SIGNAL_H

