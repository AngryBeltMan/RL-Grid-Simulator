#include "bellman/policy_table.h"
#include "bellman/q_table.h"
#include "common.h"
#include "simulation/grid.h"
#include <training/model_policy_evaluation.h>

double evaluatePolicy(policy_table_t policy, q_table_t qTable, q_table_t scratchQTable, grid_t model, uint32_t stateIndex, uint32_t episodeLen) {
    // Get the immediate rewards of every state
    for (uint32_t state = 0; state < GRID_nStates(model); state++) {
        ACTION_ITER(action) {
            double immReward;
            GRID_stepProcess(model, state, action, &immReward);
            QTABLE_set(scratchQTable, state, action, immReward);
        }
    }

    // From a starting state
    for (uint32_t episode = 1; episode < episodeLen; episode++) {
        for (uint32_t state = 0; state < GRID_nStates(model); state++) {
            ACTION_ITER(action) { // During the last iteration, variable action is the 'a' in Q_π(s,a), it doesn't follow the policy
                double immReward;
                uint32_t nextState = GRID_stepProcess(model, state, action, &immReward); // S'
                // Compute the state value
                double stateValue = 0;
                ACTION_ITER(action2) {
                    stateValue += PT_actionProbability(policy, nextState, action2) * QTABLE_get(scratchQTable, nextState, action2);
                }
                QTABLE_set(qTable, state, action, immReward + DISCOUNT * stateValue);
            }
        }
        q_table_t temp = scratchQTable; scratchQTable = qTable; qTable = temp; // swap the tables
    }
    q_table_t temp = scratchQTable; scratchQTable = qTable; qTable = temp; // swap the tables

    double stateValue = 0;
    ACTION_ITER(action) {
        stateValue += PT_actionProbability(policy, stateIndex, action) * QTABLE_get(qTable, stateIndex, action);
    }
    return stateValue;
}
