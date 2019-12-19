/*
 * ctester -- a C test framework resembling GTest
 */
#pragma once
#define _GNU_SOURCE

#include <errno.h>
#include <float.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * State structure passed to tests.
 * \internal
 */
struct ctester_test_case_state_t {
	int failed;  //<<< Stores the line number where a failure occurred
	int warning; //<<< Stores the number of warnings issued from this test
};

/**
 * Structure storing information on test cases.
 *
 * This structure is populated from constructors automatically generated for
 * each test.
 *
 * \internal
 */
struct ctester_test_case_list_t {
	char *full_test_name; //<<< Fully qualified test name, test_case.test
	char *test_case_name; //<<< Test case name
	char *test_name;      //<<< Test name
	void (*test_body)(struct ctester_test_case_state_t *ctester_state); //<<< Pointer to the test's wrapping function
	int state;            //<<< State, used internally in ::main.
	int number_of_tests;  //<<< Used to store the number of tests in this case, only used in the first test of a case
	struct ctester_test_case_list_t *next; //<<< Pointer to the next test, or NULL
};
extern struct ctester_test_case_list_t *ctester_test_root; //<<< Global variable holding the head of the test list

#define _CTESTER_INDENT "     "

/**
 * Format specifier for all formats supported by the assertion macros.
 *
 * By default, variables will be interpreted as pointers (i.e. printed in hex, assuming size_t size).
 */
#define _CTESTER_FMT(x) _Generic((x), \
	unsigned char: "%u", \
	unsigned short: "%u", \
	unsigned int: "%u", \
	unsigned long int: "%lu", \
	unsigned long long int: "%llu", \
	char: "%d", \
	short: "%d", \
	int: "%d", \
	long int: "%ld", \
	long long int: "%lld", \
	double: "%g", \
	long double: "%lg", \
	float: "%f", \
	char *: "%s", \
	unsigned char *: "%s", \
	default: "%p")

/**
 * \defgroup err_printing Error printing
 * @{
 * \internal
 */
