#version 120

varying	float	alpha;

void main()
{	
	vec4 color		= vec4(1.0-alpha,0.0,alpha,0.5);
	gl_FragData[0]	= color;
	gl_FragData[1]	= color * vec4(0.5, 0.5, 0.5, 1.0);

}
