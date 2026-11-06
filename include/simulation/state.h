#pragma once
#include <common.h>

//
typedef struct state_internal_s *state_t;

state_t STATE_new(state_types_t type, uint32_t index);

// Links the state such that starting from @start and taking @action the resulting state will be @end.
void STATE_link(state_t start, state_t end, action_t action);

int32_t STATE_getAdjacentIndex(state_t state, action_t action);

state_types_t STATE_getType(state_t state);

void STATE_setType(state_t state, state_types_t type);

// Only frees the current state and not any adjacent states.
void STATE_free(state_t state);


// TODO: move to separate module
void STATE_draw(state_t state, int w, int h, int x, int y);

// Print the indicies of the four adjacent states in clockwise order starting up.
// If the adjacent state(s) is null, then it will print -1 for each null value.
void STATE_printAdjacentIndices(state_t state);
