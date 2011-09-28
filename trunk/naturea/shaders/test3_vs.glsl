#version 120
#extension GL_EXT_draw_instanced: enable

attribute vec3		normal;
attribute vec3		tangent;
attribute vec2		texCoords0;
attribute vec2		sliceDescription;
varying vec3		eyeDir;
varying vec3		normalDir;
varying vec3		lightDir_ts;
varying vec3		eyeDir_ts;
varying float		alpha;
varying vec2		sliceDesc;
uniform int			instancing;
attribute mat4		transformMatrix;
attribute vec3		colorVariance;
varying vec3		colorVar;
uniform	vec3		u_colorVariance;
varying mat3		TBN_Matrix;

void main()
{
	vec4 pos;
	vec3 tangentDir;
	vec3 bitangent;
	bitangent		= cross(normal, tangent);

	if (instancing>0){
		// drawing instances
		colorVar	= colorVariance;
		pos			= gl_ModelViewMatrix * transformMatrix * ( gl_Vertex * vec4(10.0, 10.0, 10.0, 1.0));	
		mat3 T		= mat3(transformMatrix);
		tangentDir	= (gl_NormalMatrix * T * tangent	);
		normalDir	= (gl_NormalMatrix * T * normal		);
		bitangent	= (gl_NormalMatrix * T * bitangent	);

	} else {
		// drawing single geometry
		colorVar = u_colorVariance;
		pos = gl_ModelViewMatrix * ( gl_Vertex * vec4(10.0, 10.0, 10.0, 1.0));
		tangentDir	= (gl_NormalMatrix * tangent	);
		normalDir	= (gl_NormalMatrix * normal		);		
		bitangent	= (gl_NormalMatrix * bitangent	);
	}

	// calc tangent space...
	// = mat3 (tangent_vs, bitangent, normal_vs);
    TBN_Matrix[0]	=  tangentDir; 
    TBN_Matrix[1]	=  bitangent; 
    TBN_Matrix[2]	=  normalDir; 

	vec3 lpos		= (gl_LightSource[0].position).xyz;
	lightDir_ts		= normalize( lpos ) * TBN_Matrix ;
	eyeDir_ts		= normalize( pos.xyz ) * TBN_Matrix;

	gl_Position		= gl_ProjectionMatrix * pos;
	eyeDir			= pos.xyz;
	sliceDesc		= sliceDescription;
	gl_TexCoord[0]	= vec4(texCoords0, 0.0, 0.0);

	//alpha =clamp(-0.5 + 2.0*abs(dot(normalize(normalDir), normalize(eyeDir))), 0.0, gl_Color.a);
	//alpha = clamp(abs(dot(normalize(normalDir), normalize(eyeDir))), gl_Color.a, 1.0);
	//alpha = gl_Color.a;
	//gl_FrontColor = vec4(normal, alpha);
	gl_FrontColor	= gl_Color;
}

