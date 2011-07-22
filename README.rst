gmbar -- Graphical multibar for dzen2
*************************************

gmbar provides a library and a set of programs for producing graphical
multibars for dzen2.

| Homepage: http://linuxbox.fi/~vmj/gmbar/
| Download: http://linuxbox.fi/~vmj/gmbar/archive/
| Source code: http://github.com/vmj/gmbar

.. contents::


Basic usage
===========

The program `gmmembar(1)`_ produces a system memory bar, showing how much
memory is actively used, how much is used for file system buffers, and
how much is cached.  Following shows a typical invocation::

    $ gmmembar --interval=5 --logfile=gmbar.log \
               --width=100 --height=10 \
               --margin=1 --padding=2 \
               --fg=red --bg=none \
               --used=red \
               --buffers=orange \
               --cached=yellow \
               | dzen -x 15 -y 15 -w 100 -h 10

.. image:: img/gmmembar.png

The program `gmcpubar(1)`_ produces a system CPU bar, showing how much CPU
time is used in system, user, nice, and idle tasks.  Following shows a
typical invocation::

    $ gmcpubar --interval=1 --logfile=gmbar.log \
               --width=100 --height=10 \
               --margin=1 --padding=0 \
               --segment=1 --gap=1 \
               --fg=none --bg="#444444" \
               --kern=red \
               --user=orange \
               --nice=yellow \
               --idle=none \
               | dzen2 -x 15 -y 15 -w 100 -h 10

.. image:: img/gmcpubar.png

The included C library provides the ability to make custom graphical
multibars easily easily.

.. _gmmembar(1): http://www.linuxbox.fi/~vmj/gmbar/gmmembar.1.html
.. _gmcpubar(1): http://www.linuxbox.fi/~vmj/gmbar/gmcpubar.1.html


Requirements
============

GNU make and C libraries on Linux.  No attempt has been made towards
portability.  Take this as a challenge.


Installation
============

Type 'make install' to install the tools into '/usr/local/bin' and
manual pages to '/usr/local/man'.

You can also define PREFIX to install gmbar in a different prefix:
'make PREFIX=/usr' for example.  Similarly, there's DESTDIR, BINDIR,
MANDIR, and MAN1DIR for those who need them.


Authors
=======

Original author and current maintainer is Mikko Värri
(vmj@linuxbox.fi).


License
=======

gmbar is Free Software, licensed under GNU General Public License
(GPL), version 3 or later.  See LICENSE.txt file for details.
