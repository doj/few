WARNING_FLAGS += -Wall -Werror
INCLUDE_FLAGS += -I.

CXXFLAGS += $(WARNING_FLAGS) $(INCLUDE_FLAGS) -std=c++11 -MMD

ifeq ($(DEBUG),1)
CXXFLAGS += -g #-O2
else
CXXFLAGS += -O2
endif

SRCS = \
	display_info.cc \
	file_index.cc \
	help.cc \
	history.cc \
	memorymap.cc \
	normalize_regex.cc \
	progress_functor.cc \
	realmain.cc \
	regex_index.cc \
#
OBJS = $(SRCS:.cc=.o)

LIBS = -lncurses -lpthread

all:	run_test few

few:	$(OBJS) main.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

debug:	few
ifeq ($(DEBUG),1)
	gdb --args ./few realmain.cc
else
	$(MAKE) clean
	$(MAKE) $@ DEBUG=1
endif

few.1:	README.md
	cp -f $< few.md
	ronn --roff --organization=Cubic few.md
	rm -f few.md
	mv -f few $@

dist:	all few.1
	$(MAKE) clean
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
	rm -f *~ test few $(TEST_DEPS) $(TEST_OBJS) *.o *.d

-include $(TEST_DEPS)

redhat-setup:
	yum install -y gcc gcc-c++ gdb ncurses-devel rubygem-ronn

debian-setup:
	apt-get install -y ncurses-doc ruby-ronn
