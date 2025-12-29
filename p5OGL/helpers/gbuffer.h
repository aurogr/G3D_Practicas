#pragma once
#include <GL/glew.h>
#define SOLVE_FGLUT_WARNING
#include <GL/freeglut.h>

class GBuffer
{
    public:
        unsigned int gFbo;
        unsigned int gPosition;
        unsigned int gNormal;
        unsigned int gAlbedoSpec;
        unsigned int gEmissive;
        unsigned int gDepth;

        void Init();

        void Resize(unsigned int w, unsigned int h);

        void Destroy();    
};