NAME
----

msqdev/utils - Utility programs for use with Msqdev.

DESCRIPTION
-----------

In general each of the scripts in this directory perform
some unique tuning operation.
Most require that the msqdev daemon is running so that real
time data can be read.
They are usually run in the root directory of a particular
Msqdev tuning setup (this is the directory that contains all the tables).
Each should have some sort of usage description that would
be displayed with the '-h' option.
They will also contain internal comments defining their operation.

