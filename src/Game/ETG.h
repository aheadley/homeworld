/*=============================================================================
    Name    : ETG.H
    Purpose : Effects Tool.  OF THE GODS!
        Parsing and execution definitions.

    Created 11/11/1997 by lmoloney
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef ___ETG_H
#define ___ETG_H

#include "SpaceObj.h"
#include "Vector.h"
#include "Matrix.h"

/*=============================================================================
    Switches:
=============================================================================*/
#define ETG_SELECT_EXPLOSION        1           //logic for selecting explosion types
#define ETG_DEFAULT_EXPLOSIONS      1           //still support older 'explosionType' in .shp files

#ifndef HW_Release

#define ETG_ERROR_CHECKING          1           //general error checking
#define ETG_VERBOSE_LEVEL           1           //control specific output code
#define ETG_TESTING                 1           //enable effects testing
#define ETG_RELOAD_KEY              RKEY        //reload all effects key
#define ETG_DISABLEABLE             1           //disable effects
#define ETG_DETATCH_STATS           1           //display special owner detachment stats

#else //HW_Debug

#define ETG_ERROR_CHECKING          0           //general error checking
#define ETG_VERBOSE_LEVEL           0           //control specific output code
#define ETG_TESTING                 0           //enable effects testing
#define ETG_RELOAD_KEY              0
#define ETG_DISABLEABLE             0           //disable effects
#define ETG_DETATCH_STATS           0           //display special owner detachment stats

#endif //HW_Debug

/*=============================================================================
    Definitions:
=============================================================================*/
//Tokens representing type of structure in an effect program.  These are
//basically 'opcodes' where the rest of the structure is the operands.
//suffix definitions: V = Variable, C = constant (default),
//  F - float, I - Int, Co - RGB color, Ca - RGBA color, Vec - vector
typedef enum
{
       //0
   EOP_Nop,                                     //do nothing
//time-index code block comparison operations
   EOP_Before,
   EOP_BeforeVar,
   EOP_After,
   EOP_AfterVar,
   EOP_At,
   EOP_AtVar,                                   //same as at but uses a variable
   EOP_Between,
   EOP_Every,
   EOP_EveryVar,                                //same as every but uses a variable
       //10
   EOP_EffectorCCI,                             //Constant-constant, integer
   EOP_EffectorCCF,                             //                 , float
   EOP_EffectorCCCo,                            //                 , RGB
   EOP_EffectorCCCa,                            //                 , RGBA
   EOP_EffectorCVI,                             //Constant-variable, integer
   EOP_EffectorCVF,                             //                 , float
   EOP_EffectorCVCo,                            //                 , RGB
   EOP_EffectorCVCa,                            //                 , RGBA
   EOP_EffectorVCI,                             //variable-constant, integer
   EOP_EffectorVCF,                             //                 , float
       //20
   EOP_EffectorVCCo,                            //                 , RGB
   EOP_EffectorVCCa,                            //                 , RGBA
   EOP_EffectorVVI,                             //variable-variable, integer
   EOP_EffectorVVF,                             //                 , float
   EOP_EffectorVVCo,                            //                 , RGB
   EOP_EffectorVVCa,                            //                 , RGBA
   EOP_EffectorVI,                              //current-variable , integer
   EOP_EffectorVF,                              //                 , float
   EOP_EffectorVCo,                             //                 , RGB
   EOP_EffectorVCa,                             //                 , RGBA
       //30
   EOP_EffectorCI,                              //current-constant , integer
   EOP_EffectorCF,                              //                 , float
   EOP_EffectorCCo,                             //                 , RGB
   EOP_EffectorCCa,                             //                 , RGBA
//process-control
   EOP_Delete,
   EOP_End,
   EOP_Yeild,
//branching
   EOP_Goto,
   EOP_Call,
   EOP_Return,
       //40
   EOP_Spawn,
   EOP_Start,
//conditionals
   EOP_EqualVCI,                                //variable, constant, integer
   EOP_NotEqualVCI,
   EOP_GreaterVCI,
   EOP_GreaterEqualVCI,
   EOP_LessVCI,
   EOP_LessEqualVCI,
   EOP_EqualVVI,                                //variable, variable, integer
   EOP_NotEqualVVI,
       //50
   EOP_GreaterVVI,
   EOP_GreaterEqualVVI,
   EOP_LessVVI,
   EOP_LessEqualVVI,
   EOP_EqualVCF,                                //variable, constant, float
   EOP_NotEqualVCF,
   EOP_GreaterVCF,
   EOP_GreaterEqualVCF,
   EOP_LessVCF,
   EOP_LessEqualVCF,
       //60
   EOP_EqualVVF,                                //variable, variable, float
   EOP_NotEqualVVF,
   EOP_GreaterVVF,
   EOP_GreaterEqualVVF,
   EOP_LessVVF,
   EOP_LessEqualVVF,
   EOP_ZeroVI,                                  //variable, integer
   EOP_NotZeroVI,
   EOP_ZeroVF,                                  //variable, float
   EOP_NotZeroVF,
//general
       //70
   EOP_VariableCopy,                            //copy a variable
   EOP_VariableAssign,                          //assign a variable a constant value
   EOP_Function,                                //execute a function
   EOP_Alternate,                               //perform an alternate decision (random)
   EOP_Case,                                    //perform a case decision (from integer)
   EOP_LastOp,
   EOP_Resolve = 0x4000                         //resolve forward references on pass2
}
etgopcode;

