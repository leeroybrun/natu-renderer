#version 120

uniform sampler2D	colorMap;
uniform sampler2D	branch_noise_tex;
uniform sampler2D	leaf_noise_tex;
uniform sampler2D	normalMap;
uniform	sampler2D   dataMap;

uniform float		time;

uniform float		time_offset;
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

uniform float		MultiplyAmbient			;
uniform float		MultiplyDiffuse			;
uniform float		MultiplySpecular		;
uniform float		MultiplyTranslucency	;


#define sliceCnt		3
#define sliceSetsCnt	3
					 
float		fogFactor;

void	main()
{	
	vec3 eyeDir_ts2 = normalize(eyeDir_ts);
	float sizeFactor = 1.5/max(window_size.x, window_size.y);

	float t			= time*20.0*leaf_frequency*sizeFactor+time_offset;
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
	vec4 fragmentNormalLeaf = texture2D(normalMap, texCoord);
	vec4 fragmentNormalBranch = texture2D(normalMap, newPos);
	float branchFlag = fragmentNormalBranch.w + fragmentNormalLeaf.w;
	vec2 lookUpPos;
	float leaf = 1.0;
		if (branchFlag>0.5){
			// trunk / branch 
			leaf = 0.0;
			fragmentNormal = fragmentNormalBranch;
			fragmentNormal.xyz = fragmentNormal.xyz*2.0 - vec3(1.0);
			/* pseudo-parallax mapping */
			//float height = fragmentNormal.z;			
			//float hsb = height * scale + bias;    
			//vec2 normalLookUp = newPos + (hsb * eyeDir_ts.xy);
			//
			//fragmentNormal = texture2D( normalMap, normalLookUp );
			lookUpPos = newPos;
			

		} else {
			// foliage
			fragmentNormal = fragmentNormalLeaf;
			fragmentNormal.xyz = fragmentNormal.xyz*2.0 - vec3(1.0);
			lookUpPos = texCoord;
			// if normal runs to the negative half-space
			if (fragmentNormal.z<0.0){
				fragmentNormal = -fragmentNormal;
			}

		}
		if (gl_FrontFacing){
			fragmentNormal.z = -fragmentNormal.z;
		}
		// float h = gl_FrontMaterial.shininess;
		// vec3 E = -eyeDir_ts2;
		// vec3 Refl = reflect(-lightDir_ts, normalize ( fragmentNormal.xyz ));
		// float RdotE = max(dot(Refl, E),0.0);
		float NdotL = clamp ( dot ( normalize ( fragmentNormal.xyz ) , normalize ( lightDir_ts ) ) , 0.0, 1.0);
		// float NodotE = clamp ( abs(dot ( vec3(0.0, 0.0, 1.0), eyeDir_ts2 )) , 0.0, 1.0);
		

		//float spec = pow( RdotE, h );
		//vec4 ambient = gl_LightSource[0].ambient;
		//vec4 diffuse = gl_FrontLightProduct[0].diffuse * NdotL * NodotE;
		//vec4 specular = gl_FrontLightProduct[0].specular * spec;

		vec4 decal_color = pow(texture2D(colorMap, lookUpPos), vec4(1.5));

		//vec3 translucency_in_light = translucency * other_cpvcolor.rgb * gl_LightSource[0].diffuse.rgb ;
		vec3 final_translucency;// * (shadow_intensity * ReduceTranslucencyInShadow)* MultiplyTranslucency;
		vec4 final_ambient = vec4(0.0);
		vec4 final_diffuse = vec4(0.0);
		if (leaf>0.0){
			// leaf
			float mNdotL = max ( -dot ( normalize ( fragmentNormal.xyz ) , normalize ( lightDir_ts ) ) , 0.0);
			vec4 noise = texture2D(leaf_noise_tex, 0.5*fragmentNormal.xz*t);
			float noise1f = noise.x*2.0-1.0;
			//mNdotL += noise1f;
			NdotL += noise1f;
			final_translucency = decal_color.rgb * mNdotL * MultiplyTranslucency;
			final_ambient = decal_color * gl_LightSource[0].ambient * MultiplyAmbient;
			final_diffuse = decal_color * NdotL * gl_FrontLightProduct[0].diffuse * MultiplyDiffuse;
		} 
		else {
			// branch
			final_translucency = vec3(0.0, 0.0, 0.0);
			final_ambient = decal_color * gl_LightSource[0].ambient * MultiplyAmbient;
			final_diffuse = decal_color * NdotL * MultiplyDiffuse;
		}
		color.rgb = final_ambient.rgb + final_diffuse.rgb + final_translucency;
		color.a = decal_color.a;

		
		
		
	
	//color.rgb = pow (color.rgb, vec3(1.5));
	if (color.a<0.5){discard;}
	
	color.a = clamp(-3.5+5.0*abs(dot(normalize(normalDir.xz), normalize(eyeDir.xz))), 0.0, 1.0);
	color.a *= gl_Color.a;
	//color.rgb = vec3(z*0.001);
	gl_FragData[0] = color;
	//gl_FragData[1] = vec4(0.0, 0.0, 0.0, 1.0);
	gl_FragData[1] = color * vec4(0.2, 0.2, 0.2, 1.0);
/*
	vec4 color = vec4 (1.0, 0.0, 0.0, 1.0);
	
	gl_FragData[0] = color * gl_Color;
	gl_FragData[1] = color * vec4(0.5, 0.5, 0.5, 1.0);
*/	
}
