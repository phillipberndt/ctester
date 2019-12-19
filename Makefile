CFLAGS=-fPIC -g -O -std=c11 -Wall -Wextra

.PHONY: test clean all

all: test

ctester-test: ctester-test.o ctester.o

ctester-test.o: ctester-test.c ctester.h
	$(CC) -c $(CFLAGS) -o $@ $<

ctester.o: ctester.c ctester.h
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f *.o ctester-test

test: ctester-test
	# ctester self-test
	# =================
	# This is a bit elaborate because we want to test expected failures, too.
	# The code extracts all test cases and tests from the binary, then counts the number of
	# "EXPECT_FAILURE"s in the source code of the associated test (assuming all tests start
	# with TEST(foo, bar) { and end with } on its own line), then runs each test individually
	# and checks whether the number of failures adds up.
	# Finally, it runs the test itself, which will return 0 only if all tests passed.
	./ctester-test -l | tr '.' ' ' | while read TEST_CASE TEST; do \
		EXPECT_FAILS=$$(sed -ne '/TEST('$$TEST_CASE'\s*,\s*'$$TEST')/b intest; d; : intest; /^}$$/d; /EXPECT_FAILURE/p; n; b intest;' ctester-test.c | wc -l); \
		ACTUAL_FAILS=$$(./ctester-test -t $$TEST_CASE.$$TEST 2>&1 | grep ": Failure." | wc -l); \
		if [ "$$EXPECT_FAILS" -ne "$$ACTUAL_FAILS" ]; then \
			echo "Expected $$EXPECT_FAILS soft failures in the following test, but got $$ACTUAL_FAILS:"; \
			echo ./ctester-test -t $$TEST_CASE.$$TEST; \
			./ctester-test -t $$TEST_CASE.$$TEST; \
			exit 1; \
		fi \
	done; \
	./ctester-test >/dev/null 2>&1
