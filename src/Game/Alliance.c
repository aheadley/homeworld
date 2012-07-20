/*=============================================================================
    Name    : Alliance.c
    Purpose : This file handles all of the logic for forming and breaking alliances

    Created 7/31/1998 by ddunlop
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#include <stdio.h>
#include "Alliance.h"
#include "GameChat.h"
#include "Chatting.h"
#include "utility.h"
#include "Universe.h"
#include "CommandNetwork.h"
#include "CommandLayer.h"
#include "CommandWrap.h"
#include "Strings.h"
#include "SoundEvent.h"

/*=============================================================================
    Data:
=============================================================================*/

color allianceMessageColor = colRGB(255,255,255);

/*=============================================================================
    Function prototypes for some private functions:
=============================================================================*/

//void alliancePrintNamesNice(char *dest, uword alliance);
void allianceRemovePlayerShipsFromAttackList(SelectAnyCommand *list,uword playerindex,CommandToDo *todo);
void allianceCancelAttackOrders(uword playerone, uword playertwo);

/*=============================================================================
    Function logic for Alliances:
=============================================================================*/


/*-----------------------------------------------------------------------------
    Name        : allianceFormWith
    Description : sends a request to form an alliance with a player.
    Inputs      :
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void allianceFormWith(udword playerindex)
{
    char temp[256];

    // Just request to ally with the person you requested.
    sprintf(temp, "%s %s", strGetString(strAllianceRequest), playerNames[playerindex]);
//    alliancePrintNamesNice(temp, (uword)PLAYER_MASK(playerindex));
    gcProcessGameTextMessage(temp,allianceMessageColor);

    sendAllianceRequest(PLAYER_MASK(playerindex),(uword)sigsPlayerIndex,ALLIANCE_PACKET, 0);

    bitSet(universe.players[sigsPlayerIndex].AllianceProposals, PLAYER_MASK(playerindex));

    universe.players[sigsPlayerIndex].AllianceProposalsTimeout = universe.univUpdateCounter + UNIVERSE_UPDATE_RATE * 90;

/*    // are we in an alliance now ??
    if ((universe.players[sigsPlayerIndex].Allies&ALL_PLAYER_MASK)==0)
    {
        // no we are not in an alliance
        // straigtforward request to ally
        sprintf(temp, "%s ", strGetString(strAllianceRequest));
        alliancePrintNamesNice(temp, (uword)PLAYER_MASK(playerindex));
        gcProcessGameTextMessage(temp,allianceMessageColor);
        sendAllianceRequest(PLAYER_MASK(playerindex),(uword)sigsPlayerIndex,ALLIANCE_PACKET, 0);
        universe.players[sigsPlayerIndex].AllianceProposals=PLAYER_MASK(playerindex);
    }
    // do i accept the proposed new ally in our alliance
    else if (playerindex == universe.players[sigsPlayerIndex].AllianceRequestToConfirm)
    {
        // yes i accept him into our allience
        // send a message to the request initiator telling him of my acceptance.
        sprintf(temp, "%s ", strGetString(strAllianceConfirm));
        alliancePrintNamesNice(temp, (uword)PLAYER_MASK(playerindex));
        gcProcessGameTextMessage(temp,allianceMessageColor);
        sendAllianceRequest(PLAYER_MASK(universe.players[sigsPlayerIndex].AllianceRequestInitiator),(uword)sigsPlayerIndex,ALLIANCE_GRANTED, 0);

        universe.players[sigsPlayerIndex].AllianceRequestToConfirm = -1;
    }
    else
    {
        // otherwise i want a new person to join our allience
        // must ask all of my allies if he can join or not.
        sprintf(temp, "%s ", strGetString(strAskForPermision));
        alliancePrintNamesNice(temp, (uword)PLAYER_MASK(playerindex));
        gcProcessGameTextMessage(temp,allianceMessageColor);

        sendAllianceRequest(universe.players[sigsPlayerIndex].Allies, (uword)sigsPlayerIndex, ALLIANCE_NEWJOIN, (ubyte)playerindex);

        // save all of the people that must confirm this alliance request.
        universe.players[sigsPlayerIndex].AllianceConfirms = universe.players[sigsPlayerIndex].Allies;
        universe.players[sigsPlayerIndex].AllianceRequestToConfirm = (ubyte)playerindex;
    }*/
}

