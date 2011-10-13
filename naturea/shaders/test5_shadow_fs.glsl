#version 120
#define SHADOW_TRESHOLD 0.0001

uniform sampler2D	colorMap;
uniform sampler2D	branch_noise_tex;
uniform sampler2D	leaf_noise_tex;
uniform sampler2D	normalMap;
uniform	sampler2D   dataMap;
uniform sampler2D   depthMap;
uniform sampler2D	seasonMap;
uniform float		season;
uniform float		time;

uniform	vec2		movementVectorA;
uniform	vec2		movementVectorB;
uniform vec2		window_size;

uniform vec4		wood_amplitudes;
uniform vec4		wood_frequencies;
uniform float		leaf_amplitude;
uniform float		leaf_frequency;
uniform float		transition_control;
uniform float		near;
uniform	float		far;

varying vec2		sliceDesc;

varying float		time_offset_v;

uniform float		dither;

#define sliceCnt		3
#define sliceSetsCnt	3
#define	texCols			18.0


void	main()
{	
	float sizeFactor = 1.5/max(window_size.x, window_size.y);

	float dist = gl_FragCoord.z;
	float inv_dist = 1.0/dist;
	float t			= 10.0*(time+time_offset_v)*leaf_frequency*sizeFactor;
	vec2 movVectorA = movementVectorA;
	vec2 movVectorB = movementVectorB;

	vec2 texC		= gl_TexCoord[0].st;
	vec2 fpos		= clamp ( texC + sliceDesc , sliceDesc, sliceDesc+vec2(1.0, 1.0) ) / vec2(sliceCnt,sliceSetsCnt);
	
	vec2 mv1 = texture2D(dataMap, fpos).xy*2.0 - vec2(1.0);
	float mv_time = 0.01 * (time+time_offset_v);
	vec2 amp1 = wood_amplitudes.y * ( texture2D(branch_noise_tex, mv1 * mv_time * wood_frequencies.y).rg  * 2.0 - vec2(1.0));


	vec2 texCoordA	= fpos+t*movVectorA;
	vec2 texCoordB	= fpos+t*movVectorB; // gl_TexCoord[0].st+t*movVectorB;
	
	vec2 oneV2 = vec2(1.0);
	vec2 b0 = vec2(0.5,0.0);
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
	float ti = (time+time_offset_v)*sizeFactor*10.0;
	vec2 si = sizeFactor* 100.0 * wood_amplitudes.xy;
	float d = length(b1-vec2(0.5));

	if ((d>0.01)){
		//angle = dist1 * (amp1.x+amp1.y) * sizeFactor * 200;
		angle = dist1*(texture2D(branch_noise_tex, (ti * b1 * wood_frequencies.y)).s*2.0 - 1.0)  * si.y * 2.0;
		//angle = (texture2D(branch_noise_tex, (ti * b1 * wood_frequencies.y)).s*2.0 - 1.0) * si.y;
		
		cosA = cos (angle); 
		sinA = sin (angle);
		difVec = (newPos - b1);
		R = mat2(	 cosA	, sinA,
 					-sinA	, cosA );
		rotatedDifVec = R*difVec;
		newPos = b1 + rotatedDifVec;
	}
	
	angle = dist0 * (texture2D(branch_noise_tex, (ti * b0 * wood_frequencies.x)).s*2.0-1.0) * si.x * 0.5;
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
	vec4 fragmentNormal;
	vec2 noiseOffset = (texCoordA+texCoordB)*sizeFactor*leaf_amplitude*0.5 / sliceCnt;
	
	vec2 texCoord = newPos + noiseOffset ;
	float branchFlag = texture2D(normalMap, texCoord).w + texture2D(normalMap, newPos).w;
	vec2 lookUpPos = newPos;
	float leaf = 1.0;
	if (branchFlag<0.1){
		// trunk / branch 
		leaf = 0.0;
	} else {
		// foliage
		lookUpPos = texCoord;
		
		leaf = (1.0/0.9)*(fragmentNormal.w-0.1);
	}
	//fragmentNormal = fragmentNormalBranch;
	float frontFacing = -1.0;
	if (gl_FrontFacing){
		frontFacing = 1.0;
	}
	vec4 decal_color = texture2D(colorMap, lookUpPos);
	float depth_tex = texture2D(depthMap, lookUpPos).x;

	// escape when transparent...
	if (decal_color.a<0.75){discard;}

	//vec3 translucency_in_light = translucency * other_cpvcolor.rgb * gl_LightSource[0].diffuse.rgb ;
	if (leaf>0.0){
		// leaf
		vec2 seasonCoord = vec2(0.5, season + 0.2*leaf - 0.0001*time_offset_v);
		vec4 seasonColor =  texture2D(seasonMap, seasonCoord);
		if (seasonColor.a<0.5){
			discard;
		}
	} 
	gl_FragData[0] = vec4(1.0, 0.0, 0.0, 1.0);
	float treshold = 0.5;
	float remapFactor = 1.0/(far-near);
	float depth = gl_FragCoord.z + frontFacing*(depth_tex*2.0 - 1.0)*remapFactor;
	

	// DITHER //
	if (transition_control<1.0 && transition_control>0.0){
		ivec2 screen_pos = ivec2(gl_FragCoord.xy);
		float c = 2.0*(1.0-transition_control);
		if (mod(screen_pos.x, c)<treshold && mod(screen_pos.y, c)<treshold){
		} else {	
			discard;
		}
	}
	
	gl_FragData[0] = vec4(1.0, 0.0, 0.0, 1.0);
	//float depth = gl_FragCoord.z + frontFacing*(depth_tex*2.0 - 1.0)*0.01*3.0;
	//if (leaf>0.0){
	//	gl_FragData[1] = color * vec4(0.1, 0.1, 0.1, 1.0);
	//} else {		
	//	gl_FragData[1] = vec4(0.0, 0.0, 0.0, 1.0);	
	//}	
	//float signal = 1.0-transition_control;
	gl_FragDepth = depth; // + 0.01*signal*signal;
	
}
