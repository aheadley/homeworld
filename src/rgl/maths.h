#ifndef _iMATHS_H
#define _iMATHS_H

#include <math.h>
#include "kgl.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern GLfloat xaxis[3];
extern GLfloat yaxis[3];
extern GLfloat zaxis[3];


void v3_output(GLfloat*);

void mat4_output(GLfloat*);

void init_sqrt_tab();
double fsqrt(double);

/* v3, 3-space vectors */

GLfloat v3_mag2(GLfloat*);
GLfloat v3_magnitude(GLfloat*);
void v3_set(GLfloat*, GLfloat, GLfloat, GLfloat);
void v3_copy(GLfloat* d, GLfloat* s);
GLfloat v3_dot(GLfloat*, GLfloat*);
void v3_normalize(GLfloat*);
/** transform a point with a projective matrix */
void v3_project(GLfloat* pout, GLfloat* m, GLfloat* pin);
void v3_scale(GLfloat*, GLfloat*, GLfloat);
void v3_add(GLfloat* c, GLfloat* a, GLfloat* b);
void v3_subtract(GLfloat* c, GLfloat* a, GLfloat* b);
void v3_negate(GLfloat*);
void v3_cross(GLfloat* c, GLfloat* a, GLfloat* b);
void v3_vecproject(GLfloat* c, GLfloat* a, GLfloat* b);
GLboolean v3_equal(GLfloat* a, GLfloat* b, GLfloat epsilon);


void v4_project(GLfloat u[4], GLfloat const v[4], GLfloat const m[16]);
void v4_projectd(GLdouble u[4], GLdouble const v[4], GLdouble const m[16]);


/* mat4, 4x4 matrices */

void mat4_get_translation(GLfloat* t, GLfloat* m);
void mat4_set_translationv(GLfloat* m, GLfloat* t);
void mat4_set_translation(GLfloat* m, GLfloat x, GLfloat y, GLfloat z);
void mat4_rotation(GLfloat* m, GLfloat* axis, GLfloat degrees);
void mat4_mult(GLfloat* c, GLfloat const* a, GLfloat const* b);
void mat4_multd(GLdouble* c, GLdouble const* a, GLdouble const* b);
void mat4_identity(GLfloat*);
void mat4_copy(GLfloat* d, GLfloat* s);
void mat4_transpose(GLfloat* d, GLfloat* s);	/** intelligent */
void mat4_inverse(GLfloat* d, GLfloat* s);
void mat4_inversed(GLdouble* d, GLdouble* s);


#endif