/*-----------------------------------------------------------------------------
    Name        : allianceBreakWith
    Description : breaks an alliance with a player.
    Inputs      : player to break alliance with
    Outputs     : none
    Return      : void
----------------------------------------------------------------------------*/
void allianceBreakWith(udword playerindex)
{
    if (bitTest(universe.players[sigsPlayerIndex].Allies,PLAYER_MASK(playerindex)))
    {
        clWrapSetAlliance(ALLIANCE_BREAKALLIANCE, (uword)sigsPlayerIndex, (uword)playerindex);
    }
}


/*-----------------------------------------------------------------------------
    Name        : allianceFormRequestRecievedCB
    Description : called when a request to from an alliance is recieved.
    Inputs      : alliance packet
    Outputs     : nont
    Return      : void
----------------------------------------------------------------------------*/
void allianceFormRequestRecievedCB(ChatPacket *packet)
{
    char temp[128];

    switch (packet->messageType)
    {
        case ALLIANCE_PACKET:
        {
            if (bitTest(universe.players[sigsPlayerIndex].AllianceProposals, PLAYER_MASK(packet->packetheader.frame)))
            {
                bitClear(universe.players[sigsPlayerIndex].AllianceProposals, PLAYER_MASK(packet->packetheader.frame));

                clWrapSetAlliance(ALLIANCE_FORMNEWALLIANCE, (uword)sigsPlayerIndex, (uword)packet->packetheader.frame);
            }
            else
            {
                sprintf(temp,"%s %s",playerNames[packet->packetheader.frame], strGetString(strAsksToFormAlliance));

                gcProcessGameTextMessage(temp, allianceMessageColor);
            }
        }
        break;
/*  OBSOLETE
        case ALLIANCE_NEWJOIN:
        {
            sprintf(temp, "%s %s %s %s",playerNames[packet->packetheader.frame], strGetString(strWants), playerNames[packet->message[0]], strGetString(strToJoin));
            gcProcessGameTextMessage(temp,allianceMessageColor);

            universe.players[sigsPlayerIndex].AllianceRequestToConfirm = (uword)packet->message[0];
            universe.players[sigsPlayerIndex].AllianceRequestInitiator = (uword)packet->packetheader.frame;
        }
        break;
        case ALLIANCE_GRANTED:
        {
            if (bitTest(universe.players[sigsPlayerIndex].AllianceConfirms,PLAYER_MASK(packet->packetheader.frame)))
            {
                bitClear(universe.players[sigsPlayerIndex].AllianceConfirms,PLAYER_MASK(packet->packetheader.frame));

                if (universe.players[sigsPlayerIndex].AllianceConfirms==0)
                {
                    sendAllianceRequest(PLAYER_MASK(universe.players[sigsPlayerIndex].AllianceRequestToConfirm),(uword)sigsPlayerIndex,ALLIANCE_PACKET, 0);
                    universe.players[sigsPlayerIndex].AllianceProposals=PLAYER_MASK(universe.players[sigsPlayerIndex].AllianceRequestToConfirm);
                    universe.players[sigsPlayerIndex].AllianceRequestToConfirm = -1;
                }
            }
        }
        break;
        */
        /* //obsolete Now
        case ALLIANCE_RUTRANSFER:
            universe.players[sigsPlayerIndex].resourceUnits += packet->data;
        break;
        */
    }
}

