#!/usr/bin/perl
use strict;

#
# Copyright (C) 2011 Jeremiah Mahler <jmmahler@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

#
# This program automates the task of iterating over a range
# of values near the current value and recording the results.
#

use Text::LookUpTable;
use Time::HiRes qw(usleep);

my $usage =  "USAGE:\n"
		 ."  msq-tuner.pl <table> -r <range %> [-d <rtdata>]\n"
		 ."\n"
		 ."  msq-tuner.pl veTable -r 10\n";

# TODO - config from command line
my $rtdata_file = "rtdata";
my $range = 20;   # range (%)
$range = $range / 100;  # convert % to fraction
my $num_points = 5;  # number of points in between range

# Whether the time or the count will trigger a change depends
# on which is smaller (since they are OR'ed together).
# Making one very large essentially disables it.
my $record_time = 30;  # time in seconds to record data after a change
my $record_count = 70;  # number of points to record after a change
my $change_delay = 20;   # time in seconds to delay after settings have changed
my $change_count = 20;  # number of points to skip after a change

#my $table_file = 'advanceTable1';
my $table_file = 'veTable1';
my $skip_first = 1;  # whether to skip any already stored real time data
my $table_type = $table_file;  # currently same as file, but this may change
my $DEBUG = 1;
my $SEP = ",";

# {{{ setup data sources/sinks

# get a time stamp for creating a unique plot file
my ($month, $day, $year, $hour, $min, $sec) = (localtime)[4, 3, 5, 2, 1, 0];
$year += 1900;
$month += 1;
my $plot_data_file = sprintf("plotdata-$table_type-%d%02d%02d.%02d:%02d:%02d", $year, $month, $day, $hour, $min, $sec);


my $table = Text::LookUpTable->load_file($table_file)
	or die "unable to open table in file '$table_file': $!";
my $orig_table = $table->copy();

die "plot data file '$plot_data_file' already exists!" if (-e $plot_data_file);
open(PLOT, "> $plot_data_file")
	or die "unable to open file '$plot_data_file': $!";

open(RTDATA, "< $rtdata_file")
	or die "unable to open file '$rtdata_file': $!";

$| = 1;  # flush STDOUT after every print

my $oldfh = select(PLOT);
$| = 1;  # flush PLOT after every print
select($oldfh);

# }}}

# {{{ configure the type
my @col_sel;
my $sx; # offset in @col_sel of x
my $sy; #   "     "    "       y
my $sv; #   "     "    "       value
if ($table_type eq 'advanceTable1') {
	print "type: advanceTable1\n" if $DEBUG;
	@col_sel = qw(rpm map advance);
	$sx = 0;  # rpm
	$sy = 1;  # map
	$sv = 2;  # advance
} elsif ($table_type eq 'veTable1') {
	print "type: veTable1\n" if $DEBUG;
	@col_sel = qw(rpm fuelload veCurr1);
	$sx = 0;  # rpm
	$sy = 1;  # fuelload
	$sv = 2;  # veCurr1
} else {
	print STDERR "unkown table type '$table_type'\n";
	exit 1;
}
# }}}

# {{{ Read the column names, configure selected positions

# The first line of the real time data should contain the column names
my $first_line = <RTDATA>;

my @col_defs = split /\s+/, $first_line;

my @col_selp;  # column selection positions
			   # Just like @col_sel but with positions not names.
for (my $i = 0; $i < @col_sel; $i++) {
	my $sel = $col_sel[$i];
	for (my $j = 0; $j < @col_defs; $j++) {
		if ($sel eq $col_defs[$j]) {
			push @col_selp, $j;
		}
	}
}

# If the number selected columns is different than the number
# found something is wrong.
if (@col_selp != @col_sel) {
	print STDERR "some required data coluns are missing\n";
	exit 1;
}

# write the column names to the plot data file
{
my $col_names = join $SEP, @col_defs;
print PLOT "$col_names\n";
}

# }}}

# {{{ signal handlers

