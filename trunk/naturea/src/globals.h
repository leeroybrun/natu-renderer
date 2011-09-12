#ifndef _GLOBALS_H
#define _GLOBALS_H
#include "GLEW/glew.h"
#include "GL/glut.h"
#include "timer.h"

#include "Vector3.h"
#include "Vector4.h"
#include "Vector2.h"
#include "Matrix4x4.h"

enum CameraMode{
	FREE,
	TERRAIN_RESTRICTED,
	TERRAIN_CONNECTED,
	WALK
};

struct Statistics{
	float	fps;
	int		primitives;
	int		house1_lod;
	int		house1_samples;
	int		house2_lod;
	int		house2_samples;
	int		bridge_lod;
	int		bridge_samples;
	int		tower1_lod;
	int		tower1_samples;
	int		tower2_lod;
	int		tower2_samples;
	int		eggbox_lod;
	int		eggbox_samples;
	int		haywagon_lod;
	int		haywagon_samples;
	int		well_lod;
	int		well_samples;
	//int		house_lod;
	//int		house_samples;


};
static int g_nula = 0;

extern	GLint					g_WinWidth;   // Window width
extern	GLint					g_WinHeight;   // Window height
extern	v3						g_window_sizes;	
extern	double					g_time;
extern	float					g_float_time;
extern	bool					g_godraysEnabled;
extern	bool					g_fastMode;
extern	v4						g_light_position;
extern	v4						g_light_direction;
extern	bool					g_drawingReflection;
extern	bool					g_showTextures;
extern  bool					g_ParallaxMappingEnabled;
extern  float					g_ParallaxScale;
extern  float					g_ParallaxBias;

extern	bool					g_draw_dtree_lod		;
extern	bool					g_draw_low_vegetation	;
extern	bool					g_draw_dtree			;
extern	bool					g_draw_light_direction  ;

extern	bool					g_ShadowMappingEnabled;
extern	m4						g_LightMVPCameraVInverseMatrix;
extern	m4						g_LightMVCameraVInverseMatrix;
extern	m4						g_LightPMatrix;

extern	int						g_multisample_count;
extern	GLuint					g_screen_multi_framebuffer;

extern	CameraMode				g_cameraMode;

extern	Statistics				g_Statistics;
extern	v4						g_terrain_border_values;
extern	v4						g_terrain_border_widths;

extern int						g_GrassCount;
extern int						g_Tree1Count;
extern int						g_Tree2Count;

extern int g_Bumpmaps	   ;
extern int g_Heightmaps	   ;
extern int g_Specularmaps  ;
extern int g_Alphamaps	   ;


/**********************************************
* DYNAMIC TREE
*/
extern v3		g_tree_wind_direction;
extern float	g_tree_wind_strength;
extern v4		g_tree_wood_amplitudes;
extern v4		g_tree_wood_frequencies;
extern float	g_tree_leaf_amplitude;
extern float	g_tree_leaf_frequency;

/**********************************************
* LOD TREE
*/
extern int		g_tree_slice_count;

extern float	g_tree_wave_amplitude;
extern float	g_tree_wave_frequency;
extern v3		g_tree_movementVectorA;
extern v3		g_tree_movementVectorB;
extern float	g_tree_wave_y_offset;
extern float	g_tree_wave_increase_factor;
extern float	g_tree_time_offset_1;
extern float	g_tree_time_offset_2;

extern float	g_leaves_MultiplyAmbient				;
extern float	g_leaves_MultiplyDiffuse				;
extern float	g_leaves_MultiplySpecular				;
extern float	g_leaves_MultiplyTranslucency			;
extern float	g_leaves_ReduceTranslucencyInShadow		;
extern float	g_leaves_shadow_intensity				;
extern v3		g_leaves_LightDiffuseColor				;

extern v3		g_snapshot_direction;
extern int		g_slice_count;

extern int		g_tree_gridSize			;
extern float	g_tree_mean_distance	;
extern float	g_tree_dither			;

extern int		g_tree_lod0_count;
extern int		g_tree_lod1_count;
extern int		g_tree_lod2_count;

#endif