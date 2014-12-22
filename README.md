fewer
=====

fewer is a program to interactively filter log files with regular expressions and view the result.
It is similar to the less(1) program, but has the ability to filter what is displayed with regular expressions.

Keys
====

The actions resulting from a key press are modelled after the less(1) program.

q, Q - quit the program..
cursor down - scroll down one line.
n - scroll down one line if no search regex is set.
cursor up, p - scroll up one line.
space, page down - scroll down one page.
b, page up - scroll up one page.
g, <, home - go to to first line.
G, >, end - go to last line.
1..9 - edit filter regular expression.
F1..F9 - edit display filter regular expression.
d - scroll down half a screen
u - scroll up half a screen
P - goto line
% - goto percentage of lines
R - repaint the screen
h - show help text
/ - enter a search regular expression
n - go to next search match if a search regex is set.
N - go to previous search match.

Links
=====

https://github.com/doj/fewer the source code is hosted on Github.
Dirk Jagdmann <doj@cubic.org> the main author, can answer your questions.
