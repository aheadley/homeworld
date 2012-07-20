/*=============================================================================
    Name    : CommandWrap.c
    Purpose : Wrapper functions for calling or sending over the network
              CommandLayer commands.

    Created 7/30/1997 by gshaw
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <string.h>
#include "Types.h"
#include "CommandLayer.h"
#include "CommandNetwork.h"
#include "CommandWrap.h"
#include "Universe.h"
#include "Globals.h"
#include "mainswitches.h"
#include "SoundEvent.h"
#include "Tutor.h"
#include "MultiplayerGame.h"
#include "SalCapCorvette.h"
#include "ShipSelect.h"
#include "Select.h"
#include "InfoOverlay.h"

/*-----------------------------------------------------------------------------
    Name        : clCommandMessage
    Description : Inserts a command message into the global gMessage variable
                  to be displayed on screen later
    Inputs      : String to be inserted into gMessage
    Outputs     : Modifies global gMessage variable
    Return      : void
----------------------------------------------------------------------------*/
void clCommandMessage(char CommandMessage[MAX_MESSAGE_LENGTH])
{
    ubyte i = 0;

    LockMutex(gMessageMutex);

    // find the next empty message slot in the global message array
    while ((i < MAX_MESSAGES)&&(gMessage[i].message[0] != (char)NULL))
    {
        i++;
    }

    gMessage[i].MessageExpire = universe.totaltimeelapsed + TW_MESSAGE_DELAY_TIME;
    strcpy(gMessage[i].message, CommandMessage);

    UnLockMutex(gMessageMutex);
}

