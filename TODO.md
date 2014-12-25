TODO
----

- search
 + 'n' goto next search result
 + 'N' goto prev search result

- 'h' show help

- support UTF8
 + currently developed on the "utf8" git branch
 + decide when to merge it to master

- use color
- create homepage on <http://llg.cubic.org>
- program exit may be slow if C++ needs to clean up a lot of objects
- better parsing of display filter, correctly separate /regex/replace/ by the middle slash
- read tab width from vim/emacs comments
- cache file index
- cache regex filter index
- check if other implementations of std::set<line_number_t> have better memory use
- when adding a regex, let it run in a background thread and apply once done
- when starting, only parse the first screen height lines from the file and display them
 + then parse the remaining lines in a background thread
- support more than 4GB lines?
 + manage all line numbers as 64bit unsigned
