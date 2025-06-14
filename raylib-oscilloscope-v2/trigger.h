// trigger.h
#ifndef TRIGGER_H
#define TRIGGER_H

int find_trigger_index_with_hysteresis(float *history, int history_index,
                                       float trigger_level_px, float trigger_hysteresis_px,
                                       int history_size, int last_trigger_index, bool *trigger_locked);

void update_trigger_indices(OscData *oscData);

#endif // TRIGGER_H

