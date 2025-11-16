#ifndef __IG_AUXILIAR__
#define __IG_AUXILIAR__

// Loads a texture and returns a pointer to its location in main memory
// It also returns the size of the texture (w, h)
unsigned char *loadTexture(const char *fileName, unsigned int &w, unsigned int &h);

// Load a file into a character string
char *loadStringFromFile(const char *fileName, unsigned int &fileLen);

#endif