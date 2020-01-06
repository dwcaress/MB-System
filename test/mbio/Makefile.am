AM_CPPFLAGS = -I$(top_srcdir)/third_party/googletest/include -I$(top_srcdir)/third_party/googlemock/include -I$(top_srcdir)/src -isystem $(GTEST_CPPFLAGS)
AM_CXXFLAGS = $(GTEST_CXXFLAGS)
AM_LDFLAGS = $(GTEST_LDFLAGS) $(GTEST_LIBS)
AM_LDFLAGS += $(top_builddir)/src/mbio/libmbio.la
AM_LDFLAGS += $(top_builddir)/third_party/googletest/lib/libgtest_main.la
AM_LDFLAGS += $(top_builddir)/third_party/googletest/lib/libgtest.la
AM_LDFLAGS += -lpthread

# if HAVE_PTHREADS
#  AM_CXXFLAGS += @PTHREAD_CFLAGS@ -DGTEST_HAS_PTHREAD=1
#  AM_LIBS += @PTHREAD_LIBS@
# else
  AM_CXXFLAGS += -DGTEST_HAS_PTHREAD=0
# endif

# TESTS -- Programs run automatically by "make check"
# check_PROGRAMS -- Programs built by "make check" but not necessarily run
TESTS =
check_PROGRAMS =

TESTS += mb_format_test
check_PROGRAMS += mb_format_test
mb_format_test_SOURCES = mb_format_test.cc

TESTS += mb_read_init_test
check_PROGRAMS += mb_read_init_test
mb_read_init_test_SOURCES = mb_read_init_test.cc

TESTS += mb_time_test
check_PROGRAMS += mb_time_test
mb_time_test_SOURCES = mb_time_test.cc
# mb_time_test_LDADD = $(top_builddir)/src/func.o
