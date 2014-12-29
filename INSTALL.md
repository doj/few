Build and Install few
=====================

few is written in C++ and requires a compiler with a good support of
the C++11 standard. It has been tested with gcc 4.9 on Linux, clang
3.4 on FreeBSD and Visual Studio 2013 on Windows.

Build on Unix
-------------

The Unix Makefiles are written in the _GNU make_ flavor. On Linux GNU
make is the default make program and should be installed by default if
you install compilers. On FreeBSD you have to install the _gmake_
program. Generally a simple call of `make` or `gmake` should compile
the software. With `make install` you can install the program and its
man page into `/usr/local/`. If you prefer a different location look
at the `PREFIX` variable in the Makefile.

Build on a Debian (derived) Linux
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use `make debian-setup` to install the pre requisites to build
the few program.

Build on a Redhat (derived) Linux
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use `make redhat-setup` to install the pre requisites to build
the few program.

Build on Windows
----------------

You'll find a Visual Studio project and solution file in the source
code directory. You can load the solution into Visual Studio and build
the few program. On Windows few uses the
[PDCurses](http://pdcurses.sourceforge.net/) library. The project
currently uses the pre-build PDCurses library. If you want to use the
build few.exe file, you'll have to copy the XXX.dll to a directory
included in your PATH.
