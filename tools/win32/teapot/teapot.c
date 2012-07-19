/*
** Vertex Culling Test
*/
#include <windows.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(GL_SGI_cull_vertex)
PFNGLCULLPARAMETERFVSGIPROC CullParameterfv;
#endif

#if defined(GL_SGI_compiled_vertex_array)
PFNGLLOCKARRAYSSGIPROC LockArrays;
PFNGLUNLOCKARRAYSSGIPROC UnlockArrays;
#endif

#if !defined(M_PI)
#define M_PI 3.14159265F
#endif

char *className = "OpenGL";
char *windowName = "Mouse my Teapot!";
int winX, winY;
int winWidth, winHeight;

HDC hDC;
HGLRC hGLRC;
HPALETTE hPalette;

void (*idleFunc)(void);

GLfloat objectXform[4][4];
float angle = 10.0F, axis[3] = { 0.0F, 0.0F, 1.0F };
/*
void drawCube(void);
void drawTorus(void);
void drawSphere(void);
#define NUM_OBJECTS (sizeof(drawObject) / sizeof(drawObject[0]))
void (*drawObject[])(void) = {
    drawTorus, drawSphere, drawCube,
};
int objectIndex;
*/
int objectNumMajor = 24, objectNumMinor = 32;
BOOL halfObject = FALSE;
BOOL redrawContinue = TRUE;
BOOL doubleBuffered = TRUE;
BOOL depthBuffered = TRUE;
BOOL drawOutlines = FALSE;
BOOL textureEnabled = FALSE;
BOOL textureReplace = FALSE;
BOOL useVertexCull = TRUE;
BOOL useFaceCull = TRUE;
BOOL useVertexArray = TRUE;
BOOL useVertexLocking = TRUE;
BOOL useLighting = TRUE;
BOOL perspectiveProj = TRUE;
//BOOL useFog = FALSE;
enum MoveModes { MoveNone, MoveObject };
enum MoveModes mode = MoveObject;

#define X_OFFSET_STEP 0.025F;
#define Y_OFFSET_STEP 0.025F;
GLfloat xOffset, yOffset;

//***************************************
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA

#define AXIS_BAR_WIDTH        0.02
#define AXIS_BAR_LENGTH       3.0
#define NUMBER_OTHER_SHIPS    15
#define OTHER_SHIP_MIN_RAD    2.0
#define OTHER_SHIP_MAX_RAD    5.0
#define OTHER_SHIP_SIZE       0.2
#define OTHER_SHIP_RANGE      (OTHER_SHIP_MAX_RAD - OTHER_SHIP_MIN_RAD)
#define OTHER_SHIP_NUM_SIZES  5
#define CONTROL_FILE          "controls.txt"

#define CAMERA_TARGET_AID     0
#define CAMERA_TARGET_SCALE   0.1

GLdouble cameraLook[3] = {0.0, 0.0, 0.0};     //camera target
GLdouble cameraLong = 0.0, cameraLat = 0.0;   //longitude/latitude
GLdouble cameraDistance = 5.0;                //distance from target
GLdouble cameraAngle = 45.0;                  //width of view for camera

GLdouble zoomInMult = .95;
GLdouble zoomOutMult = 1.05;
GLdouble panMultX =1.0, panMultY = 1.0;
GLdouble degreeMultX = 1.0, degreeMultY = 1.0;
GLdouble aspectRatio;

#define MFI_None              0x00
#define MFI_Left              0x01
#define MFI_Right             0x02
#define MFI_LeftRight         0x03
#define MFI_Control           0x04
#define MFI_Alt               0x08

int mouseButtonFlags = 0;

GLfloat motherShipLength = 2.0;
GLfloat motherShipWidth = 1.0;
GLfloat motherShipHeight = 1.0;

GLdouble clipNearDistance = 0.1;
GLdouble FOV = 90.0;

void mousePanX(int x);
void mousePanY(int y);
void mouseRotateLatitude(int x);
void mouseRotateLongitude(int y);
void mouseZoom(int xy);
void mouseSelectX(int x);
void mouseSelectY(int y);
void mouseNULL(int unused);

int mouseButtonMask = 0;
struct
{
   char *name;
   void (*function)(int x);
}
mouseAxis[] =
{
   {"panX", mousePanX},
   {"panY", mousePanY},
   {"rotateLatitude", mouseRotateLatitude},
   {"rotateLongitude", mouseRotateLongitude},
   {"zoom", mouseZoom},
   {"selectX", mouseSelectX},
   {"selectY", mouseSelectY},
   {NULL, NULL}
};

