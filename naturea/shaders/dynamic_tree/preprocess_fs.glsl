#version 120

uniform sampler2D branchMap;

uniform vec2 window_size;

//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------
#define EPSILON1		20	
#define EPSILON2		100
//--------------------------------------------------------------------------------------
// Fragment Shader
// PREPROCESS
//--------------------------------------------------------------------------------------

void main()
{	
	vec4 data;
	float minDist = 2000.0;
	float dist;
	vec4 bestData = vec4(0.0);
	vec2 lookUpPos;
	int i = 0;
	int j = 0;
	
	for (i=-EPSILON1; i<10; i++){

		for (j=-EPSILON2; j<EPSILON2; j++){
			// find the closest filled point
			float x = j / window_size.x;
			float y = i / window_size.y;
			lookUpPos =  gl_TexCoord[0].xy + (vec2(x,y));
			data = texture2D(branchMap, lookUpPos);

			//bestData = bestData + data;
			
			if (data.a>0.5){
				// some valid data

				// calc distance
				dist = length(gl_TexCoord[0].xy-lookUpPos);

				if (dist<minDist){
					minDist = dist;
					bestData = data;
				}
			}
			
		}
	}
	
	vec4 color = bestData;
	color.a = 1.0;

	gl_FragData[0] = color;
}
