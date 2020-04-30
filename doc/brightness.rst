----------
brightness
----------

change monitor brightness
=========================

:date: April 2020
:version: 0.0
:manual section: 1
:manual group: General Commands Manual

synopsis
--------
brightness `[option]` <parameter>

description
-----------
brightness allows the user to query and modify the current monitor brightness

options
-------
``-a <percentage>``
set <percentage> as the absolute brightness value
``-r <percentage>``
set <percentage> as the relative brightness value
``-q``
output the current brightness percentage

example
-------
::

    $ brightness -a 50
    $ brigthness -r -10
    $ brigthness -q
    40
