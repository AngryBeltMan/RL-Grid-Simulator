// src/simulation/grid.c

#include <simulation/grid.h>
#include <simulation/state.h>

struct grid_internals_s {
    state_t *stateGrid;
    uint32_t nColumns;
    uint32_t nStates;
    uint32_t playerPosition;
};

grid_t GRID_new(uint32_t rows, uint32_t columns) {
    grid_t grid = malloc(sizeof(struct grid_internals_s));
    assert(grid);

    grid->nColumns = columns;
    grid->nStates = rows * columns;
    grid->playerPosition = 0;
    grid->stateGrid = malloc(sizeof(state_t) * rows * columns);

    // First create all of the states of the grid
    for (int x = 0; x < columns; x++) {
        for (int y = 0; y < rows; y++) {
            grid->stateGrid[x + y * columns] = STATE_new(STATE_NORMAL, x + y * columns);
        }
    }

    // Link of the states together by the relative position
    for (int x = 0; x < columns - 1 ; x++) {
        for (int y = 0; y < rows - 1; y++) {
            state_t currState = grid->stateGrid[x + y * columns];
            state_t rightState = grid->stateGrid[x + 1 + y * columns];
            state_t downState = grid->stateGrid[x + y * columns + columns];
            STATE_link(currState, rightState, ACTION_RIGHT);
            STATE_link(currState, downState, ACTION_DOWN);
        }
    }

    // Link the last columns
    for (int row = 0;  row < rows - 1; row++) {
        state_t topColumn = grid->stateGrid[columns * (row + 1) - 1];
        state_t bottomColumn = grid->stateGrid[columns * (row + 2) - 1];
        STATE_link(topColumn, bottomColumn, ACTION_DOWN);
    }

    // Link the last rows
    size_t bottomRow = (rows - 1) * columns;
    for (int col = 0;  col < columns - 1; col++) {
        state_t leftColumn = grid->stateGrid[bottomRow + col];
        state_t rightColumn = grid->stateGrid[bottomRow + col + 1];
        STATE_link(leftColumn, rightColumn, ACTION_RIGHT);
    }

    return grid;
}

void GRID_changeTileType(grid_t grid, uint32_t stateIndex, state_types_t newType) {
    assert(grid);
    assert(stateIndex < grid->nStates);
    STATE_setType(grid->stateGrid[stateIndex], newType);
}

void GRID_setPlayerPosition(grid_t grid, uint32_t playerPostion) {
    assert(playerPostion < grid->nStates);
    grid->playerPosition = playerPostion;
}

uint32_t GRID_getPlayerPosition(grid_t grid) {
    assert(grid);
    return grid->playerPosition;
}

uint32_t GRID_nStates(grid_t grid) {
    assert(grid);
    return grid->nStates;
}

double GRID_takeAction(grid_t grid, action_t action) {
    assert(grid);
    int32_t result = STATE_getAdjacentIndex(grid->stateGrid[grid->playerPosition], action);
    if (result >= 0) {
        grid->playerPosition = result;
        return REWARDS[STATE_getType(grid->stateGrid[result])];
    }
    return REWARDS[REWARDS_LEN - 1];

}

uint32_t GRID_stepProcess(grid_t grid, uint32_t stateT, action_t actionT, double *reward) {
    assert(grid);
    assert(stateT < grid->nStates);
    state_t sT = grid->stateGrid[stateT];
    int32_t sTPlusOne = STATE_getAdjacentIndex(sT, actionT);
    if (sTPlusOne >= 0) {
        if (reward)
            *reward = REWARDS[STATE_getType(grid->stateGrid[sTPlusOne])];
        return sTPlusOne;
    }
    if (reward)
        *reward = REWARDS[REWARDS_LEN - 1]; // last reward is the reward for attempting to travel to a NULL state.
    return stateT;
}

void GRID_dbgPrint(grid_t grid) {
    assert(grid);
    for (int i = 0; i < grid->nStates; i++)
        STATE_printAdjacentIndices(grid->stateGrid[i]);
}

void GRID_free(grid_t grid) {
    assert(grid);
    free(grid);
}

void GRID_draw(grid_t grid) {
    int width = 80;
    int height = 80;
    int offset = 5;
    size_t rows = grid->nStates / grid->nColumns;
    for (int x = 0; x < grid->nColumns; x++) {
        for (int y = 0; y < rows; y++) {
            STATE_draw(grid->stateGrid[x + y * grid->nColumns], width, height, (width + offset) * x, (height + offset) * y);
        }
    }
}
