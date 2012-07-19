//
// Paco, the texture packer
//

#include <stdio.h>
#include <stdarg.h>
#include <ddraw.h>
#include "Paco.h"
#include "Main.h"
#include "Pack.h"
#include "Mex.h"
#include "lif.h"
#include "list.h"

bool exportPages = true;
bool quietMode = false;
bool polyMode = false;
bool pacOutputShared = false;
bool pacOutputPages = false;
bool pacDumpTargas = false;
bool pacExportMesh = true;
bool pacResortPeo = false;
bool pacPerformFixup = false;

sdword pacLOD;

char* errorstring = NULL;

char rawPrefix[256];

bool mainStuff(char* filePrefix, char* fileName)
{
	if (!mainStartup())
	{
        errorstring = "couldn't startup";
		return false;
	}

    if (pacResortPeo)
    {
        return meshResortPeo(filePrefix, fileName);
    }

    if (pacPerformFixup)
    {
        return meshPerformFixup(filePrefix, fileName);
    }

    if (polyMode)
    {
        meshPolyAnalysis(filePrefix, fileName);
        return true;
    }

    meshdata* mesh = meshLoad(filePrefix, fileName);
    if (mesh == NULL)
    {
        errorstring = "couldn't load mesh";
        return false;
    }

    //
    //first pack Paletted textures

    (void)meshCountPalettes(filePrefix, mesh);
    for (list_crc32::iterator i = crc32list.begin(); i != crc32list.end(); ++i)
    {
        crc32 crc = (*i);
        if (!meshLoadPalettedTextures(filePrefix, mesh, crc))
        {
            errorstring = "couldn't load paletted textures";
            return false;
        }
        
        if (!mainClassifyTextures())
        {
            errorstring = "couldn't classify paletted textures";
            return false;
        }

        //
        //separate unpackables from liflist
        meshQueueUnpackables(filePrefix, fileName);

        while (!pacPack(false)) ;
    }

    //
    //now pack Alpha textures

    if (!meshLoadTextures(filePrefix, mesh, true))
    {
        errorstring = "couldn't load alpha textures";
        return false;
    }

    if (!mainClassifyTextures())
    {
        errorstring = "couldn't classify alpha textures";
        return false;
    }

    //
    //separate unpackables from liflist
    meshQueueUnpackables(filePrefix, fileName);

    while (!pacPack(true)) ;

    //cleanup process
    pacCleanup();

    //
    //export process

    //name the pages
    pacNamePages();

    //export the pages as LiFs
    lifExportPages(filePrefix);

    //export a new mesh w/ updated data
    delete mesh;
    if (pacExportMesh)
    {
        meshExportPagedMesh(filePrefix, fileName);
    }

    return true;
}

int main(int argc, char* argv[])
{
    char filePrefix[1024];
    char fileName[1024];
    char* hwData;
    sdword iRace, iFile;

    if (argc < 3)
    {
        puts("Paco, the texture packer");
        puts("");
        puts("usage: paco [-s] [-v] [-d] [-m] [-t] [-r] <race> <ship> [lod]");
        puts("   eg. paco r1 advancesupportfrigate 2");
        puts("");
        puts("-s option will create sharedtex.mif in directory");
        puts("-v verbosely displays the contents of all pages");
        puts("-d dumps contents of pages as .TGA files");
        puts("-m disable creation of .PEO file");
        puts("-t disable creation of PAGE*LIF files");
        puts("-r resort .PEO polygons by material, ONLY");
        puts("-u perform uv fixups found in uv.txt");
        puts("");
        puts("race: r1, r2, traders, etc.");
        puts("ship: heavyinterceptor, targetdrone, etc.");
        return 1;
    }

    iRace = 1;
    iFile = 2;
    exportPages = true;
    quietMode = false;
    polyMode = false;
    pacOutputShared = false;
    pacOutputPages = false;
    pacDumpTargas = false;

    for (sdword index = 1; index < 4; index++)
    {
        if (argv[index] == NULL)
        {
            break;
        }
        if (argv[index][0] == '/' || argv[index][0] == '-')
        {
            switch (toupper(argv[index][1]))
            {
            case 'S':
                pacOutputShared = true;
                break;

            case 'V':
                pacOutputPages = true;
                break;

            case 'D':
                pacDumpTargas = true;
                break;

            case 'P':
                polyMode = true;
                break;
            
            case 'Q':
                quietMode = true;
                break;

            case 'M':
                pacExportMesh = false;
                break;

            case 'T':
                exportPages = false;
                break;

            case 'R':
                pacResortPeo = true;
                break;

            case 'U':
                pacPerformFixup = true;
                break;

            default:
                fprintf(stderr, "unrecognized commandline switch: '%c'\n",
                        toupper(argv[index][1]));
            }
            iRace++;
            iFile++;
            argc--;
        }
    }

    hwData = getenv("HW_DATA");
    if (hwData == NULL)
    {
        fprintf(stderr, "-- no HW_DATA environment variable --\n");
        return 1;
    }
    strcpy(filePrefix, hwData);
    strcat(filePrefix, "\\");
    strcat(filePrefix, argv[iRace]);
    strcat(filePrefix, "\\");
    strcat(filePrefix, argv[iFile]);
    strcat(filePrefix, "\\rl0\\lod0\\");

    strcpy(fileName, argv[iFile]);
    strcat(fileName, ".geo");

    strcpy(rawPrefix, argv[iRace]);
    strcat(rawPrefix, "\\");
    strcat(rawPrefix, argv[iFile]);
    strcat(rawPrefix, "\\rl0\\lod0\\");

    printf("\nPaco: %s %s ", argv[iRace], argv[iFile]);

    mainStartupOnce();

    if (argc == 4)
    {
        sdword i = argv[iFile+1][0] - '0';

        pacLOD = i;

        rawPrefix[strlen(rawPrefix) - 2] = '0' + i;
        filePrefix[strlen(filePrefix) - 2] = '0' + i;

        printf("%c\n", '0' + i);

        if (pacOutputShared)
        {
            meshOpenSharedMIF(rawPrefix);
        }

        if (!mainStuff(filePrefix, fileName))
        {
            if (errorstring != NULL)
            {
                fprintf(stderr, "-- %s --\n", errorstring);
            }
            return 1;
        }

        if (pacOutputShared)
        {
            meshCloseSharedMIF();
        }
    }
    else
    {
        if (polyMode)
        {
            printf("\n");
        }
        for (sdword i = 0; i < 5; i++)
        {
            pacLOD = i;

            rawPrefix[strlen(rawPrefix) - 2] = '0' + i;
            filePrefix[strlen(filePrefix) - 2] = '0' + i;

            if (!polyMode)
            {
                printf("%c", '0' + i);
            }

            if (pacOutputShared)
            {
                meshOpenSharedMIF(rawPrefix);
            }

            if (!mainStuff(filePrefix, fileName))
            {
                printf("\n");
                fprintf(stderr, "-- %s --\n", errorstring);
                return 1;
            }

            if (pacOutputShared)
            {
                meshCloseSharedMIF();
            }
        }

        printf("\n");
    }

    if (pacResortPeo)
    {
        printf(".PEO file(s) resorted\n");
    }

    return 0;
}
