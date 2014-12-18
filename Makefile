WARNING_FLAGS += -Wall -Werror
INCLUDE_FLAGS += -I.

CXXFLAGS += $(WARNING_FLAGS) $(INCLUDE_FLAGS) -std=c++11 -MMD -g

SRCS = realmain.cc memorymap.cc file_index.cc regex_index.cc
OBJS = $(SRCS:.cc=.o)

all:	run_test fewer

fewer:	$(OBJS) main.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

TEST_SRCS = $(shell find . -name '*_gtest.cc') gtest/gtest-all.cc gtest/gtest_main.cc $(SRCS)
TEST_OBJS = $(TEST_SRCS:.cc=.o)
TEST_DEPS = $(TEST_SRCS:.cc=.d)

test:	$(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

run_test:	test
	./$<

debug_test:	test
	gdb $<

clean:
	rm -f *~ test fewer $(TEST_DEPS) $(TEST_OBJS) *.o *.d

-include $(TEST_DEPS)
