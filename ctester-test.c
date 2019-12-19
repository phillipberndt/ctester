#include "ctester.h"

/**
 * \defgroup example Example use of ctester
 * @{
 */

int Factorial(int n) {
  int result = 1;
  for (int i = 1; i <= n; i++) {
    result *= i;
  }

  return result;
}

TEST(FactorialTest, Negative) {
  EXPECT_EQ(1, Factorial(-5));
  EXPECT_EQ(1, Factorial(-1));
  EXPECT_GT(Factorial(-10), 0);
}

TEST(FactorialTest, Zero) {
  EXPECT_EQ(1, Factorial(0));
}

TEST(FactorialTest, Positive) {
  EXPECT_EQ(1, Factorial(1));
  EXPECT_EQ(2, Factorial(2));
  EXPECT_EQ(6, Factorial(3));
  EXPECT_EQ(40320, Factorial(8));
}

TEST(FactorialTest, FlawedTest) {
	EXPECT_EQ(0, Factorial(1), "This test will fail! Since we do not have stream i/o in C, this uses printf syntax. E.g.: %d", 0); // EXPECT_FAILURE
}

/// @}



/**
 * \defgroup ctester_self_test Self test & advanced features
 * @{
 *
 * The EXPECT_FAILURE comments used here are evaluated in the self-check
 * included in the Makefile.
 */

TEST(AssertionMacros, Eq) {
	ASSERT_EQ(1, 1);
	ASSERT_EQ(1.5, 1.5);
	ASSERT_EQ(-1, -1);
	ASSERT_EQ(1.5f, 1.5f);

	EXPECT_EQ(1, 2); // EXPECT_FAILURE
	EXPECT_EQ(1.5, 2.5); // EXPECT_FAILURE
	EXPECT_EQ(-1, -2); // EXPECT_FAILURE
	EXPECT_EQ(1.5f, 2.5f); // EXPECT_FAILURE
}

TEST(AssertionMacros, DISABLED_EqFail) {
	ASSERT_EQ(1, 2); // EXPECT_FAILURE
}

TEST(AssertionMacros, Ne) {
	ASSERT_NE(1, 2);
	ASSERT_NE(1.5, 2.5);
	ASSERT_NE(-1, -2);
	ASSERT_NE(1.5f, 2.5f);

	EXPECT_NE(1, 1); // EXPECT_FAILURE
	EXPECT_NE(1.5, 1.5); // EXPECT_FAILURE
	EXPECT_NE(-1, -1); // EXPECT_FAILURE
	EXPECT_NE(1.5f, 1.5f); // EXPECT_FAILURE
}

TEST(AssertionMacros, Gt) {
	ASSERT_GT(1, 0);
	ASSERT_GT(1.5, 1.0);
	ASSERT_GT(-1, -2);
	ASSERT_GT(1.5f, 1.0f);

	EXPECT_GT(1, 1); // EXPECT_FAILURE
	EXPECT_GT(1, 2); // EXPECT_FAILURE
	EXPECT_GT(1.5, 2.0); // EXPECT_FAILURE
	EXPECT_GT(-1, 1); // EXPECT_FAILURE
	EXPECT_GT(1.5f, 2.0f); // EXPECT_FAILURE
}

TEST(AssertionMacros, Lt) {
	ASSERT_LT(0, 1);
	ASSERT_LT(1.0, 1.5);
	ASSERT_LT(-2, -1);
	ASSERT_LT(1.0f, 1.5f);

	EXPECT_LT(2, 2); // EXPECT_FAILURE
	EXPECT_LT(2, 1); // EXPECT_FAILURE
	EXPECT_LT(2.0, 1.5); // EXPECT_FAILURE
	EXPECT_LT(1, -1); // EXPECT_FAILURE
	EXPECT_LT(2.0f, 1.5f); // EXPECT_FAILURE
}

