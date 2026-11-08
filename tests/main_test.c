#include "acutest.h"
#include "grid_tests.c"
// test_q_table.c
//
// Unit tests for src/bellman/q_table.c using acutest.h
// Not covered on purpose: QTABLE_free (unimplemented) and QTABLE_clearDependent.

#include "acutest.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <bellman/q_table.h>

#define N_STATES 5

// ---------------------------------------------------------------------------
// QTABLE_new
// ---------------------------------------------------------------------------

void q_table_test_new_not_null(void) {
    q_table_t table = QTABLE_new(N_STATES);
    TEST_CHECK(table != NULL);
}

void test_new_all_entries_nan(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t s = 0; s < N_STATES; s++) {
        for (size_t a = 0; a < N_ACTIONS; a++) {
            TEST_CHECK(isnan(QTABLE_get(table, s, a)));
            TEST_MSG("Entry (state=%zu, action=%zu) should start as NAN", s, a);
        }
    }
}

void test_new_single_state(void) {
    q_table_t table = QTABLE_new(1);
    TEST_CHECK(table != NULL);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        TEST_CHECK(isnan(QTABLE_get(table, 0, a)));
    }
}

// ---------------------------------------------------------------------------
// QTABLE_set / QTABLE_get
// ---------------------------------------------------------------------------

void test_set_get_roundtrip(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 2, 1, 3.5);
    TEST_CHECK(QTABLE_get(table, 2, 1) == 3.5);
    TEST_MSG("Got %f", QTABLE_get(table, 2, 1));
}

void test_set_overwrite(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 1, 0, 1.0);
    QTABLE_set(table, 1, 0, -7.25);
    TEST_CHECK(QTABLE_get(table, 1, 0) == -7.25);
    TEST_MSG("Got %f", QTABLE_get(table, 1, 0));
}

void test_set_does_not_affect_other_entries(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 2, 1, 9.0);

    for (size_t s = 0; s < N_STATES; s++) {
        for (size_t a = 0; a < N_ACTIONS; a++) {
            if (s == 2 && a == 1) continue;
            TEST_CHECK(isnan(QTABLE_get(table, s, a)));
            TEST_MSG("Entry (state=%zu, action=%zu) was unexpectedly modified", s, a);
        }
    }
}

void test_set_get_zero_and_negative(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 0, 0, 0.0);
    QTABLE_set(table, 0, 1, -100.0);
    TEST_CHECK(QTABLE_get(table, 0, 0) == 0.0);
    TEST_CHECK(QTABLE_get(table, 0, 1) == -100.0);
}

void test_set_get_all_entries_distinct(void) {
    q_table_t table = QTABLE_new(N_STATES);
    // Write a unique value into every slot, then verify nothing aliases.
    for (size_t s = 0; s < N_STATES; s++) {
        for (size_t a = 0; a < N_ACTIONS; a++) {
            QTABLE_set(table, s, a, (double)(s * 100 + a));
        }
    }
    for (size_t s = 0; s < N_STATES; s++) {
        for (size_t a = 0; a < N_ACTIONS; a++) {
            TEST_CHECK(QTABLE_get(table, s, a) == (double)(s * 100 + a));
            TEST_MSG("(state=%zu, action=%zu) expected %f got %f",
                     s, a, (double)(s * 100 + a), QTABLE_get(table, s, a));
        }
    }
}

void test_set_last_state_last_action(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, N_STATES - 1, N_ACTIONS - 1, 42.0);
    TEST_CHECK(QTABLE_get(table, N_STATES - 1, N_ACTIONS - 1) == 42.0);
}

void test_set_nan_value(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 1, 1, 5.0);
    QTABLE_set(table, 1, 1, NAN);
    TEST_CHECK(isnan(QTABLE_get(table, 1, 1)));
}

// ---------------------------------------------------------------------------
// QTABLE_highestQ
// ---------------------------------------------------------------------------

void test_highestQ_all_set(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        QTABLE_set(table, 3, a, (double)a);  // max is N_ACTIONS - 1
    }
    TEST_CHECK(QTABLE_highestQ(table, 3) == (double)(N_ACTIONS - 1));
    TEST_MSG("Got %f", QTABLE_highestQ(table, 3));
}

void test_highestQ_max_in_first_action(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 0, 0, 10.0);
    for (size_t a = 1; a < N_ACTIONS; a++) {
        QTABLE_set(table, 0, a, 1.0);
    }
    TEST_CHECK(QTABLE_highestQ(table, 0) == 10.0);
}

void test_highestQ_max_in_last_action(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS - 1; a++) {
        QTABLE_set(table, 0, a, 1.0);
    }
    QTABLE_set(table, 0, N_ACTIONS - 1, 10.0);
    TEST_CHECK(QTABLE_highestQ(table, 0) == 10.0);
}

