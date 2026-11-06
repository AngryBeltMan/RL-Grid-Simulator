#pragma once
#include <common.h>

typedef struct grid_internals_s *grid_t;

// Note: @rows == @columns. Otherwise, the states will be improperly linked and undefined behavior will occur.
grid_t GRID_new(uint32_t rows, uint32_t columns);

void GRID_changeTileType(grid_t grid, uint32_t stateIndex, state_types_t newType);

void GRID_setPlayerPosition(grid_t grid, uint32_t playerPosition);

uint32_t GRID_getPlayerPosition(grid_t grid);

uint32_t GRID_nStates(grid_t grid);

// Player takes action updating the player's state. Returns the reward associated with that action.
double GRID_takeAction(grid_t grid, action_t action);

// Start with inital state S_t and take action descision A_t as inputs. Returns state transition S_t+1 and reward R_t+1.
// Arguments:
// @grid - Non null grid.
// @stateT - S_t, starting index
// @actionT - A_t.
// @reward - R_t+1, optional. If @reward != Null, then @reward will be overwritten with the corresponding reward value.
// @return - S_t+1.
uint32_t GRID_stepProcess(grid_t grid, uint32_t stateT, action_t actionT, double *reward);

void GRID_dbgPrint(grid_t grid);

void GRID_free(grid_t grid);

// TODO: Move to its separate module
void GRID_draw(grid_t grid);