/*-----------------------------------------------------------------------------
    Name        : clFlashShips
    Description : Sets all ships in selection to flash
    Inputs      : selection - the selection to flash
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void clFlashShips(SelectCommand *selection)
{
    udword i;

    for (i = 0; i < selection->numShips; i++)
    {
        selection->ShipPtr[i]->flashtimer = taskTimeElapsed;
    }
}

void SpeechEventsForFuel(SelectCommand *selectcom)
{
    sdword i;
    for(i=0;i<selectcom->numShips;i++)
    {
        if(selectcom->ShipPtr[0]->fuel == 0.0f)
        {
            /////////////////////
            //fuel speech event....
            //event num: COMM_Strike_OutOfFuelMove
            //battle chatter


            //////////////////
            return;
        }
    }


}

void clWrapMove(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    if (playPackets|universePause) return;

    makeShipsControllable(selectcom,COMMAND_MOVE);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Move Command Received");
#endif
    }

    SpeechEventsForFuel(selectcom);

    // ships selected to move flash for a few frames to indicate the selection
    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        if (tutorial)
        {
            tutGameMessage("Game_MoveIssued");
        }
        clMove(comlayer,selectcom,from,to);
    }
    else
    {
        clSendMove(comlayer,selectcom,from,to);
    }
}

void clWrapMpHyperspace(CommandLayer *comlayer,SelectCommand *selectcom,vector from,vector to)
{
    udword i;

    if (playPackets|universePause) return;

    //makeShipsControllable(selectcom,COMMAND_MP_HYPERSPACEING);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Hyperspace Command Received");
#endif
    }

    // make the ships unselectable

    // ships selected to move flash for a few frames to indicate the selection
    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clMpHyperspace(comlayer,selectcom,from,to);
    }
    else
    {
        clSendMpHyperspace(comlayer,selectcom,from,to);
    }
    for(i=0;i<selectcom->numShips;)
    {
        bitClear(selectcom->ShipPtr[i]->flags,SOF_Selectable);
        if(clRemoveShipFromSelection(&selSelected, selectcom->ShipPtr[i]))
        {
            ioUpdateShipTotals();
            continue;
        }
        i++;
    }
}


void clWrapAttack(CommandLayer *comlayer,SelectCommand *selectcom,AttackCommand *attackcom)
{
    if (playPackets|universePause) return;

    makeShipsControllable(selectcom,COMMAND_ATTACK);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips((SelectCommand *)attackcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Attack Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        if((tutorial==TUTORIAL_ONLY) && !tutEnable.bAttack)
            return;

        clAttack(comlayer,selectcom,attackcom);
    }
    else
    {
        clSendAttack(comlayer,selectcom,attackcom);
    }
}

void clWrapFormation(CommandLayer *comlayer,SelectCommand *selectcom,TypeOfFormation formation)
{
    if (playPackets|universePause) return;

    makeShipsControllable(selectcom,COMMAND_NULL);
    if (selectcom->numShips == 0) return;

    if (!((formation >= 0) && (formation < NO_FORMATION)))
    {
        dbgFatalf(DBG_Loc,"Invalid formation %d",formation);
    }

    if ((!universe.aiplayerProcessing) && (!((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)))
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Formation Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        if((tutorial==TUTORIAL_ONLY) && !tutEnable.bFormation)
            return;

        clFormation(comlayer,selectcom,formation);
    }
    else
    {
        clSendFormation(comlayer,selectcom,formation);
    }
}

void clWrapDock(CommandLayer *comlayer,SelectCommand *selectcom,DockType dockType,ShipPtr dockwith)
{
    if (playPackets|universePause) return;

    //Although research ships have docking capabilities, they aren't allowed to be
    //ordered to dock:
    makeShipsControllable(selectcom,COMMAND_DOCK);
    if (selectcom->numShips == 0) return;

    if(!(dockType & DOCK_FOR_RETIRE))
    {
         MakeSelectionNonResearchShips(selectcom);
    }

    if (selectcom->numShips == 0)
    {
        return;
    }

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Dock Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        if((tutorial==TUTORIAL_ONLY) && !tutEnable.bDock)
            return;

        clDock(comlayer,selectcom,dockType,dockwith);
    }
    else
    {
        clSendDock(comlayer,selectcom,dockType,dockwith);
    }

    if (!universe.aiplayerProcessing)
    {
        if (selectcom->ShipPtr[0]->dockvars.dockship != NULL)
        {
            if (dockType == DOCK_FOR_RETIRE)
            {
                if (isCapitalShip(selectcom->ShipPtr[0]))
                {
                    speechEvent(selectcom->ShipPtr[0], COMM_Cap_Retire, 0);
                }
                else
                {
                    speechEvent(selectcom->ShipPtr[0], COMM_Strike_Retired, 0);
                }
            }
            else
            {
                speechEvent(selectcom->ShipPtr[0], COMM_Strike_Dock, selectcom->ShipPtr[0]->dockvars.dockship->shiptype);
            }
        }
    }
}

void clWrapDeterministicBuild(udword command,
                              CommandLayer* comlayer, sdword numShips,
                              ShipType shipType, ShipRace shipRace,
                              uword playerIndex, ShipPtr creator)
{
    if (playPackets) return;

    if (!universe.aiplayerProcessing)
    {
#if CL_TEXTFEEDBACK
        //@todo verbosify this lame message
        if (enableTextFeedback) clCommandMessage("New Build Command");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clDeterministicBuild(command, comlayer, numShips, shipType, shipRace, playerIndex, creator);
    }
    else
    {
        clSendDeterministicBuild(command, comlayer, numShips, shipType, shipRace, playerIndex, creator);
    }
}

void clWrapCreateShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    if (playPackets) return;

    if (!universe.aiplayerProcessing)
    {
#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("New Ship Created");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clCreateShip(comlayer,shipType,shipRace,playerIndex,creator);
    }
    else
    {
        clSendCreateShip(comlayer,shipType,shipRace,playerIndex,creator);
    }

}

void clWrapBuildShip(CommandLayer *comlayer,ShipType shipType,ShipRace shipRace,uword playerIndex,ShipPtr creator)
{
    if (playPackets) return;

    if (!universe.aiplayerProcessing)
    {
#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("New Ship Being Built");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clBuildShip(comlayer,shipType,shipRace,playerIndex,creator);
    }
    else
    {
        clSendBuildShip(comlayer,shipType,shipRace,playerIndex,creator);
    }

}

void clWrapCollectResource(CommandLayer *comlayer,SelectCommand *selectcom,ResourcePtr resource)
{
    if (playPackets|universePause) return;

    makeShipsControllable(selectcom,COMMAND_COLLECTRESOURCE);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

        if (resource != NULL)
        {
            resource->flashtimer = taskTimeElapsed;
        }
#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Harvest Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clCollectResource(comlayer,selectcom,resource);
    }
    else
    {
        clSendCollectResource(comlayer,selectcom,resource);
    }

}

void clWrapProtect(CommandLayer *comlayer,SelectCommand *selectcom,ProtectCommand *protectcom)
{
    if (playPackets|universePause) return;

    makeShipsControllable(selectcom,COMMAND_NULL);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips((SelectCommand *)protectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Protect Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clProtect(comlayer,selectcom,protectcom);
    }
    else
    {
        clSendProtect(comlayer,selectcom,protectcom);
    }

}

void clWrapSpecial(CommandLayer *comlayer,SelectCommand *selectcom,SpecialCommand *targets)
{
    sdword i,sals;
    SelectCommand *copycom=NULL;
    sdword sizeofcopycom;
	Ship *salptr;
    if (playPackets|universePause) return;

    makeShipsControllable(selectcom,COMMAND_SPECIAL);
    if (selectcom->numShips == 0) return;

    sizeofcopycom = sizeofSelectCommand(selectcom->numShips);
    copycom = memAlloc(sizeofcopycom,"clspecialfix", Pyrophoric);
    memcpy(copycom,selectcom,sizeofcopycom);

    if (!universe.aiplayerProcessing)
    {
        //if the special operation has a target, flash the targets,
        //else flash the ships performing the operation
        if (targets)
        {
            clFlashShips((SelectCommand *)targets);
        }
        else
        {
            clFlashShips(selectcom);
        }

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Special Ability Activated");
#endif
    }

    if(selectcom->ShipPtr[0]->playerowner == universe.curPlayerPtr)
    {
        //count number of salcap corvettes
        sals = 0;
        for(i=0;i<selectcom->numShips;i++)
        {
            if(selectcom->ShipPtr[i]->shiptype == SalCapCorvette)
            {
                salptr=selectcom->ShipPtr[i];
				sals++;
            }
        }
        if(sals > 0)
        {
            //some salcaps in list
            //make sure sals > numNeededToSalvage of first target...
            if(targets->TargetPtr[0]->objtype == OBJ_ShipType)
			{
				if(!(targets->TargetPtr[0]->attributes & ATTRIBUTES_StripTechnology))
                {
                    //target isn't a technology whore.
                    if(((Ship *)targets->TargetPtr[0])->salvageInfo != NULL)
    				{
    					//if it is NULL we wouldn't care because you would only need
    					//one salcap which we for sure have
    					if(((Ship *)targets->TargetPtr[0])->salvageInfo->numNeededForSalvage - ((Ship *)targets->TargetPtr[0])->salvageNumTagged[universe.curPlayerIndex] > sals)
    					{
    						//number still needed minus number
    						//already going for target is more than just ordered
    						///need more
    						//play speech event
    						speechEvent(salptr, SP_Pilot_SCVetteNotEnough, 0);
    						goto nonormalsalevent;
    					}
    				}
                }
			}
			if(isThereAnotherTargetForMe(selectcom->ShipPtr[0],(SelectAnyCommand *)targets))
			{
				speechEvent(selectcom->ShipPtr[0], COMM_SCVette_Salvage, 0);
			}
        }
		
        /* salvage command given */
