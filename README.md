few(1)
========

NAME
----
few - a file viewer with regular expression filters.

SYNOPSIS
--------
**few** [--regex '/REGEX/flags']\* [--df '/REGEX/REPLACE/flags']\* [--search '/REGEX/flags'] [--tabwidth 'NUM'] [--goto 'NUM'] [-v] [--color] [-h|-?|--help] 'FILE'

DESCRIPTION
-----------
few is a program to interactively filter log files with regular expressions and view the result.
It is similar to the less(1) program, but has the ability to filter what is displayed with regular expressions.

OPTIONS
-------
* **--regex** '/REGEX/flags':
  preset regular expressions to filter the file.

* **--df** '/REGEX/REPLACE/flags':
  preset a display filter regular expression and replace text.

* **--search** '/REGEX/flags':
  preset search regular expression.

* **--tabwidth** 'NUM':
  set the width of a tab character in spaces.

* **--goto** 'NUM':
  go to line number NUM. If NUM is not included in the currently filtered lines, go to the previous filtered line.

* **-v**:
  increase verbosity for certain operations.

* **--color**:
  enable color if the terminal supports color.

* **-h**, **-?**, **--help**:
  show help text.

Keys
----
The actions resulting from a key press are modelled after the less(1) program.

* **q**, **Q**:
  quit the program..
* **cursor down**:
  scroll down one line.
* **n**:
  scroll down one line if no search regex is set.
* **cursor up**, **p**:
  scroll up one line.
* **space**, **page down**:
  scroll down one page.
* **b**, **page up**:
  scroll up one page.
* **g**, **<**, **home**:
  go to to first line.
* **G**, **>**, **end**:
  go to last line.
* **1** .. **9**:
  edit filter regular expression.
* **F1** .. **F9**:
  edit display filter regular expression.
* **d**:
  scroll down half a screen
* **u**:
  scroll up half a screen
* **P**:
  goto line
* **%**:
  goto percentage of lines
* **R**:
  repaint the screen
* **h**:
  show help text. This will show the few man page in the few editor. To return to your file, press **q**.
* **/**:
  enter a search regular expression
* **n**:
  go to next search match if a search regex is set.
* **N**:
  go to previous search match.

Regular Expressions
-------------------
For filters (key **0** .. **9**), display filters (key **F1**
.. **F9**) and search (key **/**) ECMAScript (javascript) style
regular expressions, as provided by the C++11 regular expression
standard library are used. You will typically enter the regular
expression after the few program has started and will see the file
display change if the regular expression matches. You can also preset
these regular expressions with the command line arguments.

The standard form of these regular expression has the following format:

/regex/flags

The regular expression is surrounded by two forward slash characters.
The flags are optional.

See <http://www.cplusplus.com/reference/regex/ECMAScript/> for details
on the regular expression syntax. It is similar to perl regular
expressions.

### Regular Expression Flags
The regular expression can be modified with optional flag characters,
the following characters are currently supported:

* **i**:
  case insensitive match
* **!**:
  negative match

By default the regular expression is a positive match. Only lines in
the file that (partially) match the regular expression will be
displayed. If you use the '!' flag the regular expression becomes a
negative match. Only lines that do *not* match the regular expression
are displayed.

### Regular Expression Short Form
If you do not need to supply flags for the regular expression and the
regular expression does not start with a forward slash, you can omit
the surrounding slashes.

If you want a negative match and your regular expression does not
start with a '!' character and you don't need additional flags, you
can start your regular expression with a '!' character.

The few program will convert the short forms to the regular form.

### Display Filter Regular Expressions

Display Filter change the way the lines are displayed. They take the
form of a perl regular expression substitute:

/regex/replace/flags

Because the parsing of this format is currently naive, the strings
_regex_ and _replace_ may not contain any forward slash characters.
The display filter allows the optional 'i' flag to indicate case
insensitive matching.  The replace string can contain back references
to the regular expression. The following back references or available:

* **$1** .. **$99**:
  numbered back reference to a match group.
* **$&**:
  the entire match.
* **$`**:
  prefix to the match.
* **$'**:
  suffix to the match.
* **$$**:
  a single $ character.

See <http://www.cplusplus.com/reference/regex/regex_replace/> for
details on how the C++ regex library is matching and replaceing.

### Regular Expressions and wide characters

The few program is able to display wide characters and files encoded
in UTF-8. However because it is designed to work with large files, it
will not attempt to match the regular expressions on the wide
character representations of the file processed. The (display) filter
regular expression is matched on the raw byte sequence of the
file. This is usually not a problem if you match ASCII characters.

The search regular expression however is matched against the wide
character representation of the displayed lines.

Links and URLs
--------------

few will attempt to detect URLs and links. If it detects a link, URL
or email address few will underline the match. You can click the
underlined text to have the link opened in a web browser or email
client. On Unix you need to set the BROWSER environment variable if
you don't like the default web browser firefox.

ENVIRONMENT VARIABLES
---------------------

* **BROWSER**:
  name of web browser to launch. If not set defaults to firefox.

AUTHOR
------
few is written by Dirk Jagdmann <doj@cubic.org>. You can write him
an email and ask questions about this program.

RESOURCES
---------
the source code is hosted on Github: <https://github.com/doj/few>

SEE ALSO
--------
less(1)
