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

LIBS = -lncurses -lpthread

##############################################################################
# build and debug the few program

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

RONN=ronn --organization=Cubic --manual="User Commands"

few.1:	README.md
	cp -f $< fewR.md
	$(RONN) --roff fewR.md
	rm -f fewR.md
	mv -f few $@

few.html:	README.md
	cp -f $< fewH.md
	$(RONN) --html fewH.md
	rm -f fewH.md
	mv -f fewH.html $@

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
	rm -f *~ test few few.1 few.html $(TEST_DEPS) $(TEST_OBJS) *.o *.d

show-log:
	git log --oneline --abbrev-commit --all --graph

##############################################################################
# install packages to build the program

redhat-setup:
	yum install -y gcc gcc-c++ gdb ncurses-devel rubygem-ronn

debian-setup:
	apt-get install -y ncurses-doc ruby-ronn
