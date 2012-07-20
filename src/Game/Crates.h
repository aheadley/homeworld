/*=============================================================================
    Name    : crates.h
    Purpose : contains defines/[prototypes for all crate gameplay
              related code

    Created bryce in november
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef CRATES_H
#define CRATES_H
 
#include "SpaceObj.h"
      
//prototypes      
void cratesUpdate();    
void cratesReportCratePlacement(Derelict *crate);
void crateInit();

       
#endif //CRATES_H 
