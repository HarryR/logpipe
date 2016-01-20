#include "minunit.h"
#include "steps.h"
#include "logmeta.h"


MU_TEST(test_steps_empty) {
	logsteps_t steps;
	logsteps_init(&steps);

	mu_check(logsteps_count(&steps) == 0);
	mu_check(logsteps_idx(&steps) == 0);
	mu_check(logsteps_step(&steps, NULL, NULL) <= 0);
	mu_check(logsteps_step(&steps, NULL, NULL) <= 0);
	mu_check(logsteps_idx(&steps) == 0);

	logsteps_restart(&steps);
	mu_check(logsteps_idx(&steps) == 0);

	logsteps_free(&steps);
}


MU_TEST(test_steps_one) {
	logsteps_t steps;
	logmeta_t meta;
	str_t buf;

	str_init(&buf);
	logsteps_init(&steps);
	logmeta_init(&meta);

	mu_check(logsteps_add(&steps, "debug.randblank") == 1);
	mu_check(logsteps_count(&steps) == 1);
	mu_check(logsteps_step(&steps, &buf, &meta) != 0);
	mu_check(logsteps_step(&steps, &buf, &meta) == 0);
	mu_check(logsteps_idx(&steps) == 1);

	logsteps_restart(&steps);
	mu_check(logsteps_idx(&steps) == 0);

	logmeta_clear(&meta);
	logsteps_free(&steps);
}


MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(test_steps_empty);
    MU_RUN_TEST(test_steps_one);
}


int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return ! minunit_fail && ! minunit_assert;
}