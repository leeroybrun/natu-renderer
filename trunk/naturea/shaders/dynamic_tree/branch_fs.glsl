#


varying vec3 normal_vs;
varying vec3 tangent_vs;
varying float level;
varying vec4 vPos;
varying vec3 lightDir;


uniform sampler2D color_texture;

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

	//gl_FragColor = gl_Color;
}