#pragma once
#include <simulation/state.h>

typedef struct policy_table_inner_s *policy_table_t;

policy_table_t PT_newRandomized(uint32_t size);

policy_table_t PT_newZeros();

double PT_actionProbability(policy_table_t policy, uint32_t stateIndex, action_t action);

// Chooses an action randomly based on their probability distribution
action_t PT_action(policy_table_t policy, uint32_t stateIndex);

// Chooses an action randomly based on their probability distribution
void PT_setAction(policy_table_t policy, uint32_t stateIndex, double probabilities[N_ACTIONS]);

void PT_free(policy_table_t policy);

// TODO: Move to own dedicated header file.
void PT_draw(policy_table_t policy, uint32_t columns);