#define EOP_LowestConditional       EOP_EqualVCI
#define EOP_HighestConsitional      EOP_NotZeroVF

//flags for a parse table entry
#define PTF_Parameters              1           //opcode has parameters in parenthesis
#define PTF_RestOfLine              2           //has parameters, but not in parenteses
#define PTF_Return                  4           //opcode expects a return value
#define PTF_ReturnOptional          8           //opcode can have a return value

//variable types.  Bit fields to permit specification of a variety of types
#define EVT_Void                    0
#define EVT_Int                     1
#define EVT_Float                   2
#define EVT_RGB                     3
#define EVT_RGBA                    4
#define EVT_Pointer                 5
#define EVT_Constant                6           //constant number
#define EVT_Label                   7           //not really a variable, but saves on code
#define EVT_ConstLabel              8           //offset in the constData block
#define EVT_VarLabel                9           //offset in the variables block
#define EVT_This                    10          //include a 'this' pointer

//amounts to allocate during parsing of an effect
#define ETG_StartupParseSize        16384
#define ETG_EachFrameParseSize      16384
#define ETG_TimeIndexParseSize      8192
#define ETG_VarTableParseLength     256
#define ETG_NewParticleLength       8
#define ETG_ConstDataPool           32768

//parser modes
#define EPM_Startup                 0           //defining startup code block
#define EPM_EachFrame               1           //defining eachframe code block
#define EPM_TimeIndex               2           //defining time index code block
#define ETG_NumberCodeBlocks        3           //total number of code blocks
#define EPM_Variables               3           //defining variables
#define EPM_Nothing                 64          //not in any type of block
#define EPM_Constant                EPM_Nothing //define constants, such as texture animations

//alternate stuffs
#define EAT_Int                     0           //alternate type (int/float)
#define EAT_Float                   1
#define ETG_MaxAlternates           32          //maximum number of alternates in a single effect

//general
#define ETG_MaxParams               9           //max number of parameters in an function call
#define ETG_VariableParams          128         //variable number of parameters (like the ... operator)
#define ETG_NestStackDepth          10          //max depth of nesting stack
#define ETG_EventListLength         256         //max number of effect events
#define ETG_DecisionMax             128         //max size of an alternate opcode's offset table
#define ETG_NumberMeshes            128         //number of ETG-specific meshes

//etg script file
#ifdef _WIN32
#define ETG_Directory               "ETG\\"
#else
#define ETG_Directory               "ETG/"
#endif
#define ETG_Script                  "etg.script"
#define ETG_NumberTestKeys          8

//event types
#define EDT_AccumDamage             0
#define EDT_ProjectileHit           1
#define EDT_BeamHit                 2
#define EDT_Collision               3
#define EDT_Special                 4
#define EDT_NumberExplosionTypes    5
#define EGT_GunFire                 0           //muzzle flash effect
#define EGT_GunHit                  1           //bullet hit effect
#define EGT_GunBullet               2           //effect for the actual bullet
#define EGT_BulletDestroyed         3           //bullet destroyed by defense fighter
//#define EGT_BulletExpired           5           //bullet expired because it ran out of time
#define EGT_NumberGunTypes          5

