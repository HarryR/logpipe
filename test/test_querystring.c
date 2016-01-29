#include "querystring.h"
#include "minunit.h"

MU_TEST(test_querystring_basic1) {
	str_t test1_input = str_init_cstr("key=val&derp=merp");
	pair_t *test1_output = querystring_parse(&test1_input);
	str_clear(&test1_input);

	mu_check(test1_output != NULL);

	str_t key = str_init_cstr("key");
	str_t val = str_init_cstr("val");
	mu_check(strpair_bykey(test1_output, &key) != NULL);
	mu_check(strpair_byval(test1_output, &val) != NULL);
	str_clear(&key);
	str_clear(&val);

	str_t derp = str_init_cstr("derp");
	str_t merp = str_init_cstr("merp");
	mu_check(strpair_bykey(test1_output, &derp) != NULL);
	mu_check(strpair_byval(test1_output, &merp) != NULL);
	pair_t *found = strpair_byval(test1_output, &merp);
	mu_check(str_eq(&found->key, &derp));
	mu_check(str_eq(&found->val, &merp));
	str_clear(&key);
	str_clear(&val);
}

MU_TEST_SUITE(test_suite) {
    MU_RUN_TEST(test_querystring_basic1);
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return minunit_fail ? 1 : 0;
}