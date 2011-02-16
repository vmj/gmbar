gmbar -- Graphical multibar for dzen2
*************************************

gmbar provides a library and a set of programs for producing graphical
multibars for dzen2.

| Download: http://linuxbox.fi/~vmj/gmbar/
| Source code: http://github.com/vmj/gmbar

.. contents::


Basic usage
===========

The program gmmembar produces a system memory bar, showing how much
memory is actively used, how much is used for file system buffers, and
how much is cached.  Following shows a typical invocation::

    $ gmmembar --interval=5 --logfile=gmmembar.log \
               --width=100 --height=10 --margin=1 --padding=1 \
               --fg=red --used=red --buffers=yellow --cached=orange

The program gmcpubar produces a system CPU bar, showing how much CPU
time is used in system, user, nice, and idle tasks.  Following shows a
typical invocation::

    $ gmcpubar --interval=1 --logfile=gmcpubar.log \
               --width=100 --height=10 --margin=1 --padding=1 \
               --fg=red --kern=red --user=orange --nice=yellow \
               --idle=green

The included C library provides the ability to make custom graphical
multibars easily easily.


Requirements
============

GNU make and C libraries on Linux.  No attempt has been made towards
portability.  Take this as a challenge.


Installation
============

Type 'make' and copy the 'bin/gm{mem,cpu}bar' to a convenient place,
like ~/bin.


Authors
=======

Original author and current maintainer is Mikko Värri
(vmj@linuxbox.fi).


License
=======

gmbar is Free Software, licensed under GNU General Public License
(GPL), version 3 or later.  See LICENSE.txt file for details.
