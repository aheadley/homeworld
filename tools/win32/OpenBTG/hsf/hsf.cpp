#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    char identifier[12];
    int  version;
    int  nLights;
} HSFFileHeader;

typedef struct
{
    int   type;
    float x, y, z;
    float h, p, b;
    float coneAngle;
    float edgeAngle;
    unsigned char red, green, blue;
    float intensity;
} HSFLight;

enum LightType
{
    L_DistantLight,
    L_PointLight,
    L_SpotLight,
    L_LinearLight,
    L_AreaLight,
    L_AmbientLight
};

int main(int argc, char* argv[])
{
    HSFFileHeader header;
    HSFLight light;

    if (argc != 2)
    {
        puts("usage: hsf file.hsf");
        exit(-1);
    }

    FILE* in = fopen(argv[1], "rb");
    if (in == 0)
    {
        puts("could not open file");
        exit(-1);
    }

    fread(&header, sizeof(header), 1, in);
    for (int i = 0; i < header.nLights; i++)
    {
        printf("%d", i);
        fread(&light, sizeof(light), 1, in);

        switch (light.type)
        {
        case L_AmbientLight:
            printf(" ambient");
            break;
        case L_DistantLight:
            printf(" distant");
            break;
        case L_PointLight:
            printf(" point");
            break;
        case L_SpotLight:
            printf(" spot");
            break;
        default:
            printf(" unknown");
        }

        printf(" rgb [%3d %3d %3d]", light.red, light.green, light.blue);
        printf(" xyz [%0.3f %0.3f %0.3f]", light.x, light.y, light.z);
        printf(" i %0.3f", light.intensity);

        printf("\n");
    }

    puts("done.");
    fclose(in);

    return 0;
}
