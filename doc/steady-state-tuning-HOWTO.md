# STEADY STATE TUNING - HOWTO

Steady state tuning involves situations where all operating
variables are steady except one.
The most obvious examples is tuning an engine at idle.
And simple hill climbing techniques can be used to optimize
the single variable.

The following describes how steady state tuning is performed
with the msqdev system.

## EDITING TABLES

In order to vary a single variable the table must be modified.
Often the current position of a variable is not precisely at one
point and is instead a combination of points nearby.
Because of this it is necessary to edit a range of values around
this point.

One way to accomplish this is to edit the text based look
up tables ([liblookuptable][liblookuptable]) with the
[Vi][vi] text editor.

  [vi]: http://www.vim.org
  [liblookuptable]: https://github.com/jmahler/liblookuptable

The text editor Vim has the ability to substitute for some
pattern in the text some other text.

[Vi][vi] is capable of replacing every occurrence of a pattern in a file
with some other text.

    :% s/XXX/12.5/g

Since we want to replace a range of values we can substitute the unique
identifier (XXX) as the pattern.

The general procedure is as follows:

  1. Place the pattern ('XXX') in all entries to be replaced.
  2. Use the pattern substitution to assign a value (see above).
  3. Write out the file (:w).
  4. Send a SIGHUP to msqdev (kill -HUP 'cat pid') to
	 write the values to the ecu.
  5. Undo (u) to revert back to the pattern ('XXX').
  6. go to step 2 (repeat)

## MONITORING

As values are changed there will be intermediate transient values
which are invalid and should be ignored.
By monitoring the relevant values it can be determined when the
data is stable enough to produce valid recordings.

To monitor advanceTable1

	shell$ msq-rtmonitor rtdata rpm map advance

or to monitor veTable1.

	shell$ msq-rtmonitor rtdata rpm fuelload veCurr1 pulseWidth1

## HILL CLIMBING

As the independent input variable is changed the dependent
output will vary.
If many changes were made and the data is plotted the
result would be a hill (if we assume there is no local minima).

The following procedure produces a plot of the hill in
real time as the variable is modified and the data
is also record so that it can be analyzed/plotted afterwards.

The file "rtdata" stores all incoming data points but only
a subset of these are needed.
To filter these the msq-rtmonitor program is used.
Also, by starting and stopping this program this
excludes can be used to exclude the unwanted transient values.

Filter the real time data and append it to the plotdata.
This command is started/stopped as needed to include/exclude the data.

To filter veTable1

	shell$ msq-rtmonitor rtdata veCurr1 rpm >> plotdata

or to filter advanceTable1

	shell$ msq-rtmonitor rtdata advance rpm >> plotdata

The data can be plotted as it is updated by using [feedGnuplot][feedgp].

To plot advanceTable1

	shell$ tail -f plotdata | feedGnuplot --points --stream --domain --xlabel advance --ylabel rpm

or to plot veTable1

	shell$ tail -f plotdata | feedGnuplot --points --stream --domain --xlabel veCurr1 --ylabel rpm

And afterwards the data can has been recorded can be plotted with essentially
the same command as previously but without the '--stream' option.

	shell$ cat plotdata | feedGnuplot --points --domain --xlabel advance --ylabel rpm

  [feedgp]: https://github.com/dkogan/feedgnuplot

## AUTHOR

Jeremiah Mahler <jmmahler@gmail.com><br>
<http://www.google.com/profiles/jmmahler#about>

## COPYRIGHT

Copyright &copy; 2011, Jeremiah Mahler.  All Rights Reserved.<br>
This project is free software and released under
the [GNU General Public License][gpl].

 [gpl]: http://www.gnu.org/licenses/gpl.html