/*-----------------------------------------------------------------------------
    Name        : alliancePrintNamesNice
    Description : returns a string witht the player names printed nicely.
    Inputs      : The string to print to, and the alliance.s
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void alliancePrintNamesNice(char *dest, uword alliance)
{
    udword i,j,index,numallies=0;

    for (i=0;i<universe.numPlayers;i++)
    {
        if (bitTest(alliance,PLAYER_MASK(i)))
        {
            if (i!=sigsPlayerIndex)
            {
                numallies++;
            }
        }
    }

    if (numallies >= 2)
    {
        for (j=0,index=0;j<universe.numPlayers;j++)
        {
            if ( (bitTest(alliance, PLAYER_MASK(j))) &&
                  (j != sigsPlayerIndex) )
            {
                strcat(dest,playerNames[j]);
                index++;
                if (index<numallies-1)
                {
                    strcat(dest, ", ");
                }
                else if (index==numallies-1)
                {
                    strcat(dest, " ");
                    strcat(dest,strGetString(strAnd));
                    strcat(dest, " ");
                }
            }
        }
    }
    else
    {
        for (j=0,index=0;j<universe.numPlayers;j++)
        {
            if ( (bitTest(alliance, PLAYER_MASK(j))) &&
                  (j != sigsPlayerIndex) )
            {
                strcat(dest,playerNames[j]);
                break;
            }
        }
    }
}


/*-----------------------------------------------------------------------------
    Name        : allianceSetAlliance
    Description : Sets an alliance based on a player bitmask.
    Inputs      : player bitmask
    Outputs     :
    Return      : void
----------------------------------------------------------------------------*/
void allianceSetAlliance(udword AllianceType, uword playerone, uword playertwo)
{
    char   temp[128];

    universe.collUpdateAllBlobs = TRUE;                     //alliances mean the blobs have to be re-created from scratch
    switch (AllianceType)
    {
        // New alliance is being formed.
        case ALLIANCE_FORMNEWALLIANCE:
        {
            // update player information about an alliance
            if (universe.players[playerone].playerState != PLAYER_DEAD)
            {
                bitSet(universe.players[playerone].Allies, PLAYER_MASK(playertwo));
                bitClear(universe.players[playerone].AllianceProposals, PLAYER_MASK(playertwo));
                if (playerone == universe.curPlayerIndex)   // is this the current player
                {
                    // play the Alliance formed speech event
                    speechEventFleet(COMM_F_AllianceFormed, playertwo, playerone);
                }

            }
            if (universe.players[playertwo].playerState != PLAYER_DEAD)
            {
                bitSet(universe.players[playertwo].Allies, PLAYER_MASK(playerone));
                bitClear(universe.players[playertwo].AllianceProposals, PLAYER_MASK(playerone));
                if (playertwo == universe.curPlayerIndex)   // is this the current player
                {
                    // play the Alliance formed speech event
                    speechEventFleet(COMM_F_AllianceFormed, playerone, playertwo);
                }
            }

            // function cancels any attack orders between allied players
            allianceCancelAttackOrders(playerone, playertwo);

            // print out an appropriate message to the player if their alliance has changed
            if (sigsPlayerIndex==playerone)
            {
                strcpy(temp," ");
                sprintf(temp, "%s %s", strGetString(strAllianceFormed), playerNames[playertwo]);
                //alliancePrintNamesNice(temp,newalliance);
                gcProcessGameTextMessage(temp, allianceMessageColor);
            }
            if (sigsPlayerIndex==playertwo)
            {
                strcpy(temp," ");
                sprintf(temp, "%s %s", strGetString(strAllianceFormed), playerNames[playerone]);
                //alliancePrintNamesNice(temp,newalliance);
                gcProcessGameTextMessage(temp, allianceMessageColor);
            }

/* OBSOLETE
            for (index=0;index<universe.numPlayers;index++)
            {
                if (universe.players[index].playerState != PLAYER_DEAD)
                {
                    if (playerone = index)   // this player is in the alliance
                    {
                        // mark this players Allies field (exclude himself)
                        universe.players[index].Allies = newalliance^PLAYER_MASK(index);

                        if (index == universe.curPlayerIndex)   // is this the current player
                        {
                            // play the Alliance formed speech event
                            speechEventFleet(COMM_F_AllianceFormed, universe.players[index].Allies, index);
                        }
                    }
                }
            }*/
        }
        break;
        // current alliance is beging broken.
        case ALLIANCE_BREAKALLIANCE:
        {
            // update player information about an alliance
            if (universe.players[playerone].playerState != PLAYER_DEAD)
            {
                bitClear(universe.players[playerone].Allies, PLAYER_MASK(playertwo));
                bitClear(universe.players[playerone].AllianceProposals, PLAYER_MASK(playertwo));
                if (playerone == universe.curPlayerIndex)   // is this the current player
                {
                    // play the Alliance broken speech event
                    speechEventFleet(COMM_F_AllianceBroken, playertwo, playerone);
                }

            }
            if (universe.players[playertwo].playerState != PLAYER_DEAD)
            {
                bitClear(universe.players[playertwo].Allies, PLAYER_MASK(playerone));
                bitClear(universe.players[playertwo].AllianceProposals, PLAYER_MASK(playerone));
                if (playertwo == universe.curPlayerIndex)   // is this the current player
                {
                    // play the Alliance broken speech event
                    speechEventFleet(COMM_F_AllianceBroken, playerone, playertwo);
                }
            }

            // print out an appropriate message to the player if their alliance has changed
            if (sigsPlayerIndex==playerone)
            {
                strcpy(temp," ");
                sprintf(temp, "%s %s", strGetString(strAllianceBroken), playerNames[playertwo]);
                gcProcessGameTextMessage(temp, allianceMessageColor);
            }
            if (sigsPlayerIndex==playertwo)
            {
                strcpy(temp," ");
                sprintf(temp, "%s %s", playerNames[playerone], strGetString(strHasBrokenAlliance));
                gcProcessGameTextMessage(temp, allianceMessageColor);
            }
        }
        break;
/*            // update the player information for an existing alliance
            for (index=0;index<universe.numPlayers;index++)
            {
                if (universe.players[index].Allies == (curalliance^PLAYER_MASK(index))) // is this guy in the alliance?
                {
                    if (index == universe.curPlayerIndex)   // is this the current player
                    {
                        // play the Alliance broken speech event
                        speechEventFleet(COMM_F_AllianceBroken, universe.players[index].Allies, index);
                    }

                    if (bitTest(newalliance, PLAYER_MASK(index)))   // is he still in the alliance?
                    {
                        if (newalliance != 0)   // there are still others in the alliance
                            universe.players[index].Allies = newalliance^PLAYER_MASK(index);
                        else    // no one left in the alliance
                            universe.players[index].Allies = 0;
                    }
                    else    // this person left the alliance
                        universe.players[index].Allies = 0;
                }
            }

            if (universe.players[sigsPlayerIndex].AllianceConfirms!=0)
            {
                universe.players[sigsPlayerIndex].AllianceConfirms &= universe.players[sigsPlayerIndex].Allies;
                if (universe.players[sigsPlayerIndex].AllianceConfirms==0)
                {
                    // same as a confirm from your allies, therefore send the request to allie.
                    sendAllianceRequest(PLAYER_MASK(universe.players[sigsPlayerIndex].AllianceRequestToConfirm),(uword)sigsPlayerIndex,ALLIANCE_PACKET, 0);
                                        universe.players[sigsPlayerIndex].AllianceProposals=PLAYER_MASK(universe.players[sigsPlayerIndex].AllianceRequestToConfirm);
                                        universe.players[sigsPlayerIndex].AllianceRequestToConfirm = -1;
                }
            }
            // print out an appropriate message
            if (bitTest(curalliance, PLAYER_MASK(sigsPlayerIndex)))
            {
                if (bitTest(newalliance, PLAYER_MASK(sigsPlayerIndex)))
                {
                    for (index=0;index<universe.numPlayers;index++)
                    {
                        if ( (bitTest(curalliance, PLAYER_MASK(index))) &&
                             (!bitTest(newalliance, PLAYER_MASK(index))) )
                        {
                            sprintf(temp, "%s %s", playerNames[index], strGetString(strHasBrokenAlliance));
                            gcProcessGameTextMessage(temp, allianceMessageColor);
                            break;
                        }
                    }
                }
                else
                {
                    strcpy(temp," ");
                    sprintf(temp, "%s ", strGetString(strAllianceBroken));
                    alliancePrintNamesNice(temp,curalliance);
                    gcProcessGameTextMessage(temp, allianceMessageColor);
                }
            }
        }
        break;*/
    }
}

