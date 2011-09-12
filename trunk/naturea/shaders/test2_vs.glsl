#version 120

attribute vec3			normal;
attribute vec3			tangent;
attribute vec2			texCoords0;

varying vec3			eyeDir;
varying vec3			normalDir;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	eyeDir = (gl_ModelViewMatrix * gl_Vertex).xyz;
	normalDir = (gl_NormalMatrix * normal);
	gl_FrontColor = vec4(normal, gl_Color.a);
	gl_TexCoord[0] = vec4(texCoords0, 0.0, 0.0);
	
}

