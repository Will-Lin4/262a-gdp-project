% Developing The Global Dataplane

This file describes things that someone working on developing the
GDP should know.  It is (extremely) incomplete.

See the files `README-compiling.md` and `README-admin.md`
before you read this file.

If you are working with the Berkeley GDP group, it is ***essential***
that you get a login on <https://gdp.cs.berkeley.edu> so you have access
to the news, forums, and wiki.  We welcome account requests from bona
fide researchers.  If you are a spammer, well, find someplace else to
spread your poison.


Setting Debug Flags
===================

You can turn on debugging output using a command line flag,
conventionally "`-D`_pattern_`=`_level_".  The _pattern_ specifies
which flags should be set and _level_ specifies how much
should be printed; zero indicates no output, and more output
is added as the values increase.

By convention _level_ is no greater than 127, and values 100
and above may modify the base behavior of the program (i.e.,
do more than just printing information).

Each debug flag has a hierarchical name with (by convention)
"." as the separator, for example, "`gdp.proto`" to indicate
the protocol processing of the GDP.  The "what(1)" program on
a binary will show you which debug flags are available
including a short description.  (The `what` program isn't available
on Linux; it can be simulated using `strings | grep '@(#)'`).

To enable debugging for all patterns, use "`*=`_level_" or
just "_level_", for example "`*=18`" or "`18`".

<!-- vim: set ai sw=4 sts=4 ts=4 : -->
<!-- Use "pandoc -s -o README-developers.html README-developers.md" to process this to HTML -->
