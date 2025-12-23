#pragma once
#include <GL/glew.h>
#define SOLVE_FGLUT_WARNING
#include <GL/freeglut.h>

class FrameBuffer
{
    public:
        unsigned int idFbo;
        unsigned int idColorBuffer;
        unsigned int idDepthBuffer;

        void Init();
        void Resize(unsigned int w, unsigned int h);
        void Destroy();   
};