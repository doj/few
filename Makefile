# GNU Makefile for the few program

##############################################################################
# Compiler Flags

WARNING_FLAGS += -Wall
INCLUDE_FLAGS += -I. -Iunix

ifeq ($(DEBUG),1)
CXXFLAGS += -g
else
CXXFLAGS += -O3
endif

ifeq ($(shell uname),FreeBSD)
CXX := c++ # FreeBSD's clang++ typically
else
WARNING_FLAGS += -Werror
endif

ifeq ($(shell uname),Darwin)
LIBS += -lcurses
else
LIBS += -lncursesw
endif

ifeq ($(USE_CLANG),1)
CXX := clang++
CXXFLAGS += -stdlib=libc++
LDFLAGS += -stdlib=libc++
endif

ifneq ($(SYSROOT),)
CXX := $(SYSROOT)/bin/$(CXX)
INCLUDE_FLAGS += -idirafter /usr/include
LDFLAGS += -Wl,-rpath,$(SYSROOT)/lib64
export LD_LIBRARY_PATH += $(SYSROOT)/lib
endif

CXXFLAGS += $(WARNING_FLAGS) $(INCLUDE_FLAGS) -std=c++11 -MMD

##############################################################################
# source code

SRCS := $(shell find . -name '*.cc' -and -not -name '*_gtest.cc')
OBJS := $(SRCS:.cc=.o)

##############################################################################
# Linker Flags

LIBS += -lpthread

##############################################################################
# installation

PREFIX ?= /usr/local

##############################################################################
# build and debug the few program

all:	run_test few TAGS

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
	chmod 644 $@

few.html:	few.md
	$(RONN) --html $<

%.E:	%.cc
	$(CXX) $(CXXFLAGS) -E -o $@ $<

##############################################################################
# test the program

TEST_SRCS := $(shell find . -name '*_gtest.cc') $(SRCS)
TEST_OBJS := $(TEST_SRCS:.cc=.o)
TEST_DEPS := $(TEST_SRCS:.cc=.d)

test:	$(TEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

run_test:	test
	./$<

ifeq ($(DEBUG),1)
debug_test:	test
	gdb $<
else
debug_test:	clean
	$(MAKE) $@ DEBUG=1
endif

-include $(TEST_DEPS)

##############################################################################
# misc targets

install:	all
	install -D -m 755 few $(DESTDIR)/$(PREFIX)/bin/few
# because generation of few.1 will overwrite the few executable, we can not list few.1 as a dependency for install:
# call another make process to generate and install the man page
	$(MAKE) install_man

install_man:	few.1
	install -D -m 644 few.1 $(DESTDIR)/$(PREFIX)/man/man1/few.1

TODAY=$(shell date '+%Y%m%d')
dist:	few.1 few.html
	$(MAKE) clean
	rm -f /tmp/few-$(TODAY) few-$(TODAY).tar.gz .emacs.desktop
	ln -s "$(PWD)" /tmp/few-$(TODAY)
	cd /tmp ; tar c --exclude-vcs --exclude-backups --dereference few-$(TODAY) | gzip -9 > /tmp/few-$(TODAY).tar.gz
	rm /tmp/few-$(TODAY)
	mv /tmp/few-$(TODAY).tar.gz "$(PWD)/"

TAGS:
	etags $(SRCS)

clean:
	rm -f test few few.md few.tar.gz TAGS
	-find . -name '*~' -or -name '*.o' -or -name '*.d'  -or -name '*.E' | xargs rm

distclean:	clean
	rm -f few.1 few.html few-$(TODAY).tar.gz

show-log:
	git log --oneline --abbrev-commit --all --graph

##############################################################################
# install packages to build the program

redhat-setup:
	yum install -y gcc gcc-c++ gdb ncurses-devel rubygem-ronn

debian-setup:
	apt-get install -y ncurses-doc ruby-ronn libncursesw5-dev

emerge:
	emerge --ask app-text/ronn sys-libs/ncurses
