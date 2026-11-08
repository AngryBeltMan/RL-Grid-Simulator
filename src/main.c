#include "bellman/policy_table.h"
#include "bellman/q_table.h"
#include "training/model_policy_evaluation.h"
#include <training/policy_iteration.h>

int main(int argc, char* args[]) {
    // strtod
    const size_t gridSideLen = 5;
    const size_t nStates = gridSideLen * gridSideLen;
    const int screenWidth = gridSideLen * 100;
    const int screenHeight = gridSideLen * 100;
    const int targetFPS = 30;


    grid_t grid = GRID_new(gridSideLen, gridSideLen);
    GRID_changeTileType(grid, 1, STATE_GOAL);
    GRID_changeTileType(grid, 8, STATE_HAZARD);
    GRID_dbgPrint(grid);

    policy_table_t policy = PT_newRandomized(gridSideLen * gridSideLen);
    q_table_t qTable = QTABLE_new(gridSideLen * gridSideLen);
    q_table_t qTableScratch = QTABLE_new(gridSideLen * gridSideLen);

    double greedyProb[N_ACTIONS];

    double *v = calloc(gridSideLen * gridSideLen, sizeof(double));
    uint8_t b;
    for (int i = 0; i < 4; i++) {
        // RAND_bytes(&b, sizeof(uint8_t));
        // b %= gridSideLen * gridSideLen; // constrain to a valid state index
        // double discountedReturn = evaluatePolicy(policy, qTable, qTableScratch, grid, b, 25);

        // v = truncatedPolicyIteration(policy, grid, qTable, 9, v);
        // v = policyIteration(policy, grid, qTable, 0.001, v);
        v = valueIteration(policy, grid, qTable, v);
        assert(v);
        // memset(v, 0, sizeof(double) * gridSideLen * gridSideLen);
        // action_t highestAction = QTABLE_highestAction(qTable, b);
        // memset(greedyProb, 0, sizeof(double) * N_ACTIONS);
        // greedyProb[highestAction] = 1.0;
        // PT_setAction(policy, b, greedyProb);
        QTABLE_clear(qTable);
        // QTABLE_clearDependent(qTable, grid, policy, b);

    }

    // evaluatePolicy(policy, qTable, qTableScratch, grid, 9, 10);
    // ACTION_ITER(action) {
    //     printf("action %s action value %f\n", ACTIONS_STR[action], QTABLE_get(qTable, 9, action));
    // }

    InitWindow(screenWidth, screenHeight, "Reinforcement Learning Sim");

    if (targetFPS > 0) SetTargetFPS(targetFPS);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        GRID_draw(grid);
        PT_draw(policy, gridSideLen);
        EndDrawing();
    }
}
