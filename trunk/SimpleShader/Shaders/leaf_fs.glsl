#
uniform sampler2D	col1_texture;
void main()
{	
	vec4 color = texture2D(col1_texture, gl_TexCoord[0].st);
	if (color.a<=0.1){
		discard;
	}
	//gl_FragColor = gl_Color;
	gl_FragColor = color;

}