struct
{
   char *name;
   void (*function)(int x);
}
mouseFunction[] =
{
   {"noneX",      mouseNULL,           },    //no button
   {"noneY",      mouseNULL,           },
   {"leftX",      mouseNULL,           },    //left button
   {"leftY",      mouseNULL,           },
   {"rightX",     mouseNULL,           },    //right button
   {"rightY",     mouseNULL,           },
   {"leftRightX", mouseNULL,           },    //left/right button
   {"leftRightY", mouseNULL,           },
   {"ctrlX",      mouseNULL,           },    //ctrl key
   {"ctrlY",      mouseNULL,           },
   {"__A__",      mouseNULL,           },
   {"__A__",      mouseNULL,           },
   {"__A__",      mouseNULL,           },
   {"__A__",      mouseNULL,           },
   {"__A__",      mouseNULL,           },
   {"__A__",      mouseNULL,           },
   {"altX",       mouseNULL,           },    //alt key
   {"altY",       mouseNULL,           },
   {NULL, NULL                         }
};

BOOL bMouseCapture = FALSE;

    static GLfloat	whiteAmbient[] = {0.3, 0.3, 0.3, 1.0};
    static GLfloat	redAmbient[] = {0.3, 0.1, 0.1, 1.0};
    static GLfloat	greenAmbient[] = {0.1, 0.3, 0.1, 1.0};
    static GLfloat	blueAmbient[] = {0.1, 0.1, 0.3, 1.0};
    static GLfloat	whiteDiffuse[] = {1.0, 1.0, 1.0, 1.0};
    static GLfloat	redDiffuse[] = {1.0, 0.0, 0.0, 1.0};
    static GLfloat	greenDiffuse[] = {0.0, 1.0, 0.0, 1.0};
    static GLfloat	blueDiffuse[] = {0.0, 0.0, 1.0, 1.0};
    static GLfloat	whiteSpecular[] = {1.0, 1.0, 1.0, 1.0};
    static GLfloat	redSpecular[] = {1.0, 0.0, 0.0, 1.0};
    static GLfloat	greenSpecular[] = {0.0, 1.0, 0.0, 1.0};
    static GLfloat	blueSpecular[] = {0.0, 0.0, 1.0, 1.0};

BOOL drawSelectionRect = FALSE;
GLint selectionX1, selectionX2, selectionY1, selectionY2;

//*********************************************************************
GLdouble otherShipSize[OTHER_SHIP_NUM_SIZES] = {0.01, 0.02, 0.1, 0.7, 1.5};
struct
{
    GLdouble size;
    GLfloat *ambientFront;
    GLfloat *ambientBack;
    GLfloat *diffuseFront;
    GLfloat *diffuseBack;
}
otherShipAttribs[OTHER_SHIP_NUM_SIZES] =
{
{0.01, whiteAmbient, whiteAmbient, whiteDiffuse, whiteDiffuse},
{0.02, redAmbient, redAmbient, redDiffuse, redDiffuse},
{0.1, greenAmbient, greenAmbient, greenDiffuse, greenDiffuse},
{0.3, blueAmbient, blueAmbient, blueDiffuse, blueDiffuse},
{1.0, whiteAmbient, whiteAmbient, whiteDiffuse, whiteDiffuse}
};

struct
{
   GLdouble radius;
   GLdouble angle;
   GLdouble vector[3];
   int otherShipIndex;
}
otherShip[NUMBER_OTHER_SHIPS];

void
matrixIdentity(GLfloat m[4][4])
{
    m[0][0] = 1.0F; m[0][1] = 0.0F; m[0][2] = 0.0F; m[0][3] = 0.0F;
    m[1][0] = 0.0F; m[1][1] = 1.0F; m[1][2] = 0.0F; m[1][3] = 0.0F;
    m[2][0] = 0.0F; m[2][1] = 0.0F; m[2][2] = 1.0F; m[2][3] = 0.0F;
    m[3][0] = 0.0F; m[3][1] = 0.0F; m[3][2] = 0.0F; m[3][3] = 1.0F;
}

void
setProjection(void)
{
    aspectRatio = (GLfloat) winWidth / (GLfloat) winHeight;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//	glFrustum(-0.5F*aspectRatio, 0.5F*aspectRatio, -0.5F, 0.5F, 0.5, cameraDistance * 2.0);
	gluPerspective(FOV, aspectRatio, clipNearDistance, cameraDistance * 2.0);

#if defined(GL_SGI_cull_vertex)
	if (CullParameterfv) {
	    GLfloat eye[4] = { 0.0F, 0.0F, 0.0F, 1.0F };

	    CullParameterfv(GL_CULL_VERTEX_EYE_POSITION_SGI, eye);
	}
#endif
    glMatrixMode(GL_MODELVIEW);
}

