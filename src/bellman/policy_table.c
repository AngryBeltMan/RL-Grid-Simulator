#include <bellman/policy_table.h>
typedef struct {
    double actions[N_ACTIONS];
} state_actions_t;

// Generates an evenly distributed random actions where all the actions total to 1.
static state_actions_t SA_stdRand() {
    state_actions_t sa;
    uint32_t sum = 0;
    uint16_t n[N_ACTIONS];
    assert(RAND_bytes((uint8_t *)&n, sizeof(uint16_t) * N_ACTIONS) && "Failed to generate rnadom number");
    for (int i = 0; i < N_ACTIONS; i++) sum += n[i];
    for (int i = 0; i < N_ACTIONS; i++) {
        sa.actions[i] = (double)n[i] / sum;
    }
    return sa;
}

// Draws four lines for the four possible movement actions along with a circle with a radius representing the chance of staying in place.
static void SA_draw(state_actions_t sa, double magnitude, int centerX, int centerY ) {
    // Drawing function is harded coded and only based on a five action system.
    assert(N_ACTIONS == 5);
    DrawCircleLines(centerX, centerY, magnitude * sa.actions[ACTION_STAY], RED);
    DrawLine(centerX, centerY, centerX, centerY - (magnitude * sa.actions[ACTION_UP]), ORANGE);
    DrawLine(centerX, centerY, centerX + (magnitude * sa.actions[ACTION_RIGHT]), centerY, VIOLET);
    DrawLine(centerX, centerY, centerX, centerY + (magnitude * sa.actions[ACTION_DOWN]), BLUE);
    DrawLine(centerX, centerY, centerX - (magnitude * sa.actions[ACTION_LEFT]), centerY, GREEN);
}

static void SA_print(state_actions_t sa) {
    printf("State Action probabilities ");
    for (int i = 0; i < N_ACTIONS; i++) {
        printf("%.2f ", sa.actions[i]);
    }
    printf("\n");
}

struct policy_table_inner_s {
    state_actions_t *stateActions;
    uint32_t nStateActions;
};


policy_table_t PT_newRandomized(uint32_t size) {
    policy_table_t pt = malloc(sizeof(struct policy_table_inner_s));
    assert(pt);

    pt->stateActions = malloc(sizeof(state_actions_t) * size);
    pt->nStateActions = size;
    assert(pt->stateActions);

    for (uint32_t i = 0; i < size; i++)
        pt->stateActions[i] = SA_stdRand();
    return pt;
}

double PT_actionProbability(policy_table_t policy, uint32_t stateIndex, action_t action) {
    assert(policy);
    assert(stateIndex < policy->nStateActions);
    return policy->stateActions[stateIndex].actions[action];
}

action_t PT_action(policy_table_t policy, uint32_t stateIndex) {
    assert(policy);
    assert(stateIndex < policy->nStateActions);
    state_actions_t stateAction = policy->stateActions[stateIndex];

    uint32_t randN;
    RAND_bytes((uint8_t *)&randN, sizeof(uint32_t));
    double normN = (double)randN / UINT32_MAX;
    for (action_t action = 0; action < N_ACTIONS; action++) {
        if (normN < stateAction.actions[action]) {
            return action;
        }
        normN -= stateAction.actions[action];
    }
    assert(false && "Unreachable case");
}

void PT_setAction(policy_table_t policy, uint32_t stateIndex, double probabilities[N_ACTIONS]) {
    assert(policy);
    // TODO: check if probabilites are properly normalize or normalize here
    memcpy(policy->stateActions[stateIndex].actions, probabilities, sizeof(double) * N_ACTIONS);
}

void PT_draw(policy_table_t policy, uint32_t columns) {
    int width = 80;
    int height = 80;
    int offset = 5;
    size_t rows = policy->nStateActions / columns;
    for (int x = 0; x < columns; x++) {
        for (int y = 0; y < rows; y++) {
            // STATE_draw(policy->stateActions[x + y * grid->nColumns], width, height, (width + offset) * x, (height + offset) * y);
            SA_draw(policy->stateActions[x + y * columns], width / 2., (width + offset) * x + width / 2, (height + offset) * y + height / 2);
        }
    }
}
