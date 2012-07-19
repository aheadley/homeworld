#include <string.h>
#include <stdio.h>
#include "options.h"

char OptCommand;	    // a|d|v|x (add|delete|view)
int  OptCompression;	// true/false
int  OptPathnames;      // true/false
int  OptNewer;	        // true/false
int  OptMove;           // true/false
int  OptOverwrite;		// true/false

void optDefaultsSet(void)
{
	OptCommand = 'a';
	OptCompression = 1;
	OptPathnames = 1;
	OptNewer = 0;
	OptMove = 0;
	OptOverwrite = 1;
}

//
//	override OptXXXX variables with user argument
//
//	return 1 if arg is actually an argument (ie, if it
//	starts with a '-'.  return 0 otherwise.
//
int optProcessArgument(char *arg)
{
	if (arg[0] != '-')
		return 0;

	switch (arg[1])
	{
	case 'a':
	case 'd':
	case 'v':
	case 'x':
	case 'f':
	case 'u':
		OptCommand = arg[1];
		break;
	case 'c':
		if (arg[2] == '0' || arg[2] == '1')
			OptCompression = arg[2] - '0';
		else
		{
			printf("WARNING: Invalid compression setting %s -- using -c0\n", arg);
			OptCompression = 0;			
			break;
		}
		break;
	case 'm':
		if (arg[2] == '0' || arg[2] == '1')
			OptMove = arg[2] - '0';
		else
		{
			printf("WARNING: Invalid option setting %s -- using -m0\n", arg);
			OptMove = 0;
		}
		break;
	case 'n':
		if (arg[2] == '0' || arg[2] == '1')
			OptNewer = arg[2] - '0';
		else
		{
			printf("WARNING: Invalid option setting %s -- using -n0\n", arg);
			OptNewer = 0;
		}
		break;
	case 'o':
		if (arg[2] == '0' || arg[2] == '1')
			OptOverwrite = arg[2] - '0';
		else
		{
			printf("WARNING: Invalid option setting %s -- using -o0\n", arg);
			OptOverwrite = 0;
		}
		break;
	case 'p':
		if (arg[2] == '0' || arg[2] == '1')
			OptPathnames = arg[2] - '0';
		else
		{
			printf("WARNING: Invalid option setting %s -- using -p0\n", arg);
			OptPathnames = 0;
		}
		break;
	default:
		printf("WARNING: Undefined option %s\n", arg);
	}

	return 1;
}
