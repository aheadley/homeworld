#!/bin/perl

print "bigPaco\n";

chdir("/homeworld/datasrc");

@dirs = ('p1','p2','r1','r2','traders','derelicts');

sub doDir
{
    my $dirname = shift @_;
    opendir(DIR, $dirname) || die("can't opendir $dirname:$!");
    my @dir = readdir(DIR);
    closedir(DIR);

    foreach (@dir)
    {
        if (!(/\./))
        {
            system("onepaco.bat $dirname $_");
        }
    }
}

foreach (@dirs)
{
    doDir($_);
}

