#
//==============================================================================
//  God Rays FRAGMENT shader   
//  inspired by: http://fabiensanglard.net/lightScattering/
//==============================================================================
uniform sampler2D rtex;
uniform sampler2D otex;
//uniform sampler2D btex;

uniform vec3		tintColor;
uniform float		tintFactor;

const int			NUM_SAMPLES = 30;
const int			BLOOM_SAMPLES = 4;
const float			bloomDistance = 0.05/BLOOM_SAMPLES;
uniform vec2		lightPositionOnScreen;
uniform float		lightDirDOTviewDir;

void main(void)
{

	vec4  origColor = texture2D(otex, gl_TexCoord[0].st);
	origColor.rgb *= (tintColor*tintFactor);
	origColor.rgb = pow(origColor.rgb,1/1.2);

	
	//origColor = vec4(0.1)+origColor;
	vec4  raysColor = texture2D(rtex, gl_TexCoord[0].st);
	//float bcolor   = texture2D(btex,  gl_TexCoord[0].st).x;
	
	vec4 bloom = vec4(0.0);
	float xd,yd,l;
	float divideFactor = 0.8 * BLOOM_SAMPLES*BLOOM_SAMPLES;
	//if (length(raysColor.xyz)<0.4){
		for (int x=-BLOOM_SAMPLES; x<BLOOM_SAMPLES; x++){
			for (int y=-BLOOM_SAMPLES; y<BLOOM_SAMPLES; y++){
				xd = float(x)/float(BLOOM_SAMPLES);
				yd = float(y)/float(BLOOM_SAMPLES);
				l = sqrt(xd*xd+yd*yd);
				vec4 c = texture2D(rtex, gl_TexCoord[0].st + vec2(xd,yd)*l*bloomDistance);
				if (length(c.rgb)>0.1){
					bloom += c;
					divideFactor+= l;
				}
			}
		}
		origColor += (bloom/divideFactor);
		
	//}
	
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
	
	/*/
	gl_FragColor = origColor;
	/*/
}

