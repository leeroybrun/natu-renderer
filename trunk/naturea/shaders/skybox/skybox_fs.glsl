#
//==============================================================================
//  God Rays FRAGMENT shader   
//  inspired by: http://fabiensanglard.net/lightScattering/
//==============================================================================
uniform vec4 skycolor;
uniform sampler2D skybox_tex;

void main(void)
{
	vec4 origColor = texture2D(skybox_tex, gl_TexCoord[0].st);
	
	gl_FragData[0] = origColor;
	gl_FragData[1] = skycolor;
}

