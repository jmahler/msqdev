package MSQ::RTData;

use strict;
use warnings;
use Carp;

use IO::File;
use Text::CSV;

# DEVNOTE
# This module is essentially a wrapper around Text::CSV
# to make it easier to use.

=head1 NAME

MSQ::RTData - msqdev real time data

=head1 DESCRIPTION

This module provides operations for reading and writing real time
data as part of the msqdev system.

=head1 SYNOPSIS

  TODO

=head1 INSTALLATION

To use this library without installing it as a package it is
necessary to modify a users Perl library paths (PERL5LIB).

One way is to create a perl5 lib directory in the users
directory and modify it by using the ~/.bashrc

  export PERL5LIB="$HOME/lib/perl5"

Then from within $HOME/lib/perl5 it can be linked (ln -s) to
the directory containing this package.

=head1 OPERATIONS

=cut

# {{{ new()
=head2 new()

=cut

sub new {
	my $class = shift;
	my $file = shift;
	my $cols = shift;

	my $csv = Text::CSV->new({
			#eol => "\r\n",  # XXX - this effects <STDIN>
			eol => "\n",
		});
	unless ($csv) {
		carp("unable to create new Text::CSV object: " . Text::CSV->error_diag());
		return;
	}

	my $fh = new IO::File;
	if (-e $file) {
		if (defined $cols) {
			carp("cannot redefine columns when file already exists");
			return;
		}

		unless ($fh->open("+< $file")) {
			carp("unable to open file '$file': $!");
			return;
		}

		# read column names
		$cols = $csv->getline($fh);
	} else {
		if (! defined $cols) {
			carp("columns must be defined in order to initialize a new file");
			return;
		}
		# create new file
		unless ($fh->open("+> $file")) {
			carp("unable to open file '$file': $!");
			return;
		}
		# add the columns as the first line
		$csv->print($fh, $cols);
	}

	# build the hash ref columns
	my %cols_hr;
	for (my $i = 0; $i < @$cols; $i++) {
		$cols_hr{$cols->[$i]} = $i;
	}

	bless {
		fh  => $fh,
		csv => $csv,
		columns => $cols,
		columns_hr => \%cols_hr,
	}, $class;

}
# }}}

# {{{ columns()

=head2 columns()

Returns a list of the column names at their corresponding offset.

Column names are defined in the constructor (new()).

=cut

sub columns {
	my $self = shift;

	return @{$self->{columns}};
}
# }}}

# {{{ columns_hr()

=head2 columns_hr()

Returns the columns as a hash reference with the key name
and value of the offset in the row.

=cut

sub columns_hr {
	my $self = shift;

	return %{$self->{columns_hr}};
}
# }}}

# {{{ seek_start()

=head2 seek_start()

Seek to the start so that the next call to getline will return
the first row of data.

Returns: TRUE on success, FALSE on error

=cut

sub seek_start {
	my $self = shift;

	my $fh = $self->{fh};
	my $csv = $self->{csv};

	seek($fh, 0, 0);  # start of file

	$csv->getline($fh);  # skip the column names

	return 1;  # OK
}
# }}}

# {{{ seek_last()

=head2 seek_last()

Seek to the last row so that the next call to getline will return
the last row of data.

Returns: TRUE on success, FALSE on error

=cut

sub seek_last {
	my $self = shift;

	my $fh = $self->{fh};
	my $csv = $self->{csv};

	seek($fh, 0, 0);  # start of file

	# find the last position
	my @pos;
	while ($csv->getline($fh)) {
		push @pos, tell($fh);
	}

	seek($fh, $pos[-2], 0);

	return 1;  # OK
}
# }}}

# {{{ seek_end()

=head2 seek_end()

Seek to the end and skip any currently present data.
If no new data has been added after this call getline()
will fail with an eof().

Returns: TRUE on success, FALSE on error

=cut

sub seek_end {
	my $self = shift;

	my $fh = $self->{fh};

	seek($fh, 0, 2);  # end of file

	return 1;  # OK
}
# }}}

# {{{ getline()

=head2 getline()

Get an array reference of the next available row of values.

Returns: TRUE on success, FALSE on error or EOF

To determine if an error occured or the end of file was reached
use eof().

=cut

sub getline {
	my $self = shift;

	my $csv = $self->{csv};
	my $fh = $self->{fh};

	my $res = $csv->getline($fh);
	if (! $res) {
		if ($csv->eof()) {
			seek($fh, 0, 1);  # reset eof
		} else {
			carp("getline() error: " . $csv->error_diag());
		}
	}

	return $res;
}
# }}}

# {{{ eof()

=head2 eof()

Returns: TRUE if last getline() hit eof, FALSE otherwise

=cut

sub eof {
	my $self = shift;

	my $csv = $self->{csv};

	return $csv->eof();
}
# }}}

# {{{ print()

=head2 print()

Print new values to the file.

=cut

sub print {
	my $self = shift;
	my $colref = shift;

	my $csv = $self->{csv};
	my $fh = $self->{fh};

	$csv->print($fh, $colref);
}
# }}}

1;
