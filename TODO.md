TODO
----

- create homepage on <http://llg.cubic.org>
- test on OsX
- test on Windows
 + fix to_wide() with invalid characters
 + add some more to_wide() tests
- better parsing of display filter, correctly separate /regex/replace/ by the middle slash
- read tab width from vim/emacs comments
- cache file index
- cache regex filter index
- when starting, only parse the first screen height lines from the file and display them
 + then parse the remaining lines in a background thread
- support more than 4GB lines?
 + manage all line numbers as 64bit unsigned
- split realmain.cc into components
- maybe search in background? However while searching in background the user can modify the display_info object which will invalidate the iterators that the background search would use.
- detect emails
- parse default command line arguments from FEWOPTIONS environment variable
- when reading STDIN, don't use temp file but store in memory
 + parse the regular expressions upon reading from STDIN
