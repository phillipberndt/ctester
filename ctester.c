/**
 * \mainpage Index page
 *
 * ctester -- a C test framework resembling GTest
 *
 * # Overview
 *
 * This is a test framework for GNU C running on POSIX platforms with an interface
 * resembling GTest.
 *
 * The use restriction is hard, the code uses both POSIX and GNU C features. It is
 * compatible with GCC and CLANG.
 *
 * See test.c for documentation and usage information.
 */

#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE

#include "ctester.h"

#include <getopt.h>
#include <fnmatch.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>

#define _CTESTER_STATE_DEFAULT 0
#define _CTESTER_STATE_SCHEDULED 1
#define _CTESTER_STATE_FAILED 2
#define _CTESTER_STATE_SUCCEEDED 3

#define _CTESTER_INFO_WARNING    "   WARN   "
#define _CTESTER_INFO_THICK_BAR  "=========="
#define _CTESTER_INFO_THIN_BAR   "----------"
#define _CTESTER_INFO_RUN        " RUN      "
#define _CTESTER_INFO_OK         "      OK  "
#define _CTESTER_INFO_FAILED     "  FAILED  "
#define _CTESTER_INFO_PASSED     "  PASSED  "

struct ctester_test_case_list_t *ctester_test_root;

/**
 * printf(), but output `info` in ANSI color code `color` preceding the normal
 * output.
 */
static void print_info(int color, char *info, char *format, ...) {
	static int fp_1_is_tty = -1;

	if(fp_1_is_tty < 0) {
		fp_1_is_tty = isatty(1);
	}

	va_list args;
	va_start(args, format);

	if(fp_1_is_tty) {
		printf("\033[%dm[%s]\033[0m ", color, info);
	}
	else {
		printf("[%s] ", info);
	}
	vprintf(format, args);
}

/**
 * Return number of wall-clock miliseconds since some fixed date. Only deltas
 * of the returned value are used.
 */
static unsigned long get_clock_ms() {
	struct timespec tp;
	if(clock_gettime(CLOCK_MONOTONIC, &tp) == -1) {
		print_info(33, _CTESTER_INFO_WARNING, "Failed to retrieve value of monotonic clock. Timing info will be wrong.\n");
		return 0;
	}
	return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}

/**
 * Print help to stdout.
 */
void print_help(const char *binary_name) {
	puts("This binary contains ctester test cases.\n\nSyntax:\n");
	printf(" %s [-h] [-l] [-t <pattern>]\n", binary_name);
	puts("\n"
		"Where\n"
		"  -h               Prints this help.\n"
		"  -l               Lists available test cases.\n"
		"  -t <pattern>     Specifies a fnmatch(3) pattern to specify which test\n"
		"                   cases to run.\n"
		"\n"
	);
}

/**
 * List all available test cases.
 */
void print_list() {
	struct ctester_test_case_list_t *test = ctester_test_root;
	while(test) {
		printf("%s\n", test->full_test_name);
		test = test->next;
	}
}

