TODO
----

- 'h' show help

- create homepage on <http://llg.cubic.org>
- test on FreeBSD
- test on OsX
- test on Windows
- better parsing of display filter, correctly separate /regex/replace/ by the middle slash
- read tab width from vim/emacs comments
- cache file index
- cache regex filter index
- when starting, only parse the first screen height lines from the file and display them
 + then parse the remaining lines in a background thread
- support more than 4GB lines?
 + manage all line numbers as 64bit unsigned
- split realmain.cc into components

- color
 + developed on the "color" branch
 + decide if it's worth to merge, since curses color sucks so much
