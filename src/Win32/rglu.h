/*=============================================================================
    Name    : rglu.h
    Purpose : glu* functions for Homeworld

    Created 7/6/1998 by khent
    Copyright Relic Entertainment, Inc.  All rights reserved.
=============================================================================*/

#ifndef _RGLU_H
#define _RGLU_H

void rgluPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
GLint rgluProject(GLdouble objx, GLdouble objy, GLdouble objz,
                  GLdouble const* model, GLdouble const* proj,
                  GLint const* viewport,
                  GLdouble* winx, GLdouble* winy, GLdouble* winz);
GLint rgluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
                    GLdouble const* model, GLdouble const* proj,
                    GLint const* viewport,
                    GLdouble* objx, GLdouble* objy, GLdouble* objz);
void rgluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez,
                GLdouble centerx, GLdouble centery, GLdouble centerz,
                GLdouble upx, GLdouble upy, GLdouble upz);
void rgluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top);
char* rgluErrorString(GLenum err);

#endif
