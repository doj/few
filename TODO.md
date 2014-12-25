TODO
----

- search
 + 'n' goto next search result
 + 'N' goto prev search result

- 'h' show help

- upon the initial parse of the file, have all preconfigured regex filters work on them
- when starting, only parse the first screen height lines from the file and display them
 + then parse the remaining lines in a background thread
- when adding a regex, let it run in a background thread and apply once done

- support UTF8
 + currently developed on the "utf8" git branch
 + decide when to merge it to master

- use color
- read tab width from vim/emacs comments
- create homepage on <http://llg.cubic.org>
- better parsing of display filter, correctly separate /regex/replace/ by the middle slash
- support more than 4GB lines?
 + manage all line numbers as 64bit unsigned
- cache file index
- cache regex filter index