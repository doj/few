fewer(1)
========

NAME
----
fewer - a file viewer with regular expression filters.

SYNOPSIS
--------
**fewer** [--regex 'REGEX']* [--tabwidth 'NUM'] 'FILE'

DESCRIPTION
-----------
fewer is a program to interactively filter log files with regular expressions and view the result.
It is similar to the less(1) program, but has the ability to filter what is displayed with regular expressions.

OPTIONS
-------
* **--regex** 'REGEX':
  preset regular expressions to filter the file.

* **--tabwidth** 'NUM':
  set the width of a tab character in spaces.

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
  show help text
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
expression after the fewer program has started and will see the file
display change if the regular expression matches. You can also preset
these regular expressions with the command line arguments.

The standard form of these regular expression has the following format:

/regex/flags

The regular expression is surrounded by two forward slash characters.

See <http://www.cplusplus.com/reference/regex/ECMAScript/> for details
on the regular expression syntax. It is similar to perl regular
expressions.

### Regular Expression Flags
The regular expression can be modified with flag characters, the
following characters are currently supported:

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

The fewer program will convert the short forms to the regular form.

### Display Filter Regular Expressions

Display Filter change the way the lines are displayed. They take the
form of a perl regular expression substitute:

/regex/replace/

Because the parsing of this format is currently naive, the strings
_regex_ and _replace_ may not contain any forward slash characters.

The display filter allows the 'i' flag to indicate case insensitive
matching.

AUTHOR
------
fewer is written by Dirk Jagdmann <doj@cubic.org>. You can write him
an email and ask questions about this program.

RESOURCES
---------
the source code is hosted on Github: <https://github.com/doj/fewer>

SEE ALSO
--------
less(1)
