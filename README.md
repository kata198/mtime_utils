#mtime\_utils

mtime\_utils is a collection of tools for accessing and working with mtime (modification time) associated with files.


get\_mtime
----------

get\_mtime reads in a list of files from stdin, one per line, and outputs in the same order:

The filename  (A Tab Character)  (File Modification Time)

The format of the modification time can be controlled by arguments passed to get\_mtime.

The default format of modification time is the "ctime" format ( Weekday 3-letter-month 24Hour:Minute:Second 4Year )

Epoch Time: You can print epoch time (seconds since JAN-1-1970) by passing \-\-epoch

Custom Format: You can output in a custom format by passing \-\-format="strftime format here" (see man strftime for format chars)


sort\_mtime
-----------

sort\_mtime reads in a list of files frm stdin, one per line, and outputs sorted by mtime.

Default is descending order, with oldest first. You can pass \-r to reverse order (newest on top).


Combining
---------

These tools are perfect for use with the "find" command, for example:

	find . -name '*.gcda' | sort_mtime | get_mtime

The above will sort all .gcda (profiling) files in descending order (newest on bottom), and display the ctime next to each file.
