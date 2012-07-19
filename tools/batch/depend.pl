#!/bin/perl

# ------

$exceptFile = 'Win32\Homeworld.ex';

$CCMD0 = '        @$(CCMD0) -Fo$(DestDir)\$^&.obj';
$CCMD1 = '        @$(CCMD1) -Fo$(DestDir)\$^&.obj';
$CCMD2 = '        @$(CCMD2) -Fo$(DestDir)\$^&.obj';
$CCMD3 = '        @$(CCMDI) -Fo$(DestDir)\$^&.obj';
$CCMD4 = '        @$(CCMDI) -O2 -Fo$(DestDir)\$^&.obj';
$CCMD5 = '        @$(CCMDI) -O2 -Qvec -Qpf -Fo$(DestDir)\$^&.obj';

$CCMDCPP = '        @$(CCMD0CPP) -Fo$(DestDir)\$^&.obj';

# kas -> kp -> c & h
# this only works in 4NT:  $CCMDKAS = "        \@DEL /Q /Y /E \$(SrcDir6)\\\$^&.kp\n        \@\$(CCPRE) \$? > \$(SrcDir6)\\\$^&.kp\n        \@\$\(KAS2C\) \$(SrcDir6)\\\$^&.kp \$(SrcDir6)\\\$^&.c \$(SrcDir6)\\\$^&.h\n        \@\$(CCMD) -Fo\$(DestDir)\\\$^&.obj";
#$CCMDKAS = "        \@IF EXIST \$(SrcDir6)\\\$^&.kp ( DEL \$(SrcDir6)\\\$^&.kp )\n        \@\$(CCPRE) \$? > \$(SrcDir6)\\\$^&.kp\n        \@\$\(KAS2C\) \$(SrcDir6)\\\$^&.kp \$(SrcDir6)\\\$^&.c \$(SrcDir6)\\\$^&.h\n        \@\$(CCMD) -Fo\$(DestDir)\\\$^&.obj";
$CCMDKAS = "        \@IF EXIST \$(SrcDir6)\\\$^&.kp ( DEL \$(SrcDir6)\\\$^&.kp )\n        \@\$(CCPRE) -I\$\(\%HW_Root\)\\src\\game \@\@\@ > \$(SrcDir6)\\\$^&.kp\n        \@\$\(KAS2C\) \$(SrcDir6)\\\$^&.kp \$(SrcDir6)\\\$^&.c \$(SrcDir6)\\\$^&.h\n        \@\$(CCMD) -Fo\$(DestDir)\\\$^&.obj";

$CCMD = $CCMD2;
$clevel = 2;

if (($ENV{'HW_Level'} =~ /HW_Debug/i) or ($ENV{'HW_LEVEL'} =~ /HW_Debug/i))
{
    $CCMD = $CCMD0;
    $clevel = 0;
}
elsif (($ENV{'HW_Level'} =~ /HW_Interim/i) or ($ENV{'HW_LEVEL'} =~ /HW_Interim/i))
{
    $CCMD = $CCMD1;
    $clevel = 1;
}

@dirs = ('Ships','Sigs','Game','Win32','JPG',"..\\src\\SinglePlayer","..\\src\\Tutorials");

%projectHeaders = ();
%exceptions = ();

# ------

