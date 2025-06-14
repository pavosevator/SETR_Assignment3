#include "unity.h"
#include "cmdproc.h"
#include "i2c.h"
#include "data.h"
#include "uart.h"
#include "heater.h"
#include "gpio.h"

//Configuration
void setUp(void) {
    // optional: runs before each test
}

void tearDown(void) {
    // optional: runs after each test
}

void test_i2c_init_should_return_OK(void) {
    TEST_ASSERT_EQUAL(0, i2c_init());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_i2c_init_should_return_OK);
    return UNITY_END();
}