nonormalsalevent:;
    }
        if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        if((tutorial==TUTORIAL_ONLY) && !tutEnable.bSpecial)
            return;

        for(i=0;i<copycom->numShips;)
        {
            if(copycom->ShipPtr[i]->shiptype == SalCapCorvette)
            {
                //sal in thing, remove and order seperatly
                SelectCommand selectOne;
                selectOne.numShips = 1;
                selectOne.ShipPtr[0] = copycom->ShipPtr[i];
                clSpecial(comlayer,&selectOne,targets);
                copycom->numShips--;
                copycom->ShipPtr[i] = copycom->ShipPtr[copycom->numShips];
                continue;
            }
            i++;
        }
        if (copycom->numShips > 0)
        {
            clSpecial(comlayer,copycom,targets);
        }
    }
    else
    {

        for(i=0;i<copycom->numShips;)
        {
            if(copycom->ShipPtr[i]->shiptype == SalCapCorvette)
            {
                //sal in thing, remove and order seperatly
                SelectCommand selectOne;
                selectOne.numShips = 1;
                selectOne.ShipPtr[0] = copycom->ShipPtr[i];
                clSendSpecial(comlayer,&selectOne,targets);
                copycom->numShips--;
                copycom->ShipPtr[i] = copycom->ShipPtr[copycom->numShips];
                continue;
            }
            i++;
        }
        if (copycom->numShips > 0)
        {
            clSendSpecial(comlayer,copycom,targets);
        }
    }
    memFree(copycom);
}

