
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "types.h"
#include "commandnetworksimple.h"

// 'PACK'
#define VALIDCHECK 0x4b434150
#define HEADERCHECK 0x44414548

#define print printf

FILE *fp;
int verbose = 0;

ubyte *GetNextPacket(sdword *retsize)
{
    udword size;
    udword validcheck;
    ubyte *packet;

    *retsize = -1;

tryagain:

    if (fread(&validcheck,sizeof(udword),1,fp) == 0) return NULL;
    if (validcheck == HEADERCHECK)      // skip headers
    {
        if (fread(&size,sizeof(udword),1,fp) == 0) return NULL;
        packet = malloc(size);
        fread(packet,size,1,fp);
        free(packet);
        goto tryagain;
    }
    if (validcheck != VALIDCHECK)
    {
        return NULL;
    }
    if (fread(&size,sizeof(udword),1,fp) == 0) return NULL;
    packet = malloc(size);

    fread(packet,size,1,fp);
    *retsize = size;

    return packet;
}

void PrintSelection(NetSelection *netselect)
{
    sdword i;
    for (i=0;i<netselect->numShips;i++)
    {
        print("%d ",netselect->ShipID[i].shipNumber);
    }
}

void PrintTargets(NetTargetsSelection *nettargets)
{
    TargetID targetID;
    sdword i;
    for (i=0;i<nettargets->numTargets;i++)
    {
        targetID = nettargets->TargetID[i];
        if (targetID.objtype == 0)
        {
            print("%d ",targetID.objNumber);
        }
        else
        {
            print("(%d,%d) ",targetID.objtype,targetID.objNumber);
        }
    }
}

