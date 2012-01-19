
NAME
----

Msqdev - Megasquirt development (tuning) system

INTRODUCTON
-----------

Why do we need another system for tuning [Megasquirt][megasquirt]?

1. Most are Windows based ([TunerStudio][tunerstudio], [MegaTune][megatune])
   which is difficult and costly to develop under.

2. Most solutions so far have been all encompasing gui programs
   ([TunerStudio][tunerstudio], [MegaTune][megatune], [MegaTunix][megatunix])
   which are very complex and prone to bugs.
   This also makes the barrier for the addition of new features very high.

The design of Msqdev avoids these shortcomings in several ways:

1. It is open source and developed under Linux.

2. It uses text based tables, configuration and data which has many benefits:

   1. Data is not obscured by an application specific file format.

   2. A table can be edited using a simple text editor if needed.<br>
      And this format is intuitive and easy to understand.

   3. Changes can be recorded using a version control system such as [Git][git].

   4. Writing automated tuning programs is as easy as
      writing a program to edit a text file.

3. The daemon (msqdev) which monitors the files and communicates
   to the Megasquirt controller is very simple and
   less prone to bugs.

4. Data is easily recorded. And this makes it easy to
   use programs such as [R][R] to analyze and plot the data.

5. The small footprint also allows for the possibility of creating tuning
   programs on small devices such as phones.

More in depth documentation is provided in the 'doc' directory
of this package.

AUTHOR
------

Jeremiah Mahler <jmmahler@gmail.com><br>
<https://plus.google.com/101159326398579740638/about>

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
 [R]: http://www.r-project.org
 [git]: http://www.git-scm.com