#define ETG_AsteroidEffect          0           //effect by resource type enumeration
#define ETG_GaseousEffect           1
#define ETG_NumberResourceTypes     2

#define DMG_NumberDamageTypes       3
#define DMG_Light                   0
#define DMG_Heavy                   1
#define DMG_Dying                   2
#define DMG_All                     3

#ifdef _WIN32
#define ETG_DefaultBoom             "ETG\\genericBOOM.ebg"
#define ETG_DefaultBlast            "ETG\\defaultBlast.ebg"
#else
#define ETG_DefaultBoom             "ETG/genericBOOM.ebg"
#define ETG_DefaultBlast            "ETG/defaultBlast.ebg"
#endif
#define ETG_UpdateRoundOff          (UNIVERSE_UPDATE_PERIOD / 2.0f)

//default bullet colors
#define ETG_DefaultBulletColor      colRGB(100,184,255)
#define ETG_R1BulletColor           colRGB(255,180,0)

//parameter indices and whatnot
#define ETG_LengthVariable          2           //length parameter of bullet effects
#define ETG_SpecialDurationParam    1           //duration of special effects
#define ETG_BulletDurationParam     3           //duration of bullet effects
#define ETG_ResourceLengthParam     0           //length parameter of resourcing effects
#define ETG_ResourceRadiusParam     1           //width parameter of resourcing effects
#define ETG_ResourceDurationParam   2           //duration parameter of resourcing effects
#define ETG_NumberParameters        8           //number of parameters supported by variable-parameter functions

//effect flags
#define EF_Rendered                 0x00000010
#define EF_ToBeDeleted              0x00000001
#define EM_ToBeDeleted              0x0000000f
#define EM_ForceDelete              0x00000003
#define EAF_Position                SOF_AttachPosition
#define EAF_Velocity                SOF_AttachVelocity
#define EAF_Coordsys                SOF_AttachCoordsys
#define EAF_NLips                   SOF_AttachNLips
#define EAF_PosVel                  (SOF_AttachPosition | SOF_AttachVelocity)
#define EAF_PosCoord                (SOF_AttachPosition | SOF_AttachCoordsys)
#define EAF_VelCoord                (SOF_AttachVelocity | SOF_AttachCoordsys)
#define EAF_Full                    (SOF_AttachPosition | SOF_AttachCoordsys | SOF_AttachVelocity | SOF_AttachNLips)
#define EAF_AllButNLips             (SOF_AttachPosition | SOF_AttachCoordsys | SOF_AttachVelocity)
#define EAF_PlayingSpeech           0x20000000
#define EAF_OwnerCoordSys           0x01000000

//effect history stuff
#define EH_DefaultSize              -1
#define EH_TimeElapsed              1.0f

//effect special operations
#define ESO_SelfDeleting            0x00000001  //effect has a delete() opcode
#define ESO_SortForward             0x00000002  //effect should be sorted forward, in front of ships
#define ESO_WorldRender             0x00000004  //effect will be rendered like a planet, i.e. really far away
//#define ESO_ForceVisible            0x00000008  //force effect to stay in render list

//optimization tweakable defaults
#define ETG_HistoryScalarMin        64
#define ETG_HistoryScalar           256
#define ETG_SoftwareScalarDamage    0.707f      //square root(0.5)
#define ETG_SoftwareScalarHit       0.707f      //square root(0.5)
#define ETG_SoftwareScalarFire      0.866f      //square root(0.75)

#define ETG_TokenDelimiters         ", \t"

/*=============================================================================
    Type definitions:
=============================================================================*/
struct Effect;
struct etgeffectstatic;
struct Ship;

