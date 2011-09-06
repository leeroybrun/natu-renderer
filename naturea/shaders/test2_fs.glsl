#version 120

uniform sampler2D	colorMap;
uniform sampler2D	displacementMap;
uniform sampler2D	normalMap;
uniform	sampler2D   dataMap;

uniform float		time;

uniform float		time_offset;
uniform float		wave_amplitude;
uniform float		wave_frequency;
uniform	vec2		movementVectorA;
uniform	vec2		movementVectorB;
uniform float		wave_y_offset;
uniform float		wave_increase_factor;	
void main()
{	
	
	float t = time*wave_frequency+time_offset;
	float st = sin(t)*0.5 + 0.5;
	vec2 movVectorA = movementVectorA;
	vec2 movVectorB = movementVectorB;
	//vec2 movVectorA = vec2(0.0, 1.0);//movementVectorA;
	//vec2 movVectorB = vec2(0.0, 1.0);//movementVectorB;
	vec2 texCoordA = gl_TexCoord[0].st+t*movVectorA;
	vec2 texCoordB = gl_TexCoord[0].st+t*movVectorB; // gl_TexCoord[0].st+t*movVectorB;



	texCoordA = (texture2D(displacementMap, texCoordA).st*2.0 - vec2(1.0))*(wave_y_offset + gl_TexCoord[0].t*wave_increase_factor);
	texCoordB = (texture2D(displacementMap, texCoordB).st*2.0 - vec2(1.0))*(wave_y_offset + gl_TexCoord[0].t*wave_increase_factor);
	vec2 texCoord = gl_TexCoord[0].st+(texCoordA+texCoordB)*wave_amplitude;// texture2D(displacementMap, ).st;
	
	vec4 color = texture2D(colorMap, gl_TexCoord[0].st+(texCoordA)*wave_amplitude);
	if (color.a<0.1){discard;}
	gl_FragData[0] = color;
	gl_FragData[1] = color * vec4(0.5, 0.5, 0.5, 1.0);


}
