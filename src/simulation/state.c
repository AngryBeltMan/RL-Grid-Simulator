#include <simulation/state.h>
// 4 refers to the possible directions a starting state take to travel to a new state.
// These refer to up, right, down, and left.
#define DIRECTIONS 4

struct state_internal_s {
    state_t adjacentStates[DIRECTIONS];
    state_types_t stateType;
    uint32_t index;
};

state_t STATE_new(state_types_t type, uint32_t index) {
    state_t state = malloc(sizeof(struct state_internal_s));
    assert(state && "Error allocating state");
    for (int i = 0; i < 4; i++) { state->adjacentStates[i] = NULL; }
    state->stateType = type;
    state->index = index;
    return state;
}

void STATE_link(state_t start, state_t end, action_t action) {
    assert(action != ACTION_STAY && start && end);
    assert(start != end);
    start->adjacentStates[action] = end;
    end->adjacentStates[INVERSE_ACTION[action]] = start;
}

int32_t STATE_getAdjacentIndex(state_t state, action_t action) {
    assert(state);
    if (action == ACTION_STAY) {
        return state->index;
    }
    // Check if the player will walk out of bounds by taking the action. If it doesn't then we return the resulting index, otherwise, we return back an invalid negative index.
    return state->adjacentStates[action] ? state->adjacentStates[action]->index : -1;
}
state_types_t STATE_getType(state_t state) {
    assert(state);
    return state->stateType;
}

void STATE_setType(state_t state, state_types_t type) {
    assert(state);
    state->stateType = type;
}

void STATE_free(state_t state) {
    assert(state);
    free(state);
}

void STATE_draw(state_t state, int w, int h, int x, int y) {
    assert(state);
    Color color = DARKGRAY;
    switch (state->stateType) {
        case STATE_NORMAL: { } break;
        case STATE_HAZARD: { color = RED; } break;
        case STATE_GOAL: { color = LIME; } break;
    }

    DrawRectangleLines(x, y, w, h, color);
    char msg[8];
    snprintf(msg, 8, "%i", state->index);
    DrawText(msg, x + w / 4, y + h / 2, 8, BLACK);
}

void STATE_printAdjacentIndices(state_t state) {
    assert(state);
    printf("State %i indices: ", state->index);
    for (int direction = 0; direction < DIRECTIONS; direction++) {
        state_t adjState = state->adjacentStates[direction];
        if (adjState) {
            printf("%i ", adjState->index);
        } else {
            printf("-1 ");
        }
    }
    printf("\n");
}
