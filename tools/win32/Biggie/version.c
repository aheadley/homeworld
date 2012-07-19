#include <stdio.h>
#include "version.h"
#include "bigfile.h"

void verDisplay(void)
{
	printf("BIGGIE Version %s  [%s%s]\n", BIGGIE_VERSION, BF_FILE_HEADER, BF_VERSION);
}