void PrintPacketInfo(ubyte *packet,udword size)
{
    ubyte *curpacket;
    udword numberOfCommandsLeft;
    uword commandType;

#define hwpacket ((HWPacketHeader *)packet)

    numberOfCommandsLeft = (udword)(hwpacket)->numberOfCommands;

    if (verbose)
    {       
        print("\n%x Pkt %d F: %d S: %d #= %d (PktNum %d Rand %d Univ %f Blob %f)",
                hwpacket->type,hwpacket->frame,hwpacket->from,size,numberOfCommandsLeft,
                hwpacket->checksums.packetnum,hwpacket->checksums.randcheck,hwpacket->checksums.univcheck,hwpacket->checksums.blobcheck);       

        if (hwpacket->type != PACKETTYPE_SYNC)
            return;
    }
    else
    {
        if (hwpacket->type == PACKETTYPE_SYNC)
        {
            if (numberOfCommandsLeft == 0)
                print(".");
            else
                print("\nPkt %d: F: %d S: %d #= %d",hwpacket->frame,hwpacket->from,size,numberOfCommandsLeft);
        }
        else
        {
            print("\nUnknown Pkt Size %d:",size);
            return;
        }
    }

    curpacket = packet + sizeof(HWPacketHeader);

    while (numberOfCommandsLeft > 0)
    {
        commandType = ((HWCommandHeader *)curpacket)->commandType & 255;
        curpacket += sizeof(HWCommandHeader);

        switch (commandType)
        {
            case COMMANDTYPE_MOVE:
            {
                NetMoveCommand *movecom = (NetMoveCommand *)curpacket;
                print("\n\tMOVE: From: %f %f %f To: %f %f %f Selection: ",movecom->from.x,movecom->from.y,movecom->from.z,
                                                                         movecom->to.x,movecom->to.y,movecom->to.z);
                PrintSelection(&movecom->selection);
                curpacket += sizeofNetMoveCommand(movecom->selection.numShips);
            }
            break;

            case COMMANDTYPE_MP_HYPERSPACE:
            {
                NetMoveCommand *movecom = (NetMoveCommand *)curpacket;
                print("\n\tMOVEHP: From: %f %f %f To: %f %f %f Selection: ",movecom->from.x,movecom->from.y,movecom->from.z,
                                                                         movecom->to.x,movecom->to.y,movecom->to.z);
                PrintSelection(&movecom->selection);
                curpacket += sizeofNetMoveCommand(movecom->selection.numShips);
            }
            break;

            case COMMANDTYPE_ATTACK:
            {
                NetAttackSelection *netattackselection = (NetAttackSelection *)curpacket;
                NetSelection *netselection = (NetSelection *)(curpacket + sizeofNetAttackSelection(netattackselection->numTargets));
                print("\n\tATTACK: Selection: ");
                PrintSelection(netselection);
                print("attacks Targets: ");
                PrintTargets(netattackselection);
                curpacket += sizeofNetAttackCommand(netattackselection->numTargets,netselection->numShips);
            }
            break;

            case COMMANDTYPE_SPECIAL:
            {
                NetTargetsSelection *nettargetsselection = (NetTargetsSelection *)curpacket;
                sdword nettargetsselectionNumShips = nettargetsselection->numTargets;
                NetSelection *netselection = (NetSelection *)(curpacket + sizeofNetTargetsSelection(nettargetsselectionNumShips));
                print("\n\tSPECIAL: Selection: ");
                PrintSelection(netselection);
                if (nettargetsselectionNumShips > 0)
                {
                    print("targets: ");
                    PrintTargets(nettargetsselection);
                }
                else
                {
                    print("No Targets Present");
                }
                curpacket += sizeofNetSpecialCommand(nettargetsselectionNumShips,netselection->numShips);
            }
            break;

            case COMMANDTYPE_FORMATION:
            {
                NetFormationCommand *formationcom = (NetFormationCommand *)curpacket;
                print("\n\tFORMATION %d: Selection: ",formationcom->typeOfFormation);
                PrintSelection(&formationcom->selection);
                curpacket += sizeofNetFormationCommand(formationcom->selection.numShips);
            }
            break;

            case COMMANDTYPE_DOCK:
            {
                NetDockCommand *dockcom = (NetDockCommand *)curpacket;
                print("\n\tDOCK Type:%d Dockwith: %d",dockcom->dockType,dockcom->dockwithID);
                print("Selection: ");
                PrintSelection(&dockcom->selection);
                curpacket += sizeofNetDockCommand(dockcom->selection.numShips);
            }
            break;

            case COMMANDTYPE_LAUNCHMULTIPLE:
            {
                NetLaunchMultipleCommand *launchcom = (NetLaunchMultipleCommand *)curpacket;
                print("\n\tLAUNCH: From: %d Selection: ",launchcom->launchfromID.shipNumber);
                PrintSelection(&launchcom->selection);
                curpacket += sizeofNetLaunchMultipleCommand(launchcom->selection.numShips);
            }
            break;

            case COMMANDTYPE_MISC:
            {
                NetMiscCommand *misccom = (NetMiscCommand *)curpacket;
                switch (misccom->miscCommand)
                {
                    case MISCCOMMAND_HALT:
                        print("\n\tHALT: ");
                        break;

                    case MISCCOMMAND_SCUTTLE:
                        print("\n\tSCUTTLE: ");
                        break;

                    case MISCCOMMAND_TACTICS:
                        print("\n\tTACTICS: %d ",misccom->miscData);
                        break;

                    case MISCCOMMAND_KAMIKAZE:
                        print("\n\tKAMIKAZE: ");
                        break;

                    case MISCCOMMAND_PARADE:
                        print("\n\tMILITARY PARADE");
                        break;

                    default:
                        print("\n\tUNKNOWN MISC COMMAND %d: ",misccom->miscCommand);
                        break;
                }
                print("Selection: ");
                PrintSelection(&misccom->selection);
                curpacket += sizeofNetMiscCommand(misccom->selection.numShips);
            }
            break;

            case COMMANDTYPE_AUTOLAUNCH:
            {
                NetAutolaunchCommand *launchcom = (NetAutolaunchCommand *)curpacket;
                print("\n\tAUTOLAUNCH: Player %d turns on %d",launchcom->playerIndex,launchcom->OnOff);
                curpacket += sizeofNetAutolaunchCommand;
            }
            break;

            case COMMANDTYPE_ALLIANCEINFO:
            {
                NetAllianceCommand *alliancecom = (NetAllianceCommand *)curpacket;
                print("\n\tALLIANCE: Msg: %d Cur: %d New: %d",alliancecom->AllianceMsgType, alliancecom->CurAlliance, alliancecom->NewAlliance);
                curpacket += sizeofNetAllianceCommand;
            }
            break;

            case COMMANDTYPE_CREATESHIP:
            {
                NetCreateShipCommand *createcom = (NetCreateShipCommand *)curpacket;
                print("\n\tCREATESHIP: %d %d of Player %d Built by Ship: %d",createcom->shipType,createcom->shipRace,createcom->playerIndex,createcom->creatorID.shipNumber);
                curpacket += sizeofNetCreateShipCommand;
            }
            break;

            case COMMANDTYPE_BUILDSHIP:
            {
                NetCreateShipCommand *createcom = (NetCreateShipCommand *)curpacket;
                print("\n\tBUILDSHIP: %d %d of Player %d Built by Ship: %d",createcom->shipType,createcom->shipRace,createcom->playerIndex,createcom->creatorID.shipNumber);
                curpacket += sizeofNetCreateShipCommand;
            }
            break;

            case COMMANDTYPE_COLLECTRESOURCE:
            {
                NetCollectResourceCommand *collectcom = (NetCollectResourceCommand *)curpacket;
                print("\n\tHARVEST: Selection: ");
                PrintSelection(&collectcom->selection);
                print("Resouce: %d",collectcom->resourceID.resourceNumber);
                curpacket += sizeofNetCollectResourceCommand(collectcom->selection.numShips);
            }
            break;

            case COMMANDTYPE_PROTECT:
            {
                NetProtectSelection *netprotectselection = (NetProtectSelection *)curpacket;
                NetSelection *netselection = (NetSelection *)(curpacket + sizeofNetProtectSelection(netprotectselection->numShips));
                print("\n\tGUARD: Selection: ");
                PrintSelection(netselection);
                print("Protecting: ",netprotectselection);
                curpacket += sizeofNetProtectCommand(netprotectselection->numShips,netselection->numShips);
            }
            break;

            case COMMANDTYPE_RUTRANSFER:
            {
                NetRUTransferferCommand *rucommand = (NetRUTransferferCommand *)curpacket;
                print("\n\tRUTransfer: from %d to %d RU %d Flags %d",rucommand->from,rucommand->to,rucommand->resourceUnits,rucommand->flags);
                curpacket+=sizeof(NetRUTransferferCommand);
            }
            break;

            case COMMANDTYPE_GODSYNC:
            {
                print("\n\tSYNCCHECK");
                curpacket += sizeof(GodSyncCommand);
            }
            break;

            case COMMANDTYPE_DETERMINISTICBUILD:
            {
                NetDeterministicBuildCommand* buildcom = (NetDeterministicBuildCommand*)curpacket;
                print("\n\tDBUILD: %d #:%d (%d %d) Player %d by Ship %d",buildcom->subCommand,buildcom->numShips,
                      buildcom->shipType,buildcom->shipRace,buildcom->playerIndex,buildcom->creatorID.shipNumber);
                curpacket += sizeofNetDeterministicBuildCommand;
            }
            break;

            case COMMANDTYPE_RESEARCHINFO:
            {
                NetResearchCommand *researchcommand = (NetResearchCommand *)curpacket;
                print("\n\tRESEARCH: %d %d %d %d",(udword)researchcommand->ResearchMsgType, (udword)researchcommand->playerindex,
                      (udword)researchcommand->labnum, (udword)researchcommand->tech);
                curpacket+=sizeofNetResearchCommand;
            }
            break;

            default:
                print("\n\tUNKNOWN COMMAND %d",commandType);
                return;
        }

        numberOfCommandsLeft--;
    }
}

int main(int argc,char *argv[])
{
    char *filename;

    sdword size;
    ubyte *packet;

    if (argc < 2)
    {
        print("Usage: PktView filename.pkts [verbose]");
        return 0;
    }

    filename = argv[1];
    
    verbose = 0;
    if (argc >= 3)
    {
        if (stricmp(argv[2],"verbose") == 0)
        {
            verbose = 1;
        }
    }

    fp = fopen(filename,"rb");
    if (fp == NULL)
    {
        print("Error opening file %s",filename);
        return 0;
    }

    for (;;)
    {
        packet = GetNextPacket(&size);
        if (packet == NULL)
            break;
        if (size <= 0)
        {
            free(packet);
            break;
        }

        PrintPacketInfo(packet,size);
        free(packet);
    }

    fclose(fp);
    return 0;
}

