// include/bellman/q_table.h

#pragma once
#include <bellman/policy_table.h>
#include <simulation/grid.h>

// Tabular version of the state action function.
// Each state and action has its own discrete discounted return.
// The q-table doesn't calculate the discounted return, instead it assumes all of the discounted returns are calculated by the current policy.
typedef struct q_table_inner_s *q_table_t;

q_table_t QTABLE_new(size_t nStates);

// Arguments:
// @state - state pair associated with @q.
// @action - action pair associated with @q.
// @q - action value.
void QTABLE_set(q_table_t table, size_t state, size_t action, double q);

double QTABLE_get(q_table_t table, size_t state, size_t action);

// Returns the largest state action value associated with the state.
double QTABLE_highestQ(q_table_t table, size_t state);

// Returns the action that results in the highest action value.
action_t QTABLE_highestAction(q_table_t table, size_t state);

// Deletes all of the action values entries that use the action value entry for the state @stateIndex.
// This also deletes the entry for the state @stateIndex. Function is useful for greedy policy improvement functions.
// Arguments:
// @table - Non null pointer to the q-table. the action value table that needs to be updated following policy improvement.
// @model - the game model.
// @stateIndex - index entry to the policy improved state.
void QTABLE_clearDependent(q_table_t table, grid_t model, policy_table_t policy, uint32_t stateIndex);

// Deletes all of entries. Doesn't free up any memory.
void QTABLE_clear(q_table_t table);

// TODO check
int QTABLE_deleteEntry(q_table_t table, uint32_t stateIndex, action_t action);

void QTABLE_free(q_table_t table);
