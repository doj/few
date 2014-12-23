WARNING_FLAGS += -Wall -Werror
INCLUDE_FLAGS += -I.

CXXFLAGS += $(WARNING_FLAGS) $(INCLUDE_FLAGS) -std=c++11 -MMD

ifeq ($(DEBUG),1)
CXXFLAGS += -g #-O2
else
CXXFLAGS += -O2
endif

SRCS = realmain.cc memorymap.cc file_index.cc regex_index.cc display_info.cc help.cc normalize_regex.cc progress_functor.cc
OBJS = $(SRCS:.cc=.o)

LIBS = -lncurses -lpthread

all:	run_test fewer

fewer:	$(OBJS) main.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

debug:	fewer
ifeq ($(DEBUG),1)
	gdb --args ./fewer /tmp/medium.txt
else
	$(MAKE) clean
	$(MAKE) $@ DEBUG=1
endif

fewer.1:	README.md
	cp -f $< fewer.md
	ronn --roff --organization=Cubic fewer.md
	rm -f fewer.md
	mv -f fewer $@

dist:	clean fewer.1
	echo 'TODO: create .tar.gz of source code'

TEST_SRCS = $(shell find . -name '*_gtest.cc') gtest/gtest-all.cc gtest/gtest_main.cc $(SRCS)
TEST_OBJS = $(TEST_SRCS:.cc=.o)
TEST_DEPS = $(TEST_SRCS:.cc=.d)

test:	$(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

run_test:	test
	./$<

debug_test:	test
ifeq ($(DEBUG),1)
	gdb $<
else
	$(MAKE) clean
	$(MAKE) $@ DEBUG=1
endif

clean:
	rm -f *~ test fewer $(TEST_DEPS) $(TEST_OBJS) *.o *.d

-include $(TEST_DEPS)

redhat-setup:
	yum install -y gcc gcc-c++ gdb ncurses-devel

debian-setup:
	apt-get install -y ncurses-doc ruby-ronn
