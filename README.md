
NAME
----

msqdev - file based MegaSquirt control

INTRODUCTON
-----------

msqdev provides a daemon that enables control of a [MegaSquirt][megasquirt]
controller through files and directories.
This design is analogous to the way that Linux provides access
to its devices using files and directories under /dev/.

This design has several benefits compared to the typical monster monolith
design found in [MegaSquirt][megasquirt], [TunerStudio][tunerstudio] and [MegaTunix][megatunix].

  * simple - Changing settings and reading data is as easy as editing
    and reading from a text file.

  * experimentation - Experimenting with settings in a methodical way
    is easily accomplished by writing programs that edit text files.
    A simple example is a script to optimize ignition timing during idle
    or steady cruising.
    Using a simple hill climbing technique it could alter the timing until
    the rpm is maximized.

  * compatibility - All settings are in text files and upgrades between
    version can be accomplished by writing scripts that convert these files.

  * embeddable - The daemon is small and no gui libraries are required
    making it possible to run it on phones or other devices.

 [megasquirt]: http://www.megasquirt.info
 [tunerstudio]: http://www.efianalytics.com/TunerStudio/
 [megatunix]: http://megatunix.sourceforge.net
 [msextra]: http://www.msextra.com

## FAQ

**Q:** Where can I ask a question?  
**A:**
Send the author an email.

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

