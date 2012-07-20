#ifndef _TUTORIAL_H
#define _TUTORIAL_H

#include "FEFlow.h"

typedef enum
{
    TUT_BASIC_INTRO,
    TUT_LESSON_ROTATING,
    TUT_LESSON_ZOOM,
    TUT_LESSON_FOCUS,
    TUT_LESSON_CANCEL_FOCUS,
    TUT_LESSON_SELECT_ONE,
    TUT_LESSON_SELECT_FIVE,
    TUT_LESSON_CANCEL_SELECT,
    TUT_LESSON_GROUP,
    TUT_LESSON_2D_MOVE,
    TUT_LESSON_3D_MOVE,
    TUT_LESSON_ATTACK,
    TUT_LESSON_FORMATION,
    TUT_LESSON_HARVEST,
    TUT_LESSON_SENSORS_MANAGER,
    TUT_LESSON_SENSORS_MANAGER_MOVE,
	TUT_LESSON_ENTER_BUILD_MANAGER,
	TUT_LESSON_BUILD_SHIP,
    TUT_BASIC_FINALE,
    TUT_ADVANCED_INTRO,
    TUT_LESSON_DOCK,
    TUT_LESSON_GUARD,
    TUT_LESSON_FLEET_VIEW,
    TUT_LESSON_MOTHERSHIP,
    TUT_LESSON_SPECIAL_OPS,
    TUT_LESSON_TACTICAL_OVERLAY,
    TUT_ADVANCED_FINALE,
    TUT_TOTAL_LESSONS
} TUTORIAL_LESSONS;

typedef struct
{
	real32 value;		// lesson storage variable for float
	real32 value2;		// lesson storage variable for 2nd float
	sdword count;		// used to count the number of times a task is done within a lesson
	sdword lessonID;	// set to current lesson number when a task for that lesson is done
	Ship *shipPtr;		// used to store ship pointers for some lessons
} TutLessonVar;

//extern TutLessonVar tutVar;
//extern sdword tutLesson;
//extern regionhandle tutRegion;

//void tutBasicTutorial(char *name, featom *atom);
//void tutAdvancedTutorial(char *name, featom *atom);
//void tutStartup(void);
//void tutExecute(regionhandle reg);
//void tutShutdown(void);

#endif // #ifndef _TUTORIAL_H