//*****************************************************
//searches for a single mouse control axis
void mouseAxisRead(char *token)
{
   int index, fIndex;
   char *p;

   if ((p = strtok(NULL, " =\n\t")) == NULL)                 //return on NULL token
      return;

   for (index = 0; mouseAxis[index].name != NULL; index++)// search all axis structs
   {
      if (strcmp(mouseAxis[index].name, token) == 0)     //if matching string
      {
         for (fIndex = 0; mouseFunction[fIndex].name != NULL; fIndex++)
         {
            if (strcmp(mouseFunction[fIndex].name, p) == 0)//another matching name
            {
               mouseFunction[fIndex].function = mouseAxis[index].function;
               break;
            }
         }
         return;
      }
   }
}

//configures control inputs for the mouse from the file CONTROL_FILE
void mouseControlsSet(void)
{
#define MAX_FILE_LINE      256
   FILE *inFile;
   char string[MAX_FILE_LINE];
   char *p;

   if ((inFile = fopen(CONTROL_FILE, "rt")) != NULL)
   {
      while (fgets(string, MAX_FILE_LINE, inFile) != NULL)
      {
         if ((p = strtok(string, " =\n\t")) != NULL)
         {
            if (strcmp(p, "zoomInMult") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &zoomInMult);
            }
            if (strcmp(p, "zoomOutMult") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &zoomOutMult);
            }
            if (strcmp(p, "panMultX") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &panMultX);
            }
            if (strcmp(p, "panMultY") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &panMultY);
            }
            if (strcmp(p, "degreeMultX") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &degreeMultX);
            }
            if (strcmp(p, "degreeMultY") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &degreeMultY);
            }
            if (strcmp(p, "motherShipLength") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%f", &motherShipLength);
            }
            if (strcmp(p, "motherShipWidth") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%f", &motherShipWidth);
            }
            if (strcmp(p, "motherShipHeight") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%f", &motherShipHeight);
            }
            if (strcmp(p, "clipNearDistance") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &clipNearDistance);
            }
            if (strcmp(p, "FOV") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &FOV);
            }
            if (strcmp(p, "size0") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &otherShipAttribs[0].size);
            }
            if (strcmp(p, "size1") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &otherShipAttribs[1].size);
            }
            if (strcmp(p, "size2") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &otherShipAttribs[2].size);
            }
            if (strcmp(p, "size3") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &otherShipAttribs[3].size);
            }
            if (strcmp(p, "size4") == 0)
            {
               p = strtok(NULL, " =\n\t");
               sscanf(p, "%lf", &otherShipAttribs[4].size);
            }
            mouseAxisRead(string);
         }
      }
      fclose(inFile);
   }
}

GLvoid init(GLvoid)
{
   int index;
   time_t timer;
    GLfloat	maxObjectSize;
    GLdouble	near_plane, far_plane;
    GLsizei	width, height;

    GLfloat	ambientProperties[] = {0.7, 0.7, 0.7, 1.0};
    GLfloat	diffuseProperties[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat	specularProperties[] = {1.0, 1.0, 1.0, 1.0};

    mouseControlsSet();

    srand(time(&timer));

    for (index = 0; index < NUMBER_OTHER_SHIPS; index++)
    {
       otherShip[index].radius = (GLfloat)rand() / (GLfloat)RAND_MAX *
          OTHER_SHIP_RANGE + OTHER_SHIP_MIN_RAD;
       otherShip[index].angle = (GLfloat)rand() * 360.0 / (GLfloat)RAND_MAX;
       otherShip[index].vector[0] = (GLfloat)rand() / (GLfloat)RAND_MAX;
       if (rand() >= RAND_MAX / 2)
         otherShip[index].vector[0] = -otherShip[index].vector[0];
       otherShip[index].vector[1] = (GLfloat)rand() / (GLfloat)RAND_MAX;
       if (rand() >= RAND_MAX / 2)
         otherShip[index].vector[1] = -otherShip[index].vector[1];
       otherShip[index].vector[2] = (GLfloat)rand() / (GLfloat)RAND_MAX;
       if (rand() >= RAND_MAX / 2)
         otherShip[index].vector[2] = -otherShip[index].vector[2];

       otherShip[index].otherShipIndex = rand() % OTHER_SHIP_NUM_SIZES;

       printf("[%d]: r=%f a=%f v=(%f, %f, %f)\n", index, otherShip[index].radius, otherShip[index].angle, otherShip[index].vector[0], otherShip[index].vector[1], otherShip[index].vector[2]);
    }

    width = 1024.0;
    height = 768.0;

    auxInitPosition( width/4, height/4, width/2, height/2);

    auxInitDisplayMode( AUX_RGB | AUX_DEPTH | AUX_DOUBLE );

    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClearDepth( 1.0 );

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);

    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientProperties);
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseProperties);
    glLightfv( GL_LIGHT0, GL_SPECULAR, specularProperties);
    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);

    glEnable( GL_LIGHT0 );

    glMatrixMode( GL_PROJECTION );
    aspectRatio = (GLfloat) width / height;
    gluPerspective( cameraAngle, aspectRatio, 3.0, 7.0 );
    glMatrixMode( GL_MODELVIEW );

    near_plane = 3.0;
    far_plane = 7.0;
    maxObjectSize = 3.0;
}

