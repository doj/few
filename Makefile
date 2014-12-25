# Makefile for the few program

##############################################################################
# Compiler Flags

WARNING_FLAGS += -Wall -Werror
INCLUDE_FLAGS += -I.

CXXFLAGS += $(WARNING_FLAGS) $(INCLUDE_FLAGS) -std=c++11 -MMD

ifeq ($(DEBUG),1)
CXXFLAGS += -g #-O2
else
CXXFLAGS += -O2
endif

##############################################################################
# source code

SRCS := $(shell find -name '*.cc' -and -not -name '*_gtest.cc')
OBJS := $(SRCS:.cc=.o)

##############################################################################
# Linker Flags

LIBS = -lncursesw -lpthread

##############################################################################
# build and debug the few program

all:	run_test few

few:	$(OBJS) main.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

debug:	few
ifeq ($(DEBUG),1)
	gdb --args ./few UTF-8-demo.txt
else
	$(MAKE) clean
	$(MAKE) $@ DEBUG=1
endif

RONN=ronn --organization=Cubic --manual="User Commands"

few.md:	README.md TODO.md
	cat $^ > $@

few.1:	few.md
	$(RONN) --roff $<
	mv -f few $@

few.html:	few.md
	$(RONN) --html $<

##############################################################################
# test the program

TEST_SRCS := $(shell find . -name '*_gtest.cc') $(SRCS)
TEST_OBJS := $(TEST_SRCS:.cc=.o)
TEST_DEPS := $(TEST_SRCS:.cc=.d)

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

-include $(TEST_DEPS)

##############################################################################
# misc targets

dist:	all few.1 few.html
	$(MAKE) clean
	echo 'TODO: create .tar.gz of source code'

clean:
	rm -f *~ test few $(TEST_DEPS) $(TEST_OBJS) *.o *.d few.md

distclean:	clean
	rm -f few.1 few.html

show-log:
	git log --oneline --abbrev-commit --all --graph

##############################################################################
# install packages to build the program

redhat-setup:
	yum install -y gcc gcc-c++ gdb ncurses-devel rubygem-ronn

debian-setup:
	apt-get install -y ncurses-doc ruby-ronn libncursesw5-dev