// ****** Structures for various p-code operations.  The first member is the actual opcode
//EOP_Nop
typedef struct
{
    etgopcode opcode;
}
etgnop;
//EOP_Yield, EOP_Delete, EOP_End
typedef struct
{
    etgopcode opcode;
}
etgprocesscontrol;
//EOP_Before/EOP_After/EOP_At
typedef struct
{
    etgopcode opcode;
    sdword codeBytes;                           //bytes to skip if wrong time
    real32 time;                                //time delimiter
}
etgbeforeafterat;
//EOP_Before/EOP_After/EOP_At
typedef struct
{
    etgopcode opcode;
    sdword codeBytes;                           //bytes to skip if wrong time
    udword variable;                            //time delimiter variable
}
etgbeforeafteratvar;
//EOP_Between
typedef struct
{
    etgopcode opcode;
    real32 start;                               //bounds of when to execute
    real32 end;
    udword startVar;                            //variable bounds
    udword endVar;
    sdword codeBytes;                           //bytes to skip if wrong time
}
etgbetween;
//EOP_Effector[CC,CV,VC,VV][I,F,Co,Ca]
typedef struct
{
    etgopcode opcode;
    udword start, end;                          //end-points of range, cast to different types
    udword effector;                            //offset in variable block
    udword rate;                                //offset in rate block
    ubyte effectorID;                           //current effectorID
}
etgeffector;
//EOP_Effector[C,V][I,F,Co,Ca] (current to variable or constant)
typedef struct
{
    etgopcode opcode;
    udword end;                                 //end of range, cast to different types
    udword effector;                            //offset in variable block
    udword rate;                                //offset in rate block
    ubyte effectorID;                           //current effectorID
}
etgseffector;
//EOP_Goto, EOP_Call
typedef struct
{
    etgopcode opcode;
    udword codeBlock;
    udword branchTo;                            //location to branch to
}
etgbranch;
//EOP_Return
typedef struct
{
    etgopcode opcode;                           //no other info needed
}
etgreturn;
//EOP_xxxx same structure for all conditionals
typedef struct
{
    etgopcode opcode;
    udword param0, param1;                      //can be constants or variable offsets
    sdword codeBytes;                           //bytes of conditional code
//    sdword elseBytes;                           //bytes of conditional else code
}
etgconditional;
//EOP_VariableCopy
typedef struct
{
    etgopcode opcode;
    udword source, dest;                         //source and destination of var copy
}
etgvariablecopy;
//EOP_Function
typedef struct
{
    etgopcode opcode;
    udword (*function)(void);                   //function to call
    udword nParameters;                         //number of parameters to pass
    udword returnValue;                         //variable to assign return value to, if any
    bool passThis;                              //shall we pass a reference to this effect?
    struct
    {
        udword type;                            //type of parameter, see above
        udword param;                           //constant, variable or pointer
    }
    parameter[1];                               //ragged array of parameters to pass
}
etgfunctioncall;
//EOP_Alternate, EOP_Variable
typedef udword decisionoffset;
typedef struct
{
    udword opcode;
    udword criteria;                            //variable or alternate to use for criteria
    udword tableLength;                         //number of offsets in following arrays
    udword codeBytes;                           //length of the total alternate table (for branching)
    decisionoffset *offset;                     //actual decision members relative to this opcode
}
etgdecision;

//opcode-handler function dispatch table entry
typedef sdword (*ophandlefunction)(struct Effect *effect, struct etgeffectstatic *stat, ubyte *opcode);
typedef struct
{
    sdword length;                              //length of opcode structure
    ophandlefunction function;                  //handler function
}
ophandleentry;

//opcode-parsing table entry
typedef sdword (*opparsefunction)(struct etgeffectstatic *stat, ubyte *dest, char *opcode, char *params, char *ret);
typedef struct
{
    char *name;                                 //corresponding op-code name, if any
    udword flags;                               //parsing flags
    opparsefunction function;                   //function to handle parsing/creation of the opcode
}
opparseentry;

//function-call-resolve function pointer, for hand-tweaking the results of a function call.
typedef void (*etgresolvefunction)(struct etgeffectstatic *stat, etgfunctioncall *call);
//function-parsing table entry
typedef struct
{
    char *name;                                 //name of function
    void *function;                             //function pointer
    ubyte returnType;                           //return value type, if any
    ubyte passThis;
    etgresolvefunction resolve;
    ubyte nParams;                              //number of parameters to this function
    ubyte type[ETG_MaxParams];                  //parameter types
}
opfunctionentry;

