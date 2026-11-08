#pragma once
#include <simulation/grid.h>
#include <bellman/policy_table.h>
#include <bellman/q_table.h>

// Evaluates the policy at the given state while also filling in the give state action pairs in the q-table.
double evaluatePolicy(policy_table_t policy, q_table_t table, q_table_t scratchTable, grid_t model, uint32_t stateIndex, uint32_t episodeLen);
