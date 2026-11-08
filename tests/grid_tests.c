// test_grid.c
//
// Unit tests for src/simulation/grid.c using acutest.h
// Not covered on purpose: GRID_draw (needs graphics) and GRID_dbgPrint (print only).
//
// Assumptions taken from grid.c itself:
//  - Tiles are indexed as column + row * nColumns.
//  - REWARDS is indexed by state type, and REWARDS[REWARDS_LEN - 1] is the
//    reward for walking into a wall.
//  - STATE_link links in one direction per call, but the grid expects
//    INVERSE_ACTION[a] to undo action a on a linked pair (the Q-table code
//    relies on this too). The tests that need it say so.

#include "acutest.h"

#include <stdint.h>
#include <simulation/grid.h>
#include <simulation/state.h>

#define SIZE 3  // 3x3 grid, 9 states

#define WALL_REWARD (REWARDS[REWARDS_LEN - 1])

static action_t LEFT(void) { return INVERSE_ACTION[ACTION_RIGHT]; }
static action_t UP(void)   { return INVERSE_ACTION[ACTION_DOWN]; }

// Expected result of moving from @s in a SIZE x SIZE grid for RIGHT/DOWN.
// Returns @s itself if the move is blocked by a wall.
static uint32_t expectedRight(uint32_t s) {
    return (s % SIZE < SIZE - 1) ? s + 1 : s;
}
static uint32_t expectedDown(uint32_t s) {
    return (s / SIZE < SIZE - 1) ? s + SIZE : s;
}

// ---------------------------------------------------------------------------
// GRID_new / GRID_nStates / player position
// ---------------------------------------------------------------------------

void test_new_not_null(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    TEST_CHECK(grid != NULL);
    GRID_free(grid);
}

void test_new_nStates(void) {
    grid_t g1 = GRID_new(1, 1);
    grid_t g3 = GRID_new(3, 3);
    grid_t g5 = GRID_new(5, 5);
    TEST_CHECK(GRID_nStates(g1) == 1);
    TEST_CHECK(GRID_nStates(g3) == 9);
    TEST_CHECK(GRID_nStates(g5) == 25);
    TEST_MSG("Got %u, %u, %u", GRID_nStates(g1), GRID_nStates(g3), GRID_nStates(g5));
    GRID_free(g1);
    GRID_free(g3);
    GRID_free(g5);
}

void test_new_player_starts_at_zero(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    TEST_CHECK(GRID_getPlayerPosition(grid) == 0);
    GRID_free(grid);
}

void test_setPlayerPosition_roundtrip(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    for (uint32_t p = 0; p < GRID_nStates(grid); p++) {
        GRID_setPlayerPosition(grid, p);
        TEST_CHECK(GRID_getPlayerPosition(grid) == p);
        TEST_MSG("Set %u, got %u", p, GRID_getPlayerPosition(grid));
    }
    GRID_free(grid);
}

// ---------------------------------------------------------------------------
// GRID_stepProcess
// ---------------------------------------------------------------------------

void test_step_right_down_from_origin(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    TEST_CHECK(GRID_stepProcess(grid, 0, ACTION_RIGHT, NULL) == 1);
    TEST_CHECK(GRID_stepProcess(grid, 0, ACTION_DOWN, NULL) == SIZE);
    GRID_free(grid);
}

void test_step_matches_expected_for_every_state(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    for (uint32_t s = 0; s < GRID_nStates(grid); s++) {
        uint32_t r = GRID_stepProcess(grid, s, ACTION_RIGHT, NULL);
        uint32_t d = GRID_stepProcess(grid, s, ACTION_DOWN, NULL);
        TEST_CHECK(r == expectedRight(s));
        TEST_MSG("RIGHT from %u: expected %u, got %u", s, expectedRight(s), r);
        TEST_CHECK(d == expectedDown(s));
        TEST_MSG("DOWN from %u: expected %u, got %u", s, expectedDown(s), d);
    }
    GRID_free(grid);
}

void test_step_left_up_blocked_at_top_left_edges(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    // Top row cannot go UP, left column cannot go LEFT.
    for (uint32_t col = 0; col < SIZE; col++) {
        TEST_CHECK(GRID_stepProcess(grid, col, UP(), NULL) == col);
        TEST_MSG("UP from top row state %u moved to %u", col, GRID_stepProcess(grid, col, UP(), NULL));
    }
    for (uint32_t row = 0; row < SIZE; row++) {
        uint32_t s = row * SIZE;
        TEST_CHECK(GRID_stepProcess(grid, s, LEFT(), NULL) == s);
        TEST_MSG("LEFT from left column state %u moved to %u", s, GRID_stepProcess(grid, s, LEFT(), NULL));
    }
    GRID_free(grid);
}

