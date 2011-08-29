#version 120

attribute vec3			normal;
attribute vec3			tangent;
attribute vec2			texCoords0;



void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	
	gl_TexCoord[0] = vec4(texCoords0, 0.0, 0.0);
	
}

