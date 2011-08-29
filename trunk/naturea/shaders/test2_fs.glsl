#version 120

uniform sampler2D	colorMap;
uniform sampler2D	displacementMap;
uniform float		time;

void main()
{	
	float t = time*0.1;
	vec2 movVectorA = vec2(0.0, 1.0);
	vec2 movVectorB = vec2(0.0, 0.33);
	vec2 texCoordA = gl_TexCoord[0].st+t*movVectorA;
	vec2 texCoordB = gl_TexCoord[0].st-t*movVectorB;
	texCoordA = texture2D(displacementMap, texCoordA).st;
	texCoordB = texture2D(displacementMap, texCoordB).st;
	vec2 texCoord = gl_TexCoord[0].st+(texCoordA+texCoordB)*0.1;// texture2D(displacementMap, ).st;
	
	vec4 color = texture2D(colorMap, texCoord);

	gl_FragData[0] = color;
	gl_FragData[1] = color * vec4(0.5, 0.5, 0.5, 1.0);


}