void
resize(void)
{
    setProjection();
    glViewport(0, 0, winWidth, winHeight);
}

void matrixMultVector(GLdouble vOut[], GLdouble vIn[], GLdouble matrix[])
{
   vOut[0] = matrix[4 * 0 + 0] * vIn[0] + matrix[4 * 0 + 1] * vIn[1] + matrix[4 * 0 + 2] * vIn[2] + matrix[4 * 0 + 3] * vIn[3];
   vOut[1] = matrix[4 * 1 + 0] * vIn[0] + matrix[4 * 1 + 1] * vIn[1] + matrix[4 * 1 + 2] * vIn[2] + matrix[4 * 1 + 3] * vIn[3];
   vOut[2] = matrix[4 * 2 + 0] * vIn[0] + matrix[4 * 2 + 1] * vIn[1] + matrix[4 * 2 + 2] * vIn[2] + matrix[4 * 2 + 3] * vIn[3];
   vOut[3] = matrix[4 * 3 + 0] * vIn[0] + matrix[4 * 3 + 1] * vIn[1] + matrix[4 * 3 + 2] * vIn[2] + matrix[4 * 3 + 3] * vIn[3];
// vOut[0] = matrix[4 * 0 + 0] * vIn[0] + matrix[4 * 1 + 0] * vIn[1] + matrix[4 * 2 + 0] * vIn[2] + matrix[4 * 3 + 0] * vIn[3];
// vOut[1] = matrix[4 * 0 + 1] * vIn[0] + matrix[4 * 1 + 1] * vIn[1] + matrix[4 * 2 + 1] * vIn[2] + matrix[4 * 3 + 1] * vIn[3];
// vOut[2] = matrix[4 * 0 + 2] * vIn[0] + matrix[4 * 1 + 2] * vIn[1] + matrix[4 * 2 + 2] * vIn[2] + matrix[4 * 3 + 2] * vIn[3];
// vOut[3] = matrix[4 * 0 + 3] * vIn[0] + matrix[4 * 1 + 3] * vIn[1] + matrix[4 * 2 + 3] * vIn[2] + matrix[4 * 3 + 3] * vIn[3];
}

void positionCamera(void)
{
   GLdouble eyeIn[4], eyeOut[4];
   GLdouble matrix[16];

   eyeIn[0] = cameraDistance + cameraLook[0];
   eyeIn[1] = eyeIn[2] = eyeIn[3] = 0.0;

   //transform the camera eye point according to the latitude and longitude
   glPushMatrix();
      glLoadIdentity();
      glRotated(cameraLong, 0.0, 1.0, 0.0);              //logitude rotation
      glRotated(cameraLat, 0.0, 0.0, 1.0);               //latitude rotation
//      glTranslated(cameraLook[0], cameraLook[1], cameraLook[2]);
      //!!! apply other operations on eye point
      glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
   glPopMatrix();

   matrixMultVector(eyeOut, eyeIn, matrix);              //transform eye point

   eyeOut[0] += cameraLook[0];
   eyeOut[1] += cameraLook[1];
   eyeOut[2] += cameraLook[2];

   gluLookAt(eyeOut[0], eyeOut[1], eyeOut[2], cameraLook[0], cameraLook[1], cameraLook[2], 0.0, 0.0, 1.0);
}

GLvoid drawLight(GLvoid)
{
    glPushAttrib(GL_LIGHTING_BIT);
    	glDisable(GL_LIGHTING);
    	glColor3f(1.0, 1.0, 1.0);
    	auxSolidDodecahedron(0.1);
    glPopAttrib();
}