void test_highestQ_all_negative(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        QTABLE_set(table, 1, a, -10.0 - (double)a);  // max is -10 (action 0)
    }
    TEST_CHECK(QTABLE_highestQ(table, 1) == -10.0);
    TEST_MSG("Got %f", QTABLE_highestQ(table, 1));
}

void test_highestQ_ties(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        QTABLE_set(table, 2, a, 4.0);
    }
    TEST_CHECK(QTABLE_highestQ(table, 2) == 4.0);
}

void test_highestQ_ignores_other_states(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        QTABLE_set(table, 0, a, 1.0);
        QTABLE_set(table, 1, a, 1000.0);
    }
    TEST_CHECK(QTABLE_highestQ(table, 0) == 1.0);
    TEST_MSG("Got %f (values from state 1 leaked in)", QTABLE_highestQ(table, 0));
}

void test_highestQ_skips_nan_entries_in_middle(void) {
    q_table_t table = QTABLE_new(N_STATES);
    // Action 0 set, the rest left as NAN except the last
    QTABLE_set(table, 0, 0, 2.0);
    QTABLE_set(table, 0, N_ACTIONS - 1, 8.0);
    TEST_CHECK(QTABLE_highestQ(table, 0) == 8.0);
    TEST_MSG("Got %f", QTABLE_highestQ(table, 0));
}

// NOTE: This test documents expected behaviour and currently FAILS against the
// implementation: maxQ is initialised from action 0 even when it is NAN, and
// `currQ > NAN` is always false, so the result stays NAN.
void test_highestQ_first_action_nan(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 0, N_ACTIONS - 1, 6.0);  // action 0 remains NAN
    TEST_CHECK(QTABLE_highestQ(table, 0) == 6.0);
    TEST_MSG("Got %f", QTABLE_highestQ(table, 0));
}

void test_highestQ_all_nan(void) {
    q_table_t table = QTABLE_new(N_STATES);
    TEST_CHECK(isnan(QTABLE_highestQ(table, 0)));
}

// ---------------------------------------------------------------------------
// QTABLE_highestAction
// ---------------------------------------------------------------------------

void test_highestAction_unique_max_each_position(void) {
    for (size_t best = 0; best < N_ACTIONS; best++) {
        q_table_t table = QTABLE_new(N_STATES);
        for (size_t a = 0; a < N_ACTIONS; a++) {
            QTABLE_set(table, 1, a, a == best ? 10.0 : 1.0);
        }
        TEST_CHECK(QTABLE_highestAction(table, 1) == (action_t)best);
        TEST_MSG("Expected action %zu, got %d", best, (int)QTABLE_highestAction(table, 1));
    }
}

void test_highestAction_all_negative(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        QTABLE_set(table, 0, a, -50.0);
    }
    QTABLE_set(table, 0, N_ACTIONS - 1, -1.0);
    TEST_CHECK(QTABLE_highestAction(table, 0) == (action_t)(N_ACTIONS - 1));
    TEST_MSG("Got %d", (int)QTABLE_highestAction(table, 0));
}

void test_highestAction_tie_returns_first(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        QTABLE_set(table, 2, a, 5.0);
    }
    // Implementation uses strict '>' so the earliest action wins ties.
    TEST_CHECK(QTABLE_highestAction(table, 2) == (action_t)0);
    TEST_MSG("Got %d", (int)QTABLE_highestAction(table, 2));
}

void test_highestAction_ignores_other_states(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        QTABLE_set(table, 0, a, a == 0 ? 3.0 : 1.0);                    // best: 0
        QTABLE_set(table, 1, a, a == N_ACTIONS - 1 ? 100.0 : 0.0);      // best: last
    }
    TEST_CHECK(QTABLE_highestAction(table, 0) == (action_t)0);
}

void test_highestAction_consistent_with_highestQ(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        QTABLE_set(table, 4, a, (double)((a * 7) % 5) - 2.0);
    }
    action_t best = QTABLE_highestAction(table, 4);
    TEST_CHECK(QTABLE_get(table, 4, best) == QTABLE_highestQ(table, 4));
    TEST_MSG("Q of best action = %f, highestQ = %f",
             QTABLE_get(table, 4, best), QTABLE_highestQ(table, 4));
}

void test_highestAction_skips_nan_in_middle(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 0, 0, 1.0);
    QTABLE_set(table, 0, N_ACTIONS - 1, 9.0);
    TEST_CHECK(QTABLE_highestAction(table, 0) == (action_t)(N_ACTIONS - 1));
    TEST_MSG("Got %d", (int)QTABLE_highestAction(table, 0));
}

// NOTE: Same bug as test_highestQ_first_action_nan. Expected to FAIL until
// the initial maxQ handling is fixed (the function returns 0 here).
void test_highestAction_first_action_nan(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 0, N_ACTIONS - 1, 6.0);  // action 0 remains NAN
    TEST_CHECK(QTABLE_highestAction(table, 0) == (action_t)(N_ACTIONS - 1));
    TEST_MSG("Got %d", (int)QTABLE_highestAction(table, 0));
}

