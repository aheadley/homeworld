#!/bin/perl
#05 May 99, khent

if ($#ARGV < 1)
{
    print <<END;
BigProc: process a file listing removing exclusions given
         in a separate file.

Usage:   perl bigproc.pl [-s] bigexclude.txt bigcontents.txt
         -s switch disables output of excluded files

         output will be dumped to stdout
         exclusions may contain wildcard [*?] characters
END
    exit;
}

$silent = 0;

foreach (@ARGV)
{
    if (/^-/)
    {
        $silent = 1;
    }
    elsif (defined($excludefile))
    {
        $listingfile = $_;
    }
    else
    {
        $excludefile = $_;
    }
}

%listing = ();
@listing = ();
@exclude = ();
@output = ();

# -----

sub fixupPath
{
    # re-slashify the pathname
    $_ = shift @_;
    tr/\\/\//;
    s{///}{/}g;
    s{//}{/}g;
    lc $_;
}

sub getExcludes
{
    # put exclusions directly into @exclude
    my $exclude;
    open(IN, $excludefile) || die("can't open $excludefile: $!");
    while (<IN>)
    {
        chomp;
        if (length($_) > 2)
        {
            split;
            push (@exclude, fixupPath($_[0]));
        }
    }
    close(IN);
}

sub getListing
{
    # hash listing to avoid duplicates
    open(IN, $listingfile) || die("can't open $listingfile: $!");
    while (<IN>)
    {
        chomp;
        if (length($_) > 2)
        {
            split;
            $listing{fixupPath($_[0])} = 1;
            push(@listing, fixupPath($_[0]));
        }
    }
    close(IN);
}

sub doExclusion
{
    for my $x (@exclude)
    {
        $_ = $x;
        # escape regex metachars but not glob chars
        s:([].+^\-\${}[|]):\\$1:g;
        # convert wildcards to regex
        s/\*/.*/g;
        s/\?/.?/g;
        # evaluator
        my $matchsub = eval 'sub { $_[0] =~ m|^' . $_ . '$|io }';

        foreach (keys(%listing))
        {
            if (&$matchsub($_))
            {
                # increment exclusion count
                $listing{$_}++;
                print "exclude $_ with $x\n" if (!$silent);
            }
        }
    }

    # order the output similarly to input
    for $x (@listing)
    {
        if ($listing{$x} == 1)
        {
            push(@output, $x);
            $listing{$x} = 0;
        }
    }
}

# -----

getExcludes();
getListing();

doExclusion();

map { print "$_\n"; } @output;
