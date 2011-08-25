#
//==============================================================================
//  Grass VERTEX shader   
//
//==============================================================================
#define TIME_REDUCTION 0.01
varying vec3	eye;
varying vec3	normal;
uniform float	time;
uniform sampler2D grass_wave_texture;

// shadow mapping
uniform mat4	LightMVPCameraVInverseMatrix;
varying	vec4	lightSpacePosition;
//uniform vec2	wind_direction;

void main()
{
	vec2 wind_direction = vec2(0.2, 0.3);
	normal = gl_NormalMatrix * gl_Normal;	
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_FrontColor = gl_Color;
	
	vec4 position = gl_ModelViewMatrix * gl_Vertex;
	vec4 vertex = gl_Vertex;
	eye = vec3(position);
	vec2 dudv = vec2(0.0, 0.0);
	//float h = mod(gl_MultiTexCoord0.t*3.0,3.0);

	if (gl_MultiTexCoord0.t>0.0){
		// shift upper vertices...
		float posWeight = 0.1;
		dudv = (texture2D(grass_wave_texture, wind_direction*time*TIME_REDUCTION+posWeight*vertex.xz).xy - vec2(0.5))*2.0;
		//dudv = vec2(wind_direction.x*sin(time+posWeight*position.x), wind_direction.y*sin(time+posWeight*position.y));

	}
	//col = vec4 (dudv*5.0, 1.0, 1.0);
	position.x = dudv.x + position.x;
	position.z = dudv.y + position.z;
	gl_Position = gl_ProjectionMatrix * (position);//gl_ModelViewMatrix * gl_Vertex;
	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	vec2 d = dudv*5.0;
	
	lightSpacePosition = LightMVPCameraVInverseMatrix*(position);
	//gl_FrontColor = vec4(d*0.5+0.5, 0.0, 1.0);
	
}