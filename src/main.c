#include "bellman/policy_table.h"
#include "bellman/q_table.h"
#include "training/model_policy_evaluation.h"
#include <openssl/rand.h>
#include <simulation/grid.h>

int main(int argc, char* args[]) {
    // strtod
    const int screenWidth = 300;
    const int screenHeight = 300;
    const int targetFPS = 300;

    grid_t grid = GRID_new(3, 3);
    GRID_changeTileType(grid, 1, STATE_GOAL);
    GRID_dbgPrint(grid);

    policy_table_t policy = PT_newRandomized(9);
    q_table_t qTable = QTABLE_new(9);
    q_table_t qTableScratch = QTABLE_new(9);

    double greedyProb[N_ACTIONS];
    uint8_t b;
    for (int i = 0; i < 100; i++) {
        RAND_bytes(&b, sizeof(uint8_t));
        b %= 9;
        // b = 0;
        double discountedReturn = evaluatePolicy(policy, qTable, qTableScratch, grid, b, 9);
        action_t highestAction = QTABLE_highestAction(qTable, b);
        printf("discount return %f\n", discountedReturn);
        printf("best action %s\n", ACTIONS_STR[highestAction]);
        memset(greedyProb, 0, sizeof(double) * N_ACTIONS);
        greedyProb[highestAction] = 1.0;
        PT_setAction(policy, b, greedyProb);
        QTABLE_clear(qTable);
        // QTABLE_clearDependent(qTable, grid, policy, b);

    }


    InitWindow(screenWidth, screenHeight, "Reinforcement Learning Sim");

    if (targetFPS > 0) SetTargetFPS(targetFPS);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(WHITE);
        GRID_draw(grid);
        PT_draw(policy, 3);
        EndDrawing();
    }
}