// Assumes linking is bidirectional (INVERSE_ACTION undoes a move).
void test_step_inverse_returns_to_origin(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    for (uint32_t s = 0; s < GRID_nStates(grid); s++) {
        ACTION_ITER(action) {
            uint32_t next = GRID_stepProcess(grid, s, action, NULL);
            if (next == s) continue;  // blocked by a wall
            uint32_t back = GRID_stepProcess(grid, next, INVERSE_ACTION[action], NULL);
            TEST_CHECK(back == s);
            TEST_MSG("State %u, action %d -> %u, inverse -> %u", s, (int)action, next, back);
        }
    }
    GRID_free(grid);
}

void test_step_does_not_move_player(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    GRID_setPlayerPosition(grid, 4);
    GRID_stepProcess(grid, 4, ACTION_RIGHT, NULL);
    TEST_CHECK(GRID_getPlayerPosition(grid) == 4);
    TEST_MSG("Player moved to %u", GRID_getPlayerPosition(grid));
    GRID_free(grid);
}

void test_step_reward_normal_tile(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    double reward = -12345.0;
    uint32_t next = GRID_stepProcess(grid, 0, ACTION_RIGHT, &reward);
    TEST_CHECK(next == 1);
    TEST_CHECK(reward == REWARDS[STATE_NORMAL]);
    TEST_MSG("Expected %f, got %f", REWARDS[STATE_NORMAL], reward);
    GRID_free(grid);
}

void test_step_reward_wall(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    double reward = -12345.0;
    uint32_t last = GRID_nStates(grid) - 1;
    uint32_t next = GRID_stepProcess(grid, last, ACTION_RIGHT, &reward);
    TEST_CHECK(next == last);
    TEST_CHECK(reward == WALL_REWARD);
    TEST_MSG("Expected %f, got %f", WALL_REWARD, reward);

    reward = -12345.0;
    next = GRID_stepProcess(grid, last, ACTION_DOWN, &reward);
    TEST_CHECK(next == last);
    TEST_CHECK(reward == WALL_REWARD);
    GRID_free(grid);
}

void test_step_null_reward_pointer_ok(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    // Both a normal move and a wall bump must be safe with reward == NULL.
    TEST_CHECK(GRID_stepProcess(grid, 0, ACTION_RIGHT, NULL) == 1);
    TEST_CHECK(GRID_stepProcess(grid, SIZE * SIZE - 1, ACTION_DOWN, NULL) == SIZE * SIZE - 1);
    GRID_free(grid);
}

void test_step_single_tile_grid_all_blocked(void) {
    grid_t grid = GRID_new(1, 1);
    ACTION_ITER(action) {
        double reward = 0.0;
        TEST_CHECK(GRID_stepProcess(grid, 0, action, &reward) == 0);
        TEST_CHECK(reward == WALL_REWARD);
        TEST_MSG("Action %d: got reward %f", (int)action, reward);
    }
    GRID_free(grid);
}

// ---------------------------------------------------------------------------
// GRID_changeTileType
// ---------------------------------------------------------------------------

void test_changeTileType_changes_reward(void) {
    // Every real tile type (all but the final "wall" reward slot).
    for (int t = 0; t < (int)REWARDS_LEN - 1; t++) {
        grid_t grid = GRID_new(SIZE, SIZE);
        GRID_changeTileType(grid, 4, (state_types_t)t);

        double reward = 0.0;
        uint32_t next = GRID_stepProcess(grid, 1, ACTION_DOWN, &reward);  // 1 -> 4
        TEST_CHECK(next == 4);
        TEST_CHECK(reward == REWARDS[t]);
        TEST_MSG("Type %d: expected reward %f, got %f", t, REWARDS[t], reward);
        GRID_free(grid);
    }
}

void test_changeTileType_does_not_affect_other_tiles(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    GRID_changeTileType(grid, 4, (state_types_t)0);
    GRID_changeTileType(grid, 4, STATE_NORMAL);

    double reward = 0.0;
    GRID_stepProcess(grid, 0, ACTION_RIGHT, &reward);  // 0 -> 1, untouched tile
    TEST_CHECK(reward == REWARDS[STATE_NORMAL]);
    GRID_free(grid);
}

void test_changeTileType_back_to_normal(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    GRID_changeTileType(grid, 1, (state_types_t)0);
    GRID_changeTileType(grid, 1, STATE_NORMAL);

    double reward = 0.0;
    GRID_stepProcess(grid, 0, ACTION_RIGHT, &reward);
    TEST_CHECK(reward == REWARDS[STATE_NORMAL]);
    TEST_MSG("Got %f", reward);
    GRID_free(grid);
}

