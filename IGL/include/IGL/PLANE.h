#ifndef __PLANEFILE__
#define __PLANEFILE__

// We describe a plane in normalized coordinates
//*************************************************

// Vertices count
const int planeNVertex = 4; // 4 vertices 

// Vertices position
const float planeVertexPos[] = { 
	// Face z = 1
	-1.0f,	-1.0f,	 0.0f, 
	 1.0f,	-1.0f,	 0.0f, 
	-1.0f,	 1.0f,	 0.0f, 
	 1.0f,	 1.0f,	 0.0f, 
 };

#endif