void doRedraw(void)
{
   int index;
    static GLfloat	lightPosition0[] = {1.0, 1.0, 1.0, 1.0};
    static GLfloat	angle = 0.0;

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glPushMatrix();

//    	latitude += 4.0;
//    	longitude += 2.5;

//    	polarView( radius, 0, latitude, longitude );
      positionCamera();

//create a self-illuminated light object
    glPushMatrix();
//        angle += 6.0;
        glRotated(angle, 1.0, 0.0, 1.0);
        glTranslated( 0.0, 5.0, 0.0);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition0);
        drawLight();
    glPopMatrix();

//create a white ball at the camera target
#if CAMERA_TARGET_AID
    glPushMatrix();
    glPushAttrib(GL_LIGHTING_BIT);
    	glDisable(GL_LIGHTING);
    	glColor3f(1.0, 1.0, 1.0);
      glLoadIdentity();
      glTranslated(cameraLook[0], cameraLook[1], cameraLook[2]);
      auxSolidCube(CAMERA_TARGET_SCALE);
    glPopAttrib();
    glPopMatrix();
#endif //CAMERA_TARGET_AID
   //create a blue torus object
//    glPushAttrib(GL_LIGHTING_BIT);

        glMaterialfv(GL_BACK, GL_AMBIENT, greenAmbient);
        glMaterialfv(GL_BACK, GL_DIFFUSE, greenDiffuse);
        glMaterialfv(GL_FRONT, GL_AMBIENT, blueAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, blueDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, blueSpecular);
        glMaterialf(GL_FRONT, GL_SHININESS, 50.0);

    glPushMatrix();
        glScalef(motherShipLength, motherShipWidth, motherShipHeight);
        auxSolidSphere(1.0);
    glPopMatrix();

//    glPopAttrib();

    //create a red bar on X, Y and Z axes
       glMaterialfv(GL_BACK, GL_AMBIENT, greenAmbient);
       glMaterialfv(GL_BACK, GL_DIFFUSE, greenDiffuse);
       glMaterialfv(GL_FRONT, GL_AMBIENT, redAmbient);
       glMaterialfv(GL_FRONT, GL_DIFFUSE, redDiffuse);
       glMaterialfv(GL_FRONT, GL_SPECULAR, redSpecular);
       glMaterialf(GL_FRONT, GL_SHININESS, 50.0);

       glPushMatrix();                                   //X
         glScalef(AXIS_BAR_LENGTH, AXIS_BAR_WIDTH, AXIS_BAR_WIDTH);
         glTranslated(AXIS_BAR_LENGTH / 4.0, 0, 0);
         auxSolidCube(1.0);
       glPopMatrix();

       glPushMatrix();                                   //Y
         glScalef(AXIS_BAR_WIDTH, AXIS_BAR_LENGTH, AXIS_BAR_WIDTH);
         glTranslated(0, AXIS_BAR_LENGTH / 4.0, 0);
         auxSolidCube(1.0);
       glPopMatrix();

       glPushMatrix();                                   //Z
         glScalef(AXIS_BAR_WIDTH, AXIS_BAR_WIDTH, AXIS_BAR_LENGTH);
         glTranslated(0, 0, AXIS_BAR_LENGTH / 4.0);
         auxSolidCube(1.0);
       glPopMatrix();

    //create a bunch of cubes to represent other ships about the 'mother ship'
       for (index = 0; index < NUMBER_OTHER_SHIPS; index++)
       {
          glPushMatrix();
          glRotated(otherShip[index].angle,
               otherShip[index].vector[0],
               otherShip[index].vector[1],
               otherShip[index].vector[2]);
          glTranslated(otherShip[index].radius, 0, 0);
          glMaterialfv(GL_FRONT, GL_AMBIENT, otherShipAttribs[otherShip[index].otherShipIndex].ambientFront);
          glMaterialfv(GL_BACK, GL_AMBIENT, otherShipAttribs[otherShip[index].otherShipIndex].ambientBack);
          glMaterialfv(GL_FRONT, GL_DIFFUSE, otherShipAttribs[otherShip[index].otherShipIndex].diffuseFront);
          glMaterialfv(GL_BACK, GL_DIFFUSE, otherShipAttribs[otherShip[index].otherShipIndex].diffuseBack);
          auxSolidCube(otherShipAttribs[otherShip[index].otherShipIndex].size);
          glPopMatrix();
       }

    glPopMatrix();

    //draw the selection rectangle if applicable
    if (drawSelectionRect)
    {
        glRecti(selectionX1, selectionY1, selectionX2, selectionY2);
    }

    glFlush();
    SwapBuffers(hDC);
}

/*****************************************************************/

