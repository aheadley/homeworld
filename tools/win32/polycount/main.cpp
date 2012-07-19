//
// main.cpp
//

#include <stdio.h>
#include "mex.h"

int main(int argc, char* argv[])
{
    meshdata* mesh;

    if (argc != 2)
    {
        puts("usage: polycount filename");
    }

    mesh = meshLoad("", argv[1]);
    if (mesh == NULL)
    {
        return 1;
    }

    printf("%d", meshCountPolygons(mesh));

    return 0;
}
