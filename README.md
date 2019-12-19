# ctester

This is a small test framework for GNU C running on POSIX platforms with an
interface resembling GTest.

The use restriction is hard, the code uses both POSIX and GNU C features. It is
compatible with GCC and CLANG.

See `test.c` for documentation and usage information. You may also run Doxygen
on the source tree to generate rudimentary documentation.

## How to use
```c
#include <ctester.h>
```
and link against `ctester.o`.

Then write tests like this:

```c
TEST(FactorialTest, Negative) {
	EXPECT_EQ(1, 1);
	EXPECT_GT(1, 0);
	ASSERT_LE(0, 1);
	ASSERT_TRUE(1 == 1);
	ASSERT_DOUBLE_NE(1., 1. + 100 * DBL_EPSILON);
	ASSERT_DEATH(*((int*)NULL) = 1);
	ASSERT_EXIT(exit(0), 0);
	ASSERT_EQ(1, 0 + 3*17);
}
```

This will output:

```text
[----------] 1 test from TestSuite
[ RUN      ] TestSuite.TestName
     a.c:9: Child crashed while running *((int*)((void *)0)) = 1 with signal 11.
     a.c:11: Failure.
         Expected: 1 == 0 + 3*17 but
         1 == 1,
         0 + 3*17 == 51
[  FAILED  ] TestSuite.TestName (149 ms total)
[----------] 1 test from TestSuite (149 ms total)
[==========] 1 test from 1 test case ran. (149 ms total)
[  FAILED  ] 1 test, listed below:
[  FAILED  ] TestSuite.TestName
```


See `ctester-test.c` for more examples.

## Known bugs
GCC might complain about missing functions if compiling with `-O0`. Try compiling with optimizations.
