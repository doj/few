TODO
----

- search
 + 'n' goto next search result
 + 'N' goto prev search result

- 'h' show help

- color
 + developed on the "color" branch
 + decide if it's worth to merge, since curses color sucks so much

- create homepage on <http://llg.cubic.org>
- program exit may be slow if C++ needs to clean up a lot of objects
- better parsing of display filter, correctly separate /regex/replace/ by the middle slash
- read tab width from vim/emacs comments
- cache file index
- cache regex filter index
- check if other implementations of std::set<line_number_t> have better memory use
- when starting, only parse the first screen height lines from the file and display them
 + then parse the remaining lines in a background thread
- support more than 4GB lines?
 + manage all line numbers as 64bit unsigned