int main(int argc, char *argv[]) {
	// Command line parsing
	const char *pattern = "*";
	int character;
	while((character = getopt(argc, argv, "hlt:")) != -1) {
		switch(character) {
			case 'h':
				print_help(argv[0]);
				exit(0);
				break;
			case 'l':
				print_list();
				exit(0);
				break;
			case 't':
				pattern = strdup(optarg);
				break;
			default:
				print_help(argv[0]);
				exit(1);
				break;
		}
	}

	setvbuf(stdout, NULL, _IONBF, 0);

	// Select all test cases matching the pattern
	struct ctester_test_case_list_t *test = ctester_test_root;
	struct ctester_test_case_list_t *test_case_start = test;
	int total_test_count = 0, total_test_case_count = 0, total_disabled_tests = 0;
	while(test) {
		if(strcmp(test->test_case_name, test_case_start->test_case_name)) {
			test_case_start = test;
		}
		if(fnmatch(pattern, test->full_test_name, 0) == 0) {
			if(strncmp(test->test_name, "DISABLED_", sizeof("DISABLED_") - 1) == 0 && strcmp(test->full_test_name, pattern) != 0) {
				total_disabled_tests++;
				print_info(33, _CTESTER_INFO_WARNING, "Test %s is disabled. Give its name using -t explicitly if you want to run it.\n", test->full_test_name);
				test = test->next;
				continue;
			}
			if(!test_case_start->number_of_tests) {
				total_test_case_count++;
			}
			total_test_count++;
			test_case_start->number_of_tests++;
			test->state = _CTESTER_STATE_SCHEDULED;
		}

		test = test->next;
	}

	// Run all test cases and fetch some statistics
	test = ctester_test_root;
	test_case_start = NULL;
	unsigned long overall_start_time = get_clock_ms();
	unsigned long test_case_start_time = overall_start_time;

	unsigned passed_tests = 0, failed_tests = 0;

	print_info(32, _CTESTER_INFO_THICK_BAR, "Running %d test%s from %d test case%s.\n", total_test_count, total_test_count == 1 ? "" : "s", total_test_case_count, total_test_case_count == 1 ? "" : "s");
	while(test) {
		if(!test_case_start || strcmp(test->test_case_name, test_case_start->test_case_name)) {
			if(test_case_start && test_case_start->number_of_tests) {
				print_info(32, _CTESTER_INFO_THIN_BAR, "%d test%s from %s (%lu ms total)\n", test_case_start->number_of_tests, test_case_start->number_of_tests == 1 ? "" : "s", test_case_start->test_case_name, get_clock_ms() - test_case_start_time);
			}
			test_case_start = test;
			if(test_case_start->number_of_tests) {
				print_info(32, _CTESTER_INFO_THIN_BAR, "%d test%s from %s\n", test_case_start->number_of_tests, test_case_start->number_of_tests == 1 ? "" : "s", test_case_start->test_case_name);
				test_case_start_time = get_clock_ms();
			}
		}

		if(test->state == _CTESTER_STATE_SCHEDULED) {
			struct ctester_test_case_state_t state;
			memset(&state, 0, sizeof(struct ctester_test_case_state_t));
			test->state = _CTESTER_STATE_SCHEDULED;

			print_info(32, _CTESTER_INFO_RUN, "%s\n", test->full_test_name);

			unsigned long test_start_time = get_clock_ms();
			// This is where the actual test case is executed
			test->test_body(&state);
			unsigned long test_run_time = get_clock_ms() - test_start_time;

			if(state.failed == 0) {
				test->state = _CTESTER_STATE_SUCCEEDED;
				passed_tests++;
				print_info(state.warning == 0 ? 32 : 33, _CTESTER_INFO_OK, "%s (%lu ms total)\n", test->full_test_name, test_run_time);
			}
			else {
				test->state = _CTESTER_STATE_FAILED;
				failed_tests++;
				print_info(31, _CTESTER_INFO_FAILED, "%s (%lu ms total)\n", test->full_test_name,test_run_time);
			}
		}

		test = test->next;
	}
	if(test_case_start && test_case_start->number_of_tests) {
		print_info(32, _CTESTER_INFO_THIN_BAR, "%d test%s from %s (%lu ms total)\n", test_case_start->number_of_tests, test_case_start->number_of_tests == 1 ? "" : "s", test_case_start->test_case_name, get_clock_ms() - test_case_start_time);
	}
	print_info(32, _CTESTER_INFO_THICK_BAR, "%d test%s from %d test case%s ran. (%lu ms total)\n", total_test_count, total_test_count == 1 ? "" : "s", total_test_case_count, total_test_case_count == 1 ? "" : "s", get_clock_ms() - overall_start_time);

	// Output the overall status and list the failed test cases a second time
	if(passed_tests) {
		print_info(32, _CTESTER_INFO_PASSED, "%d test%s\n", passed_tests, passed_tests == 1 ? "" : "s");
	}
	if(total_disabled_tests) {
		print_info(33, _CTESTER_INFO_WARNING, "%d test%s disabled.\n", total_disabled_tests, total_disabled_tests == 1 ? " is" : "s are");
	}
	if(failed_tests) {
		print_info(31, _CTESTER_INFO_FAILED, "%d test%s, listed below:\n", failed_tests, failed_tests == 1 ? "" : "s");
		test = ctester_test_root;
		while(test) {
			if(test->state == _CTESTER_STATE_FAILED) {
				print_info(31, _CTESTER_INFO_FAILED, "%s\n", test->full_test_name);
			}
			test = test->next;
		}
		printf("\n\n %d FAILED TEST%s\n", failed_tests, failed_tests == 1 ? "" : "s");

		return 1;
	}

	return 0;
}