// ---------------------------------------------------------------------------
// QTABLE_deleteEntry
// ---------------------------------------------------------------------------

void test_deleteEntry_existing_returns_zero(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 2, 1, 3.0);
    TEST_CHECK(QTABLE_deleteEntry(table, 2, 1) == 0);
}

void test_deleteEntry_sets_nan(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 2, 1, 3.0);
    QTABLE_deleteEntry(table, 2, 1);
    TEST_CHECK(isnan(QTABLE_get(table, 2, 1)));
}

void test_deleteEntry_missing_returns_minus_one(void) {
    q_table_t table = QTABLE_new(N_STATES);
    TEST_CHECK(QTABLE_deleteEntry(table, 0, 0) == -1);
}

void test_deleteEntry_twice(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 3, 0, 1.0);
    TEST_CHECK(QTABLE_deleteEntry(table, 3, 0) == 0);
    TEST_CHECK(QTABLE_deleteEntry(table, 3, 0) == -1);
}

void test_deleteEntry_zero_value_counts_as_existing(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 1, 1, 0.0);
    TEST_CHECK(QTABLE_deleteEntry(table, 1, 1) == 0);
    TEST_CHECK(isnan(QTABLE_get(table, 1, 1)));
}

void test_deleteEntry_only_affects_target(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t s = 0; s < N_STATES; s++) {
        for (size_t a = 0; a < N_ACTIONS; a++) {
            QTABLE_set(table, s, a, (double)(s * 10 + a));
        }
    }
    QTABLE_deleteEntry(table, 2, 1);

    for (size_t s = 0; s < N_STATES; s++) {
        for (size_t a = 0; a < N_ACTIONS; a++) {
            if (s == 2 && a == 1) {
                TEST_CHECK(isnan(QTABLE_get(table, s, a)));
            } else {
                TEST_CHECK(QTABLE_get(table, s, a) == (double)(s * 10 + a));
                TEST_MSG("(state=%zu, action=%zu) was modified by deleteEntry", s, a);
            }
        }
    }
}

void test_deleteEntry_affects_highest(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t a = 0; a < N_ACTIONS; a++) {
        QTABLE_set(table, 0, a, (double)a);
    }
    QTABLE_deleteEntry(table, 0, N_ACTIONS - 1);  // remove the maximum
    TEST_CHECK(QTABLE_highestQ(table, 0) == (double)(N_ACTIONS - 2));
    TEST_MSG("Got %f", QTABLE_highestQ(table, 0));
}

// ---------------------------------------------------------------------------
// QTABLE_clear
// ---------------------------------------------------------------------------

void test_clear_resets_all_entries(void) {
    q_table_t table = QTABLE_new(N_STATES);
    for (size_t s = 0; s < N_STATES; s++) {
        for (size_t a = 0; a < N_ACTIONS; a++) {
            QTABLE_set(table, s, a, 1.0 + (double)s);
        }
    }
    QTABLE_clear(table);
    for (size_t s = 0; s < N_STATES; s++) {
        for (size_t a = 0; a < N_ACTIONS; a++) {
            TEST_CHECK(isnan(QTABLE_get(table, s, a)));
            TEST_MSG("Entry (state=%zu, action=%zu) not cleared", s, a);
        }
    }
}

void test_clear_on_fresh_table(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_clear(table);
    TEST_CHECK(isnan(QTABLE_get(table, 0, 0)));
}

void test_clear_then_reuse(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 1, 0, 5.0);
    QTABLE_clear(table);
    QTABLE_set(table, 1, 0, 7.0);
    TEST_CHECK(QTABLE_get(table, 1, 0) == 7.0);
    TEST_MSG("Got %f", QTABLE_get(table, 1, 0));
}

void test_clear_makes_deleteEntry_fail(void) {
    q_table_t table = QTABLE_new(N_STATES);
    QTABLE_set(table, 1, 0, 5.0);
    QTABLE_clear(table);
    TEST_CHECK(QTABLE_deleteEntry(table, 1, 0) == -1);
}

// ---------------------------------------------------------------------------

