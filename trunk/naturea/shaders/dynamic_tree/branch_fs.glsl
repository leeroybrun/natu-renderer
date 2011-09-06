#version 120


varying vec3 normal_vs;
varying vec3 tangent_vs;
varying float level;
varying vec4 vPos;
varying vec3 lightDir;

varying vec2			b0_origin;
varying vec2			b1_origin;
varying vec2			b2_origin;

uniform sampler2D color_texture;
uniform vec2			window_size;

void main()
{	

//vec4 color = texture2D(color_texture, gl_TexCoord[0].xy);
	float h = gl_FrontMaterial.shininess;
	vec3 lightDir = gl_LightSource[0].position.xyz;

	vec3 N = normalize(normal_vs);
	vec3 L = normalize(lightDir);
	vec3 E = normalize(-vPos.xyz);
	vec3 R = reflect(-L, N);
	float RdotE = max(dot(R, E),0.0);
	float NdotL = max(dot(N, L),0.0);
	//float RdotE = abs(dot(R, E));
	//float NdotL = abs(dot(N, L));
	float spec = pow( RdotE, h );
	vec4 texColor = texture2D(color_texture, gl_TexCoord[0].xy);
	vec4 ambient = gl_LightSource[0].ambient;
	vec4 diffuse = gl_FrontLightProduct[0].diffuse * NdotL;
	vec4 specular = gl_FrontLightProduct[0].specular * spec;
	vec3 color = ((texColor) * (ambient + diffuse) + specular).xyz;
	//vec3 color = (texColor * diffuse).xyz;
	
	//gl_FragData[0] = gl_Color;

	gl_FragData[0] = vec4(color, 1.0);
	gl_FragData[1] = vec4(0.0, 0.0, 0.0, 1.0);
	//gl_FragColor = vec4(1.0/level * texture2D(color_texture, gl_TexCoord[0].xy).xyz, 1.0);
	gl_FragData[2] = vec4(normal_vs, 0.0);


	// calc frag distance from its branch origin... 
	vec2 fpos = gl_FragCoord.xy;
	fpos = fpos/window_size;
	vec2 b0 = b0_origin * 0.5 + 0.5;
	float dist = length(fpos - b0);

	gl_FragData[3] = vec4(dist, 0.0, 0.0, 1.0);
	//gl_FragColor = gl_Color;
}