void
idleRedraw(void)
{
    if (!redrawContinue) {
        idleFunc = NULL;
    }
    doRedraw();
}

void
redraw(void)
{
    if (!idleFunc) {
	idleFunc = idleRedraw;
    }
}

/*****************************************************************/

/* these functions implement a simple trackball-like motion control */
BOOL trackingMotion = FALSE;
float lastPos[3];
DWORD lastTime;
int startX, startY;

void
ptov(int x, int y, int width, int height, float v[3])
{
    float d, a;

    /* project x,y onto a hemi-sphere centered within width, height */
    v[0] = (2.0F*x - width) / width;
    v[1] = (height - 2.0F*y) / height;
    d = (float) sqrt(v[0]*v[0] + v[1]*v[1]);
    v[2] = (float) cos((M_PI/2.0F) * ((d < 1.0F) ? d : 1.0F));
    a = 1.0F / (float) sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    v[0] *= a;
    v[1] *= a;
    v[2] *= a;
}

//**********************************************************
//called in response to the mouse moving, updates the camera
#define MMT_None        0
#define MMT_Pan         1
#define MMT_Rotate      2
#define MMT_Zoom        3
int mouseMoveTypeX = MMT_None;
int mouseMoveTypeY = MMT_None;
int oldMouseX, oldMouseY;

void mouseFunctionChangeCheck(int x, int y)
{
   static void (*oldFunctionX)(int _x) = NULL;
   static void (*oldFunctionY)(int _y) = NULL;

   if (oldFunctionX != mouseFunction[mouseButtonFlags * 2 + 0].function)
   {
      oldFunctionX = mouseFunction[mouseButtonFlags * 2 + 0].function;
      oldMouseX = x;
      drawSelectionRect = FALSE;
   }
   if (oldFunctionY != mouseFunction[mouseButtonFlags * 2 + 1].function)
   {
      oldFunctionY = mouseFunction[mouseButtonFlags * 2 + 1].function;
      oldMouseY = y;
      drawSelectionRect = FALSE;
   }
}

void mousePanX(int x)
{
   GLdouble lookIn[4], lookOut[4];
   GLdouble matrix[16];

   if (x == oldMouseX)
   {
      return;
   }
   //tranlate in X according to game
   lookIn[1] = (GLfloat)(oldMouseX - x) * panMultX;//!!! * cameraDistance;
   lookIn[0] = lookIn[2] = lookIn[3] = 0.0;
   //transform the camera eye point according to the latitude
   glPushMatrix();
      glLoadIdentity();
      glRotated(cameraLat, 0.0, 0.0, 1.0);               //latitude rotation
      glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
   glPopMatrix();

   matrixMultVector(lookOut, lookIn, matrix);              //transform eye point

   cameraLook[0] += lookOut[0];
   cameraLook[1] += lookOut[1];
   cameraLook[2] += lookOut[2];
   oldMouseX = x;
   redraw();
}

void mousePanY(int y)
{
}

void mouseRotateLatitude(int x)
{
   cameraLat += (GLdouble)(x - oldMouseX) * degreeMultX;
   if (cameraLat >= 360.0)
   {
      cameraLat -= 360.0;
   }
   if (cameraLat < 0.0)
   {
      cameraLat += 0.0;
   }
   oldMouseX = x;
   redraw();
}

void mouseRotateLongitude(int y)
{
   cameraLong += (GLdouble)(y - oldMouseY) * degreeMultY;
   if (cameraLong < -89.9)
   {
      cameraLong = -89.9;
   }
   if (cameraLong > 89.9)
   {
      cameraLong = 89.9;
   }
   oldMouseY = y;
   redraw();
}

void mouseZoom(int y)
{
   GLdouble mult = 1.0;
   int index;
   if (y < oldMouseY)
   {
      for (index = y; index < oldMouseY; index++)
      {
         mult *= zoomInMult;
      }
   }
   else
   {
      for (index = oldMouseY; index < y; index++)
      {
         mult *= zoomOutMult;
      }
   }
   cameraDistance *= mult;
   setProjection();
   oldMouseY = y;
   redraw();
}

void mouseSelectX(int x)
{
    if (x == oldMouseX)
    {
        drawSelectionRect = TRUE;
        selectionY1 = selectionY1 = oldMouseY;
    }
    selectionX1 = min(oldMouseX, x);
    selectionX2 = max(oldMouseX, x) + 1;
}

void mouseSelectY(int y)
{
    if (y == oldMouseY)
    {
        drawSelectionRect = TRUE;
        selectionX1 = selectionX1 = oldMouseX;
    }
    selectionY1 = min(oldMouseY, y);
    selectionY2 = max(oldMouseY, y) + 1;
}

