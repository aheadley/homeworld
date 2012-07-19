/*********************************************************************
Contains leak strings for the game and who they went to...

*********************************************************************/

#define MAX_LEAK_STRING_LENGTH      200

//this contains the leakString HEADER only...the plumber utility
//will then patch onto the end of this a unique leakstring
static char leakString[MAX_LEAK_STRING_LENGTH] = "Bryce %s Pasechnik and Janik %s Joire Rule";
