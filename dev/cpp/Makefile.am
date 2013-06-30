AM_CXXFLAGS = -Wall -Os

bin_PROGRAMS = test

test_SOURCES = test.cpp

LINK_FLAGS = $(AM_CXXFLAGS) -I$(srcdir)/../src -L$(srcdir)/../src -lblipkit

test_CXXFLAGS = $(LINK_FLAGS)