void allianceRemovePlayerShipsFromAttackList(SelectAnyCommand *list,uword playerindex,CommandToDo *todo)
{
    sdword index;

    for (index=0;index<list->numTargets;index++)
    {
        if ((list->TargetPtr[index]->objtype == OBJ_ShipType) &&
            ( ((Ship *)list->TargetPtr[index])->playerowner->playerIndex == playerindex ) )
        {
            RemoveAttackTargetFromExtraAttackInfo(list->TargetPtr[index],todo);
            list->numTargets--;
            list->TargetPtr[index] = list->TargetPtr[list->numTargets];
        }
    }
}

void allianceCancelAttackOrders(uword playerone, uword playertwo)
{
    Node        *search;
    CommandToDo *todo;

    search = universe.mainCommandLayer.todolist.head;

    while (search != NULL)
    {
        todo = (CommandToDo *)listGetStructOfNode(search);

        if ( (todo->ordertype.order == COMMAND_ATTACK) ||
             (todo->ordertype.attributes & COMMAND_IS_PASSIVEATTACKING) )
        {
            dbgAssert(todo->selection->ShipPtr[0] != NULL);
            //if attack command contains player one ships
            if (todo->selection->ShipPtr[0]->playerowner->playerIndex == playerone)
            {
               allianceRemovePlayerShipsFromAttackList(todo->attack,playertwo,todo);
            }
            else if (todo->selection->ShipPtr[0]->playerowner->playerIndex == playertwo)
            {
               allianceRemovePlayerShipsFromAttackList(todo->attack,playerone,todo);
            }

        }

        search = search->next;
    }
}

