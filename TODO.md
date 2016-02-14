TODO
----

- print position in file as percentage in upper right corner
- test on OsX
- Windows
 + check http://www.chiark.greenend.org.uk/~sgtatham/doit/ to launch browser on Windows
 + somehow create an installer or at least a batch file to copy the .exe and .dll somewhere
 + check http://www.projectpluto.com/win32a.htm
- maybe use readline for input?
 + http://tiswww.case.edu/php/chet/readline/readline.html#SEC41
 + https://github.com/ulfalizer/readline-and-ncurses/blob/master/rlncurses.c
- read tab width from vim/emacs comments
- cache file index
- cache regex filter index
- when starting, only parse the first screen height lines from the file and display them
 + then parse the remaining lines in a background thread
- support more than 4GB lines?
 + manage all line numbers as 64bit unsigned
- split realmain.cc into components
- maybe search in background? However while searching in background the user can modify the display_info object which will invalidate the iterators that the background search would use.
- support hidden filters
- when pressing 'G' key, check if file grew and (re)load new part