void mouseNULL(int unused)
{
}

/*****************************************************************/

void
setupPalette(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE* pPal;
    int pixelFormat = GetPixelFormat(hDC);
    int paletteSize;

    DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
    if (!(pfd.dwFlags & PFD_NEED_PALETTE ||
	  pfd.iPixelType == PFD_TYPE_COLORINDEX))
    {
	return;
    }

    paletteSize = 1 << pfd.cColorBits;
    pPal = (LOGPALETTE*)
	malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
    pPal->palVersion = 0x300;
    pPal->palNumEntries = paletteSize;

    /* start with a copy of the current system palette */
    (void) GetSystemPaletteEntries(hDC, 0, paletteSize, &pPal->palPalEntry[0]);

    {
	/* fill in an RGBA color palette */
	int redMask = (1 << pfd.cRedBits) - 1;
	int greenMask = (1 << pfd.cGreenBits) - 1;
	int blueMask = (1 << pfd.cBlueBits) - 1;
	int i;

	for (i=0; i<paletteSize; ++i) {
	    pPal->palPalEntry[i].peRed =
		    (((i >> pfd.cRedShift) & redMask) * 255) / redMask;
	    pPal->palPalEntry[i].peGreen =
		    (((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
	    pPal->palPalEntry[i].peBlue =
		    (((i >> pfd.cBlueShift) & blueMask) * 255) / blueMask;
	    pPal->palPalEntry[i].peFlags = 0;
	}
    }

    hPalette = CreatePalette(pPal);
    free(pPal);

    if (hPalette) {
	SelectPalette(hDC, hPalette, FALSE);
	RealizePalette(hDC);
    }
}

void
setupPixelformat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR),	/* size of this pfd */
	1,				/* version num */
	PFD_DRAW_TO_WINDOW |		/* support window */
	PFD_SUPPORT_OPENGL,		/* support OpenGL */
	PFD_TYPE_RGBA,			/* color type */
	8,				/* 8-bit color depth */
	0, 0, 0, 0, 0, 0,		/* color bits (ignored) */
	0,				/* no alpha buffer */
	0,				/* alpha bits (ignored) */
	0,				/* no accumulation buffer */
	0, 0, 0, 0,			/* accum bits (ignored) */
	0,				/* depth buffer (filled below)*/
	0,				/* no stencil buffer */
	0,				/* no auxiliary buffers */
	PFD_MAIN_PLANE,			/* main layer */
	0,				/* reserved */
	0, 0, 0,			/* no layer, visible, damage masks */
    };
    int SelectedPixelFormat;
    BOOL retVal;

    if (doubleBuffered) {
        pfd.dwFlags |= PFD_DOUBLEBUFFER;
    }

    if (depthBuffered) {
	pfd.cDepthBits = 16;
    }

    SelectedPixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (SelectedPixelFormat == 0) {
	MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed\n", "Error",
		MB_ICONERROR | MB_OK);
	exit(1);
    }

    retVal = SetPixelFormat(hDC, SelectedPixelFormat, &pfd);
    if (retVal != TRUE) {
	MessageBox(WindowFromDC(hDC), "SetPixelFormat failed", "Error",
		MB_ICONERROR | MB_OK);
	exit(1);
    }
}

