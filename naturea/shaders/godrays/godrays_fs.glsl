#
//==============================================================================
//  God Rays FRAGMENT shader   
//  inspired by: http://fabiensanglard.net/lightScattering/
//==============================================================================
uniform sampler2D rtex;
uniform sampler2D otex;

const int NUM_SAMPLES = 32;

uniform vec2 lightPositionOnScreen;
uniform float lightDirDOTviewDir;

void main(void)
{
	vec4 origColor = texture2D(otex, gl_TexCoord[0].st);
	vec4 raysColor = texture2D(rtex, gl_TexCoord[0].st);
	//====================================================================
	// Sun position on screen - check algorithm
	//--------------------------------------------------------------------
	//vec2 l = gl_TexCoord[0].st - lightPositionOnScreen;
	//if (length(l)<0.03){
	//	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	//} else {
	//	gl_FragColor = origColor;
	//}
	//return;
	//--------------------------------------------------------------------
	if (lightDirDOTviewDir>0.0){
		float exposure	= 0.1/NUM_SAMPLES;
		float decay		= 1.0;
		float density	= 0.3;
		float weight	= 6.0;
		float illuminationDecay = 1.0;

		vec2 deltaTextCoord = vec2( gl_TexCoord[0].st - lightPositionOnScreen);
		vec2 textCoo = gl_TexCoord[0].st;
		deltaTextCoord *= 1.0 / float(NUM_SAMPLES) * density;
	


		for(int i=0; i < NUM_SAMPLES ; i++)
		{
			textCoo -= deltaTextCoord;
			vec4 tsample = texture2D(rtex, textCoo );
			tsample *= illuminationDecay * weight;
			raysColor += tsample;
			illuminationDecay *= decay;
		}
		raysColor *= exposure * lightDirDOTviewDir;
		gl_FragColor = origColor + raysColor;
	} else {
		gl_FragColor = origColor;
	}
}