# Ctrl-C
$SIG{INT} = sub {
	print "restoring original table\n" if $DEBUG;
	$orig_table->save_file();
	`kill -HUP \`cat pid\``;

	exit 0;
};

# }}}

# plan variables
my $next_val = -1;
my @write_vals;  # values to write to the map
my @points;


my $state_time = time() - 100;  # initial time is long ago to get things started
my $state_count = 0;
my $state = 'plan_needed';  	 # plan_needed | record | change_delay
my $count = 0;

my $last_t;

# main loop
print "start main()\n" if $DEBUG;
while (1) {

	# recording, waiting to change to change_delay
	if ($state eq 'record' and
			((time() - $state_time > $record_time)
				 or ($count - $state_count > $record_count))) {
		# change the values

		$next_val++; 

		# quit unless there are more values left
		last unless ($next_val < @write_vals);

		my $write_val = $write_vals[$next_val];
		print "next val: $write_val\n" if $DEBUG;

		# write the new values
		foreach my $point (@points) {
			$table->set($point->[0], $point->[1], $write_val);
			$table->save_file();
		}
		`kill -HUP \`cat pid\``;

		$state_time = time();
		$state_count = $count;

		print "state: $state -> change_delay\n" if $DEBUG;
		$state = 'change_delay';
	}
	if ($state eq 'record' and $DEBUG) {
		my $t = ($record_time - (time() - $state_time));
		my $c = ($record_count - ($count - $state_count));

		if ($t != $last_t) {
			$last_t = $t;
			print "state (record) time left, count left: $t, $c\n"
		}
	}


	if ($state eq 'change_delay' and
			((time() - $state_time > $change_delay)
					or ($count - $state_count > $change_count))) {
		$state_time = time();
		$state_count = $count;

		print "state: $state -> record\n" if $DEBUG;
		$state = 'record';
	}
	if ($state eq 'change_delay' and $DEBUG) {
		my $t = ($change_delay - (time() - $state_time));
		my $c = ($change_count - ($count - $state_count));
		if ($t != $last_t) {
			$last_t = $t;
			print "state (change_delay) time left, count left: $t, $c\n";
		}
	}


	# get all new real time data, save to plot file
	while (my $line = <RTDATA>) {
		next if $skip_first;

		my @vals = split /\s+/, $line;  # all values
		$count++;
		
		# get the select values
		my @sel_vals = ();  # select values
		for (my $i = 0; $i < @col_selp; $i++) {
			push @sel_vals, $vals[$col_selp[$i]];
		}

		if ($state eq 'record') {
			# save the values to the PLOT file
			my $vals_str = join ",", @vals;
			#print "record: $sel_vals_str\n" if $DEBUG;
			print PLOT "$vals_str\n";
		}

		# {{{ build a plan
		if ($state eq 'plan_needed') {
			print "building a plan\n" if $DEBUG;

			# The poins for the plan are created here because
			# they need one point of real time data to find the current point.

			$next_val = -1;

			my $cur_x_val = $sel_vals[$sx];
			my $cur_y_val = $sel_vals[$sy];
			my $cur_val = $sel_vals[$sv];

			# lookup points around current x, y
			# lookup_points(x_val, y_val, range)
			@points = $table->lookup_points($cur_x_val, $cur_y_val, 3);

			my $start_val = $cur_val - ($cur_val * $range);
			my $end_val = $cur_val + ($cur_val * $range);
			my $inc = ($end_val - $start_val) / $num_points;
			$inc = 0.01 if ($inc < 0.01);  # make the values sane
			print "plan: start=$start_val, end=$end_val, inc=$inc\n" if $DEBUG;

			for (my $i = $start_val; $i <= $end_val; $i += $inc) {
				push @write_vals, $i;
			}

			print "state: $state -> record\n" if $DEBUG;
			$state = 'record';
		}
		# }}}

	}
	$skip_first = 0;
	seek(RTDATA, 0, 1); # reset EOF

	usleep(2e4);    	# slow things down
}

print "restoring original table\n" if $DEBUG;
$orig_table->save_file();
`kill -HUP \`cat pid\``;

