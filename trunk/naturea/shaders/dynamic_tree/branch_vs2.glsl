#version 120

/*************************************************
* HIERARCHICAL VERTEX DISPLACEMENT
**************************************************/
#define	texCols  18.0
#define WHITE	vec4(1.0, 1.0, 1.0, 1.0)
#define RED		vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN	vec4(0.0, 1.0, 0.0, 1.0)
#define BLUE	vec4(0.0, 0.0, 1.0, 1.0)
#define BLACK	vec4(0.0, 0.0, 0.0, 1.0)
#define ONE2    vec2(1.0,1.0)
#define ONE3    vec3(1.0,1.0,1.0)
#define ONE4    vec4(1.0,1.0,1.0,1.0)
//uniform sampler2D		branch_turbulence_tex;
uniform float			branch_count;
uniform float			time;
uniform sampler2D		data_tex;
uniform sampler2D		branch_noise_tex;
uniform sampler2D		leaf_noise_tex;
uniform vec3			wind_direction;
uniform float			wind_strength;
uniform vec4			wood_amplitudes;
uniform vec4			wood_frequencies;

attribute vec3			normal;
attribute vec3			tangent;
attribute vec4			x_vals;
attribute float			branch_index;
attribute vec2			texCoords0;

float				time_faster;
varying vec3		normal_vs;
varying vec3		tangent_vs;
varying float		level;
vec4 color;	
varying vec4 vPos;
vec3					oVec=vec3(1.0, 1.0, 1.0);