sub doFile
{
    my ($dirname, $filename) = @_;
    my @includes = ();
    local *FILE;

    if (!open(FILE, "$dirname/$filename"))
    {
        foreach (@dirs)
        {
            if (open(FILE, "$_/$filename"))
            {
                goto DONE;
            }
        }
        return ();
    }
DONE:
    while (<FILE>)
    {
        if (/^\#include/)
        {
            chomp;
            tr/<>/"/;   #" pacify font-lock mode
            s/(.*?)"(.*?)"(.*)/\2/;
            push(@includes, lc $_);
        }
    }
    close(FILE);

    my $fname;
    foreach $fname (@includes)
    {
        if (!$processed{$fname})
        {
            $processed{$fname} = 1;
            my @incs = doFile($dirname, $fname);
            foreach (@incs)
            {
                push(@includes, lc $_);
            }
        }
    }

    sort @includes;
}

sub doDir
{
    my $dirname = shift @_;
    local *DIR;
    opendir(DIR, $dirname) || die("can't opendir $dirname:$!");
    my @dir = readdir(DIR);
    closedir(DIR);

    foreach (@dir)
    {
        if (/\.c$/ || /\.kas$/)
        {
            local %processed = ();
            my $kas = /\.kas$/;
            my $fname = lc $_;
            (my $objname = $fname) =~ s/\.(.*?)$/\.obj/;
            if ($kas)
            {
               print "$objname: ..\\$dirname\\$fname";
            }
            else
            {
               print "$objname: $fname";
            }
            my %includes = ();
            my $include;
            my @includes = doFile($dirname, $fname);
            foreach $include (@includes)
            {
                if (!$includes{$include})
                {
                    $includes{$include} = 1;
                    if ($projectHeaders{$include})
                    {
                        print " ..\\$projectHeaders{$include}\\$include";
                    }
                }
            }
            if ($kas)
            {
               (my $cname = $fname) =~ s/\.(.*?)$/\.c/;
               my $line = "\n$CCMDKAS ..\\generated\\$cname\n\n";
               my $inc = "..\\$dirname\\$fname";
               foreach $include (@includes)
               {
                   if ($projectHeaders{$include})
                   {
                       $_ = $include;
                       $inc .= " ..\\..\\src\\$projectHeaders{$_}\\$include" if /\.h$/i;
                   }
               }
               $line =~ s/\@\@\@/$inc/;
               print $line;
            }
            else
            {
               my $line = "\n$CCMD ..\\$projectHeaders{$fname}\\$fname\n\n";
               if (defined($exceptions{$fname}))
               {
                   my $ex = $exceptions{$fname};
                   if ($ex > 2)
                   {
                       print "\n" . ${"CCMD$ex"} . " ..\\$projectHeaders{$fname}\\$fname\n\n";
                   }
                   else
                   {
                       if ($clevel <= $ex)
                       {
                           print $line;
                       }
                       else
                       {
                           print "\n" . ${"CCMD$ex"} . " ..\\$projectHeaders{$fname}\\$fname\n\n";
                       }
                   }
               }
               else
               {
                   print $line;
               }
            }
        }
        elsif (/\.cpp$/)
        {
            local %processed = ();
            my $fname = lc $_;
            (my $objname = $fname) =~ s/\.cpp$/\.obj/;
            print "$objname: $fname";
            my %includes = ();
            my $include;
            my @includes = doFile($dirname, $fname);
            foreach $include (@includes)
            {
                if (!$includes{$include})
                {
                    $includes{$include} = 1;
                    if ($projectHeaders{$include})
                    {
                        print " ..\\$projectHeaders{$include}\\$include";
                    }
                }
            }
            my $line = "\n$CCMDCPP ..\\$projectHeaders{$fname}\\$fname\n\n";
            if (defined($exceptions{$fname}))
            {
                my $ex = $exceptions{$fname};
                if ($ex > 2)
                {
                    print "\n" . ${"CCMD$ex"} . " ..\\$projectHeaders{$fname}\\$fname\n\n";
                }
                else
                {
                    if ($clevel <= $ex)
                    {
                        print $line;
                    }
                    else
                    {
                        print "\n" . ${"CCMD$ex"} . " ..\\$projectHeaders{$fname}\\$fname\n\n";
                    }
                }
            }
            else
            {
                print $line;
            }
        }
    }
}

sub grabHeaders
{
    my $dirname = shift @_;
    local *DIR;
    opendir(DIR, $dirname) || die("can't opendir $dirname:$!");
    my @dir = readdir(DIR);
    closedir(DIR);

    foreach (@dir)
    {
        $projectHeaders{lc $_} = $dirname;
    }
}

sub grabExceptions
{
    my $line;
    open(EXCEPT, $exceptFile) || die("can't open $exceptFile:$!");
    while (<EXCEPT>)
    {
        chomp;
        if (length($_) > 2)
        {
            @_ = split;
            my $file = $_[0];
            my $ex = $_[1];
            $exceptions{$file} = $ex;
        }
    }
    close EXCEPT;
}

# ------

grabExceptions();

foreach (@dirs) { grabHeaders($_); }
foreach (@dirs) { doDir($_); }

