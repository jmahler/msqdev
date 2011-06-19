
NAME
----

msqdev - file based control of a MegaSquirt ecu

INTRODUCTON
-----------

msqdev provides file based control of tables/settings in a [MegaSquirt][megasquirt] ecu.
This design is analogous to the way that Linux provides access
to its devices using files and directories under /dev.

There are several programs included with this package.

 * msqdev - Creates the communication interface between the files
   and the [Megasquirt][megasquirt] ecu.

 * msq-rtmonitor - Displays the selected values of incoming real time data.

 * msq-speed\_tuner - Varies values in a table at the current position
   and records data to be analyzed.
   (steady state tuning)

 * msq-accel\_tuner - Tunes by comparing two accelerations with different
   settings under the same conditions.
   (constant acceleration)

For more specific information see the programs help (-h) output.

INSTALLATION
------------

The liblookuptable library is required in order to use msqdev.
It can be downloaded from [liblookuptable][liblookuptable].
Depending on where this library is installed it may be necessary
to update the include paths for the Gcc compiler.
This can be accomplished by using the environment variable 'CPLUS\_INCLUDE\_PATH'.
See the Gcc man page for more info (man 1 gcc).

  [liblookuptable]: https://github.com/jmahler/liblookuptable

AUTHOR
------

Jeremiah Mahler <jmmahler@gmail.com><br>
<http://www.google.com/profiles/jmmahler#about>

COPYRIGHT
---------

Copyright &copy; 2011, Jeremiah Mahler.  All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

 [gpl]: http://www.gnu.org/licenses/gpl.html

 [megasquirt]: http://www.megasquirt.info
 [tunerstudio]: http://www.efianalytics.com/TunerStudio/
 [megatunix]: http://megatunix.sourceforge.net
 [msextra]: http://www.msextra.com
 [megatune]: http://www.megasquirt.info/megatune.htm


