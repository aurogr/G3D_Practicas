#version 330 core

in vec3 inPos;		

void main()
{
	mat4 model = mat4(1.0);

	mat4 view = mat4(1.0);
	view[3].z = -6.0;

	mat4 proj = mat4(0.0);
	float n = 1.0;
	float f = 10.0;
	proj[0][0] = 1.17320508;
	proj[1][1] = 1.17320508;
	proj[2][2] = (f+n)/(n-f);
	proj[2][3] = (2*f*n)/(n-f);
	proj[3][2] = -1;

	gl_Position =  proj * view * model * vec4(inPos, 1.0);
}
