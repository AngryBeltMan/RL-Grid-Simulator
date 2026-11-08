#pragma once
#include <simulation/grid.h>
#include <bellman/policy_table.h>
#include <bellman/q_table.h>
// Uses truncated policy iteration to find v_k(s).
// Returns a pointer to the non-freed value array. This value then can be fed back into policy iteration again for further policy improvement.
// The pointer passed into the function (@arrayV) shouldn't be used since it may point to freed memory. The argument @arrayV cannot be set to NULL since it is a single pointer to the head of the array and not a double pointer.
// Arguments:
// @policy - Policy table, cannot be NULL.
// @model - Since this is a model based learning technique, function expects access to the model.
// @qTable - Action value table that is written to: Q-table, functionality wise, doesn't serve a very important purpose. As of the moment, cannot not be NULL.
// @nIterations - Number of iterations for evaulating the state values before policy improvement. A higher value leads to a more accurate policy value and a quicker convergence to the optimal policy.
// @arrayV - Must point to heap allocated memory and must have enough space for nStates number of doubles. Do not use pointer after function call.
// @return - Returns a heap allocated pointer to the next state value. Use it instead of @arrayV.
double* truncatedPolicyIteration(policy_table_t policy, grid_t model, q_table_t qTable, uint32_t nIterations, double* arrayV);

// Uses value iteration (a single step of policy evaluation) to find v_k(s).
// Returns a pointer to the non-freed value array. This value then can be fed back into policy iteration again for further policy improvement.
// The pointer passed into the function (@arrayV) shouldn't be used since it may point to freed memory. The argument @arrayV cannot be set to NULL since it is a single pointer to the head of the array and not a double pointer.
// Arguments:
// @policy - Policy table, cannot be NULL.
// @model - Since this is a model based learning technique, function expects access to the model.
// @qTable - Action value table that is written to: Q-table, functionality wise, doesn't serve a very important purpose. As of the moment, cannot not be NULL.
// @arrayV - Must point to heap allocated memory and must have enough space for nStates number of doubles. Do not use pointer after function call.
// @return - Returns a heap allocated pointer to the next state value. Use it instead of @arrayV.
double* valueIteration(policy_table_t policy, grid_t model, q_table_t qTable, double* arrayV);

// Uses the standard implementation of policy iteration to find v_k(s). Standard implementation stops after a finite number of steps once there is minimal improvement in the state value approximation, it is sufficiently accurate.
// Returns a pointer to the non-freed value array. This value then can be fed back into policy iteration again for further policy improvement.
// The pointer passed into the function (@arrayV) shouldn't be used since it may point to freed memory. The argument @arrayV cannot be set to NULL since it is a single pointer to the head of the array and not a double pointer.
// Arguments:
// @policy - Policy table, cannot be NULL.
// @model - Since this is a model based learning technique, function expects access to the model.
// @qTable - Action value table that is written to: Q-table, functionality wise, doesn't serve a very important purpose. As of the moment, cannot not be NULL.
// @minDifference - The minimum change from value iteration before the value iteration is allowed to stop.
// @arrayV - Must point to heap allocated memory and must have enough space for nStates number of doubles. Do not use pointer after function call.
// @return - Returns a heap allocated pointer to the next state value.
double* policyIteration(policy_table_t policy, grid_t model, q_table_t qTable, double minDifference, double* arrayV);