//entry in a variable-name list for this effect
typedef struct
{
    char *name;                                 //name of variable
    udword offset;                              //offset in variable block of this effect
    udword rateOffset;                          //offset in the variable rate block or 0xffffffff
    udword initial;                             //initial value of type, if any
    ubyte bInitial;                             //flag set nonzero if there is an initial value
    ubyte type;                                 //type of variable
    ubyte size;                                 //optional size of pointer-variable, not needed for dword types
    ubyte codeBlock;                            //etgParseMode when created
}
etgvarentry;

//structure for an entry on the brace nesting stack
typedef void (*nestfunction)(sdword codeBlock, sdword offset, sdword newOffset, ubyte *userData);
typedef void (*nestopenfunction)(sdword codeBlock, sdword offset, ubyte *userData);
typedef struct
{
    nestopenfunction openFunction;              //function to call on brace open, if any
    nestfunction function;                      //function to call on brace close
    sdword codeBlock;                           //which block it started in (EPM_Startup etc.)
    sdword offset;                              //offset the conditional instruction
    ubyte *userData;                            //whatever you like
}
etgnestentry;

//structure for parsing conditional statements
typedef struct
{
    char symbol[4];                             //comparisson symbol
    char transpose[4];                          //symbol to use if parameters are reversed
    etgopcode opcode[4];                        //variable-constant-integer
                                                //variable-variable-integer
                                                //variable-constant-Float
                                                //variable-variable-Float
}
etgcondentry;

//structure for a code block within an effect's static info block
typedef struct
{
    ubyte *code;                                //actual code block
    udword length;                              //length of code block
    udword offset;                              //current offset in code block (if applicable)
}
etgcodeblock;

//structure for making an alternate decision
typedef struct
{
    udword type;
    udword low;
    udword high;
}
etgalternate;

typedef struct
{
    etgalternate alt;
    etgdecision *decision;
}
etgextalt;

//structure for an effect object's static information
typedef struct etgeffectstatic
{
    char *name;                                 //name of effect
    struct etgeffectstatic *softwareVersion;    //version of effect to play in software, if any
    udword nParticleBlocks;                     //number of particle block pointers to allocate
    udword variableSize;                        //size of variables block
    udword rateSize;                            //size of variables rates block.
    sdword effectSize;                          //total size of the effect (sans particle systems)
    ubyte *variableOffsets;                     //variable initialization data
    udword *variableInitData;                   //variable init data
    sdword initLength;                          //length of variable init data
    ubyte *constData;                           //constant data, such as texture animations
    udword constLength;                         //length of constant data
    sdword nDecisions;                          //data for making alternate decisions
    etgalternate *decisions;
    udword *alternateOffsets;                   //offset tables for alternates
    etgcodeblock codeBlock[ETG_NumberCodeBlocks];//startup, eachframe and time index
    sdword nHistoryList;                        //length of history list
    sdword iHistoryList;                        //index into history list
    real32 *historyList;                        //actual history list
//    bool8  bSelfDeleting;                       //has a delete() opcode
//    char   pad[3];                              //round the size up
    udword specialOps;                          //special operations this effect does
}
etgeffectstatic;

//strucutre for LOD of effects
typedef struct
{
    sdword nLevels;                             //number of levels
    etgeffectstatic *level[1];                  //array of levels
}
etglod;

//structure for an entry in the effect event registry
typedef struct
{
//    char *name;
    struct etgeffectstatic *effectStatic;
    sdword loadCount;
}
etgevent;

//entry in the p-code call stack
typedef struct
{
    udword codeBlock;
    udword codeOffset;
}
etgcallstackentry;

//effector-rate structures for RGB and RGBA types
typedef struct
{
    udword red, green, blue;
}
etgratergb;
typedef struct
{
    udword red, green, blue, alpha;
}
etgratergba;

