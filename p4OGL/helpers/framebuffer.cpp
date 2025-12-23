#include "helpers\framebuffer.h"

void FrameBuffer::Init()
{
    // Create the FBO
    glGenFramebuffers(1, &idFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, idFbo);

    // Create the gbuffer textures
    glGenTextures(1, &idColorBuffer);
    glGenTextures(1, &idDepthBuffer);
}

void FrameBuffer::Resize(unsigned int w, unsigned int h){
	glBindTexture(GL_TEXTURE_2D, idColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, idDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, idFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, idColorBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, idDepthBuffer, 0);

	const GLenum buffs[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, buffs);

	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		exit(-1);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Destroy()
{
	if (idFbo != 0) {
		glDeleteFramebuffers(1, &idFbo);
		idFbo = 0;
	}

	glDeleteTextures(1, &idColorBuffer);
	glDeleteTextures(1, &idDepthBuffer);
}