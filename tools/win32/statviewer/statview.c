#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#define GOD_LIKE_SYNC_CHECKING


#include "TYPES.H"
#include "shipdefs.h"
#include "classdefs.h"
#include "maxmultiplayer.h"
#include "gamestats.h"

#include "spaceobj.h"


typedef struct
{
    sdword numPlayers;
    char names[MAX_MULTIPLAYER_PLAYERS+1][20];
}GameStatsDebugHeader;

#define OP_NONE		0
#define OP_COMPARE	1
#define OP_SYNC_TXT 2
#define OP_VIEW		3
#define OP_SYNC_COMPARE 4

int main(int argc,char *argv[])
{
	FILE *fp1,*fp2;
	sdword options=OP_NONE;
	char *filename1;
	char *filename2;
	char name1[25];
	char name2[25];
	char ch = 't';
	sdword i;
	GameStatsDebugHeader header1;
	GameStatsDebugHeader header2;
	GameStats machine1;
	GameStats machine2;
	sdword frame1;
	sdword frame2;
	sdword sizetoread=0;
	sdword totalread=0;

	sdword numShips1,numShips2;
	sdword univFrame1,univFrame2;
	sdword shipcount,sizeofsnapshot;

	GodWritePacketHeader *m1Header;
	GodWritePacketHeader *m2Header;

	UniverseSnapShot *m1SnapShot;
	UniverseSnapShot *m2SnapShot;

    sdword sizeofHeader;

	Ship ship1;
	Ship ship2;

	memset(&ship1,0,sizeof(Ship));
	memset(&ship2,0,sizeof(Ship));

	if(argc < 2)
	{
explainnetparse:
		printf("\nStat File Parser and Comparer");
		printf("\nUsage: statview <options> <...>");
		printf("\n\nOptions:   /c  <filename1> <filename2>");
		printf("\n           Compare two different stat files.");
		printf("\n			 /d	<filename1> <filename2>");
		printf("\n			 Compare two different sync*.txt files");
        printf("\n           /v <filename1> ");
        printf("\n           view a SyncDump.txt type file");
		printf("\n			 /cs <file1> <file2>");
		printf("\n			 compare two syncdump.txt files");
		printf("\n\n\n\n");
		return 0;
	}

    //Parse Options
	if(argc > 2)
	{
		for(i=1;i<argc;i++)
		{
			if(strcmp(argv[i],"/c")==0)
			{
				options = OP_COMPARE;
				if(argc != 4)
				{
					printf("\nIncorrect # of argument for option /c.\n\n");
					goto explainnetparse;
				}
				filename1 = argv[i+1];
				filename2 = argv[i+2];				
				break;
			}
			else if(strcmp(argv[i],"/d")==0)
			{
				options = OP_SYNC_TXT;
				if(argc != 4)
				{
					printf("\nIncorrect # of argument for option /d.\n\n");
					goto explainnetparse;
				}
				filename1 = argv[i+1];
				filename2 = argv[i+2];				
				break;
			}
			else if(strcmp(argv[i],"/cs")==0)
			{
				options = OP_SYNC_COMPARE;
				if(argc != 4)
				{
					printf("\nIncorrect # of argument for option /cs.\n\n");
					goto explainnetparse;
				}
				filename1 = argv[i+1];
				filename2 = argv[i+2];				
				break;
			}
			else if(strcmp(argv[i],"/v")==0)
			{
				options = OP_VIEW;
				filename1 = argv[i+1];
				break;
			}
			else
			{
				printf("\n\nUnknown options \"%s\"\n\n",argv[i]);
				goto explainnetparse;
			}
		}
	}

	switch(options)
	{
	case OP_NONE:
		break;
	case OP_COMPARE:
		if((fp1 = fopen(filename1,"rb"))==NULL)
		{
			printf("\n\nError opening %s.",filename1);
			return 0;
		}
		if((fp2 = fopen(filename2,"rb"))==NULL)
		{
			printf("\n\nError opening %s.",filename2);
			return 0;
		}
		
		//read in header (see universe.c for file structure ;)
		fread(&header1,sizeof(GameStatsDebugHeader),1,fp1);
		fread(&header2,sizeof(GameStatsDebugHeader),1,fp2);

		totalread += sizeof(GameStatsDebugHeader);
		printf("\nGame:\nNum Players: %d",header1.numPlayers);
		for(i=0;i<header1.numPlayers;i++)
		{
			printf("\nPlayer %d name: %s",i,header1.names[i]);
		}
		printf("\n\nHit Enter to Begin\n\nKilo Bytes Read/GameFrame#/GameTime H:M:S\n\n");
		sizetoread = sizeof(GameStats)-sizeof(PlayerStats)*(MAX_MULTIPLAYER_PLAYERS+1-header1.numPlayers);				
		getch();
		while(1)
		{
			fread(&frame1,sizeof(sdword),1,fp1);
			fread(&frame2,sizeof(sdword),1,fp2);
			
			if(feof(fp1) || feof(fp2))
			{
				printf("\n\nEnd of GameStatFrames.");
				break;
			}
			
			if(frame1 != frame2)
			{
				printf("\n\nSYNC ERROR:  Frame #'s are out of sync! Values %d and %d",frame1,frame2);
				_asm int 3;
				break;
			}
			{
				sdword seconds,minutes,hours;
				seconds = (udword) (((real32)frame1)*(1.0f/16.0f));
				hours = seconds/3600;
				seconds-=hours*3600;
				minutes = seconds/60;
				seconds-=minutes*60;
				fread(&machine1,sizetoread,1,fp1);
				fread(&machine2,sizetoread,1,fp2);
				totalread += sizetoread;
				printf("\r%d / %d / %02d:%02d:%02d",totalread/1000,frame1,hours,minutes,seconds);
				
			}
			if(feof(fp1) || feof(fp2))
			{
				printf("\n\nEnd of GameStatFrames.");
				break;
			}
			
			{
				sdword *m1=(sdword *)&machine1;
				sdword *m2=(sdword *)&machine2;

				for(i=0;i<(sdword)(sizeof(GameStats)-sizeof(PlayerStats)*(MAX_MULTIPLAYER_PLAYERS+1-header1.numPlayers))/4;i++)
				{
					if(*m1 != *m2)
					{
						printf("  -   Sync problem in block at sdword %d.  M1: %d  M2: %d\nHit b to int 3.",i,*m1,*m2);
						ch = getch();
						if(ch == 'q' || ch == 'Q')
							return 0;
						if(ch == 'b')
							_asm int 3;

					}
					m1++;
					m2++;
				}
			}
			//getch();
		}

		break;
	case OP_SYNC_TXT:
		if((fp1 = fopen(filename1,"rb"))==NULL)
		{
			printf("\n\nError opening %s.",filename1);
			return 0;
		}
		if((fp2 = fopen(filename2,"rb"))==NULL)
		{
			printf("\n\nError opening %s.",filename2);
			return 0;
		}
		fread(name1,20,1,fp1);
		fread(name2,20,1,fp2);
		
		

		fread(&univFrame1,sizeof(sdword),1,fp1);
		fread(&univFrame2,sizeof(sdword),1,fp2);

		if(univFrame1 != univFrame2)
		{
			printf("\nFrame # differences in File!!: M1: %d    M2: %d",univFrame1,univFrame2);
			exit(0);	
		}


		fread(&numShips1,sizeof(sdword),1,fp1);
		fread(&numShips2,sizeof(sdword),1,fp2);

		if(numShips1 != numShips2)
		{
			printf("\nSync Error: Number of Ships different: M1: %d    M2:  %d",numShips1,numShips2);
			exit(0);	
		}

		printf("\nM1: %s       M2: %s",name1,name2);
		printf("\nUniverse Frame #: %d",univFrame1);
		printf("\nTotal Number Of Ships: %d\n",numShips1);
		shipcount = 0;
		while(1)
		{
			fread(&ship1,sizeof(Ship),1,fp1);
			fread(&ship2,sizeof(Ship),1,fp2);
			shipcount++;

			printf("\rShip #: %d",shipcount);
			if(feof(fp1) || feof(fp2))
			{
				printf("\n\nEnd of sync Files.");
				break;
			}
			
			{
				sdword *m1 = (sdword *) &ship1;												
				sdword *m2 = (sdword *) &ship2;												

				for(i=0;i<sizeof(Ship)/4;i++)
				{
					if(*m1 != *m2)
					{
						printf("\nSync Error at Ship %d, sdword %d.  M1:  %d   M2:   %d\nHit b to int 3, q to quit, anything to continue.\n",shipcount,i,*m1,*m2);
						ch = getch();
						if(ch == 'b')
							_asm int 3;
						if(ch == 'q')
							exit(0);					
					}
					m1++;
					m2++;
				}
			}			
		}		
		break;
	case OP_VIEW:
		if((fp1 = fopen(filename1,"rb"))==NULL)
		{
			printf("\n\nError opening %s.",filename1);
			return 0;
		}

		sizeofHeader = sizeof(GodWritePacketHeader);
        m1Header = (GodWritePacketHeader *) malloc(sizeofHeader);

        fread(m1Header,sizeofHeader,1,fp1);
        		
        printf("\nMachine From         : %s",m1Header->playerName);
		printf("\nWindow Size #        : %d",m1Header->windowSize);
				
        for(i=0;i<m1Header->windowSize;i++)
        {
            //read size
            fread(&sizeofsnapshot,sizeof(sdword),1,fp1);
            //read snap shot!
            m1SnapShot = (UniverseSnapShot *)malloc(sizeofsnapshot);
            fread(m1SnapShot,sizeofsnapshot,1,fp1);
            printf("\nPacket %d     Univ Frame %d         NumShips %d",i,m1SnapShot->univUpdateCounterValue,m1SnapShot->numShips);
            free(m1SnapShot);
        }
        free(m1Header);
        break;
	case OP_SYNC_COMPARE:
		if((fp1 = fopen(filename1,"rb"))==NULL)
		{
			printf("\n\nError opening %s.",filename1);
			return 0;
		}
		if((fp2 = fopen(filename2,"rb"))==NULL)
		{
			printf("\n\nError opening %s.",filename2);
			return 0;
		}

		sizeofHeader = sizeof(GodWritePacketHeader);
        m1Header = (GodWritePacketHeader *) malloc(sizeofHeader);
        m2Header = (GodWritePacketHeader *) malloc(sizeofHeader);

        fread(m1Header,sizeofHeader,1,fp1);
        fread(m2Header,sizeofHeader,1,fp2);
        		
        printf("\nMachine 1 From  : %s       Machine 2 From  : %s",m1Header->playerName,m2Header->playerName);
		printf("\nWindow Size #   : %d       Window Size #   : %d\n\n",m1Header->windowSize,m2Header->windowSize);
				
        for(i=0;i<m1Header->windowSize;i++)
        {
            //read size
            fread(&sizeofsnapshot,sizeof(sdword),1,fp1);
            //read snap shot!
            m1SnapShot = (UniverseSnapShot *)malloc(sizeofsnapshot);
            fread(m1SnapShot,sizeofsnapshot,1,fp1);

			//read size from 2
            fread(&sizeofsnapshot,sizeof(sdword),1,fp2);
            //read snap shot!
            m2SnapShot = (UniverseSnapShot *)malloc(sizeofsnapshot);
            fread(m2SnapShot,sizeofsnapshot,1,fp2);

			if(m1SnapShot->numShips != m2SnapShot->numShips)
			{
				printf("\nPacket %d differs on # of ships:  M1: %d  M2: %d\n",i,m1SnapShot->numShips,m2SnapShot->numShips);
				_asm int 3;
			}
			if(m1SnapShot->univUpdateCounterValue != m2SnapShot->univUpdateCounterValue)
			{
				printf("\nPacket %d differs on Frame #'s: M1: %d   M2 %d\n",i,m1SnapShot->univUpdateCounterValue,m2SnapShot->univUpdateCounterValue);
				_asm int 3;
			}
			{
				sdword j,k;
				printf("\rWindow Packet# %d",i);
					
				for(j=0;j<(sdword) m1SnapShot->numShips;j++)
				{
					sdword *m1 = (sdword *) &m1SnapShot->ship[j];												
					sdword *m2 = (sdword *) &m2SnapShot->ship[j];
					m1SnapShot->ship[j].collInfo.selCircleDepth=0.0f;
					m2SnapShot->ship[j].collInfo.selCircleDepth=0.0f;
					m1SnapShot->ship[j].gravwellTimeEffect=0.0f;
					m2SnapShot->ship[j].gravwellTimeEffect=0.0f;				
					
					for(k=0;k<sizeof(Ship)/4;k++)
					{
						if(*m1 != *m2 && k != 78)
						{
                            printf("\nSync Anomaly at packet %d, ship # %d, sdword %d.  M1: %d  M2: %d",i,j,k,*m1,*m2);
                            ch =getch();
                            if(ch == 'b')
                                _asm int 3;
                            if(ch == 'q')
                                exit(0);
						}						
						m1++;
						m2++;
					}
				}
			}
            free(m1SnapShot);
            free(m2SnapShot);
        }
        free(m1Header);
        free(m2Header);
        break;
	}

	return 0;
}