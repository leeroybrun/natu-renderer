#version 330 compatibility
//==============================================================================
//  Terrain FRAGMENT shader   
//
//==============================================================================
#define SCALE 1.0
#define DIST 0.0001
#define SHADOW_TRESHOLD 0.0001

uniform sampler2D terrain_tex_01;
uniform sampler2D terrain_tex_02;
uniform sampler2D terrain_tex_03;
uniform sampler2D terrain_tex_04;
uniform sampler2D terrain_tex_05;
uniform vec4    border_widths;
uniform vec4	border_values;
uniform vec2	visibleHeightInterval;


varying vec3	eye;
varying vec3	normal;
varying float	height;
//varying float	fogFactor;
varying vec4	position;

uniform sampler2D shadowMap;
varying vec4	lightSpacePosition;
vec4 lpos;
varying	vec4	lightProjSpacePosition;
uniform int		shadowMappingEnabled;
uniform mat4    LightProjectionMatrix;
uniform mat4    LightMVPmatrix;
uniform int		fastMode;

void main()
{
	if (height>visibleHeightInterval.y || height<visibleHeightInterval.x){
		//gl_FragData[0] = vec4(1.0, 0.0, 0.0, 1.0);
		//gl_FragData[1] = vec4(0.0, 1.0, 0.0, 1.0);
		//return;
		discard;
	}
	if (fastMode>0){
		gl_FragData[0] = vec4(gl_FogFragCoord/500.0, 0.0, 0.0, gl_FogFragCoord);
		gl_FragData[1] = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}
	vec3 N = normalize(normal);
	vec3 L = normalize(gl_LightSource[0].position.xyz);
	vec3 E = normalize(-eye);
	vec3 R = normalize(-reflect(L,N));

	float NdotL = max(dot(normalize(N),normalize(L)), 0.0);
	float RdotE = max(dot(R,E),0.0);

	vec4 Ia = gl_FrontLightProduct[0].ambient;
	vec4 Id = gl_FrontLightProduct[0].diffuse * NdotL;
	//vec4 Is = gl_FrontLightProduct[0].specular * pow(RdotE, gl_FrontMaterial.shininess);
	vec4 Is = gl_FrontLightProduct[0].specular * pow(RdotE, 2.0);

	//-----------------------------------------------------------
	// multitexturing :

	vec4 tex_color, tex_color1, tex_color2;
	//vec4 border_values	= vec4(10.0, 5.0, 2.0, 0.0);
	//vec4 border_widths  = vec4(5.0, 2.0, 1.0, 0.5);


	if (height > border_values.x)
	{
		tex_color1 = texture2D(terrain_tex_01, gl_TexCoord[0].st*SCALE);
		tex_color2 = texture2D(terrain_tex_02, gl_TexCoord[0].st*SCALE);
		tex_color = mix(tex_color2, tex_color1, min(max((height - border_values.x)/border_widths.x, 0.0), 1.0));
	}
	else if (height > border_values.y)
	{
		tex_color1 = texture2D(terrain_tex_02, gl_TexCoord[0].st*SCALE);
		tex_color2 = texture2D(terrain_tex_03, gl_TexCoord[0].st*SCALE);
		tex_color = mix(tex_color2, tex_color1, min(max((height - border_values.y)/border_widths.y, 0.0), 1.0));
	}
	else if (height > border_values.z)
	{
		tex_color1 = texture2D(terrain_tex_03, gl_TexCoord[0].st*SCALE);
		tex_color2 = texture2D(terrain_tex_04, gl_TexCoord[0].st*SCALE);
		tex_color = mix(tex_color2, tex_color1, min(max((height - border_values.z)/border_widths.z, 0.0), 1.0));
	}
	else
	{
		tex_color1 = texture2D(terrain_tex_04, gl_TexCoord[0].st*SCALE);
		tex_color2 = texture2D(terrain_tex_05, gl_TexCoord[0].st*SCALE);
		tex_color = mix(tex_color2, tex_color1, min(max((height - border_values.w)/border_widths.w, 0.0), 1.0));
	}

	// shadow
	lpos = (lightSpacePosition/lightSpacePosition.w * 0.5) + vec4(0.5);
	float depthEye  = lpos.z;
	float depthLight= texture2D(shadowMap, lpos.xy).x;
		// lightSpacePosition = LightProjectionMatrix * LightViewModelMatrix * CameraViewInvereseMatrix * gl_ModelViewMatrix * gl_Vertex
		
		
		//float lightDistance = length(lightSpacePosition);
		//float depthLight  = texture2D(shadowMap, l.xy).a;
		//float depthDifference = lightDistance - depthLight;
		/*
		vec4 L = lightProjSpacePosition;
		vec4 l = (L/L.w + vec4(1.0,1.0,1.0,0.0))*0.5;
		float depthCamera = l.z;
		float depthLight0  = texture(shadowMap, l.xy).r;
		vec2 coord = l.xy + vec2(DIST, 0.0);
		float depthLight1 = texture(shadowMap, coord).r;
		coord = l.xy + vec2(-DIST, 0.0);
		float depthLight2 = texture(shadowMap, coord).r;
		coord = l.xy + vec2(0.0, DIST);
		float depthLight3 = texture(shadowMap, coord).r;
		coord = l.xy + vec2(0.0, -DIST);
		float depthLight4 = texture(shadowMap, coord).r;

		float depthDifference = depthCamera - depthLight0;
		if (depthDifference > 0.001){
			shade =shade - 0.05;
		}
		depthDifference = depthCamera - depthLight1;
		if (depthDifference > 0.001){
			shade =shade - 0.05;
		}
		depthDifference = depthCamera - depthLight2;
		if (depthDifference > 0.001){
			shade =shade - 0.05;
		}
		depthDifference = depthCamera - depthLight3;
		if (depthDifference > 0.001){
			shade =shade - 0.05;
		}
		depthDifference = depthCamera - depthLight4;
		if (depthDifference > 0.001){
			shade =shade - 0.05;
		}
		*/
		
		
		//shade = depthDifference;
	//}
	float shade = 1.0;
	if ((depthEye - depthLight) > SHADOW_TRESHOLD){
		shade = 0.5;
	}
	vec4 color = gl_FrontLightModelProduct.sceneColor + (Ia + Id)*tex_color +Is;
	color.rgb *= shade;
	//vec4 color = (Ia + Id)*tex_color +Is;
	//vec4 color = gl_FrontLightModelProduct.sceneColor;// + (Ia + Id)*tex_color +Is;
	gl_FragData[0] = color;
	//gl_FragData[0] = shade * color;// mix(gl_Fog.color, color, fogFactor );
	gl_FragData[1] = vec4(0.0, 0.0, 0.0, 1.0);
}