void clWrapHalt(CommandLayer *comlayer,SelectCommand *selectcom)
{
    if (playPackets|universePause) return;

    makeShipsControllable(selectcom,COMMAND_HALT);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Halt Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clHalt(comlayer,selectcom);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_HALT,0);
    }
}

void clWrapScuttle(CommandLayer *comlayer,SelectCommand *selectcom)
{
    if (playPackets|universePause) return;

    //makeShipsControllable(selectcom);     // always allow scuttling (in case ship gets stuck in dock bay or something)
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Scuttle Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clScuttle(comlayer,selectcom);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_SCUTTLE,0);
    }
}

void clWrapAutoLaunch(udword OnOff,udword playerIndex)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clAutoLaunch(OnOff,playerIndex);
    }
    else
    {
        clSendAutoLaunch(OnOff,playerIndex);
    }
}

void clWrapSetAlliance(udword AllianceType, uword playerone, uword playertwo)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetAlliance(AllianceType, playerone, playertwo);
    }
    else
    {
        clSendSetAlliance(AllianceType, playerone, playertwo);
    }

}

void clWrapLaunchMultipleShips(CommandLayer *comlayer,SelectCommand *selectcom,ShipPtr launchFrom)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clLaunchMultipleShips(comlayer,selectcom,launchFrom);
    }
    else
    {
        clSendLaunchMultipleShips(comlayer,selectcom,launchFrom);
    }
}

void clWrapSetTactics(CommandLayer *comlayer,SelectCommand *selectcom,TacticsType tacticstype)
{
    if (playPackets|universePause) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Tactics Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetTactics(comlayer,selectcom,tacticstype);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_TACTICS,tacticstype);
    }
}

void clWrapRUTransfer(CommandLayer *comlayer,sdword fromIndex,sdword toIndex, sdword resourceUnits,ubyte flags)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        //needed for ResourceDeductions that are deterministic (building crud)
        clRUTransfer(comlayer,toIndex,fromIndex,resourceUnits,flags);
    }
    else
    {
        clSendRuTransfer(comlayer,fromIndex,toIndex,resourceUnits,flags);
    }

}

void clWrapSetKamikaze(CommandLayer *comlayer,SelectCommand *selectcom)
{
    if (playPackets|universePause) return;

    makeShipsControllable(selectcom,COMMAND_NULL);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Kamikaze Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetKamikaze(comlayer,selectcom);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_KAMIKAZE,0);  //0 = dummy num
    }
}

void clWrapSetMilitaryParade(CommandLayer *comlayer,SelectCommand *selectcom)
{
    if (playPackets|universePause) return;

    makeShipsControllable(selectcom,COMMAND_NULL);
    if (selectcom->numShips == 0) return;

    if (!universe.aiplayerProcessing)
    {
        clFlashShips(selectcom);

#if CL_TEXTFEEDBACK
        if (enableTextFeedback) clCommandMessage("Parade Command Received");
#endif
    }

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetMilitaryParade(comlayer,selectcom);
    }
    else
    {
        clSendMisc(comlayer,selectcom,MISCCOMMAND_PARADE,0);  //0 = dummy num
    }
}

void clWrapResearch(udword type, udword playernum, udword labnum, udword tech)
{
    if (playPackets) return;

    if ((!multiPlayerGame) && (!recordFakeSendPackets))
    {
        clSetResearch(type, playernum, labnum, tech);
    }
    else
    {
        clSendResearch(type, playernum, labnum, tech);
    }
}