/*=============================================================================
    Data:
=============================================================================*/
//dispatch tables for standard events
extern etgeffectstatic *etgDefaultBoom;
extern etgeffectstatic *etgDefaultBlast;
extern etglod *etgDeathEventTable[NUM_RACES][NUM_CLASSES][EDT_NumberExplosionTypes];
extern etglod *etgDeathEventTableDerelict[NUM_DERELICTTYPES][EDT_NumberExplosionTypes];
extern etglod *etgGunEventTable[NUM_RACES][NUM_GUN_SOUND_TYPES][EGT_NumberGunTypes];
extern etglod *etgResourceEffectTable[NUM_RACES][ETG_NumberResourceTypes];
extern etglod *etgTractorBeamEffectTable[NUM_RACES];
extern etglod *etgSpecialPurposeEffectTable[EGT_NumberOfSpecialEffects];
extern etglod *etgDamageEffectTable[NUM_CLASSES][DMG_NumberDamageTypes];
extern etglod *etgHyperspaceEffect;
extern color etgBulletColor[NUM_RACES][NUM_GUN_SOUND_TYPES];
extern sdword etgBigDeathFactor[NUM_RACES][NUM_CLASSES];
extern sdword etgBigDeathFactorDerelict[NUM_DERELICTTYPES];
extern ubyte etgDeathModeByGunType[];
extern sdword etgEffectsEnabled;
extern bool etgErrorRecoverable;
extern bool etgErrorEncountered;


//variables for ETG user tweaks:
extern real32 etgSoftwareScalarDamage;          //scale certain effects down when in software mode.
extern real32 etgSoftwareScalarHit;             //scale certain effects down when in software mode.
extern real32 etgSoftwareScalarFire;            //scale certain effects down when in software mode.
extern sdword etgHistoryScalar;                 //from etgHistoryScalarMin to 256 inclusive
extern sdword etgHistoryScalarMin;
extern bool   etgDamageEffectsEnabled;
extern bool   etgHitEffectsEnabled;
extern bool   etgFireEffectsEnabled;
extern bool   etgBulletEffectsEnabled;

/*=============================================================================
    Macros:
=============================================================================*/
#define etgFunctionSize(n)  (sizeof(etgfunctioncall) + sizeof(sdword) * 2 * ((n) - 1))
#define etgEffectSize(n)  (sizeof(Effect) + sizeof(ubyte *) * ((n) - 1))
#define etgDecisionSize(n) (sizeof(etgdecision) + sizeof(decisionoffset) * ((n) - 1))
#define etgLODSize(n)      (sizeof(etglod) + sizeof(etgeffectstatic *) * ((n) - 1))
//#define etgEffectStaticSize(n) (sizeof(etgeffectstatic) + sizeof(real32) * ((n) - 1))
#define etgHistoryRegister(stat)  if ((stat)->historyList != NULL) etgHistoryRegisterFunction(stat);

/*=============================================================================
    Functions:
=============================================================================*/

//startup/shutdown effects module
void etgStartup(void);
void etgShutdown(void);
void etgReset(void);
void etgReloadReset(void);
void etgFixupUV(void);

//load in an effect template from an effect file and then delete.
etgeffectstatic *etgEffectCodeLoad(char *fileName);
etgeffectstatic *etgEffectStaticFind(char *name, bool bRegister);
void etgEffectCodeDelete(etgeffectstatic *stat, bool bFullDelete);

//create effects
void etgEffectCodeStart(struct etgeffectstatic *stat, struct Effect *effect, sdword nParams, ...);
void etgEffectDelete(struct Effect *effect);
void *etgEffectCreate(etgeffectstatic *stat, void *owner, vector *pos, vector *vel, matrix *coordsys, real32 nLips, udword flags, sdword nParams, ...);
bool etgFrequencyExceeded(etgeffectstatic *stat);
void etgHistoryRegisterFunction(etgeffectstatic *stat);

//update effects
bool etgEffectUpdate(struct Effect *effect, real32 timeElapsed);
void etgEffectDraw(struct Effect *effect);
void etgShipDied(struct Ship *deadDuck);
sdword etgDeleteEffectsOwnedBy(struct Ship *owner);

//mesh registry stuff
meshdata *etgMeshRegister(char *filename);
void etgMeshRegistryReset(void);

//etg random stream
extern udword etgFRandom(real32 low, real32 high);

// Save Game stuff
sdword saveEtglodGunEventToIndex(etglod *lod);
etglod *saveIndexToEtglodGunEvent(sdword index);

#endif //___ETG_H

