WARNINGS_FLAGS += -Wall -Werror
INCLUDE_FLAGS += -I.

CXXFLAGS += $(WARNING_FLAGS) $(INCLUDE_FLAGS) -std=c++11

SRCS = realmain.cc memorymap.cc
OBJS = $(SRCS:.cc=.o)

all:	run_test fewer

fewer:	$(OBJS) main.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

TEST_SRCS = $(shell find . -name '*_gtest.cc') gtest/gtest-all.cc gtest/gtest_main.cc
TEST_OBJS = $(TEST_SRCS:.cc=.o)

test:	$(TEST_OBJS) $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

run_test:	test
	./$<

debug_test:	test
	gdb $<

clean:
	rm -f *~ *.o test fewer
