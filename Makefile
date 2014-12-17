WARNINGS_FLAGS += -Wall -Werror
INCLUDE_FLAGS += -I.

CXXFLAGS += $(WARNING_FLAGS) $(INCLUDE_FLAGS)

all:	run_test

TEST_SRCS = $(shell find . -name '*_gtest.cc') gtest/gtest-all.cc gtest/gtest_main.cc
TEST_OBJS = $(TEST_SRCS:.cc=.o)

test:	$(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

run_test:	test
	./$<

debug_test:	test
	gdb $<

clean:
	rm -f *~ *.o test
