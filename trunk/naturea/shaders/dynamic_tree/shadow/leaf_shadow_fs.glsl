#version 120


uniform sampler2D frontDecalMap;
uniform sampler2D seasonMap;
uniform	float season;

varying float			time_offset_v;
varying float			leafSpecificNumber;

void main()
{	
	vec4 decal_color = texture2D(frontDecalMap, gl_TexCoord[0].xy );
	vec2 seasonCoord = vec2(0.5, season + 0.2*leafSpecificNumber - 0.0001*time_offset_v);
	
	vec4 seasonColor =  texture2D(seasonMap, seasonCoord);
	
	decal_color.a *= seasonColor.a;

	if (decal_color.a < 0.5)
	{
		discard;
	}
	gl_FragColor = decal_color;
}
