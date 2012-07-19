#!/bin/perl

chdir("/homeworld/data");

if ($#ARGV == -1)
{
    print STDERR "must specify at least one directory\n";
    exit;
}

sub doSubDir
{
    my $origdirname = shift @_;
    my @subs = ('lod0','lod1','lod2','lod3','lod4');
    my $sub;
    my $total = 0;
    foreach (@subs)
    {
        $sub = $_;
        my $dirname = "${origdirname}/rl0/$sub";
        $sub =~ s/\D//g;

        my @geoname = split('/', $dirname);

        my $num = `polycount $dirname/$geoname[1].geo`;
        if ($num > 0)
        {
            $total += $num;
            $totals[$sub] += $num;
            $hits[$sub]++;
        }
        $num += 0;
        printf("%6d", $num);
    }
    printf("  [%5d]", $total);
    $totals[5] += $total;
    $hits[5]++;
    @splitname = split('/', $origdirname);
    print "  $splitname[$#splitname]\n";
}

sub doDir
{
    my $dirname = shift @_;
    opendir(DIR, $dirname) or return;
    my @dir = readdir(DIR);
    closedir(DIR);

    foreach (@dir)
    {
        if (!(/\./))
        {
            doSubDir("$dirname/$_");
        }
    }
}

foreach (@ARGV)
{
    my $dir = $_;
    local @totals = (0,0,0,0,0,0);
    local @hits   = (0,0,0,0,0,0);

    print "\n";

    doDir($dir);

    print "=" x 39 . "\n";

    my $grandtotal = pop(@totals);
    for (@totals)
    {
        printf("%6d", $_);
    }
    printf("   %6d  $dir total\n", $grandtotal);

    my $i = 0;
    for (@totals)
    {
        if ($hits[$i] > 0)
        {
            printf("%6d", $_ / $hits[$i]);
        }
        else
        {
            print " " x 5 . "?";
        }
        $i++;
    }
    if ($hits[5] > 0)
    {
        $grandtotal /= $hits[5];
        printf("    %5d  $dir average\n", $grandtotal);
    }
    else
    {
        print " " x 8 . "?  $dir average\n";
    }
}

