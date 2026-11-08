#include "bellman/policy_table.h"
#include "bellman/q_table.h"
#include "simulation/grid.h"
#include <training/policy_iteration.h>

// Write the state action values using the state values obtained from policy/truncated-policy iteration.
static void writeQTable(policy_table_t policy, grid_t model, q_table_t qTable, double *arrayV) {
    uint32_t nStates = GRID_nStates(model);
    double greedyProb[N_ACTIONS];
    for (uint32_t stateIndex = 0; stateIndex < nStates; stateIndex++) {
        ACTION_ITER(action) {
            double immReward;
            uint32_t sPrime = GRID_stepProcess(model, stateIndex, action, &immReward);
            double q = immReward + DISCOUNT * arrayV[sPrime];
            QTABLE_set(qTable, stateIndex, action, q);
        }
        memset(greedyProb, 0, sizeof(double) * N_ACTIONS);
        action_t highestAction = QTABLE_highestAction(qTable, stateIndex);
        greedyProb[highestAction] = 1;
        PT_setAction(policy, stateIndex, greedyProb);
    }
}

double* truncatedPolicyIteration(policy_table_t policy, grid_t model, q_table_t qTable, uint32_t nIterations, double *arrayV) {
    assert(policy && model && qTable && arrayV);
    uint32_t nStates = GRID_nStates(model);
    double *arrayVNext = malloc(sizeof(double) * nStates);
    assert(arrayVNext);

    for (uint32_t j = 0; j < nIterations; j++) {
        for (uint32_t stateIndex = 0; stateIndex < nStates; stateIndex++) {
            double vNext = 0;
            ACTION_ITER(action) {
                double prob = PT_actionProbability(policy, stateIndex, action);
                double immReward;
                // Since the model is deterministic we do not additional loops to summate the mean immediete reward and next state's (S') discounted return
                // Both summations can be collapsed into a single non looping expression.
                uint32_t sPrime = GRID_stepProcess(model, stateIndex, action, &immReward);
                assert(!isnan(arrayV[sPrime]));
                double discountedReturn = DISCOUNT * arrayV[sPrime];
                vNext += prob * (immReward + discountedReturn);
            }
            arrayVNext[stateIndex] = vNext;
        }
        // Swap pointers. This setup guarantees that arrayV will always point to the improved array (arrayVNext) and that arrayVNext and arrayV do not point to the same memory address.
        double* temp = arrayV; arrayV = arrayVNext; arrayVNext = temp;
    }

    // Setting arrayVNext to NULL doesn't affect the address arrayV points to since it is just a pointer copy, so it will still point to freed memory.
    free(arrayVNext); arrayVNext = NULL;
    writeQTable(policy, model, qTable, arrayV);
    return arrayV;
}

double* valueIteration(policy_table_t policy, grid_t model, q_table_t qTable, double* arrayV) {
    return truncatedPolicyIteration(policy, model, qTable, 1, arrayV);
}

double* policyIteration(policy_table_t policy, grid_t model, q_table_t qTable, double minDifference, double *arrayV) {
    assert(policy && model && qTable && arrayV);
    uint32_t nStates = GRID_nStates(model);
    double *arrayVNext = malloc(sizeof(double) * nStates);
    assert(arrayVNext);
    double difference;
    do {
        difference = 0;
        for (uint32_t stateIndex = 0; stateIndex < nStates; stateIndex++) {
            double vNext = 0;
            ACTION_ITER(action) {
                double prob = PT_actionProbability(policy, stateIndex, action);
                double immReward;
                // Since the model is deterministic we do not additional loops to summate the mean immediete reward and next state's (S') discounted return
                // Both summations can be collapsed into a single non looping expression.
                uint32_t sPrime = GRID_stepProcess(model, stateIndex, action, &immReward);
                assert(!isnan(arrayV[sPrime]));
                double discountedReturn = DISCOUNT * arrayV[sPrime];
                vNext += prob * (immReward + discountedReturn);
            }
            arrayVNext[stateIndex] = vNext;
            difference += pow(vNext - arrayV[stateIndex], 2);
        }
        // Swap pointers. This setup guarantees that arrayV will always point to the improved array (arrayVNext) and that arrayVNext and arrayV do not point to the same memory address.
        double* temp = arrayV; arrayV = arrayVNext; arrayVNext = temp;
        difference = sqrt(difference);
    } while (difference > minDifference);

    free(arrayVNext); arrayVNext = NULL;
    writeQTable(policy, model, qTable, arrayV);
    return arrayV;

}
