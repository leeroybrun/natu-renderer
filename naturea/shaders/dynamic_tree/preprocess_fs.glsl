#version 120

uniform sampler2D branchMap;

//--------------------------------------------------------------------------------------
// Defines
//--------------------------------------------------------------------------------------
#define EPSILON			5

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
	for (i=-EPSILON; i<EPSILON; i++){

		for (j=-EPSILON; j<EPSILON; j++){
			// find the closest filled point
			lookUpPos =  gl_TexCoord[0].xy + (vec2(i,j)/EPSILON);
			data = texture2D(branchMap, lookUpPos);

			bestData = bestData + data;
			/*
			if (data.a>0.5){
				// some valid data

				// calc distance
				dist = length(gl_TexCoord[0].xy-lookUpPos);

				if (dist<minDist){
					minDist = dist;
					bestData = data;
				}
			}
			*/
		}
	}
	
	vec4 color = bestData*0.1;
	color.a = 1.0;

	gl_FragData[0] = color;
}
