#include "bellman/policy_table.h"
#include "bellman/q_table.h"
#include "common.h"
#include "simulation/grid.h"
#include <training/model_policy_evaluation.h>

static double evaluateQ(policy_table_t policy, q_table_t qTable, grid_t model, uint32_t stateIndex, action_t action, uint32_t remainingSteps);

double evaluatePolicy(policy_table_t policy, q_table_t qTable, q_table_t scratchQTable, grid_t model, uint32_t stateIndex, uint32_t episodeLen) {
    // double averageQ = 0;
    // for (size_t i = 0; i < N_ACTIONS; i++) {
    //     // This will cause the function to exponentially grow. Instead we should have a check to see if the move doesn't need to be traversed further.
    //     action_t action = ACTIONS[i];
    //     double p = PT_actionProbability(policy, stateIndex, action); // π(a|s)
    //     double q;
    //     // For later actions, we have likely already visited certain paths and therefore already know its action value. Therefore, there is no need to travaverse the bellman equation further.
    //     if (!isnan(QTABLE_get(qTable, stateIndex, action))) {
    //         printf("Exists (%i, %i)\n", stateIndex, action);
    //         q = QTABLE_get(qTable, stateIndex, action);
    //     } else {
    //         q = evaluateQ(policy, qTable, model, stateIndex, action, episodeLen - 1);
    //     }
    //     averageQ += p * q;
    // }
    // return averageQ;
    // Get the immediete rewards of every state
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
            ACTION_ITER(action) {
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
        if (episode + 1 < episodeLen) {
            q_table_t temp = scratchQTable; scratchQTable = qTable; qTable = temp; // swap the tables
        }
    }
    double stateValue = 0;
    ACTION_ITER(action) {
        stateValue += PT_actionProbability(policy, stateIndex, action) * QTABLE_get(qTable, stateIndex, action);
    }
    return stateValue;
}

// static double evaluateQ(policy_table_t policy, q_table_t qTable, grid_t model, uint32_t stateIndex, action_t action, uint32_t remainingSteps) {
//     // IGNORE: Note for all QTABLE_set function calls we can guarantee that it will always overwrite a nan value since this function is only called when
//
//     double immediateReward;
//     uint32_t newState = GRID_stepProcess(model, stateIndex, action, &immediateReward); // S'
//     // printf("Reward from taking action %s is %f\n", ACTIONS_STR[action], immediateReward);
//
//     if (remainingSteps < 1) {
//         QTABLE_set(qTable, stateIndex, action, immediateReward);
//         return immediateReward;
//     }
//     // TODO: Macro definition of Discount used here. May be changed to be a variable
//     double discountedReturn = DISCOUNT * evaluatePolicy(policy, qTable, model, newState, remainingSteps - 1); // Recursion occurs here
//
//     QTABLE_set(qTable, stateIndex, action, immediateReward + discountedReturn);
//     return immediateReward + discountedReturn;
// }
//     // printf("Setting (%i, %i) = %f; New state is %i\n", stateIndex, action, immediateReward, newState);
//     //     printf("Setting (%i, %i) = %f; New state is %i\n", stateIndex, action, immediateReward, newState);
//
