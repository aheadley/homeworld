/* Modified from the original wgl.c to also provide glx functions when not
   using Windows.  Enough is provided here so we can dupe SDL into thinking we
   have a valid OpenGL library when using SDL_GL_LoadLibrary(). */

#ifdef _WIN32
#define DLL __declspec(dllexport)
#define API __stdcall
#else
    #include <X11/Xutil.h>
    #define DLL
    #define API
#endif

typedef void (*PROC)();
extern PROC rglGetProcAddress(const char*);
extern unsigned char* API glGetString (unsigned int cap);
#define GL_EXTENSIONS 0x1F03

#ifdef _WIN32	/* wgl */

DLL unsigned int API wglMakeCurrent(int hdc, int hglrc) { return 1; }
DLL unsigned int API wglDeleteContext(int hglrc) { return 1; }
DLL unsigned int API wglCreateContext(int hdc) { return 1; }
DLL PROC API wglGetProcAddress(const char* string)
{
    return rglGetProcAddress(string);
}

#else	/* glx */

XVisualInfo* glXChooseVisual (Display* dpy, int screen, int* attribList)
{
	return (XVisualInfo*)1;
}

unsigned long glXCreateContext (Display* dpy, XVisualInfo* vis,
	unsigned long shareList, Bool direct)
{
	return 1;
}

void glXDestroyContext (Display* dpy, unsigned long ctx)
{
	/* ... */
}

int glXMakeCurrent (Display* dpy, unsigned long drawable, unsigned long ctx)
{
	return 1;
}

void glXSwapBuffers (Display* dpy, unsigned long drawable)
{
	/* ... */
}

int glXGetConfig (Display* dpy, XVisualInfo* vis, int attrib, int* value)
{
	/* This will never work... */
	return -1;
}

const char* glXQueryExtensionsString (Display* dpy, int screen)
{
	/* SDL requires this from an OpenGL library, but never uses it.  Either
	   way, we should play it safe... */
	return (const char*)glGetString(GL_EXTENSIONS);
}

#endif
