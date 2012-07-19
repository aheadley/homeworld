#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

//#define TESTING

typedef signed long sdword;
typedef unsigned long udword;
typedef float real32;
typedef signed int bool;

#define TRUE	1
#define FALSE	0

#define MAX_ENV_SIZE    100

//flag definitions
#define STEP_DISABLE	0x01

typedef struct
{
	char *HW_DemoValue;
	char *bigName;
}BigName;

BigName	bigNames[] = 
{
	{"Downloadable"	,"HomeworldDL.big"},
	{  "CGW"        ,"HomeworldCGW.big"},
	{  "OEM"		,"HomeworldMOE.big"},
	{  NULL			,"Homeworld.big"},
	{NULL,NULL}
};

typedef struct
{
    udword flags;                               //flags for this entry.  See above for possible values.
    void (*checklist)(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);             //function to call, NULL if none.  Called after variable is set.
    bool (*function)(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);             //function to call, NULL if none.  Called after variable is set.
    char *parameter;                            //parameter string (begins with a slash)
	char *spew;
    char *descriptor;                           //string printed in help screen
}BigStep;

#define entryStep(z,y,a,b,c)	{ z, y, a , NULL , b , c } ,
#define entryDONE()	{ 0 , NULL, NULL , NULL, NULL, NULL }

void delDataCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
bool delData(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
void generateDataContentsCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
bool generateDataContents(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
void getStuffCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
bool getStuff(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
bool cropData(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
void fileLoadsCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
bool fileLoads(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
void excludeCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
bool exclude(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
void bigBuildCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);
bool bigBuild(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm);

BigStep BiggieStepList[] =
{
	entryStep(0,delDataCheck,delData,"Deleteing Data Directory","Del Data Dir")
	entryStep(0,getStuffCheck,getStuff,"Preform various gets","Getting Gets.")
    entryStep(0,generateDataContentsCheck,generateDataContents, "Generating Data Contents File","Data Listing")
    entryStep(0,NULL,cropData, "Cropping Data Contents File","Data Croping")
	entryStep(0,fileLoadsCheck,fileLoads,"Appending contents file to fileloads.log","File Loads Append")
	entryStep(0,excludeCheck,exclude, "Excluding unwanted files","Exclusion process")
	entryStep(0,bigBuildCheck,bigBuild, "Building Big File", "Big File Build process")
    entryDONE()
};


//constant defines for all these silly builds
const char DATA_CONTENTS_NAME[] = "DataContents.txt";
const char DATA_CROP_CONTENTS_NAME[] = "datacontentsCropped.txt";
const char DATA_WITH_LOADS_NAME[] = "bigcontents.txt";
const char DATA_FINAL_CONTENTS_NAME[] = "bigcontentsCulled.txt";

//command line options


main(int argc,char *argv[])
{
	char *cmdLine;
    char *token;
    char seps[] = " /,=";
	char *HW_Root;
	char *HW_Level;
	char *HW_Data;
	char *HW_Demo;

    sdword i;

    //get the command line
    cmdLine = getenv("CMDLINE");
    //parse cmd line
    token = strtok(cmdLine,seps);
    while(token != NULL)
    {
		token = strtok(NULL,seps);
    }

	HW_Root = getenv("HW_Root");
	HW_Data = getenv("HW_Data");
	HW_Demo = getenv("HW_Demo");
	HW_Level = getenv("HW_Level");

	printf("\nEnvironment Varialbes:");
	printf("\nHW_Root=%s",(HW_Root ? HW_Root : "<unspecified, assuming blank>"));
	printf("\nHW_Level=%s",(HW_Level ? HW_Level : "<unspecified, assuming blank>"));
	printf("\nHW_Data=%s",(HW_Data ? HW_Data : "<unspecified, assuming blank>"));
	printf("\nHW_Demo=%s",(HW_Demo ? HW_Demo : "<unspecified, assuming blank>"));
	printf("\n\nHit Enter to being.");
	getch();	

	i=0;
	while(1)
	{
		if(BiggieStepList[i].function != NULL)
		{
			if(BiggieStepList[i].checklist != NULL)
			{
				if(!BiggieStepList[i].flags & STEP_DISABLE)
				{
					BiggieStepList[i].checklist(HW_Root,HW_Data,HW_Demo,HW_Level,BiggieStepList[i].parameter);
				}
			}
			i++;
			continue;
		}
		break;
	}

	i=0;
	while(1)
	{
		if(BiggieStepList[i].function != NULL)
		{
			
			if(BiggieStepList[i].flags & STEP_DISABLE)
			{
				printf("\n%d: Skipping step: %s",i,BiggieStepList[i].descriptor);
			}
			else
			{
				printf("\n%d: %s",i,BiggieStepList[i].spew);
				if(!BiggieStepList[i].function(HW_Root,HW_Data,HW_Demo,HW_Level,BiggieStepList[i].parameter))
				{
					printf("\nError in step: %s",BiggieStepList[i].descriptor);
					exit(0);
				}
			}
			i++;
			continue;
		}
		break;
	}

	return 0;
}

void yesOrNo(char *question, bool *var)
{
	printf("\n%s",question);
	while(1)
	{
		switch(getch())
		{
		case 'y':
		case 'Y':
			*var = TRUE;
			return;
		case 'n':
		case 'N':
			*var = FALSE;
			return;
		}
	}
}

dataDel = FALSE;
void delDataCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	yesOrNo("Do you want to delete your Data directory (Y/N)",&dataDel);
}

bool delData(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	char cmd[500];

	if(!dataDel)
	{
		printf("...Skipped!");
		return TRUE;
	}
	strcpy(cmd,"del /sxyzq ");
	strcat(cmd,HW_Data);
#ifdef TESTING
	printf("\n\t%s",cmd);
#else
	system(cmd);
#endif

	printf("\n\nDon't know source safe gets..so do a get on data and hit Enter to continue.");
	getch();
	return TRUE;
}


bool getStuffs=FALSE;
void getStuffCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	yesOrNo("Do you want to do a getsound and getmovies (Y/N)",&getStuffs);
}

bool getStuff(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	if(!getStuffs)
	{
		printf("...Skipped!");
		return TRUE;
	}
	system("@echo off");
	printf("\n\tGetting Sound.");
#ifdef TESTING
	printf("\n\tgetsound");
#else
	system("getsound");
#endif
	printf("\n\tGetting Movies.");
#ifdef TESTING
	printf("\n\tgetmovies");
#else
	system("getmovies");
#endif
	system("@echo on");
	return TRUE;
}



bool doData=TRUE;
void generateDataContentsCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	yesOrNo("Do you want to generate a data directory listing (Y/N)",&doData);
}
bool generateDataContents(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
    char cmd[1000] = "dir /fs /a-d ";
    
	if(!doData)
	{
		printf("...Skipped!");
		return TRUE;
	}
	
	strcat(cmd,HW_Data);
    strcat(cmd," >! ");
	strcat(cmd,DATA_CONTENTS_NAME);

#ifndef TESTING
	system(cmd);
#else
	printf("\n\t%s",cmd);
#endif
	return TRUE;
}

bool cropData(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	FILE *fp1,*fp2;
	sdword cropLength,count;
	char fchar;

	if(!doData)
	{
		printf("...Skipped!");
		return TRUE;	
	}

#ifdef TESTING
	printf("\n\tCropping Data File.");
	return TRUE;
#endif

	if((fp1 = fopen(DATA_CONTENTS_NAME,"rt"))==NULL)
	{
		printf("\n\tCouldn't open %s.",DATA_CONTENTS_NAME);
		return FALSE;
	}
	if((fp2 = fopen(DATA_CROP_CONTENTS_NAME,"wt"))==NULL)
	{
		printf("\n\tCouldn't open %s.",DATA_CROP_CONTENTS_NAME);
		return FALSE;
	}


	cropLength = strlen(HW_Data)+1;		//add one for the extra backslash
	count=0;
	while(!feof(fp1))
	{
		fchar=fgetc(fp1);
		count++;
		if(count > cropLength)
		{
			//cropping length exceeded
			fputc(fchar,fp2);
			if(fchar == '\n' || fchar == '\r')
			{
				count=0;
			}			
		}
	}

	fclose(fp1);
	fclose(fp2);
	return TRUE;
}

bool fileLoadsOn=TRUE;

void fileLoadsCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	yesOrNo("Do you want to use FileLoads.log information (Y/N)",&fileLoadsOn);	
}

bool fileLoads(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	char filename[500];
	char cmd[500];
	
	if(fileLoadsOn)
	{
		FILE *fp;
		strcpy(filename,HW_Root);
		strcat(filename,"\\exe\\FileLoads.log");
		
		fp = fopen(filename,"rt");
		if(fp == NULL)
		{
			printf("FileLoads.log not found, skipping.");
			goto nofileloads;
		}
		//append datacontents to fileloads.log into a new file.
		strcpy(cmd,"type ");
		strcat(cmd,filename);
		strcat(cmd," >! ");
		strcat(cmd,DATA_WITH_LOADS_NAME);
#ifdef TESTING
		printf("\n\t%s",cmd);
#else
		system(cmd);
#endif
		
		strcpy(cmd,"type ");
		strcat(cmd,DATA_CROP_CONTENTS_NAME);
		strcat(cmd," >> ");
		strcat(cmd,DATA_WITH_LOADS_NAME);
#ifdef TESTING
		printf("\n\t%s",cmd);
#else
		system(cmd);
#endif


	}
	else
	{
nofileloads:
		printf("\n\t/NoFileLoads option used, skipping step.");
		strcpy(filename,"type ");
		strcat(filename,DATA_CROP_CONTENTS_NAME);
		strcat(filename," >! ");
		strcat(filename,DATA_WITH_LOADS_NAME);
#ifdef TESTING
		printf("\n\t%s",filename);
#else
		system(filename);
#endif

	}
	return TRUE;
}

bool excludeYesOrNo=TRUE;
void excludeCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	yesOrNo("Do Perl script exclusion (Y/N)",&excludeYesOrNo);
}
bool exclude(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	char cmd[500];

	if(!excludeYesOrNo)
	{
		printf("...Skipped!");
		return TRUE;
	}

	strcpy(cmd,"perl ");	
	strcat(cmd,HW_Root);	//d:\homeworld
	strcat(cmd,"\\tools\\batch\\bigproc.pl -s ");
	
	strcat(cmd,HW_Root);	//d:\homeworld
	strcat(cmd,"\\tools\\batch\\bigexclude");
	if(HW_Demo != NULL)
	{
		//create name of correct exclusion file.
		strcat(cmd,HW_Demo);
	}
	strcat(cmd,".txt ");
	strcat(cmd, DATA_WITH_LOADS_NAME);
	strcat(cmd," >! ");
	strcat(cmd,DATA_FINAL_CONTENTS_NAME);

#ifdef TESTING
	printf("\n\t%s",cmd);
#else
	system(cmd);
#endif
	
	return TRUE;	
}

