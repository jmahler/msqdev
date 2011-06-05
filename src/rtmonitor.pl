#!/usr/bin/perl

use Time::HiRes qw(usleep);

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
# The real time data consists of over a hundred values
# which would be cumbersome to monitor as is.
# This program filters the data so that a selection can
# be more easily viewed.
#

$usage =  "USAGE:\n"
		 ."  rtmonitor.pl <file> <col1>[ <col2> ...]\n";

if (@ARGV < 2) {
	die $usage;
}

$in_file = shift @ARGV;

open(IN, "< $in_file")
	or die "unable to open file '$in_file': $!";

@col_sel = @ARGV;  # column selections

# first line should contain the column names separated by spaces
$first_line = <IN>;

@col_selp;  # selection positions

@col_defs = split /\s+/, $first_line;

# For each selected name, find its associated
# column position.
for ($i = 0; $i < @col_sel; $i++) {
	$sel = $col_sel[$i];
	for ($j = 0; $j < @col_defs; $j++) {
		if ($sel eq $col_defs[$j]) {
			push @col_selp, $j;
		}
	}
}

$| = 1;
$skip_first = 1;  # skip any already stored data

# process all remaining lines, and tail the file
while (1) {
	while ($line = <IN>) {
		next if $skip_first;

		@parts = split /\s+/, $line;
		
		@vals = ();
		for ($i = 0; $i < @col_selp; $i++) {
			push @vals, $parts[$col_selp[$i]];
		}

		$vals = join " ", @vals;

		print "$vals\n";
	}
	$skip_first = 0;

	usleep(25e4);    # micro 10^-6 seconds
	seek(IN, 0, 1);  # reset EOF
}