TEST(AssertionMacros, Le) {
	ASSERT_LE(0, 1);
	ASSERT_LE(1.0, 1.5);
	ASSERT_LE(-2, -1);
	ASSERT_LE(1.0f, 1.5f);

	EXPECT_LE(2, 1); // EXPECT_FAILURE
	EXPECT_LE(2.0, 1.5); // EXPECT_FAILURE
	EXPECT_LE(1, -1); // EXPECT_FAILURE
	EXPECT_LE(2.0f, 1.5f); // EXPECT_FAILURE
}

TEST(AssertionMacros, Ge) {
	ASSERT_GE(1, 0);
	ASSERT_GE(1, 1);
	ASSERT_GE(1.5, 1.0);
	ASSERT_GE(1.5, 1.5);
	ASSERT_GE(-1, -2);
	ASSERT_GE(-1, -1);
	ASSERT_GE(1.5f, 1.0f);
	ASSERT_GE(1.5f, 1.5f);

	EXPECT_GE(1, 2); // EXPECT_FAILURE
	EXPECT_GE(1.5, 2.0); // EXPECT_FAILURE
	EXPECT_GE(-1, 1); // EXPECT_FAILURE
	EXPECT_GE(1.5f, 2.0f); // EXPECT_FAILURE
}

TEST(AssertionMacros, True) {
	ASSERT_TRUE(1 == 1);
	EXPECT_TRUE(1 == 2); // EXPECT_FAILURE
}

TEST(AssertionMacros, FloatEq) {
	ASSERT_FLOAT_EQ(1.f, 1.f + FLT_EPSILON);
	EXPECT_FLOAT_EQ(1.f, 1.f + 100 * FLT_EPSILON); // EXPECT_FAILURE
}

TEST(AssertionMacros, FloatNe) {
	ASSERT_FLOAT_NE(1.f, 1.f + 100 * FLT_EPSILON);
	EXPECT_FLOAT_NE(1.f, 1.f + FLT_EPSILON); // EXPECT_FAILURE
}

TEST(AssertionMacros, DoubleEq) {
	ASSERT_DOUBLE_EQ(1., 1. + DBL_EPSILON);
	EXPECT_DOUBLE_EQ(1., 1. + 100 * DBL_EPSILON); // EXPECT_FAILURE
}

TEST(AssertionMacros, DoubleNe) {
	ASSERT_DOUBLE_NE(1., 1. + 100 * DBL_EPSILON);
	EXPECT_DOUBLE_NE(1., 1. + DBL_EPSILON); // EXPECT_FAILURE
}

TEST(AssertionMacros, StrEq) {
	ASSERT_STREQ("foo", "foo");
	EXPECT_STREQ("foo", "bar"); // EXPECT_FAILURE
}

TEST(AssertionMacros, StrNe) {
	ASSERT_STRNE("foo", "bar");
	EXPECT_STRNE("foo", "foo"); // EXPECT_FAILURE
}

TEST(AssertionMacros, StrCaseEq) {
	ASSERT_STRCASEEQ("Foo", "foo");
	EXPECT_STRCASEEQ("Foo", "bar"); // EXPECT_FAILURE
}

TEST(AssertionMacros, StrCaseNe) {
	ASSERT_STRCASENE("Foo", "bar");
	EXPECT_STRCASENE("Foo", "foo"); // EXPECT_FAILURE
}

void crashes_me() {
	*((volatile int *)NULL) = 1;
	abort();
}

void does_not_crash_me() {
	printf("Hi!\n");
	exit(0);
}

void does_nothing() {
}

TEST(AssertionMacros, AssertDeath) {
	ASSERT_DEATH(crashes_me());
	EXPECT_DEATH(does_not_crash_me()); // EXPECT_FAILURE
}

TEST(AssertionMacros, AssertExit) {
	ASSERT_EXIT(does_not_crash_me(), 0);
	EXPECT_EXIT(crashes_me(), 0); // EXPECT_FAILURE
	EXPECT_EXIT(does_nothing(), 0); // EXPECT_FAILURE
}

TEST(TestFlow, AddFailure) {
	ADD_FAILURE();
}

/// @}
