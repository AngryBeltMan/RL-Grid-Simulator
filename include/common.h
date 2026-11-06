#pragma once
#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <openssl/rand.h>

// Gamma constant
#define DISCOUNT 0.8

typedef enum {
    STATE_NORMAL = 0,
    STATE_HAZARD = 1,
    STATE_GOAL = 2
} state_types_t;
// Reward values corresponding the the state_types's branches except for the last element.
// The last element is the reward/punishment from attempting to travel to a NULL state.
#define REWARDS_LEN (4)

const static double REWARDS[REWARDS_LEN] = {0, -10, 10, -10};

// 5 refers to the 5 possible actions taken at each state. Hence, we would need the action-value for each action in each state.
#define N_ACTIONS 5

typedef enum {
    ACTION_UP = 0,
    ACTION_RIGHT = 1,
    ACTION_DOWN = 2,
    ACTION_LEFT = 3,
    ACTION_STAY = 4,
} action_t;
const static uint32_t ACTIONS[N_ACTIONS] = {ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT, ACTION_STAY};
const static char* ACTIONS_STR[N_ACTIONS] = {"ACTION_UP", "ACTION_RIGHT", "ACTION_DOWN", "ACTION_LEFT", "ACTION_STAY"};
const static action_t INVERSE_ACTION[N_ACTIONS] = {ACTION_DOWN, ACTION_LEFT, ACTION_UP, ACTION_RIGHT, ACTION_STAY};
#define ACTION_ITER(_action) for (action_t _action = 0; _action < N_ACTIONS; _action++)
