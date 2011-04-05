#!/usr/bin/perl

# Use the expect perl module to simulate interactive use
use Expect;
use Getopt::Long;

# Perl trim function to remove whitespace from the start and end of the string
sub trim($) {
	my $string = shift();
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}

sub cleanup {
	$e->send("free $block\n");
	$e->expect(300, "OK", ["FAIL" => sub { die "Error: unable to free block\n"; } ]) or die "Error: unable to free block\n";
	exit 1;
}

# Parse options
GetOptions("h!"=>\$help,
           "io!"=>\$io,
           "noCleanup!"=>\$io,
           "block=s"=>\$block);
if ($help) {
	print	"Allocate and boot a block\n";
	print   "\tallocateBlock.pl [options]\n";
	print	" Options: \n";
	print	"\t-h			Displays help text\n";
	print	"\t-block [block-name]	Specifies the name of the block to boot\n";
	print	"\t-io			Boot the block as an I/O block (default is CNK)\n";
	print	"\t-noCleanup		Don't free the block if the allocation fails\n";
	exit 1;
}
if ($block eq "") {
        print "Error: you have to specify a block to allocate\n";
        exit 1;
}

# Start bg_console
$e = Expect->new;
$e->spawn("/bgsys/drivers/ppcfloor/hlcs/bin/bg_console --verbose warn")
	or die "Error: cannot run bg_console\n";
$e->expect(60, "mmcs\$")
	or die "Error: did not get console prompt\n";

# Check if the Block is already booted
$e->log_stdout(0);
$e->send("list_blocks\n");
$e->expect(3, "OK")
	or die "Error: unable to run list_blcoks\n";
$output = $e->exp_after();
$blockStatus = `echo "$output" | grep "$block" | awk '{ print \$2 }'`;
$blockStatus = trim($blockStatus);
if ($blockStatus eq "I") {
	print "Info: block is already initialized\n";
	$e->send("quit\n");
	exit 2;
}
$e->log_stdout(1);

# Allocate the block
$e->send("allocate_block $block\n");
$e->expect(60, "OK", ["Block is not free" => sub { print "Info: block already allocated\n"; } ],
	["FAIL" => sub { die "Error: unable to allocate block\n"; } ])
	or die "Error: unable to allocate block\n";

# Select the block (in case it was already allocated)
$e->send("select_block $block\n");
$e->expect(30, "OK", ["FAIL" => sub { $error = 1; } ])
	or $error = 1;
if ($error == 1) {
	print "Error: unable to select block\n";
	if (!$noCleanup) {
		cleanup();
	}
}

# Boot the block
$error = 0;
$rc = 0;
if ($io) {
	$e->send("boot_block\n");
	$e->expect(300, "OK", ["invalid block state: I" => sub { print "Info: block is already booted\n"; $rc = 2; } ],
		["FAIL" => sub { $error = 1; } ])
		or $error = 1;
	if ($error == 1) {
		print "Error: unable to boot block\n";
		if (!$noCleanup) {
			cleanup();
		}
	}
} else {
	$e->send("boot_block\n");
	$e->expect(300, "OK", ["invalid block state: I" => sub { print "Info: block is already booted\n"; $rc = 2; } ],
		["FAIL" => sub { $error = 1; } ])
		or $error = 1;
	if ($error == 1) {
		print "Error: unable to boot block\n";
		if (!$noCleanup) {
			cleanup();
		}
	}
}

# Wait until the block is booted
$e->send("wait_boot 3\n");
$e->expect(300, "OK", ["FAIL" => sub { $error = 1; } ])
	or $error = 1;
if ($error == 1) {
	print "Error: error while waiting for the block to finish booting\n";
	if (!$noCleanup) {
		cleanup();
	}
}

# Quit the console
$e->send("quit\n");

# Return 0 if block was allocated, 2 if it was already allocated
exit $rc
