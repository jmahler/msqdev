
NAME
----

msqdev - file based control of a MegaSquirt ecu

INTRODUCTON
-----------

msqdev provides file based control of tables/settings in a [MegaSquirt][megasquirt] ecu.
This design is analogous to the way that Linux provides access
to its devices using files and directories under /dev.

This design has several benefits compared to gui interfaces
such as: [MegaTune][megatune], [TunerStudio][tunerstudio] and [MegaTunix][megatunix].

  * simple - Changing settings and reading data is as easy as editing
    and reading from a text file.

  * experimentation - Experimenting with settings is easily accomplished
	by writing programs that edit text files.

  * compatibility - All settings are in text files and upgrades between
    version can be accomplished by writing scripts that convert these files.

  * embeddable - The daemon is small and no gui libraries are required
    making it possible to run it on phones or other devices.

 [megasquirt]: http://www.megasquirt.info
 [tunerstudio]: http://www.efianalytics.com/TunerStudio/
 [megatunix]: http://megatunix.sourceforge.net
 [msextra]: http://www.msextra.com
 [megatune]: http://www.megasquirt.info/megatune.htm

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

