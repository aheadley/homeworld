#define DLL __declspec(dllexport)
#define API __stdcall

typedef void (*PROC)();

DLL unsigned int API wglMakeCurrent(int hdc, int hglrc) { return 1; }
DLL unsigned int API wglDeleteContext(int hglrc) { return 1; }
DLL unsigned int API wglCreateContext(int hdc) { return 1; }
DLL PROC API wglGetProcAddress(char* string)
{
    extern PROC rglGetProcAddress(char*);
    return rglGetProcAddress(string);
}
