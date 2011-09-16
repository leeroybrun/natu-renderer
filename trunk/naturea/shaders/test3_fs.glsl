#version 120

uniform sampler2D	colorMap;
uniform sampler2D	branch_noise_tex;
uniform sampler2D	leaf_noise_tex;
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
uniform vec2		window_size;

uniform vec4		wood_amplitudes;
uniform vec4		wood_frequencies;
uniform float		leaf_amplitude;
uniform float		leaf_frequency;

varying vec3		eyeDir;
varying vec3		normalDir;
varying	float		alpha;

varying vec2		sliceDesc;

#define sliceCnt		3
#define sliceSetsCnt	3


vec2	myClamp(in vec2 coords){
	vec2 clamped = clamp( (coords+sliceDesc) , sliceDesc, sliceDesc+vec2(1.0,1.0));
	//float x = clamped.x / sliceCnt;
	//float y = clamped.y / sliceSetsCnt;
	return clamped;
}

vec2 calcLookUp(in vec2 coord){
	vec2 s = vec2(sliceCnt,sliceSetsCnt);
	return clamp ( ((coord+sliceDesc)/s) , sliceDesc/s, (sliceDesc+vec2(1.0,1.0))/s );
}						 

void	main()
{	
	vec2	lookUpCoord = calcLookUp(gl_TexCoord[0].xy);
	vec4	color = vec4(0.0, 0.0, 0.0, 1.0);
	float	angle	= wood_amplitudes.x*0.1 ;
	vec2	b0		= texture2D(dataMap, lookUpCoord).xy;
	vec2	dif0	= gl_TexCoord[0].xy - b0;
	/*
	float	cosA	= cos (angle); 
	float	sinA	= sin (angle);
	mat2	R;
	R = mat2(	 cosA	, sinA,
 				-sinA	, cosA );
	vec2 difR = R*dif0;
	*/
	vec2 texCoord = (b0 + dif0 + vec2(1.0, 1.0)*angle);
	color = texture2D(colorMap, calcLookUp(texCoord));
	if (color.a<0.5) discard;
	//color = vec4(dif0, 0.0, 0.0, 1.0);
	//color.a = 1.0;
	//color.xy = difR;
	gl_FragData[0] = color;
	gl_FragData[1] = color*vec4(0.5,0.5,0.5,1.0);

	return;
	/*
	vec2	lookUpCoord = myClamp(gl_TexCoord[0].xy) / vec2(sliceCnt, sliceSetsCnt) ;
	vec4	color;
	float	angle = wood_amplitudes.x*0.1 ;
	float	cosA;
	float	sinA;
	vec2	difVec;
	vec2	rotatedDifVec;
	vec2	fpos = gl_TexCoord[0].xy;
	mat2	R;
	vec2 b0 = texture2D(dataMap, lookUpCoord).xy;

	cosA = cos (angle); 
	sinA = sin (angle);
	difVec = (fpos - b0);
	R = mat2(	 cosA	, sinA,
 				-sinA	, cosA );
	rotatedDifVec = R*difVec;
	vec2 newPos = myClamp( b0 + rotatedDifVec );

	//color = texture2D(colorMap, newPos);
	color = texture2D(colorMap, newPos);
	if (color.a<0.5){discard;}

	//color.a = 1.0;
	gl_FragData[0] = color;
	gl_FragData[1] = color*vec4(0.5,0.5,0.5,1.0);
	*/
}
