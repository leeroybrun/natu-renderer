#version 120

attribute mat4			transformMatrix;
attribute float			param1;
varying float			alpha;

void main()
{	
	gl_Position = gl_ModelViewProjectionMatrix * transformMatrix * gl_Vertex;
	alpha = param1;
	gl_FrontColor = gl_Color;
}

