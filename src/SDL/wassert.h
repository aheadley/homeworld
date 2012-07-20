/*==========================================================================
 *
 *  Copyright (C) 1995-1997 Microsoft Corporation. All Rights Reserved.
 *
 *  File:		wassert.h
 *  Content:	assert header
 *
 ***************************************************************************/
#ifndef __WASSERT_INCLUDED__
#define __WASSERT_INCLUDED__

#ifdef _DEBUG
#define ASSERT		// Enable the Assert() macro
#endif

#ifdef ASSERT

void AssertFail(char [], char [], int, char[]);
#define Assert(f) ((f) ? (void)NULL : (void)AssertFail(#f, __FILE__, __LINE__,NULL))
#define AssertMessage(f, szMessage) ((f) ? (void)NULL : (void)AssertFail(#f, __FILE__, __LINE__, szMessage))

#else
#define Assert(f) (void)NULL		// Macro that does nothing
#define AssertMessage(f, szMessage) (void)NULL	

#endif // ~ASSERT

#endif