void animateBranchVertex(inout vec3 position)
{
//===========================================================
// Displacement method inspired by Ralf Habels article: 
//  Physically Guided Animation of Trees
//
//===========================================================
	vec3 br, bs, bt;
    //vec3 animated_vertex = position;
	
    float ttime = time;
    float mv_time = time * 0.01;
   
    //function for alpha = 0.1
	//vec4 xvals_f = 0.3326*pow4(xvals,2.0) + 0.398924*pow4(xvals,4.0);
	//vec4 xvals_deriv = 0.665201*xvals + 1.5957*pow4(xvals,3.0);
	
	//function for alpha = 0.2
	vec4 xvals_f = 0.374570*x_vals*x_vals + 0.129428*x_vals*x_vals*x_vals*x_vals;
    vec4 xvals_deriv = 0.749141*x_vals + 0.517713*x_vals*x_vals*x_vals;


	
	
	// get coord systems from branch data texture
	vec4 sv0_l	= texture2D(data_tex, vec2(2.5/texCols, branch_index/branch_count));
    vec4 sv1_l	= texture2D(data_tex, vec2(3.5/texCols, branch_index/branch_count));
    vec4 sv2_l	= texture2D(data_tex, vec2(4.5/texCols, branch_index/branch_count));
    vec4 sv3_l	= texture2D(data_tex, vec2(5.5/texCols, branch_index/branch_count));
    vec3 rv0	= texture2D(data_tex, vec2(6.5/texCols, branch_index/branch_count)).xyz;
    vec3 rv1	= texture2D(data_tex, vec2(7.5/texCols, branch_index/branch_count)).xyz;
    vec3 rv2	= texture2D(data_tex, vec2(8.5/texCols, branch_index/branch_count)).xyz;
    vec3 rv3	= texture2D(data_tex, vec2(9.5/texCols, branch_index/branch_count)).xyz;
    // motion vectors
	vec4 mv01 = texture2D(data_tex, vec2(0.5/texCols, branch_index/branch_count));
    vec4 mv23 = texture2D(data_tex, vec2(1.5/texCols, branch_index/branch_count));
    vec2 mv0 = mv01.xy;
    vec2 mv1 = mv01.zw;
    vec2 mv2 = mv23.xy;
    vec2 mv3 = mv23.zw;
    // branch lengths
	float length0 = sv0_l.w;
    float length1 = sv1_l.w;
    float length2 = sv2_l.w;
    float length3 = sv3_l.w;
    // sv vectors
	vec3 sv0 = sv0_l.xyz;
    vec3 sv1 = sv1_l.xyz;
    vec3 sv2 = sv2_l.xyz;
    vec3 sv3 = sv3_l.xyz;
    // amplitudes
	vec2 amp0 = wood_amplitudes.x * ( texture2D(branch_noise_tex, mv0 * mv_time * wood_frequencies.x).rg  * 2.0 - ONE2);
    vec2 amp1 = wood_amplitudes.y * ( texture2D(branch_noise_tex, mv1 * mv_time * wood_frequencies.y).rg  * 2.0 - ONE2);
    vec2 amp2 = wood_amplitudes.z * ( texture2D(branch_noise_tex, mv2 * mv_time * wood_frequencies.z).rg  * 2.0 - ONE2);
    vec2 amp3 = wood_amplitudes.w * ( texture2D(branch_noise_tex, mv3 * mv_time * wood_frequencies.w).rg  * 2.0 - ONE2);
    // apply animation to the vertex.
	//--------------------------------------------------------------------------------------
	
	vec3 tv;
	vec3 center;
	vec3 centerB = vec3(0.0, 0.0, 0.0);
	vec3 corr_r, corr_s;
	vec2 fu, fu_deriv, s,d;
	bs = sv0;
	br = rv0;
	bt = cross(br, bs);
	if (x_vals.x>0.0){
		level = 0.0;
		// level0
		// find t vector
		tv	= cross(rv0,sv0);
	
		// calc wind prebend offset
		amp0.x += dot(rv0, wind_direction) * wind_strength;
		amp0.y += dot(sv0, wind_direction) * wind_strength;

		// find branch origin
		center = centerB + x_vals.x * length0 * tv;
		// bend function
		fu	= xvals_f.x	* amp0;
		fu_deriv = xvals_deriv.x / length0 * amp0 ;
		s = sqrt(ONE2+fu_deriv*fu_deriv);
		d = fu / fu_deriv * (s - ONE2);
		corr_s = (tv + sv0*fu_deriv.x)/s.x * d.x;
		corr_r = (tv + rv0*fu_deriv.y)/s.y * d.y;
		//recalculate coord system of actual branch 
		bt  = normalize(tv + rv0*fu_deriv.y + sv0*fu_deriv.x);
		br	= normalize(rv0 - tv*fu_deriv.y);
		bs	= normalize(sv0 - tv*fu_deriv.x);
		// bend the center point
		centerB =  center + fu.x * sv0 + fu.y * rv0 - (corr_s+corr_r);
	}
    if (x_vals.y>0.0){
        // level1
		level = 1.0;
	    // bend branch system according to the parent branch bending
		sv1 = sv1.x * bs + sv1.y * br + sv1.z * bt;
        rv1 = rv1.x * bs + rv1.y * br + rv1.z * bt;
        //...
		tv	= cross(rv1,sv1);
		// calc wind prebend offset
		amp1.x += dot(rv1, wind_direction) * wind_strength;
		amp1.y += dot(sv1, wind_direction) * wind_strength;

        center		= centerB + x_vals.y * length1 * tv;
        fu			= xvals_f.y	 * amp1;
        fu_deriv	= xvals_deriv.y / length1 * amp1 ;
        s = sqrt(ONE2+fu_deriv*fu_deriv);
        d = fu / fu_deriv * (s - ONE2);
        corr_s = (tv + sv1*fu_deriv.x)/s.x * d.x;
        corr_r = (tv + rv1*fu_deriv.y)/s.y * d.y;
        bt  = normalize(tv + rv1*fu_deriv.y + sv1*fu_deriv.x);
        br	= normalize(rv1 - tv*fu_deriv.y);
        bs	= normalize(sv1 - tv*fu_deriv.x);
        centerB =  center + fu.x * sv1 + fu.y * rv1 - (corr_s+corr_r);
    }

	if (x_vals.z>0.0){
        // level2
		level = 2.0;
	    // bend branch system according to the parent branch bending
		sv2 = sv2.x * bs + sv2.y * br + sv2.z * bt;
        rv2 = rv2.x * bs + rv2.y * br + rv2.z * bt;
        //...
		tv	= cross(rv2,sv2);
		// calc wind prebend offset
		amp2.x += dot(rv2, wind_direction) * wind_strength;
		amp2.y += dot(sv2, wind_direction) * wind_strength;

        center		= centerB + x_vals.z * length2 * tv;
        fu			= xvals_f.z * amp2;
        fu_deriv	= xvals_deriv.z / length2 * amp2 ;
        s = sqrt(ONE2+fu_deriv*fu_deriv);
        d = fu / fu_deriv * (s - ONE2);
        corr_s = (tv + sv2*fu_deriv.x)/s.x * d.x;
        corr_r = (tv + rv2*fu_deriv.y)/s.y * d.y;
        bt  = normalize(tv + rv2*fu_deriv.y + sv2*fu_deriv.x);
        br	= normalize(rv2 - tv*fu_deriv.y);
        bs	= normalize(sv2 - tv*fu_deriv.x);
        centerB =  center + fu.x * sv2 + fu.y * rv2 - (corr_s+corr_r);
    }

	if (x_vals.w>0.0){
        // level3
		level = 3.0;
		sv3 = sv3.x * bs + sv3.y * br + sv3.z * bt;
        rv3 = rv3.x * bs + rv3.y * br + rv3.z * bt;
        tv	= cross(rv3,sv3);
		// calc wind prebend offset
		amp3.x += dot(rv3, wind_direction) * wind_strength;
		amp3.y += dot(sv3, wind_direction) * wind_strength;

        center		= centerB + x_vals.w * length3 * tv;
        fu			= xvals_f.w	 * amp3;
        fu_deriv	= xvals_deriv.w/ length3 * amp3 ;
        s = sqrt(ONE2+fu_deriv*fu_deriv);
        d = fu / fu_deriv * (s - ONE2);
        corr_s = (tv + sv3*fu_deriv.x)/s.x * d.x;
        corr_r = (tv + rv3*fu_deriv.y)/s.y * d.y;
        bt  = normalize(tv + rv3*fu_deriv.y + sv3*fu_deriv.x);
        br	= normalize(rv3 - tv*fu_deriv.y);
        bs	= normalize(sv3 - tv*fu_deriv.x);
        centerB =  center + fu.x * sv3 + fu.y * rv3 - (corr_s+corr_r);
    }
	


	// repair bt, br, bs;
	 // bt tangent along the branch
	 //bt = cross(br, bs);
	 //bs = cross(bt, br);
	 //br = cross(bs, bt);	  
	 oVec = bt; 

	 //bs = cross (bt, br);
	 //br = cross (bs, bt);

	 // br normal
	 // bs bitangent on the circle around branch
	 //oVec = vec3(abs(dot(br, bs)), abs(dot(bs,bt)), abs(dot(bt,br)));

	position = centerB + position.x*bs + position.y*br;
    //normal_vs	= normalize(normal);
	//tangent_vs	= normalize(tangent);
	tangent_vs	 = tangent.x * bt + tangent.y * bs + tangent.z * br;
	normal_vs	 = normal.x  * bt + normal.y  * bs + normal.z  * br;
	//tangent_vs = normalize(tangent_vs);
	//normal_vs = normalize(normal_vs);
	//vec3 binormal = cross(tangent_vs, normal_vs);
	//tangent_vs = cross(normal_vs, binormal);
		
}

void main()
{
	color		= WHITE;
    vec3 vertex = gl_Vertex.xyz;
    animateBranchVertex(vertex);

	gl_TexCoord[0] = vec4(texCoords0, 0.0, 0.0);
	

	
	normal_vs = gl_NormalMatrix * normal;
	tangent_vs = gl_NormalMatrix * tangent;
	
	vPos = gl_ModelViewMatrix * vec4(vertex,1.0);
	gl_FrontColor = vec4(gl_NormalMatrix * oVec, 1.0);

	//gl_FrontColor = color;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(vertex,1.0);
}

