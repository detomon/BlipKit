#!/usr/bin/perl

use strict;
use warnings;

my @commands      = ();
my %namedCommands = ();

my $longestKey      = 0;
my $longestConstant = 0;

my $outputName = "BKInstruction";

# read from input
while (<>) {
	# trim whitespace around line
	$_ =~ s/^\s*|\s*$//g;

	# ignore line if empty or beginning with #
	next if $_ =~ /^#|^$/;

	# parse line
	$_ =~ /([\w\?]+)\s+(\w+)\s+(\d+)/;

	my @command = ($1, $2, $3);

	foreach my $item (@commands) {
		if (@{$item}[2] == $3) {
			die ("duplicate number $3 (@{$item}[1], $2)\n");
		}
	}

	push (@commands, [@command]);

	if ($1 ne '?') {
		$namedCommands{$1} = \@command;
	}

	if (length ($1) > $longestKey) {
		$longestKey = length ($1);
	}

	if (length ($2) > $longestConstant) {
		$longestConstant = length ($2);
	}
}

my %replaces = ();
my $string   = '';

$replaces{'InstructionsCount'} = scalar (keys %namedCommands);

# print instructions enum
foreach my $command (@commands) {
	my @command    = @{$command};
	my $nameLength = $longestConstant;

	my $constant = $command [1];
	my $number   = $command [2];

	$string .= sprintf ("\t%-".$nameLength."s = %d,\n", $constant, $number);
}

$replaces{'InstructionsEnumItems'} = $string;

$string = '';

# sort by key
foreach my $key (sort keys %namedCommands) {
	my $name = "\"$key\",";
	my $nameLength = $longestKey + 3;

	my @command  = @{$namedCommands{$key}};
	my $constant = $command [1];

	$string .= sprintf ("\t{%-".$nameLength."s %s},\n", $name, $constant);
}

$replaces{'InstructionsNamesList'} = $string;

$string = '';

# sort by number value
foreach my $key (sort {$namedCommands{$a}->[2] <=> $namedCommands{$b}->[2]} keys %namedCommands) {
	my $name = "\"$key\",";
	my $nameLength = $longestKey + 3;

	my @command  = @{$namedCommands{$key}};
	my $constant = $command [1];

	$string .= sprintf ("\t{%-".$nameLength."s %s},\n", $name, $constant);
}

$replaces{'InstructionsValueList'} = $string;

####

sub replaceLines
{
	my $inFile  = shift;
	my $outFile = shift;
	my $values  = shift;
	my %values  = %{$values};

	while (<$inFile>) {
		if ($_ =~ /%(\w+)%/) {
			# replace whole line
			if ($_ =~ /^\s*%(\w+)%\s*$/) {
				print $outFile $values{$1};
			}
			else {
				$_ =~ s/%(\w+)%/$values{$1}/g;
				print $outFile $_;
			}
		}
		else {
			print $outFile $_;
		}
	}
}

open my $headerIn, "<$outputName.h" or die "Could not open input header file\n";
open my $sourceIn, "<$outputName.c" or die "Could not open input source file\n";

open my $headerOut, ">../$outputName.h" or die "Could not open output header file\n";
open my $sourceOut, ">../$outputName.c" or die "Could not open output source file\n";

replaceLines ($headerIn, $headerOut, \%replaces);
replaceLines ($sourceIn, $sourceOut, \%replaces);
