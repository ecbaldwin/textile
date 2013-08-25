noinst_LIBRARIES = libgtest.a
noinst_PROGRAMS = test
TESTS = test

GTEST_DIR = $(srcdir)/gtest-1.6.0

AM_CPPFLAGS = \
	-I$(top_srcdir)/lib \
	-I$(GTEST_DIR)/include \
	-I$(GTEST_DIR)

noinst_HEADERS = \
	$(GTEST_DIR)/include/gtest/*.h \
	$(GTEST_DIR)/include/gtest/internal/*.h \
	$(GTEST_DIR)/src/*

dist_noinst_DATA = data

libgtest_a_SOURCES = $(GTEST_DIR)/src/gtest-all.cc

test_LDADD = \
	$(builddir)/libgtest.a \
	$(top_builddir)/lib/libtextile.la \
	-lpthread

test_SOURCES = \
	test.cc