/*-----------------------------------------------------------------------
   Name        : allianceBreakAll
   Description : This function will break all aliances that this player has.
   Inputs      : none
   Outputs     : none
   Parameters  : none
   Return      : void
-----------------------------------------------------------------------*/
void allianceBreakAll(void)
{
    udword i;

    for (i=0;i<universe.numPlayers;i++)
    {
        if ( bitTest(universe.players[sigsPlayerIndex].Allies, PLAYER_MASK(i)) )
        {
            clWrapSetAlliance(ALLIANCE_BREAKALLIANCE, (uword)sigsPlayerIndex, (uword)i);
        }
    }
}

/*-----------------------------------------------------------------------------
    Name        : allianceIsShipAlly
    Description : Returns true if you are allied with a ship.
    Inputs      : ship, playerptr
    Outputs     : true, false
    Return      : bool
----------------------------------------------------------------------------*/
bool allianceIsShipAlly(Ship *ship, Player *player)
{
    if ((ship->playerowner == player) || (bitTest(ship->attributes, ATTRIBUTES_Defector)))
    {
        return TRUE;     // always considered ally with yourself
    }
    return(bitTest(player->Allies, PLAYER_MASK(ship->playerowner->playerIndex)));
}


/*-----------------------------------------------------------------------------
    Name        : allianceArePlayersAllied
    Description : Returns true if these two players are alied.
    Inputs      : ship, playerptr
    Outputs     : true, false
    Return      : bool
----------------------------------------------------------------------------*/
bool allianceArePlayersAllied(Player *playerone, Player *playertwo)
{
    if (playerone == playertwo)
    {
        return TRUE;    // always considered ally with yourself
    }
    return(bitTest(playerone->Allies, PLAYER_MASK(playertwo->playerIndex)));
}