bool buildBig=TRUE;
void bigBuildCheck(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	yesOrNo("Build The Big file (Y/N)",&buildBig);
}
bool bigBuild(char *HW_Root,char *HW_Data, char *HW_Demo, char *HW_Level, char *parm)
{
	char cmd[500];
	char bigname[500];
	sdword i;
	bool good;

	if(!buildBig)
	{
		printf("...Skipped!");
		return TRUE;
	}
	
	i=0;
	good=FALSE;
	while(bigNames[i].bigName != NULL)
	{
		if(HW_Demo == bigNames[i].HW_DemoValue)
		{
			strcpy(bigname,bigNames[i].bigName);
			good = TRUE;
			break;
		}
		else if((HW_Demo != NULL) && (bigNames[i].HW_DemoValue != NULL)&& (strcmp(HW_Demo,bigNames[i].HW_DemoValue)==0))
		{
			strcpy(bigname,bigNames[i].bigName);
			good = TRUE;
			break;
		}
		i++;
	}
	if(!good)
	{
		printf("\n\nUnknown HW_Demo setting: %s",HW_Demo);
		return FALSE;
	}

	strcpy(cmd,"biggie -f ");
	strcat(cmd,bigname);
	strcat(cmd," @");
	strcat(cmd,DATA_FINAL_CONTENTS_NAME);

#ifdef TESTING
	printf("\n\t%s",cmd);
#else
	system(cmd);
#endif
	return TRUE;
}
