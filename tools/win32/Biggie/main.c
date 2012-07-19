//
//	BIGGIE - Bigfile support utility
//
//	Copyright (C) 1998 Relic Entertainment Inc.
//
//	Created May 1998 by Darren Stone.
//	See bigfile.h for version notes.
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "version.h"
#include "bigfile.h"
#include "options.h"

extern char OptCommand;
extern int  OptCompression;
extern int  OptPathnames;
extern int  OptNewer;
extern int  OptOverwrite;
extern int  OptMove;

void HelpDisplay(void)
{
	printf("Bigfile support utility.  Copyright (C)1998 Relic Entertainment Inc.\n\n");

	printf("Usage:  BIGGIE options bigfile [files...]\n\n");

	printf("Primary options:\n");
	printf("-a  Add/update files to bigfile\n");
	printf("-f  Fast-create a bigfile\n");
	printf("-d  Delete files from bigfile\n");
	printf("-v  View bigfile contents\n");
	printf("-u  Create a patch bigfile (see special usage, below)\n");
	printf("-x  Extract files from bigfile\n");

	printf("\nAdditional options, as they apply to the primary options:\n");
	printf("         AFDVUX\n");
	printf("-c{0|1}  **      Compress files (1 default)\n");
	printf("-m{0|1}  **   *  Move files to/from bigfile (0 default)\n");
	printf("-n{0|1}  *    *  Only newer files (0 default)\n");
	printf("-o{0|1}       *  Overwrite existing files (1 default)\n");
	printf("-p{0|1}  **   *  Store/restore full pathnames (1 default)\n");

	printf("\nExample usage:  BIGGIE -a -n1 -p0 fonts.big \\fonts\\*.hff \\data\\*.hff\n");

	printf("\nFilelists (files with one filename per line) may be used in place of filenames.\n");
	printf("For example:  BIGGIE -a -n1 data.big dmp*.lif @otherfiles.txt\n");

	printf("\n\tCreating a \"patch\" bigfile has the following usage:\n\n");
	printf("\t  BIGGIE -u oldbigfile newbigfile patchbigfile\n\n");
	printf("\tFiles that have changed or have been added in the newbigfile\n");
	printf("\twill be placed in the patchbigfile.  A byte-for-byte compare\n");
	printf("\tis performed on every file (ie, date/time-stamps are ignored).\n");
	printf("\tAdditional options do not apply to patching.\n");
}

int main(int argc, char *argv[])
{
	char bigfilename[BF_MAX_FILENAME_LENGTH + 1];
	int numOpts = 0;
	int numFiles;
	char **filenames;

	verDisplay();

	if (argc < 3)
	{
		HelpDisplay();
		return 0;
	}

	// establish defaults for command line options
	optDefaultsSet();

	// override defaults with user options
	while ((numOpts+1) < argc && 
			optProcessArgument(argv[numOpts+1]))
		++numOpts;

	if (!numOpts)
	{
		printf("ERROR: No options specified\n");
		return 0;
	}

	strcpy(bigfilename, argv[numOpts + 1]);
	numFiles = argc - (numOpts + 2);
	filenames = argv + numOpts + 2;

	// execute primary command, with any applicable options
	if (toupper(OptCommand) == 'A')
		bigAdd(bigfilename, numFiles, filenames, 
			OptCompression, OptNewer, OptMove, OptPathnames, 1);
	else if (toupper(OptCommand) == 'F')
		bigFastCreate(bigfilename, numFiles, filenames, 
			OptCompression, OptNewer, OptMove, OptPathnames, 1);
	else if (toupper(OptCommand) == 'U')
	{
		char oldfilename[BF_MAX_FILENAME_LENGTH + 1],
			newfilename[BF_MAX_FILENAME_LENGTH + 1],
			patchfilename[BF_MAX_FILENAME_LENGTH + 1];

		if (numOpts != 1 || argc != 5)
		{
			printf("ERROR: Invalid patch option\n");
			return 0;
		}
		strcpy(oldfilename, argv[numOpts+1]);
		strcpy(newfilename, argv[numOpts+2]);
		strcpy(patchfilename, argv[numOpts+3]);
		bigPatch(oldfilename, newfilename, patchfilename, 1);
	}
	else if (toupper(OptCommand) == 'D')
		bigDelete(bigfilename, numFiles, filenames, 1);
	else if (toupper(OptCommand) == 'V')
		bigView(bigfilename, 1);
	else if (toupper(OptCommand) == 'X')
		bigExtract(bigfilename, numFiles, filenames, 
			OptNewer, OptMove, OptPathnames, OptOverwrite, 1);
	else 
		printf("ERROR: Unrecognized command\n");

	return 0;
}
