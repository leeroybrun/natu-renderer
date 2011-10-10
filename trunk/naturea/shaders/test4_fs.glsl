#version 120

uniform sampler2D	branch_noise_tex;
uniform sampler2D	leaf_noise_tex;
uniform sampler2D	seasonMap;

uniform sampler2D	color_tex_1	;
uniform sampler2D	color_tex_2	;
uniform sampler2D	normal_tex_1;
uniform sampler2D	normal_tex_2;
uniform sampler2D	branch_tex_1;
uniform sampler2D	branch_tex_2;

uniform float		season;
uniform float		time;

uniform	vec2		movementVectorA;
uniform	vec2		movementVectorB;
uniform vec2		window_size;

uniform float		varA;
uniform float		scale;
uniform float		bias;

uniform vec4		wood_amplitudes;
uniform vec4		wood_frequencies;
uniform float		leaf_amplitude;
uniform float		leaf_frequency;

varying vec3		eyeDir;
varying vec3		normalDir;
varying vec3		normal_es;
varying vec3		tangent_es;

varying vec3		lightDir_ts;
varying vec3		eyeDir_ts;
varying	float		alpha;

varying vec2		sliceDesc;

varying mat3		TBN_Matrix;

varying vec3		colorVar;
varying float		time_offset_v;

uniform float		MultiplyAmbient			;
uniform float		MultiplyDiffuse			;
uniform float		MultiplySpecular		;
uniform float		MultiplyTranslucency	;
			 
float		fogFactor;

void	main()
{	
	float dist = gl_FragCoord.z;
	float inv_dist = 1.0/dist;
	vec2 fpos	= gl_TexCoord[0].st;
	vec4 color;
	float sizeFactor = 1.5/max(window_size.x, window_size.y)*0.5;
	float t	= 5.0*(time+time_offset_v)*leaf_frequency*sizeFactor;
	vec2 texCoordA	= fpos+t*movementVectorA;
	vec2 texCoordB	= fpos+t*movementVectorB;
	texCoordA = (texture2D(leaf_noise_tex, texCoordA).st*2.0 - vec2(1.0));
	texCoordB = (texture2D(leaf_noise_tex, texCoordB).st*2.0 - vec2(1.0));
	vec2 noiseOffset = (texCoordA+texCoordB)*sizeFactor*leaf_amplitude*0.5;
	vec2 newPos = fpos;
	vec2 b0 = vec2(0.5,0.0);
	vec2 b1;
	if (sliceDesc.y>0.0){
		b1 = texture2D(branch_tex_2, fpos).zw;
	} else {
		b1 = texture2D(branch_tex_1, fpos).zw;
	}
	float dist0 = min (1.0, 2.0 * length(fpos - b0)) ; // / branchProjectedLength
	float dist1 = min (1.0, 5.0 * length(fpos - b1)); // / branchProjectedLength

	float angle;
	float cosA;
	float sinA;
	vec2  difVec;
	mat2 R;
	vec2 rotatedDifVec;
	float ti = (time+time_offset_v)*sizeFactor*10.0;
	vec2 si = sizeFactor* 100.0 * wood_amplitudes.xy;
	float d = length(b1-vec2(0.5));

	if ((d>0.01)){
		angle = dist1*(texture2D(branch_noise_tex, (ti * b1 * wood_frequencies.y)).s*2.0 - 1.0)  * si.y * 2.0;
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
	vec2 texCoord = newPos + noiseOffset ;
	vec4 fragmentNormalLeaf  ;
	vec4 fragmentNormalBranch;
	if (sliceDesc.y>0.0){
		// 90 degrees rotated billboard
		fragmentNormalLeaf   = texture2D(normal_tex_2, texCoord);
		fragmentNormalBranch = texture2D(normal_tex_2, newPos);		
	} else {
		fragmentNormalLeaf   = texture2D(normal_tex_1, texCoord);
		fragmentNormalBranch = texture2D(normal_tex_1, newPos);
		
	}
	float branchFlag = fragmentNormalBranch.w + fragmentNormalLeaf.w;
	vec2 lookUpPos;
	vec4 fragmentNormal;
	float leaf = 1.0;
	if (branchFlag<0.1){
		// trunk / branch 
		leaf = 0.0;
		fragmentNormal = fragmentNormalBranch;
		fragmentNormal.xyz = fragmentNormal.xyz*2.0 - vec3(1.0);
		lookUpPos = newPos;
	} else {
		// foliage
		fragmentNormal = fragmentNormalLeaf;
		fragmentNormal.xyz = fragmentNormal.xyz*2.0 - vec3(1.0);
		lookUpPos = texCoord;
		leaf = (1.0/0.9)*(fragmentNormal.w-0.1);
		// if normal runs to the negative half-space
		if (fragmentNormal.z<0.0){
			fragmentNormal = -fragmentNormal;
		}		
	}
	if (gl_FrontFacing){
		// flip normal
		fragmentNormal.z = -fragmentNormal.z;
	}

	vec4 decal_color;
	if (sliceDesc.y>0.0){
		decal_color = texture2D(color_tex_2, lookUpPos);
	} else {
		decal_color = texture2D(color_tex_1, lookUpPos);
	}
	// escape when transparent...
	if (decal_color.a<0.5){discard;}
	vec3 final_translucency;// * (shadow_intensity * ReduceTranslucencyInShadow)* MultiplyTranslucency;
	vec4 final_ambient = vec4(0.0);
	vec4 final_diffuse = vec4(0.0);
	float NdotL = clamp ( dot ( normalize ( fragmentNormal.xyz ) , normalize ( lightDir_ts ) ) , 0.0, 1.0);
	
	//vec3 variation = colorVar;
	if (leaf>0.0){
		// leaf
		float mNdotL = max ( -dot ( normalize ( fragmentNormal.xyz ) , normalize ( lightDir_ts ) ) , 0.0);
		vec4 noise = texture2D(leaf_noise_tex, 0.5*vec2(1.0, leaf)*t);
		float noise1f = noise.x*2.0-1.0;
		NdotL += noise1f;
		vec2 seasonCoord = vec2(0.5, season + 0.2*leaf - 0.0001*time_offset_v);
		
		vec4 seasonColor =  texture2D(seasonMap, seasonCoord);
		// discard leaf
		if (seasonColor.a<0.5){
			discard;
		}
		decal_color.rgb += seasonColor.rgb;
		decal_color.rgb *= colorVar;
		decal_color.a	*= seasonColor.a;
		final_translucency = decal_color.rgb * 0.7 * mNdotL * 0.6 * MultiplyTranslucency;
		final_ambient = decal_color * gl_LightSource[0].ambient * MultiplyAmbient;
		final_diffuse = decal_color * NdotL * 0.7 * gl_FrontLightProduct[0].diffuse * MultiplyDiffuse;
	} 
	else {
		// branch
		//decal_color = vec4(1.0, 0.0, .0, 1.0); // debug
		final_translucency = vec3(0.0, 0.0, 0.0);
		final_ambient = decal_color * gl_LightSource[0].ambient * MultiplyAmbient;
		final_diffuse = decal_color * NdotL * MultiplyDiffuse;
	}
	// compose color (phong)
	color.rgb = ( final_ambient.rgb + final_diffuse.rgb + final_translucency);
	color.a = alpha;

	// fade LOD
	color.a *= gl_Color.a;

	gl_FragData[0] = color;
	gl_FragData[1] = vec4(0.0, 0.0, 0.0, 1.0);
	return;
}
