few(1)
========

NAME
----
few - a file viewer with regular expression filters.

SYNOPSIS
--------
**few** [--regex 'regular expression']\* [--search '/REGEX/flags'] [--tabwidth 'NUM'] [--goto 'NUM'] [-v] [--color] [-h|-?|--help] ['FILE']

DESCRIPTION
-----------
few is a program to interactively filter log files with regular expressions and view the result.
It is similar to the less(1) program, but has the ability to filter what is displayed with regular expressions.

OPTIONS
-------
* **--regex** '/REGEX/flags':
  preset regular expressions to filter the file.

* **--search** '/REGEX/flags':
  preset search regular expression.

* **--tabwidth** 'NUM':
  set the width of a tab character in spaces.

* **--goto** 'NUM':
  go to line number NUM. If NUM is not included in the currently
  filtered lines, go to the previous filtered line.

* **-v**:
  increase verbosity for certain operations.

* **--color**:
  enable color if the terminal supports color.

* **-h**, **-?**, **--help**:
  show help text.

* **'FILE'**:
  the file name to use. If it is "-" the standard input is used. If no
  file name was specified standard input is used.

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
* **1** .. **9**, **0**:
  edit regular expressions 1 to 10.
* **F1** .. **F12**:
  edit regular expressions 11 to 22.
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
* **M**:
  maximize the window. Currently supported on Windows.
* **S**:
  save the currently filtered lines to a new file.

Filter Regular Expressions
-------------------
For display or filter regular expressions (key **0** .. **9**)
and search (key **/**) ECMAScript (javascript) style
regular expressions, as provided by the C++11 regular expression
standard library are used. You will typically enter the regular
expression after the few program has started and will see the file
display change if the regular expression matches. You can also preset
these regular expressions with the command line arguments.

The standard form of a _filter regular expression_ has the following format:

/regex/flags

The regular expression is surrounded by two forward slash characters.
The flags are optional.

See <http://www.cplusplus.com/reference/regex/ECMAScript/> for details
on the regular expression syntax. It is similar to perl regular
expressions.

### Regular Expression Flags
The filter regular expression can be modified with optional flag characters,
the following characters are currently supported:

* **i**:
  case insensitive match
* **!**:
  negative match

By default the filter regular expression is a positive match. Only lines in
the file that (partially) match the regular expression will be
displayed. If you use the '!' flag the regular expression becomes a
negative match. Only lines that do *not* match the regular expression
are displayed.

### Filter Regular Expression Short Form
If you do not need to use flags for the filter regular expression and the
filter regular expression does not start with a forward slash, you can omit
the surrounding slashes.

If you want a negative match and your filter regular expression does not
start with a '!' character and you don't need additional flags, you
can start your filter regular expression with a '!' character.

The few program will convert the short forms to the regular form.

### Replace Display Filter Regular Expressions

A _Replace Display Filter_ changes the way the lines are displayed. They take the
form of a perl regular expression substitute:

/regex/replace/flags

Because the parsing of this format is currently naive, the strings
_regex_ and _replace_ can not contain any forward slash characters.
The replace display filter allows the optional 'i' flag to indicate case
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

### Attribute Display Filter Regular Expression

You can use a regular expression to change the character attributes
and the colors used to display the line. This is called an _Attribute
Display Filter_. This uses the following form:

|regex|<list of attributes>,<optional color specification>

The following attributes are supported by few. Note that your curses
implementation or terminal may not support all of these attributes.

* normal
* standout
* underline
* reverse
* blink
* dim
* bold
* italic

The optional _color specification_ has the following form:

<color> on <color>

The first color specifies the foreground color. The second color
specifies the background color. The following colors are supported:

* black
* red
* green
* yellow
* blue
* magenta
* cyan
* white

To use colors, you have to start the few program with the --color
command line argument.

Example attribute display filter:

|\d|italic,bold,yellow on blue

Will print all digits in italic and bold with yellow foreground and
blue background color.

### Regular Expressions and wide characters

The few program is able to display wide characters and files encoded
in UTF-8. However because it is designed to work with large files, it
will not attempt to match the filter regular expressions on the wide
character representations of the file processed.  The replace display
filter regular expression is matched on the raw byte sequence of the
file as well. This is usually not a problem if you match ASCII
characters.

The attribute display filter regular expression however is matched
against the wide character representation of the displayed lines.

Links and URLs
--------------

few will attempt to detect URLs and links. If it detects a link, URL
or email address few will underline the match. You can click the
underlined text to have the link opened in a web browser or email
client. On Unix you need to set the BROWSER environment variable if
you don't like the default web browser firefox.

DISPLAY
-------

When you start few you'll see an inverted column of number on the left
side. You'll see the line number that is displayed right aligned. If
there's enough room on the left side of that column you'll see the
line size in characters, excluding any newline or carriage return
characters left aligned. The main part of your screen is used to show
the file contents. The bottom right corner of the main display is used
to show information on the file or results from internal processing.
If you've entered a filter, display or search regular expression they
are shown in the bottom part of the screen in an inverted block. The
lower right side

TAB KEY
-------

When you edit a regular expression for display filters or search, you
can use the tab key to auto complete words that are currently
displayed. Auto completion also works on file and directory names when
you save the filtered lines to a file.

ENVIRONMENT VARIABLES
---------------------

* **BROWSER**:
  On Unix: name of web browser to launch; If not set defaults to firefox.
  On Windows this variable is not used and few uses the system default web browser.

* **MAILER**:
  On Unix: name of the mail application; If not set defaults to thunderbird.
  On Windows this variable is not used and few uses the system default mail application.

* **FEWOPTIONS**:
  default command line arguments.

AUTHOR
------
few is written by Dirk Jagdmann <doj@cubic.org>. You can write him
an email and ask questions about this program.

RESOURCES
---------
the few homepage is <http://llg.cubic.org/tools/few/>
the source code is hosted on Github: <https://github.com/doj/few>

SEE ALSO
--------
less(1)