#define _CTESTER_ERR_PRINT2(CMP_S, a, a_value, b, b_value, custom_message, ...) \
	fprintf(stderr, _CTESTER_INDENT "%s:%d: Failure.\n" _CTESTER_INDENT "    Expected: %s but\n" _CTESTER_INDENT "    %s == ", __FILE__, __LINE__, CMP_S, a); \
	fprintf(stderr, _CTESTER_FMT(a_value), a_value); \
	fprintf(stderr, ",\n" _CTESTER_INDENT "    %s == ", b); \
	fprintf(stderr, _CTESTER_FMT(b_value), b_value); \
	if(*custom_message) { \
		fprintf(stderr, ",\n" _CTESTER_INDENT "    Message: " custom_message, ## __VA_ARGS__); \
	} \
	fprintf(stderr, "\n")

#define _CTESTER_ERR_PRINT1(CMP_S, a, a_value, custom_message, ...) \
	fprintf(stderr, _CTESTER_INDENT "%s (%s:%d): Failure.\n" _CTESTER_INDENT "    Expected: %s but\n" _CTESTER_INDENT "    %s == ", __func__, __FILE__, __LINE__, CMP_S, a); \
	fprintf(stderr, _CTESTER_FMT(a_value), a_value); \
	if(*custom_message) { \
		fprintf(stderr, ",\n" _CTESTER_INDENT "    Message: " custom_message, ## __VA_ARGS__); \
	} \
	fprintf(stderr, "\n")
/// @}

/**
 * \defgroup base_macros Base macros used to implement assertions
 * @{
 * \internal
 */

inline void _ctester_nop() {}

/// Expect `a CMP b` to hold and issue a warning if it doesn't.
#define _CTESTER_EXPECT2(CMP, a, b, custom_message, ...) \
	{ \
		__typeof__(a) a_value = a; \
		__typeof__(b) b_value = b; \
		if(!(a_value CMP b_value)) { \
			_CTESTER_ERR_PRINT2(#a " " #CMP " " #b, #a, a_value, #b, b_value, custom_message, ## __VA_ARGS__); \
			ctester_state->warning++; \
		} \
	} \
	_ctester_nop()

/**
 * Expect `a CMP b` to hold and abort the current test if it doesn't.
 */
#define _CTESTER_ASSERT2(CMP, a, b, custom_message, ...) \
	{ \
		__typeof__(a) a_value = a; \
		__typeof__(b) b_value = b; \
		if(!(a_value CMP b_value)) { \
			_CTESTER_ERR_PRINT2(#a " " #CMP " " #b, #a, a_value, #b, b_value, custom_message, ## __VA_ARGS__); \
			ctester_state->failed = __LINE__; \
			return; \
		} \
	} \
	_ctester_nop()

/**
 * Expect `PRED(a, b)` to hold and issue a warning if it doesn't.
 */
#define _CTESTER_EXPECT2P(PRED, a, b, custom_message, ...) \
	{ \
		__typeof__(a) a_value = a; \
		__typeof__(b) b_value = b; \
		if(!(PRED(a_value, b_value))) { \
			_CTESTER_ERR_PRINT2(#PRED "(" #a ", " #b ")", #a, a_value, #b, b_value, custom_message, ## __VA_ARGS__); \
			ctester_state->warning++; \
		} \
	} \
	_ctester_nop()

/**
 * Expect `PRED(a, b)` to hold and abort the current test if it doesn't.
 */
#define _CTESTER_ASSERT2P(PRED, a, b, custom_message, ...) \
	{ \
		__typeof__(a) a_value = a; \
		__typeof__(b) b_value = b; \
		if(!(PRED(a_value, b_value))) { \
			_CTESTER_ERR_PRINT2(#PRED "(" #a ", " #b ")", #a, a_value, #b, b_value, custom_message, ## __VA_ARGS__); \
			ctester_state->failed = __LINE__; \
			return; \
		} \
	} \
	_ctester_nop()

/**
 * Expect `PRED(a)` to hold and issue a warning if it doesn't.
 */
#define _CTESTER_EXPECT1P(PRED, a, custom_message, ...) \
	{ \
		__typeof__(a) a_value = a; \
		if(!(PRED(a_value))) { \
			_CTESTER_ERR_PRINT1(#PRED "(" #a ")", #a, a_value, custom_message, ## __VA_ARGS__); \
			ctester_state->warning++; \
		} \
	} \
	_ctester_nop()

/**
 * Expect `PRED(a)` to hold and abort the current test if it doesn't.
 */
#define _CTESTER_ASSERT1P(PRED, a, custom_message, ...) \
	{ \
		__typeof__(a) a_value = a; \
		if(!(PRED(a_value))) { \
			_CTESTER_ERR_PRINT1(#PRED, #a, a_value, custom_message, ## __VA_ARGS__); \
			ctester_state->failed = __LINE__; \
			return; \
		} \
	} \
	_ctester_nop()

/// @}

/**
 * \defgroup assertion_macros Assertion macros
 * @{
 */

/// Assert that `a == b` holds
#define ASSERT_EQ(a, b, ...) _CTESTER_ASSERT2(==, a, b, "" __VA_ARGS__)
/// Expect that `a == b` holds
#define EXPECT_EQ(a, b, ...) _CTESTER_EXPECT2(==, a, b, "" __VA_ARGS__)

/// Assert that `a != b` holds
#define ASSERT_NE(a, b, ...) _CTESTER_ASSERT2(!=, a, b, "" __VA_ARGS__)
/// Expect that `a != b` holds
#define EXPECT_NE(a, b, ...) _CTESTER_EXPECT2(!=, a, b, "" __VA_ARGS__)

/// Assert that `a < b` holds
#define ASSERT_LT(a, b, ...) _CTESTER_ASSERT2(<, a, b, "" __VA_ARGS__)
/// Expect that `a < b` holds
#define EXPECT_LT(a, b, ...) _CTESTER_EXPECT2(<, a, b, "" __VA_ARGS__)

/// Assert that `a <= b` holds
#define ASSERT_LE(a, b, ...) _CTESTER_ASSERT2(<=, a, b, "" __VA_ARGS__)
/// Expect that `a <= b` holds
#define EXPECT_LE(a, b, ...) _CTESTER_EXPECT2(<=, a, b, "" __VA_ARGS__)

/// Assert that `a >= b` holds
#define ASSERT_GE(a, b, ...) _CTESTER_ASSERT2(>=, a, b, "" __VA_ARGS__)
/// Expect that `a >= b` holds
#define EXPECT_GE(a, b, ...) _CTESTER_EXPECT2(>=, a, b, "" __VA_ARGS__)

/// Assert that `a > b` holds
#define ASSERT_GT(a, b, ...) _CTESTER_ASSERT2(>, a, b, "" __VA_ARGS__)
/// Expect that `a > b` holds
#define EXPECT_GT(a, b, ...) _CTESTER_EXPECT2(>, a, b, "" __VA_ARGS__)

inline int _ctester_is_true(int x) { return !!x; }
/// Assert that `a` is true
#define ASSERT_TRUE(a, ...) _CTESTER_ASSERT1P(_ctester_is_true, a, "" __VA_ARGS__)
/// Expect that `a` is true
#define EXPECT_TRUE(a, ...) _CTESTER_EXPECT1P(_ctester_is_true, a, "" __VA_ARGS__)

inline int _ctester_is_false(int x) { return !x; }
/// Assert that `a` is false
#define ASSERT_FALSE(a, ...) _CTESTER_ASSERT1P(_ctester_is_false, a, "" __VA_ARGS__)
/// Expect that `a` is false
#define EXPECT_FALSE(a, ...) _CTESTER_EXPECT1P(_ctester_is_false, a, "" __VA_ARGS__)

inline int _ctester_float_eq(float a, float b) { return fabsf(a - b) < 10 * FLT_EPSILON; }
/// Assert that a is almost equal to b
#define ASSERT_FLOAT_EQ(a, b, ...) _CTESTER_ASSERT2P(_ctester_float_eq, a, b, "" __VA_ARGS__)
/// Expect that a is not almost equal to b
#define EXPECT_FLOAT_EQ(a, b, ...) _CTESTER_EXPECT2P(_ctester_float_eq, a, b, "" __VA_ARGS__)
/// Assert that a is almost equal to b
#define ASSERT_FLOAT_NE(a, b, ...) _CTESTER_ASSERT2P(!_ctester_float_eq, a, b, "" __VA_ARGS__)
/// Expect that a is not almost equal to b
#define EXPECT_FLOAT_NE(a, b, ...) _CTESTER_EXPECT2P(!_ctester_float_eq, a, b, "" __VA_ARGS__)

inline int _ctester_double_eq(double a, double b) { return fabs(a - b) < 10 * DBL_EPSILON; }
/// Assert that a is almost equal to b
#define ASSERT_DOUBLE_EQ(a, b, ...) _CTESTER_ASSERT2P(_ctester_double_eq, a, b, "" __VA_ARGS__)
/// Expect that a is almost equal to b
#define EXPECT_DOUBLE_EQ(a, b, ...) _CTESTER_EXPECT2P(_ctester_double_eq, a, b, "" __VA_ARGS__)
/// Assert that a is not almost equal to b
#define ASSERT_DOUBLE_NE(a, b, ...) _CTESTER_ASSERT2P(!_ctester_double_eq, a, b, "" __VA_ARGS__)
/// Expect that a is not almost equal to b
#define EXPECT_DOUBLE_NE(a, b, ...) _CTESTER_EXPECT2P(!_ctester_double_eq, a, b, "" __VA_ARGS__)

/// Assert that the two strings a and b compare equal
#define ASSERT_STREQ(a, b, ...) _CTESTER_ASSERT2P(!strcmp, a, b, "" __VA_ARGS__)
/// Expect that the two strings a and b compare equal
#define EXPECT_STREQ(a, b, ...) _CTESTER_EXPECT2P(!strcmp, a, b, "" __VA_ARGS__)

/// Assert that the two strings a and b do not compare equal
#define ASSERT_STRNE(a, b, ...) _CTESTER_ASSERT2P(strcmp, a, b, "" __VA_ARGS__)
/// Expect that the two strings a and b do not compare equal
#define EXPECT_STRNE(a, b, ...) _CTESTER_EXPECT2P(strcmp, a, b, "" __VA_ARGS__)

/// Assert that the two strings a and b compare equal ignoring case
#define ASSERT_STRCASEEQ(a, b, ...) _CTESTER_ASSERT2P(!strcasecmp, a, b, "" __VA_ARGS__)
/// Expect that the two strings a and b compare equal ignoring case
#define EXPECT_STRCASEEQ(a, b, ...) _CTESTER_EXPECT2P(!strcasecmp, a, b, "" __VA_ARGS__)

/// Assert that the two strings a and b do not compare equal ignoring case
#define ASSERT_STRCASENE(a, b, ...) _CTESTER_ASSERT2P(strcasecmp, a, b, "" __VA_ARGS__)
/// Expect that the two strings a and b do not compare equal ignoring case
#define EXPECT_STRCASENE(a, b, ...) _CTESTER_EXPECT2P(strcasecmp, a, b, "" __VA_ARGS__)

/// Assert that `pred(val1)` holds
#define ASSERT_PRED1(pred, val1, ...) _CTESTER_ASSERT1P(pred, val1, "" __VA_ARGS__)
/// Expect that `pred(val1)` holds
#define EXPECT_PRED1(pred, val1, ...) _CTESTER_EXPECT1P(pred, val1, "" __VA_ARGS__)

/// Assert that `pred(val1, val2)` holds
#define ASSERT_PRED2(pred, val1, val2, ...) _CTESTER_ASSERT2P(pred, val1, val2, "" __VA_ARGS__)
/// Expect that `pred(val1, val2)` holds
#define EXPECT_PRED2(pred, val1, val2, ...) _CTESTER_EXPECT2P(pred, val1, val2, "" __VA_ARGS__)

/// \internal
#define _CTESTER_TEST_RUN_AS_CHILD(statement, status, exit_code_on_failure) \
	sighandler_t old_handler = signal(SIGCHLD, SIG_DFL); \
	pid_t child_pid = fork(); \
	if(child_pid == 0) { \
		statement; \
		exit(exit_code_on_failure); \
	} \
	while(1) { \
		pid_t ret = waitpid(child_pid, &status, 0); \
		if(ret < 0) { \
			fprintf(stderr, _CTESTER_INDENT "%s:%d: Warning.\n" _CTESTER_INDENT "    waitpid(2) returned an error: %s (%d)\n", __FILE__, __LINE__, strerror(errno), errno); \
		} \
		if(ret == child_pid) { \
			break; \
		} \
	} \
	signal(SIGCHLD, old_handler)

/// \internal
#define _CTESTER_TEST_DEATH(statement, on_failure, custom_message, ...) \
	{ \
		int status = 0; \
		_CTESTER_TEST_RUN_AS_CHILD(statement, status, 0); \
		if(!WIFSIGNALED(status)) { \
			fprintf(stderr, _CTESTER_INDENT "%s:%d: Failure.\n" _CTESTER_INDENT "    Expected: %s to crash, but the process exited with status %d.\n", __FILE__, __LINE__, #statement, WEXITSTATUS(status)); \
			if(*custom_message) { \
				fprintf(stderr, _CTESTER_INDENT "    Message: " custom_message "\n", ## __VA_ARGS__); \
			} \
			on_failure; \
		} \
		else { \
			fprintf(stderr, _CTESTER_INDENT "%s:%d: Child crashed while running %s with signal %d.\n", __FILE__, __LINE__, #statement, WTERMSIG(status)); \
		} \
	} \
	_ctester_nop()

/// Assert that the statement crashes the program (such that it exits with a signal)
#define ASSERT_DEATH(statement, ...) _CTESTER_TEST_DEATH(statement, ctester_state->failed = __LINE__; return, "" __VA_ARGS__)
/// Expect that the statement crashes the program (such that it exits with a signal)
#define EXPECT_DEATH(statement, ...) _CTESTER_TEST_DEATH(statement, ctester_state->warning++, "" __VA_ARGS__)

// \internal
#define _CTESTER_TEST_EXIT(statement, on_failure, exit_code, custom_message, ...) \
	{ \
		int status; \
		_CTESTER_TEST_RUN_AS_CHILD(statement, status, !exit_code); \
		if(WIFSIGNALED(status)) { \
			fprintf(stderr, _CTESTER_INDENT "%s:%d: Failure.\n" _CTESTER_INDENT "    Expected: %s to exit with code %d, but the process crashed with signal %d.\n", __FILE__, __LINE__, #statement, exit_code, WTERMSIG(status)); \
			if(*custom_message) { \
				fprintf(stderr, _CTESTER_INDENT "    Message: " custom_message "\n", ## __VA_ARGS__); \
			} \
			on_failure; \
		} \
		else if(WEXITSTATUS(status) != exit_code) { \
			fprintf(stderr, _CTESTER_INDENT "%s:%d: Failure.\n" _CTESTER_INDENT "    Expected: %s to exit with code %d, but the process exited with status %d.\n", __FILE__, __LINE__, #statement, exit_code, WEXITSTATUS(status)); \
			if(*custom_message) { \
				fprintf(stderr, _CTESTER_INDENT "    Message: " custom_message "\n", ## __VA_ARGS__); \
			} \
				on_failure; \
		} \
	} \
	_ctester_nop()

/// Assert that the statement exits the program normally
#define ASSERT_EXIT(statement, exit_code, ...) _CTESTER_TEST_EXIT(statement, ctester_state->failed = __LINE__; return, exit_code, "") __VA_ARGS__;
/// Expect that the statement exits the program normally
#define EXPECT_EXIT(statement, exit_code, ...) _CTESTER_TEST_EXIT(statement, ctester_state->warning++, exit_code, "") __VA_ARGS__;

/// @}

/**
 * \defgroup defining_tests Defining tests
 * @{
 */

/**
 * Define a test within a test case
 *
 * This is the main macro you'll want to use to define your test cases.
 *
 * The macro uses insertion sort to create a sorted, linked list of tests in
 * the global ctester_test_root variable from a constructor function, using
 * GNU's "constructor" attribute. It emits a function definition for a function
 * named test_case__test_name, returning void and taking a
 * ::ctester_test_case_state_t pointer as an argument.
 *
 * Example:
 * \code{.c}
 *    #include <ctester.h>
 *
 *    TEST(Factorial, ZeroReturnsOne) {
 *        ASSERT_EQ(Factorial(0), 1);
 *    }
 * \endcode
 */
#define TEST(TEST_CASE_NAME, TEST_NAME) \
	void TEST_CASE_NAME ## __ ## TEST_NAME (struct ctester_test_case_state_t *ctester_state); \
	struct ctester_test_case_list_t ctester_test_case_info_ ## TEST_CASE_NAME ## __ ## TEST_NAME = { \
		.full_test_name = #TEST_CASE_NAME "." #TEST_NAME, \
		.test_case_name = #TEST_CASE_NAME, \
		.test_name = #TEST_NAME, \
		.test_body = & TEST_CASE_NAME ## __ ## TEST_NAME, \
		.state = 0, \
		.number_of_tests = 0, \
		.next = NULL \
	}; \
	void __attribute__((constructor)) _register__ ## TEST_CASE_NAME ## __ ## TEST_NAME () { \
		struct ctester_test_case_list_t *test = & ctester_test_case_info_ ## TEST_CASE_NAME ## __ ## TEST_NAME; \
		struct ctester_test_case_list_t iter_root = { \
			.full_test_name = "", .test_case_name = "", .test_name = "", .test_body = NULL, .state = 0, \
			.number_of_tests = 0, .next = ctester_test_root \
		}; \
		struct ctester_test_case_list_t *iter = &iter_root; \
		while(iter->next && strcmp(iter->next->full_test_name, test->full_test_name) < 0) { \
			iter = iter->next; \
		} \
		if(iter->next) { \
			test->next = iter->next; \
		} \
		iter->next = test; \
		if(iter == &iter_root) { \
			ctester_test_root = test; \
		} \
	} \
	void TEST_CASE_NAME ## __ ## TEST_NAME (struct ctester_test_case_state_t *ctester_state) \

/// @}

/**
 * \defgroup flow_control Test flow control
 * @{
 */

/// Explicitly fail a test
#define _CTESTER_MSG0(...) _ctester_nop()
#define _CTESTER_MSGG(...) fprintf(stderr, _CTESTER_INDENT "    Message: " __VA_ARGS__); fprintf(stderr, "\n");
#define _CTESTER_MSGD(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, NAME, ...) NAME
#define _CTESTER_MSG(...) _CTESTER_MSGD(_0, ## __VA_ARGS__, _CTESTER_MSGG, _CTESTER_MSGG, _CTESTER_MSGG, _CTESTER_MSGG, _CTESTER_MSGG, _CTESTER_MSGG, _CTESTER_MSGG, _CTESTER_MSGG, _CTESTER_MSGG, _CTESTER_MSGG, _CTESTER_MSG0)(__VA_ARGS__)

#define FAIL(...) \
	fprintf(stderr, _CTESTER_INDENT "%s (%s:%d): FAIL() called\n", __func__, __FILE__, __LINE__); \
	_CTESTER_MSG(__VA_ARGS__); \
	ctester_state->failed = __LINE__; \
	return

/// Explicitly add a warning
#define ADD_FAILURE(...) \
	fprintf(stderr, _CTESTER_INDENT "%s (%s:%d): ADD_FAILURE() called\n", __func__, __FILE__, __LINE__); \
	_CTESTER_MSG(__VA_ARGS__); \
	ctester_state->warning++; \
	return

/// @}
