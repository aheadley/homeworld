#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

void printOptions(void)
{
        printf("\n");
        printf("FileTrunc.exe :  Command Line Options\n");
        printf("(c) 1999 Relic Entertainment Inc.\n\n");
        printf("FileTrunc SourceFile DestFile Startpos  Endpos\n\n");
        printf("    SourceFile  - The source file that you want to truncate\n");
        printf("    DestFile    - The destination filename for the truncated file\n");
        printf("    StartPos    - The position to start the truncation, in bytes\n");
        printf("    TruncLength - The size of the truncated file, in bytes\n\n");
}

int main(int argc,char *argv[])
{
    FILE *infile,*outfile;
    char *infilename;
    char *outfilename;
    long  i, startpos, truncsize, infilesize, j, granularity;
    char buffer[1024];
	long read,breakflag;

    if(argc < 5)
    {
        printOptions();
        return 0;
    }

    infilename  = argv[1];
    outfilename = argv[2];

    sscanf(argv[3], "%d", &startpos);
    sscanf(argv[4], "%d", &truncsize);

    if ((infile=fopen(infilename,"rb"))==NULL)
    {
        printf("\nError: The input file name specified could not be opened!\n\n");
        printOptions();
        return 0;
    }

    if ((outfile=fopen(outfilename,"wb"))==NULL)
    {
        fclose(infile);
        printf("\nError: The output file name specified could not be opened!\n\n");
        printOptions();
        return 0;
    }

    fseek(infile, 0, SEEK_END);

    infilesize = ftell(infile);

    if (infilesize < startpos)
    {
        fclose(infile);
        fclose(outfile);
        printf("\nError: The startpos specified is past the end of the Sourcefile!\n\n");
        printOptions();
        return 0;
    }

    granularity = 1;

    // set the granularity of the hash printing
    if (truncsize > 100000)
        granularity = 10;
    if (truncsize > 1000000)
        granularity = 100;
    if (truncsize > 10000000)
        granularity = 1000;
    if (truncsize > 100000000)
        granularity = 10000;
    if (truncsize > 1000000000)
        granularity = 100000;

    printf("\nPrinting hash marks with a granularity of %dk\n", granularity);

    fseek(infile, startpos, SEEK_SET);

    for (j=0,i=0,breakflag=0; (i < ((truncsize/1024)+1)) || (feof(infile)); i++)
    {
        read = fread(buffer, sizeof(char), 1024, infile);
		if (read != 1024)
			breakflag = 1;

        if (read > 0) fwrite(buffer, sizeof(char), read, outfile);

        j++;

        if (j == granularity)
        {
            printf("#");
            j = 0;
        }

		if (breakflag) break;
    }

    fclose(infile);
    fclose(outfile);

    return 0;
}