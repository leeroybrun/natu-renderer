#version 120


varying vec3			normal_vs;
varying vec3			tangent_vs;
varying vec4			vPos;

varying vec2			b0_origin;
varying vec2			b1_origin;
varying vec2			b2_origin;

uniform sampler2D		color_texture;
uniform vec2			window_size;

void main()
{	

	float h = gl_FrontMaterial.shininess;
	vec3 lightDir = gl_LightSource[0].position.xyz;

	vec3 N = normalize(normal_vs);
	vec3 L = normalize(lightDir);
	vec3 E = normalize(-vPos.xyz);
	vec3 R = reflect(-L, N);
	float RdotE = max(dot(R, E),0.0);
	float NdotL = max(dot(N, L),0.0);
	float spec = pow( RdotE, h );
	vec4 texColor = texture2D(color_texture, gl_TexCoord[0].xy);
	vec4 ambient = gl_LightSource[0].ambient;
	vec4 diffuse = gl_FrontLightProduct[0].diffuse * NdotL;
	vec4 specular = gl_FrontLightProduct[0].specular * spec;
	vec3 color = ((texColor) * (ambient + diffuse) + specular).xyz;


	gl_FragData[0] = vec4(color, 1.0);
	gl_FragData[1] = vec4(normal_vs, 1.0);
	gl_FragData[2] = vec4(b0_origin*0.5+vec2(0.5),b1_origin*0.5+vec2(0.5));

}