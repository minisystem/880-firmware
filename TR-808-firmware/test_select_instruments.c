#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "unity.h"
#include "select_instrument_with_soloing.h"
#include "switches.h"
#include "drums.h"

void setUp(void) {
    // Optional setup before each test
}

void tearDown(void) {
    // Optional teardown after each test
}

void test_initialStateToSoloUnswitched(void) {
    struct SoloState state = {1, false, EMPTY}; // Initial state

    state = handleInstrumentTransition(state, 3, true);  // Enter solo mode
    TEST_ASSERT_EQUAL_UINT8(3, state.currentInstrument);
    TEST_ASSERT_TRUE(state.isSolo);
    TEST_ASSERT_EQUAL_UINT8(EMPTY, state.secondaryInstrument);

    state = handleInstrumentTransition(state, 4, false); // Add secondary instrument
    TEST_ASSERT_EQUAL_UINT8(3, state.currentInstrument);
    TEST_ASSERT_TRUE(state.isSolo);
    TEST_ASSERT_EQUAL_UINT8(4, state.secondaryInstrument); 
}

void test_soloSwitchedToSoloUnswitched(void) {
    struct SoloState state = {2, true, EMPTY};

    state = handleInstrumentTransition(state, 3, false); // Try to add secondary (should fail)
    TEST_ASSERT_EQUAL_UINT8(2, state.currentInstrument);
    TEST_ASSERT_TRUE(state.isSolo);
    TEST_ASSERT_EQUAL_UINT8(EMPTY, state.secondaryInstrument); 

    state = handleInstrumentTransition(state, 4, false); // Try to add secondary (should fail)
    TEST_ASSERT_EQUAL_UINT8(2, state.currentInstrument);
    TEST_ASSERT_TRUE(state.isSolo);
    TEST_ASSERT_EQUAL_UINT8(EMPTY, state.secondaryInstrument); 

    state = handleInstrumentTransition(state, 5, true); // Switch to instrument 5, stay in solo
    TEST_ASSERT_EQUAL_UINT8(5, state.currentInstrument);
    TEST_ASSERT_TRUE(state.isSolo);
    TEST_ASSERT_EQUAL_UINT8(EMPTY, state.secondaryInstrument);

    state = handleInstrumentTransition(state, 5, true); // Try to add secondary (should fail)
    TEST_ASSERT_EQUAL_UINT8(5, state.currentInstrument);
    TEST_ASSERT_TRUE(state.isSolo);
    TEST_ASSERT_EQUAL_UINT8(EMPTY, state.secondaryInstrument); 
}


int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_initialStateToSoloUnswitched);
    RUN_TEST(test_soloSwitchedToSoloUnswitched);
    // ... run other tests similarly
    return UNITY_END();
}
