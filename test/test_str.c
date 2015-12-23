/* file minunit_example.c */
 
#include <stdio.h>
#include <string.h>

#include "str.h"
#include "minunit.h"

MU_TEST(test_ptime_epoc_secs) {
	struct tm output;
    char timestamp[50];

    // Parse with milliseconds
    str_t ex1 = str_init_cstr("887230474.472");
    memset(&output, 0, sizeof(output));
    mu_check(str_ptime_epoch_secs(&ex1, &output));
  	strftime(timestamp, sizeof(timestamp), "%s", &output);
  	// struct tm doesn't contain millis
  	mu_check(!strcmp(timestamp, "887230474"));

  	// Parse without milliseconds
    str_t ex2 = str_init_cstr("887230474");
    memset(&output, 0, sizeof(output));
    mu_check(str_ptime_epoch_secs(&ex2, &output));
    strftime(timestamp, sizeof(timestamp), "%s", &output);
    mu_check(!strcmp(timestamp, "887230474"));
}
MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(test_ptime_epoc_secs);
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return 0;
}