void test_changeTileType_first_and_last_tile(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    uint32_t last = GRID_nStates(grid) - 1;
    GRID_changeTileType(grid, 0, STATE_NORMAL);
    GRID_changeTileType(grid, last, STATE_NORMAL);
    // Still navigable afterwards.
    TEST_CHECK(GRID_stepProcess(grid, 0, ACTION_RIGHT, NULL) == 1);
    GRID_free(grid);
}

// ---------------------------------------------------------------------------
// GRID_takeAction
// ---------------------------------------------------------------------------

void test_takeAction_moves_player_right_down(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    GRID_takeAction(grid, ACTION_RIGHT);
    TEST_CHECK(GRID_getPlayerPosition(grid) == 1);
    GRID_takeAction(grid, ACTION_DOWN);
    TEST_CHECK(GRID_getPlayerPosition(grid) == 1 + SIZE);
    TEST_MSG("Got %u", GRID_getPlayerPosition(grid));
    GRID_free(grid);
}

void test_takeAction_returns_tile_reward(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    double reward = GRID_takeAction(grid, ACTION_RIGHT);
    TEST_CHECK(reward == REWARDS[STATE_NORMAL]);
    TEST_MSG("Expected %f, got %f", REWARDS[STATE_NORMAL], reward);
    GRID_free(grid);
}

void test_takeAction_reward_of_special_tile(void) {
    for (int t = 0; t < (int)REWARDS_LEN - 1; t++) {
        grid_t grid = GRID_new(SIZE, SIZE);
        GRID_changeTileType(grid, 1, (state_types_t)t);
        double reward = GRID_takeAction(grid, ACTION_RIGHT);  // 0 -> 1
        TEST_CHECK(reward == REWARDS[t]);
        TEST_MSG("Type %d: expected %f, got %f", t, REWARDS[t], reward);
        GRID_free(grid);
    }
}

void test_takeAction_wall_keeps_position(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    // Player starts at 0 (top-left): UP and LEFT are walls.
    double r1 = GRID_takeAction(grid, UP());
    TEST_CHECK(GRID_getPlayerPosition(grid) == 0);
    TEST_CHECK(r1 == WALL_REWARD);
    TEST_MSG("UP: expected %f, got %f", WALL_REWARD, r1);

    double r2 = GRID_takeAction(grid, LEFT());
    TEST_CHECK(GRID_getPlayerPosition(grid) == 0);
    TEST_CHECK(r2 == WALL_REWARD);
    TEST_MSG("LEFT: expected %f, got %f", WALL_REWARD, r2);
    GRID_free(grid);
}

void test_takeAction_wall_bottom_right(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    uint32_t last = GRID_nStates(grid) - 1;
    GRID_setPlayerPosition(grid, last);

    double r1 = GRID_takeAction(grid, ACTION_RIGHT);
    TEST_CHECK(GRID_getPlayerPosition(grid) == last);
    TEST_CHECK(r1 == WALL_REWARD);

    double r2 = GRID_takeAction(grid, ACTION_DOWN);
    TEST_CHECK(GRID_getPlayerPosition(grid) == last);
    TEST_CHECK(r2 == WALL_REWARD);
    GRID_free(grid);
}

void test_takeAction_walk_to_bottom_right_corner(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    for (int i = 0; i < SIZE - 1; i++) GRID_takeAction(grid, ACTION_RIGHT);
    for (int i = 0; i < SIZE - 1; i++) GRID_takeAction(grid, ACTION_DOWN);
    TEST_CHECK(GRID_getPlayerPosition(grid) == GRID_nStates(grid) - 1);
    TEST_MSG("Got %u", GRID_getPlayerPosition(grid));
    GRID_free(grid);
}

void test_takeAction_agrees_with_stepProcess(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    for (uint32_t s = 0; s < GRID_nStates(grid); s++) {
        ACTION_ITER(action) {
            double stepReward = 0.0;
            uint32_t expectedNext = GRID_stepProcess(grid, s, action, &stepReward);

            GRID_setPlayerPosition(grid, s);
            double takeReward = GRID_takeAction(grid, action);

            TEST_CHECK(GRID_getPlayerPosition(grid) == expectedNext);
            TEST_MSG("State %u action %d: step -> %u, takeAction -> %u",
                     s, (int)action, expectedNext, GRID_getPlayerPosition(grid));
            TEST_CHECK(takeReward == stepReward);
            TEST_MSG("State %u action %d: step reward %f, takeAction reward %f",
                     s, (int)action, stepReward, takeReward);
        }
    }
    GRID_free(grid);
}

// ---------------------------------------------------------------------------
// GRID_free
// ---------------------------------------------------------------------------

void test_free_does_not_crash(void) {
    grid_t grid = GRID_new(SIZE, SIZE);
    GRID_free(grid);
    TEST_CHECK(1);
}

// ---------------------------------------------------------------------------
// TEST_LIST = {
//
//     { NULL, NULL }
// };
