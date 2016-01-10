#include "logpipe.h"
#include "minunit.h"


MU_TEST(test_logpipe_basic) {
	int error;
	logpipe_t *pipe = logpipe_new();
	mu_check(pipe);

	// Determine how the API functions when the pipe is empty
	mu_check(logpipe_buf_get(pipe, NULL) == NULL);
	mu_check(logpipe_steps_index(pipe) == -1);
	mu_check(logpipe_run(pipe, &error) == 0);
	mu_check(error == 0);
	mu_check(logpipe_steps_index(pipe) == -1);

	// Then ensure it can run one step
	mu_check(logpipe_steps_count(pipe) == 0);
	mu_check(logpipe_steps_add(pipe, "debug.randblank"));
	mu_check(logpipe_steps_count(pipe) == 1);
	mu_check(logpipe_run(pipe, &error) == 1);
	mu_check(error == 0);

	logpipe_destroy(pipe);
}

MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(test_logpipe_basic);
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return 0;
}