#!/bin/perl

$id = shift(@ARGV);

$destdir = shift(@ARGV);
$linkfile = shift(@ARGV);

open(OUT, ">>${destdir}\\${linkfile}") or die "can't open $linkfile:$!";

if ($id == 0)
{
    for (@ARGV)
    {
        print OUT "$destdir\\$_\n";
    }
}
else
{
    for (@ARGV)
    {
        print OUT "$_\n";
    }
}

close(OUT);

