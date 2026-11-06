// src/bellman/q_table.c

#include <bellman/q_table.h>

struct q_table_inner_s {
    double *state_groups;
    uint32_t len;
};

q_table_t QTABLE_new(size_t nStates) {
    q_table_t table = malloc(sizeof(struct q_table_inner_s));
    assert(table);
    table->len = N_ACTIONS * nStates;
    table->state_groups = malloc(sizeof(double) * table->len);
    assert(table->state_groups);
    // memset(table->state_groups, 0, sizeof(double) * table->len);
    for (int i = 0; i < table->len; i++) {
        table->state_groups[i] = NAN;
    }
    return table;
}

void QTABLE_set(q_table_t table, size_t state, size_t action, double q) {
    assert(table);
    assert(state * N_ACTIONS + action < table->len);
    table->state_groups[state * N_ACTIONS + action] = q;
}

double QTABLE_get(q_table_t table, size_t state, size_t action) {
    assert(table);
    assert(state * N_ACTIONS + action < table->len);
    return table->state_groups[state * N_ACTIONS + action];
}

// TODO: Needs checking
double QTABLE_highestQ(q_table_t table, size_t state) {
    assert(table);
    // assert(state * N_ACTIONS + N_ACTIONS - 1 < table->len);
    action_t action = 0;
    double maxQ = QTABLE_get(table, state, action++);
    double currQ;
    for (; action < N_ACTIONS; action++) {
        currQ = QTABLE_get(table, state, action);
        if (!isnan(currQ) && currQ > maxQ) maxQ = currQ;
    }
    return maxQ;
}

// TODO: Needs checking
action_t QTABLE_highestAction(q_table_t table, size_t state) {
    assert(table);
    // assert(state * N_ACTIONS + N_ACTIONS - 1 < table->len);
    action_t action = 0;
    double maxQ = QTABLE_get(table, state, action++);
    double maxAction = 0;
    double q;
    for (; action < N_ACTIONS; action++) {
        q = QTABLE_get(table, state, action);
        if (!isnan(q) && q > maxQ) {
            maxAction = action;
            maxQ = q;
        }
    }
    return maxAction;
}

int QTABLE_deleteEntry(q_table_t table, uint32_t stateIndex, action_t action) {
    if (!isnan(QTABLE_get(table, stateIndex, action))) {
        // table->state_groups[stateIndex]
        QTABLE_set(table, stateIndex, action, NAN);
        return 0;
    }
    return -1;
}

static void clearState(q_table_t table, uint32_t stateIndex, uint32_t *nDeleted) {
    // Checks have already occurred in parent function
    for (action_t action = 0; action < N_ACTIONS; action++) {
        if (!QTABLE_deleteEntry(table, stateIndex, action)) {
            (*nDeleted)++;
        }
    }

}

static void adjacentStateLoop(q_table_t table, grid_t model, policy_table_t policy, uint32_t stateIndex, action_t problemAction, uint32_t *nDeleted) {
    if (PT_actionProbability(policy, stateIndex, problemAction) <= 0)
        return;

    if (QTABLE_deleteEntry(table, stateIndex, problemAction)) {
        // Entry doesn't exist. Suggests that the entry was already deleted before, end task now.
        return;
    }
    (*nDeleted)++;
    ACTION_ITER(action) {
        uint32_t adjState = GRID_stepProcess(model, stateIndex,  action, NULL);
        if (adjState != stateIndex) {
            adjacentStateLoop(table, model, policy, adjState, INVERSE_ACTION[action], nDeleted);
        }
    }

}

void QTABLE_clear(q_table_t table) {
    assert(table);
    for (int i = 0; i < table->len; i++) {
        table->state_groups[i] = NAN;
    }
}

void QTABLE_clearDependent(q_table_t table, grid_t model, policy_table_t policy, uint32_t stateIndex) {
    assert(table);
    assert(model);
    assert(stateIndex < table->len);
    uint32_t nDeleted = 0; // Count for entries deleted
    // CHECK: Since the policy changes so that the policy follows one action, we might not need to delete all the other action value entries and thus we also wouldn't need to have a separate function for recurrsion.
    clearState(table, stateIndex, &nDeleted);

    ACTION_ITER(action) {
        uint32_t adjState = GRID_stepProcess(model, stateIndex,  action, NULL);
        if (adjState != stateIndex) {
            adjacentStateLoop(table, model, policy, adjState, INVERSE_ACTION[action], &nDeleted);
        }
    }
    printf("Deleted %i entries from q-table\n", nDeleted);
}

