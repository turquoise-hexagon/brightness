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
brightness [-ar `<percentage>`] [-q]

description
-----------
brightness allows the user to query and modify the current monitor brightness

options
-------
-a `<percentage>`
    change brightness by relative <percentage>
-r `<percentage>`
    change brightness by absolute <percentage>
``-q``
    query current brightness percentage

example
-------
::

    $ brightness -a 50
    $ brigthness -r -10
    $ brigthness -q
    40
