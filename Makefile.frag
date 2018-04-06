jitfu-test-coverage:
	CCACHE_DISABLE=1 EXTRA_CFLAGS="-fprofile-arcs -ftest-coverage" TEST_PHP_ARGS="-q" $(MAKE) clean test

jitfu-test-coverage-lcov: jitfu-test-coverage
	lcov -c --directory $(top_srcdir)/bits/.libs --output-file $(top_srcdir)/coverage.info

jitfu-test-coverage-html: jitfu-test-coverage-lcov
	genhtml $(top_srcdir)/coverage.info --output-directory=$(top_srcdir)/html

jitfu-test-coverage-travis:
	CCACHE_DISABLE=1 EXTRA_CFLAGS="-fprofile-arcs -ftest-coverage" $(MAKE)