TEST_LIST = {
    // QTABLE_new
    { "new/not_null",                         q_table_test_new_not_null },
    { "new/all_entries_nan",                  test_new_all_entries_nan },
    { "new/single_state",                     test_new_single_state },

    // QTABLE_set / QTABLE_get
    { "set_get/roundtrip",                    test_set_get_roundtrip },
    { "set_get/overwrite",                    test_set_overwrite },
    { "set_get/no_side_effects",              test_set_does_not_affect_other_entries },
    { "set_get/zero_and_negative",            test_set_get_zero_and_negative },
    { "set_get/all_entries_distinct",         test_set_get_all_entries_distinct },
    { "set_get/last_state_last_action",       test_set_last_state_last_action },
    { "set_get/set_nan",                      test_set_nan_value },

    // QTABLE_highestQ
    { "highestQ/all_set",                     test_highestQ_all_set },
    { "highestQ/max_first_action",            test_highestQ_max_in_first_action },
    { "highestQ/max_last_action",             test_highestQ_max_in_last_action },
    { "highestQ/all_negative",                test_highestQ_all_negative },
    { "highestQ/ties",                        test_highestQ_ties },
    { "highestQ/ignores_other_states",        test_highestQ_ignores_other_states },
    { "highestQ/skips_nan_in_middle",         test_highestQ_skips_nan_entries_in_middle },
    { "highestQ/first_action_nan",            test_highestQ_first_action_nan },
    { "highestQ/all_nan",                     test_highestQ_all_nan },

    // QTABLE_highestAction
    { "highestAction/unique_max_each_pos",    test_highestAction_unique_max_each_position },
    { "highestAction/all_negative",           test_highestAction_all_negative },
    { "highestAction/tie_returns_first",      test_highestAction_tie_returns_first },
    { "highestAction/ignores_other_states",   test_highestAction_ignores_other_states },
    { "highestAction/consistent_with_Q",      test_highestAction_consistent_with_highestQ },
    { "highestAction/skips_nan_in_middle",    test_highestAction_skips_nan_in_middle },
    { "highestAction/first_action_nan",       test_highestAction_first_action_nan },

    // QTABLE_deleteEntry
    { "deleteEntry/existing_returns_0",       test_deleteEntry_existing_returns_zero },
    { "deleteEntry/sets_nan",                 test_deleteEntry_sets_nan },
    { "deleteEntry/missing_returns_-1",       test_deleteEntry_missing_returns_minus_one },
    { "deleteEntry/twice",                    test_deleteEntry_twice },
    { "deleteEntry/zero_is_existing",         test_deleteEntry_zero_value_counts_as_existing },
    { "deleteEntry/only_target",              test_deleteEntry_only_affects_target },
    { "deleteEntry/affects_highest",          test_deleteEntry_affects_highest },

    // QTABLE_clear
    { "clear/resets_all",                     test_clear_resets_all_entries },
    { "clear/fresh_table",                    test_clear_on_fresh_table },
    { "clear/reuse",                          test_clear_then_reuse },
    { "clear/deleteEntry_fails_after",        test_clear_makes_deleteEntry_fail },
    // grid_tests.c
    // GRID_new / nStates / player position
    { "new/not_null",                          test_new_not_null },
    { "new/nStates",                           test_new_nStates },
    { "new/player_starts_at_zero",             test_new_player_starts_at_zero },
    { "playerPosition/roundtrip",              test_setPlayerPosition_roundtrip },

    // GRID_stepProcess
    { "step/right_down_from_origin",           test_step_right_down_from_origin },
    { "step/matches_expected_every_state",     test_step_matches_expected_for_every_state },
    { "step/left_up_blocked_at_edges",         test_step_left_up_blocked_at_top_left_edges },
    { "step/inverse_returns_to_origin",        test_step_inverse_returns_to_origin },
    { "step/does_not_move_player",             test_step_does_not_move_player },
    { "step/reward_normal_tile",               test_step_reward_normal_tile },
    { "step/reward_wall",                      test_step_reward_wall },
    { "step/null_reward_pointer",              test_step_null_reward_pointer_ok },
    { "step/single_tile_all_blocked",          test_step_single_tile_grid_all_blocked },

    // GRID_changeTileType
    { "changeTileType/changes_reward",         test_changeTileType_changes_reward },
    { "changeTileType/other_tiles_untouched",  test_changeTileType_does_not_affect_other_tiles },
    { "changeTileType/back_to_normal",         test_changeTileType_back_to_normal },
    { "changeTileType/first_and_last_tile",    test_changeTileType_first_and_last_tile },

    // GRID_takeAction
    { "takeAction/moves_player",               test_takeAction_moves_player_right_down },
    { "takeAction/returns_tile_reward",        test_takeAction_returns_tile_reward },
    { "takeAction/special_tile_reward",        test_takeAction_reward_of_special_tile },
    { "takeAction/wall_keeps_position",        test_takeAction_wall_keeps_position },
    { "takeAction/wall_bottom_right",          test_takeAction_wall_bottom_right },
    { "takeAction/walk_to_corner",             test_takeAction_walk_to_bottom_right_corner },
    { "takeAction/agrees_with_stepProcess",    test_takeAction_agrees_with_stepProcess },

    // GRID_free
    { "free/does_not_crash",                   test_free_does_not_crash },

    { NULL, NULL } // End of tests entry
};
