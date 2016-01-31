#include <stdio.h>
#include <string.h>

#include "config.h"
#include "str.h"
#include "minunit.h"

MU_TEST(test_ptime_epoc_secs) {
    struct tm output;
    char timestamp[50];

    // Parse with milliseconds
    str_t ex1 = str_init_cstr("887230474.472");
    memset(&output, 0, sizeof(output));
    mu_check(str_ptime_epoch_secs(&ex1, &output));
  	portable_strftime(timestamp, sizeof(timestamp), "%s", &output);
  	// struct tm doesn't contain millis
  	mu_check(!strcmp(timestamp, "887230474"));

  	// Parse without milliseconds
    str_t ex2 = str_init_cstr("887230474");
    memset(&output, 0, sizeof(output));
    mu_check(str_ptime_epoch_secs(&ex2, &output));
	portable_strftime(timestamp, sizeof(timestamp), "%s", &output);
    mu_check(!strcmp(timestamp, "887230474"));
}

MU_TEST(test_strpair_split) {
    str_t ex1 = str_init_cstr(" derp   merp ");
    pair_t *ex1_out = strpair_split(&ex1);
    str_clear(&ex1);
    
    mu_check(strpair_count(ex1_out) == 2);
    mu_check(strpair_byval_cstr(ex1_out, "derp") != NULL);
    mu_check(strpair_byval_cstr(ex1_out, "merp") != NULL);
    strpair_clear(ex1_out);
}

MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(test_ptime_epoc_secs);
    MU_RUN_TEST(test_strpair_split);
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return minunit_fail ? 1 : 0;
}
