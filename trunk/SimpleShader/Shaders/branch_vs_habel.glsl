#version 120
uniform sampler2DRect	branch_data_tex;
uniform vec2			A;
uniform float			time;
attribute vec3			normal;
attribute vec3			tangent;
attribute vec4			x_vals;
attribute float			branch_index;

void main()
{
	vec4 vertex = gl_Vertex;
	vec3 tang = tangent;
	vec4 x = x_vals;
	float brid = branch_index; 
	gl_FrontColor = vec4(normal, 1.0);//vec4(time*0.01,0.0,0.0,1.0);
	gl_Position = gl_ModelViewProjectionMatrix * vertex;
}