LRESULT APIENTRY
WndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (message) {
    case WM_CREATE:
	hDC = GetDC(hWnd);
	setupPixelformat(hDC);
	setupPalette(hDC);
	hGLRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hGLRC);
	init();
	return 0;
    case WM_DESTROY:
	if (hGLRC) {
	    wglMakeCurrent(NULL, NULL);
	    wglDeleteContext(hGLRC);
	}
	idleFunc = NULL;
	ReleaseDC(hWnd, hDC);
	PostQuitMessage(0);
	return 0;
    case WM_SIZE:
	if (hGLRC) {
	    winWidth = (int) LOWORD(lParam);
	    winHeight = (int) HIWORD(lParam);
	    resize();
	    return 0;
	}
    case WM_PALETTECHANGED:
	if (hPalette != NULL && (HWND) wParam != hWnd) {
	    UnrealizeObject(hPalette);
	    SelectPalette(hDC, hPalette, FALSE);
	    RealizePalette(hDC);
	    redraw();
	    return 0;
	}
	break;
    case WM_QUERYNEWPALETTE:
	if (hPalette != NULL) {
	    UnrealizeObject(hPalette);
	    SelectPalette(hDC, hPalette, FALSE);
	    RealizePalette(hDC);
	    redraw();
	    return TRUE;
	}
	break;
    case WM_PAINT:
	if (hGLRC) {
	    PAINTSTRUCT ps;
	    BeginPaint(hWnd, &ps);
	    redraw();
	    EndPaint(hWnd, &ps);
	    return 0;
	}
	break;
   case WM_LBUTTONDOWN:
      if (hGLRC)
      {
         mouseButtonFlags |= MFI_Left;
         SetCapture(hWnd);
         return 0;
      }
      break;
    case WM_LBUTTONUP:
      if (hGLRC)
      {
         mouseButtonFlags &= ~MFI_Left;
         ReleaseCapture();
         return 0;
      }
      break;
   case WM_RBUTTONDOWN:
   	if (hGLRC)
      {
         mouseButtonFlags |= MFI_Right;
         SetCapture(hWnd);
   	   return 0;
   	}
      break;
    case WM_RBUTTONUP:
   	if (hGLRC)
      {
         mouseButtonFlags &= ~MFI_Right;
         ReleaseCapture();
   	   return 0;
   	}
      break;
    case WM_MOUSEMOVE:
	if (hGLRC) {
	    int x = ((int) LOWORD(lParam) << 16) >> 16;
	    int y = ((int) HIWORD(lParam) << 16) >> 16;
       mouseFunctionChangeCheck(x, y);
       mouseFunction[mouseButtonFlags * 2 + 0].function(x);
       mouseFunction[mouseButtonFlags * 2 + 1].function(y);
	    return 0;
	}
	break;
    case WM_CHAR:
	switch ((int)wParam) {
	   case VK_ESCAPE:
	      DestroyWindow(hWnd);
	      return 0;
      case VK_SPACE:
         if (bMouseCapture == FALSE)
         {
            bMouseCapture = TRUE;
            SetCapture(hWnd);
         }
         else
         {
            bMouseCapture = FALSE;
            ReleaseCapture();
         }
         return 0;
	default:
	    break;
	}
	break;
    case WM_SYSKEYDOWN:
        switch ((int)wParam)
        {
            case VK_MENU:
                if ((!(lParam & 0x80000000)) && hGLRC)
                {
                    mouseButtonFlags |= MFI_Alt;
                    SetCapture(hWnd);
                    return 0;
                }
                break;
        }
        break;
    case WM_SYSKEYUP:
        switch ((int)wParam)
        {
            case VK_MENU:
                if (hGLRC)
                {
                    mouseButtonFlags &= ~MFI_Alt;
                    ReleaseCapture();
                    return 0;
                }
                break;
        }
        break;
    case WM_KEYDOWN:
	    switch ((int)wParam)
        {
            case VK_CONTROL:
                if (hGLRC)
                {
                    mouseButtonFlags |= MFI_Control;
                    SetCapture(hWnd);
                    return 0;
                }
                break;
	        default:
	            break;
	    }
    case WM_KEYUP:
	    switch ((int)wParam)
        {
            case VK_CONTROL:
                if (hGLRC)
                {
                    mouseButtonFlags &= ~MFI_Control;
                    ReleaseCapture();
                    return 0;
                }
                break;
	        default:
	            break;
	    }
	if (hGLRC) redraw();
	return 0;
    default:
	break;
    }

    /* Deal with any unprocessed messages */
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int APIENTRY
WinMain(
    HINSTANCE hCurrentInst,
    HINSTANCE hPreviousInst,
    LPSTR lpszCmdLine,
    int nCmdShow)
{
    WNDCLASS wndClass;
    HWND hWnd;
    MSG msg;

    /* Define and register the window class */
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hCurrentInst;
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = GetStockObject(WHITE_BRUSH);
    wndClass.lpszMenuName = NULL;
    wndClass.lpszClassName = className;
    RegisterClass(&wndClass);

    /* Figure out a default size for the window */
    winWidth = GetSystemMetrics(SM_CYSCREEN) / 3;
    winHeight = GetSystemMetrics(SM_CYSCREEN) / 3;

    /* Create a window of the previously defined class */
    hWnd = CreateWindow(
	className, windowName,
	WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	winX, winY, winWidth, winHeight,
	NULL, NULL, hCurrentInst, NULL);

    /* Map the window to the screen */
    ShowWindow(hWnd, nCmdShow);

    /* Force the window to repaint itself */
    UpdateWindow(hWnd);

    /* Message loop */
    while (1) {
	while (idleFunc &&
	       PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == FALSE)
	{
	    (*idleFunc)();
	}
	if (GetMessage(&msg, NULL, 0, 0) != TRUE) {
	    break;
	}
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    return msg.wParam;
}


