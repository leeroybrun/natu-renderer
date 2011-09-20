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
					 

void	main()
{	
	float sizeFactor = 1.0/max(window_size.x, window_size.y);

	float t			= time*10.0*leaf_frequency*sizeFactor+time_offset;
	vec2 movVectorA = movementVectorA;
	vec2 movVectorB = movementVectorB;

	vec2 texC		= gl_TexCoord[0].st;
	vec2 fpos		= clamp ( texC + sliceDesc , sliceDesc, sliceDesc+vec2(1.0, 1.0) ) / vec2(sliceCnt,sliceSetsCnt);

	vec2 texCoordA	= fpos+t*movVectorA;
	vec2 texCoordB	= fpos+t*movVectorB; // gl_TexCoord[0].st+t*movVectorB;
	
	vec2 oneV2 = vec2(1.0);
	vec2 b0 = texture2D(dataMap, fpos).xy;
	vec2 b1 = texture2D(dataMap, fpos).zw;

	float dist0 = min (1.0, 2.0 * length(texC - b0)) ; // / branchProjectedLength
	float dist1 = min (1.0, 5.0 * length(texC - b1)); // / branchProjectedLength

	vec4 color;
	float angle;
	float cosA;
	float sinA;
	vec2  difVec;
	mat2 R;
	vec2 rotatedDifVec;
	vec2 newPos = texC;
	float ti = time*sizeFactor*5.0;
	vec2 si = sizeFactor* 100.0 * wood_amplitudes.xy;
	float d = length(b1-vec2(0.5));

	if ((d>0.01)){
		angle = dist1*(texture2D(branch_noise_tex, (ti * b1 * wood_frequencies.y)).s*2.0 - 1.0)  * si.y;
		//angle = (texture2D(branch_noise_tex, (ti * b1 * wood_frequencies.y)).s*2.0 - 1.0) * si.y;
		
		cosA = cos (angle); 
		sinA = sin (angle);
		difVec = (newPos - b1);
		R = mat2(	 cosA	, sinA,
 					-sinA	, cosA );
		rotatedDifVec = R*difVec;
		newPos = b1 + rotatedDifVec;
	}
	
	angle = dist0 * (texture2D(branch_noise_tex, (ti * b0 * wood_frequencies.x)).s*2.0-1.0) * si.x;
	cosA = cos (angle); 
	sinA = sin (angle);
	difVec = (newPos - b0);
	R = mat2(	 cosA	, sinA,
 				-sinA	, cosA );
	rotatedDifVec = R*difVec;
	newPos = b0 + rotatedDifVec;
	newPos = clamp ( newPos  , vec2(0.0, 0.0), vec2(1.0, 1.0) );// + sliceDesc ) / vec2(sliceCnt,sliceSetsCnt);
	newPos = (newPos + sliceDesc) / vec2(sliceCnt,sliceSetsCnt);
	texCoordA = (texture2D(leaf_noise_tex, texCoordA).st*2.0 - vec2(1.0));
	texCoordB = (texture2D(leaf_noise_tex, texCoordB).st*2.0 - vec2(1.0));
	
	
	//newPos = fpos;

	vec2 texCoord = newPos+(texCoordA+texCoordB)*sizeFactor*leaf_amplitude / sliceCnt ;
	vec4 fragmentNormal = texture2D(normalMap, texCoord);
	
	float branchFlag = texture2D(normalMap, newPos).w + fragmentNormal.w;
	if (branchFlag>0.5){
		// trunk / branch 
		color = texture2D(colorMap, newPos);
		if (color.a<0.5){discard;}
	} else {
		// foliage
		color = texture2D(colorMap, texCoord);
		if (color.a<0.5){discard;}
		
	}
	
	color.a = 1.0;
	gl_FragData[0] = color;
	gl_FragData[1] = color * vec4(0.5, 0.5, 0.5, 1.0);

}
