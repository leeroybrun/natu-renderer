//-----------------------------------------------------------------------------
//  NATUR(E)AL
//  28/09/2011
//-----------------------------------------------------------------------------
//  Controls: 
//    [mouse]		 ... look direction / select&adjust controls
//    [w]            ... move forward
//    [s]            ... move backward
//    [a]            ... move left
//    [d]            ... move right
//    [q]            ... move up
//    [e]            ... move down
//    [x], [X]       ... decrease/increase the speed of movement
//    [spacebar]     ... switch between look-by-mouse / select&control-by-mouse 
//  
//-----------------------------------------------------------------------------
#define USE_ANTTWEAKBAR
#define TEST 0
#include "../common/Vector4.h"
#include "settings.h"


int		g_samples = 4;
bool	g_compressToOneTexture = true;

bool	g_ParallaxMappingEnabled = true;
float	g_ParallaxScale =   0.04;
float	g_ParallaxBias  =  -0.02;
int		g_offset = 0;
float	g_cosA	 = 0.0;
float	g_cosB	 = 0.0;
float	g_cosC	 = 0.0;

#define TERRAIN_INIT_BORDER_VAL v4(13.0f, 10.0f, 5.0f, -1.0f)
#define TERRAIN_INIT_BORDER_WID v4(2.0f, 2.0f, 2.0f, 2.0f)

v4	g_terrain_border_values = TERRAIN_INIT_BORDER_VAL;
v4	g_terrain_border_widths = TERRAIN_INIT_BORDER_WID;

float tree2min = TREE2_MIN_HEIGHT;
float tree2max = TREE2_MAX_HEIGHT;
float tree1min = TREE1_MIN_HEIGHT;
float tree1max = TREE1_MAX_HEIGHT;
float grassmin = GRASS_MIN_HEIGHT;
float grassmax = GRASS_MAX_HEIGHT;

#include <assert.h>
#include "../common/models/cube.h"
#include "World.h"
#include "globals.h"

CameraMode g_cameraMode		= TERRAIN_RESTRICTED;
int g_WinWidth				= 1280;//800	;   // Window width
int g_WinHeight				= 720; //600;   // Window height
v3  g_window_sizes			= v3(g_WinWidth, g_WinHeight, 0.0);
double g_time				= 0.0;
float g_float_time			= 0.0f;
CTimer						timer;
Statistics					g_Statistics;
int g_Bumpmaps				= 0;
int g_Heightmaps			= 0;
int g_Specularmaps			= 0;
int g_Alphamaps				= 0;

bool tqAvailable			= false;
GLuint tqid					= 0;
bool pqAvailable			= false;
GLuint pqid					= 0;

GLint result_available		= 0;

float	g_bloomDivide		= 0.8;
float	g_god_expo			= 0.06;
float	g_god_decay			= 1.0;
float	g_god_density		= 0.33;
float	g_god_weight		= 6.0;
float	g_illuminationDecay	= 3.27;


v4 g_light_position			= LIGHT_POSITION;
v4 g_light_direction		= LIGHT_DIRECTION;
bool g_light_showDir		= false;
bool g_godraysEnabled		= false;
bool g_fastMode				= false;
bool g_drawingReflection	= false;
bool g_showTextures			= false;
bool g_ShadowMappingEnabled	= false;
bool g_Draw2Shadowmap		= false;
m4 g_LightMVCameraVInverseMatrix;
m4 g_LightPMatrix;
Light* g_shadowLight;
int		g_multisample_count			= 0;
GLuint	g_screen_multi_framebuffer	= 0;

v3 g_snapshot_direction		= v3(0.0, 0.0, -1.0);
int g_slice_count			= 3;

int	g_GrassCount			= GRASS_COUNT;
int	g_Tree1Count			= TREE1_COUNT;
int	g_Tree2Count			= TREE2_COUNT;

// GLOBAL CONSTANTS____________________________________________________________
const GLfloat VECTOR_RENDER_SCALE = 0.20f;
// GLOBAL VARIABLES____________________________________________________________


// Scene orientation (stored as a quaternion)
GLfloat  g_SceneRot[]           = {0.0f, 0.0f, 0.0f, 1.0f};   
GLfloat  g_SceneTraZ            = 10.0f; // Scene translation along z-axis
GLfloat  g_SceneScale           = 1.0f;

GLuint cube_vbo_id				= 0;
GLuint cube_ebo_id				= 0;
GLuint plane_vbo_id				= 0;
GLuint plane_ebo_id				= 0;


bool     g_ShowVertexNormals    = false; // Show vertex normal/tangent/binormal
bool     g_FaceNormals          = false; // Show face normal
bool     g_Transparency         = false; // Draw transparent meshes
bool     g_WireMode             = false; // Wire mode enabled/disabled
bool     g_FaceCulling          = true; // Face culling enabled/disabled
GLfloat  g_AlphaThreshold       = 0.01f; // Alpha test threshold
bool	 g_MouseModeANT			= true;

bool	g_draw_dtree_lod		= true;
bool	g_draw_lod0				= true;
bool	g_draw_lod1				= true;
bool	g_draw_lod2				= true;
//v4		g_lodTresholds			= v4(15, 20, 50, 60);
v4		g_lodTresholds			= v4(15, 20, 1000, 1000);
bool	g_draw_low_vegetation	= true;
bool	g_draw_dtree			= true;
bool	g_draw_light_direction	= false;



/**************************
* DYNAMIC TREE
*/
v3		g_tree_wind_direction	= v3(1.0, 0.0, 0.0);
float	g_tree_wind_strength	= 0.0;
v4		g_tree_wood_amplitudes	= v4(1.0, 0.8, 0.2, 0.1);
v4		g_tree_wood_frequencies	= v4(0.0, 0.0, 0.0, 0.0);//v4(0.4, 1.2, 0.0, 0.0);
float	g_tree_leaf_amplitude	= 2.4;
float	g_tree_leaf_frequency	= 5.0;
int		g_tree_slice_count		= 3;
int		g_tree_show_slice		= 1;
int		g_tree_show_sliceSet	= 0;
float	g_tree_wave_amplitude	= 0.005;
float	g_tree_wave_frequency	= 0.5;
v3		g_tree_movementVectorA	= v3(0.0, 1.0, 0.0);
v3		g_tree_movementVectorB  = v3(1.0, 0.0, 0.0);
float	g_tree_wave_y_offset	= 0.0;
float	g_tree_wave_increase_factor = 1.0;
float	g_tree_time_offset_1	= 0.0;
float	g_tree_time_offset_2	= 0.5;		

const int	g_tree_gridSize			= 1;			// = SQRT(count of the trees)
float		g_tree_mean_distance	= 2.0;			// = how dense is the grid
float		g_tree_dither			= 1.0;			// = how far can be the tree placed from its' position in grid

float		g_dither				= 2.0;

int		g_treesamples = 0;

float	g_leaves_MultiplyAmbient			= 1.0;
float	g_leaves_MultiplyDiffuse			= 0.7;
float	g_leaves_MultiplySpecular			= 0.3;
float	g_leaves_MultiplyTranslucency		= 0.6;
float	g_leaves_ReduceTranslucencyInShadow	= 0.9;
float	g_leaves_shadow_intensity			= 1.0;
v3		g_leaves_LightDiffuseColor			= v3(0.2, 0.2, 0.2);


int		g_tree_lod0_count					 = 0;
int		g_tree_lod1_count					 = 0;
int		g_tree_lod2_count					 = 0;
int		g_tree_lod01_count					 = 0;
int		g_tree_lod12_count					 = 0;


bool	g_draw_lod1_method		= true;
bool	g_orbit					= false;
float	g_orbit_speed			= 0.1;
float	g_orbit_radius			= 10.0;
v3		g_center				= v3(0.0, 5.0, 0.0);
float	g_timeDiff				= 0;


v3*		g_viewer_position;
v3*		g_viewer_direction;

float	g_fog_density = 0.006, g_fog_start = 1.0, g_fog_end=100.0;

v3		g_tintColor = v3(1.0, 1.0, 1.0);
float	g_tintFactor= 1.0;
float	g_varA		= 1.0;

float	g_season = 0.58;

bool	g_debug = false;
float	g_CPU_fps;
float	CPU_render_time;

LODTransitionMethod g_lodTransition = LODTransitionMethod::SHIFTED_SOFT_FADE;
float   g_transitionShift		= 0.35;
float   g_transitionControl		= 0.0;

float	g_gauss_shift			= 0.5;
float	g_gauss_steep			= 0.1;
float	g_gauss_weight			= 0.05;


World* p_world;
Texture *	g_shadowmap1		= NULL;
m4		*	g_LightMVPmatrix	= NULL;
m4		*	g_LightMVPCameraVInverseMatrix = NULL;

float		g_ShadowNear= 0.0;
float		g_ShadowFar = 50.0;
float		g_CameraNear= 0.0;
float		g_CameraFar = 50.0;

#include "../common/common.h"
// Model file name
std::string g_ModelFileName     = "models/DesertOasis/H1F.pgr2";


// FORWARD DECLARATIONS________________________________________________________
void initGUI(void);
void TW_CALL loadNewModelCB(void* clientData);
void TW_CALL copyStdStringToClient(std::string& dst, const std::string& src);

//-----------------------------------------------------------------------------
// Name: cbDisplay()
// Desc: 
//-----------------------------------------------------------------------------
void cbDisplay()
{
	g_tree_lod0_count = 0;
	g_tree_lod1_count = 0;
	g_tree_lod2_count = 0;



	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	// Setup OpenGL states according to user settings
	glAlphaFunc(GL_GEQUAL, g_AlphaThreshold);

	glPolygonMode(GL_FRONT_AND_BACK, g_WireMode ? GL_LINE : GL_FILL);
	//if (g_FaceCulling) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);

	// Setup camera
	//glLoadIdentity();
	//glTranslatef(0.5f, -1.0f, -g_SceneTraZ);
	//pgr2AddQuaternionRotationToTransformation(g_SceneRot);
	//glScalef(g_SceneScale, g_SceneScale, g_SceneScale);


	g_time=timer.RealTime();
	g_float_time = g_time;
	p_world->update(g_time);
	/*
	if (pqAvailable){
	glBeginQuery(GL_PRIMITIVES_GENERATED, pqid);
	}
	*/

	// if timer query available
	if (tqAvailable){
		// measure on GPU
		//if (result_available){
		glBeginQuery(GL_TIME_ELAPSED, tqid);
		//}

		/*
		GLuint64EXT time = 0;
		glGetQueryObjectui64vEXT(tqid, GL_QUERY_RESULT, &time); // blocking CPU
		g_Statistics.fps = 1000000000.0/ double(time);*/
		//printf("FPS: %f\n",g_Statistics.fps);
		//glGetQueryObjectiv(tqid, GL_QUERY_RESULT_AVAILABLE, &result_available);
		//printf("avail: %s\n",result_available?"yes":"no");
	} 
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	p_world->draw();
	//glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glDisable(GL_MULTISAMPLE);
	// block CPU to measure time here
	//glFinish();
	

	if (tqAvailable){
		//if (result_available)
		glEndQuery(GL_TIME_ELAPSED);

		GLuint64EXT time = 0;
		glGetQueryObjectui64vEXT(tqid, GL_QUERY_RESULT, &time); // blocking CPU
		g_Statistics.fps = 1000000000.0/ double(time);
	}
	g_timeDiff = timer.RealTime() - g_time;
	g_CPU_fps = 1.0 / (g_timeDiff);
	/*
	if (pqAvailable){
	glEndQuery(GL_PRIMITIVES_GENERATED);
	GLuint64EXT count = 0;
	glGetQueryObjectui64vEXT(pqid, GL_QUERY_RESULT, &count); // blocking CPU
	g_Statistics.primitives = count;
	}
	*/
}

void initApp()
{
#if TEST
	// TEST START

	// do whatever u want... 

	system("PAUSE");

	exit(1);
	// TEST END
#endif
	timer.Reset();
	timer.Start();
	// set cube vbo
	initCube();
	//set plane vbo
	initPlane();
	p_world = new World();
	p_world->init();

	// timer query extension?
	if (isExtensionSupported(TIME_QUERY_EXTENSION)){
		tqAvailable = true;
		glGenQueries(1, &tqid);
	}

	pqAvailable = true;
	glGenQueries(1, &pqid);


}
void deinitApp()
{
	SAFE_DELETE_PTR (p_world);
	deletePlane();
	deleteCube();

	timer.Stop();

	//world.~World();
	if (tqAvailable){
		glDeleteQueries(1, &tqid);
	}
	if (pqAvailable){
		glDeleteQueries(1, &pqid);
	}
	printf("deinit done, bye\n");
	//system("PAUSE");
}

//-----------------------------------------------------------------------------
// Name: cbInitGL()
// Desc: 
//-----------------------------------------------------------------------------
void cbInitGL()
{
	// Init app GUI
	initGUI();

	// Set OpenGL state variables
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMaterialfv(GL_FRONT_AND_BACK,  GL_AMBIENT, material_amd);
	glMaterialfv(GL_FRONT_AND_BACK,  GL_DIFFUSE, material_amd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material_spe);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glPointSize(1.f);
	glLineWidth(1.0f);


	glGetIntegerv(GL_MAX_SAMPLES, &g_multisample_count);


	// init app
	initApp();

	// Load model
	//loadNewModelCB(&g_ModelFileName);
}

void cbDeinitGL()
{
	deinitApp();
	printf("deinit GL\n");
}

void TW_CALL cbMakeSlices(void* clientData)
{
	p_world->snapTree(g_snapshot_direction);
} 

void TW_CALL cbSetTree2Count(const void *value, void *clientData)
{ 
	g_Tree2Count = *(const int*)value; // for instance
	p_world->tree2_planter.plantVegetationCount(g_Tree2Count);
}
void TW_CALL cbGetTree2Count(void *value, void *clientData)
{ 
	*(int *)value = p_world->tree2_planter.count; // for instance
}

void TW_CALL cbSetTree1Count(const void *value, void *clientData)
{ 
	g_Tree1Count = *(const int*)value; // for instance
	p_world->tree1_planter.plantVegetationCount(g_Tree1Count);
}
void TW_CALL cbGetTree1Count(void *value, void *clientData)
{ 
	*(int *)value = p_world->tree1_planter.count; // for instance
}

void TW_CALL cbSetGrassCount(const void *value, void *clientData)
{ 
	g_GrassCount = *(const int*)value;  // for instance
	p_world->grass_planter.plantVegetationCount(g_GrassCount);
}
void TW_CALL cbGetGrassCount(void *value, void *clientData)
{ 
	*(int *)value = p_world->grass_planter.count;  // for instance
}

//tree2
void TW_CALL cbSetTree2Min(const void *value, void *clientData)
{ 
	tree2min = *(const float*)value;  // for instance
	p_world->tree2_planter.setNewMin(tree2min);
}
void TW_CALL cbGetTree2Min(void *value, void *clientData)
{ 
	*(float *)value = p_world->tree2_planter.height_min;  // for instance
}

void TW_CALL cbSetTree2Max(const void *value, void *clientData)
{ 
	tree2max = *(const float*)value;  // for instance
	p_world->tree2_planter.setNewMax(tree2max);
}
void TW_CALL cbGetTree2Max(void *value, void *clientData)
{ 
	*(float *)value = p_world->tree2_planter.height_max;  // for instance
}

// tree1
void TW_CALL cbSetTree1Min(const void *value, void *clientData)
{ 
	tree1min = *(const float*)value;  // for instance
	p_world->tree1_planter.setNewMin(tree1min);
}
void TW_CALL cbGetTree1Min(void *value, void *clientData)
{ 
	*(float *)value = p_world->tree1_planter.height_min;  // for instance
}

void TW_CALL cbSetTree1Max(const void *value, void *clientData)
{ 
	tree1max = *(const float*)value;  // for instance
	p_world->tree1_planter.setNewMax(tree1max);
}
void TW_CALL cbGetTree1Max(void *value, void *clientData)
{ 
	*(float *)value = p_world->tree1_planter.height_max;  // for instance
}


// grass
void TW_CALL cbSetGrassMin(const void *value, void *clientData)
{ 
	grassmin = *(const float*)value;  // for instance
	p_world->grass_planter.setNewMin(grassmin);
}
void TW_CALL cbGetGrassMin(void *value, void *clientData)
{ 
	*(float *)value = p_world->grass_planter.height_min;  // for instance
}

void TW_CALL cbSetGrassMax(const void *value, void *clientData)
{ 
	grassmax = *(const float*)value;  // for instance
	p_world->grass_planter.setNewMax(grassmax);
}
void TW_CALL cbGetGrassMax(void *value, void *clientData)
{ 
	*(float *)value = p_world->grass_planter.height_max;  // for instance
}

//-----------------------------------------------------------------------------
// Name: initGUI()
// Desc: 
//-----------------------------------------------------------------------------
void initGUI()
{
#ifdef USE_ANTTWEAKBAR
	// Initialize AntTweakBar GUI
	if (!TwInit(TW_OPENGL, NULL))
	{
		assert(0);
	}

	// Define the required callback function to copy a std::string 
	// (see TwCopyStdStringToClientFunc documentation)
	TwCopyStdStringToClientFunc(copyStdStringToClient); 

	TwWindowSize(g_WinWidth, g_WinHeight);
	TwBar *controlBar = TwNewBar("Controls");
	TwDefine(" Controls position='0 0' size='250 550' refresh=0.3 \
			 valueswidth=80 ");
	//TwAddVarRW(controlBar, "tintColor",	 TW_TYPE_COLOR3F, & g_tintColor.data, " label='tintColor' ");
	//TwAddVarRW(controlBar, "tintFactor", TW_TYPE_FLOAT, & g_tintFactor, " label='tintFactor' min=0 max=2 step=0.01 ");


	//TwAddVarRW(controlBar, "offset", TW_TYPE_INT32, & g_offset, " label='offset' min=0 max=2 step=1 ");
	//TwAddVarRW(controlBar, "varA", TW_TYPE_FLOAT, & g_varA, " label='varA' min=-2.0 max=2.0 step=0.001");
	//TwAddVarRO(controlBar, "cosB", TW_TYPE_FLOAT, & g_cosB, " label='cosB' ");
	//TwAddVarRO(controlBar, "cosC", TW_TYPE_FLOAT, & g_cosC, " label='cosC' ");

	//TwAddVarRW(controlBar, "fogDensity"	, TW_TYPE_FLOAT, & g_fog_density,	" label='fog density' min=-1000 max=1000 step=0.1");
	//TwAddVarRW(controlBar, "fogStart"	, TW_TYPE_FLOAT, & g_fog_start,		" label='fog start'	  min=-1000 max=1000 step=0.1");
	//TwAddVarRW(controlBar, "fogEnd"		, TW_TYPE_FLOAT, & g_fog_end,		" label='fog end'	  min=-1000 max=1000 step=0.1");

	// camera mode
	TwEnumVal trans_mode[] = 
	{ 
		{ LODTransitionMethod::HARD_SWITCH				, "Hard switch"					},
		{ LODTransitionMethod::CROSS_FADE				, "Cross fade"					},
		{ LODTransitionMethod::FADE_IN_BACKGROUND		, "Fade in background"			},
		{ LODTransitionMethod::SHIFTED_CROSS_FADE		, "Shifted cross fade"			},
		{ LODTransitionMethod::SHIFTED_SOFT_FADE		, "shifted soft fade"			}
	};
	TwType transition_method = TwDefineEnum("LOD transition method", trans_mode, 5);
	TwAddVarRW(controlBar, "transition method", transition_method, &g_lodTransition, " group='Visibility' ");
	TwAddVarRW(controlBar, "shift", TW_TYPE_FLOAT, & g_transitionShift, " group='Visibility' min=0 max=0.5 step=0.01");
	TwAddVarRW(controlBar, "gauss_shift",  TW_TYPE_FLOAT, & g_gauss_shift	, " group='Visibility' min=0 max=1.0 step=0.01");
	TwAddVarRW(controlBar, "gauss_steep",  TW_TYPE_FLOAT, & g_gauss_steep	, " group='Visibility' min=0 max=1.5 step=0.01");
	TwAddVarRW(controlBar, "gauss_weight", TW_TYPE_FLOAT, & g_gauss_weight, " group='Visibility' min=0 max=1.0 step=0.01");



	TwAddVarRW(controlBar, "sw_tree", TW_TYPE_BOOLCPP, & g_draw_dtree, " group='Visibility' ");
	TwAddVarRW(controlBar, "sw_lod",  TW_TYPE_BOOLCPP, & g_draw_dtree_lod, " group='Visibility'  ");
	TwAddVarRW(controlBar, "sw_lod0", TW_TYPE_BOOLCPP, & g_draw_lod0, " group='Visibility'  ");
	TwAddVarRW(controlBar, "sw_lod1", TW_TYPE_BOOLCPP, & g_draw_lod1, " group='Visibility'  ");
	TwAddVarRW(controlBar, "sw_lod2", TW_TYPE_BOOLCPP, & g_draw_lod2, " group='Visibility'  ");
	//g_lodTresholds
	TwAddVarRW(controlBar, "sw_lod_tresh0", TW_TYPE_FLOAT, & g_lodTresholds.x, " group='Visibility' label='LODtreshold0'  min=0 max=10000 step=0.1");
	TwAddVarRW(controlBar, "sw_lod_tresh1", TW_TYPE_FLOAT, & g_lodTresholds.y, " group='Visibility' label='LODtreshold1'  min=0 max=10000 step=0.1");
	TwAddVarRW(controlBar, "sw_lod_tresh2", TW_TYPE_FLOAT, & g_lodTresholds.z, " group='Visibility' label='LODtreshold2'  min=0 max=10000 step=0.1");
	TwAddVarRW(controlBar, "sw_lod_tresh3", TW_TYPE_FLOAT, & g_lodTresholds.w, " group='Visibility' label='LODtreshold3'  min=0 max=10000 step=0.1");


	TwAddVarRW(controlBar, "sw_veg", TW_TYPE_BOOLCPP, & g_draw_low_vegetation, " group='Visibility'  ");
	TwAddVarRW(controlBar, "sw_light", TW_TYPE_BOOLCPP, & g_draw_light_direction, " group='Visibility'  ");

	// GENERAL
	TwAddVarRW(controlBar, "season", TW_TYPE_FLOAT, & g_season, " group='General' min=-1 max=2 step=0.01 label='season'");
	TwAddVarRW(controlBar, "sw_orbit", TW_TYPE_BOOLCPP, & g_orbit, " group='General' label='Orbit(ON/OFF)'");
	TwAddVarRW(controlBar, "sw_orbit_speed", TW_TYPE_FLOAT, & g_orbit_speed, " group='General' min=-100 max=100 step=0.01 label='orbit speed'");
	TwAddVarRW(controlBar, "sw_orbit_radius", TW_TYPE_FLOAT, & g_orbit_radius, " group='General' min=-100 max=100 step=0.1 label='orbit radius'");
	TwAddVarRW(controlBar, "sw_lod1_ab", TW_TYPE_BOOLCPP, & g_draw_lod1_method, " group='General'  ");

	TwAddVarRW(controlBar, "snapshotDir", TW_TYPE_DIR3F, &g_snapshot_direction, 
		"group='LOD'  label='slices direction' help='direction of snapshot' ");
	TwAddVarRW(controlBar, "snapshotSlices", TW_TYPE_INT32, &g_slice_count, 
		"group='LOD' label='count of slices'  help='count of slices to generate' ");
	TwAddButton(controlBar, "make_slices", cbMakeSlices, NULL, " group='Tree' label='make slices' ");
	TwAddVarRW(controlBar, "wave_amplitude", TW_TYPE_FLOAT, &g_tree_wave_amplitude, "group='LOD' label='distortion amplitude' min=0 max=5 step=0.001 ");
	TwAddVarRW(controlBar, "wave_frequency", TW_TYPE_FLOAT, &g_tree_wave_frequency, "group='LOD' label='distortion frequency' min=0 max=5 step=0.001");
	TwAddVarRW(controlBar, "wave_y_offset", TW_TYPE_FLOAT, &g_tree_wave_y_offset, "group='LOD' label='distortion y offset' min=0 max=10 step=0.001");
	TwAddVarRW(controlBar, "wave_increase_factor", TW_TYPE_FLOAT, &g_tree_wave_increase_factor, "group='LOD' label='distortion increase' min=0 max=5 step=0.001");
	TwAddVarRW(controlBar, "time_offset1", TW_TYPE_FLOAT, &g_tree_time_offset_1, "group='LOD' label='distortion time offset 1' min=-10 max=10 step=0.001");
	TwAddVarRW(controlBar, "time_offset2", TW_TYPE_FLOAT, &g_tree_time_offset_2, "group='LOD' label='distortion time offset 2' min=-10 max=10 step=0.001");

	TwAddVarRW(controlBar, "MultiplyAmbient",			TW_TYPE_FLOAT,		&g_leaves_MultiplyAmbient			, "group='Leaves' label='k ambient' min=0 max=5 step=0.001 ");
	TwAddVarRW(controlBar, "MultiplyDiffuse",			TW_TYPE_FLOAT,		&g_leaves_MultiplyDiffuse			, "group='Leaves' label='k diffuse' min=0 max=5 step=0.001");
	TwAddVarRW(controlBar, "MultiplySpecular",			TW_TYPE_FLOAT,		&g_leaves_MultiplySpecular			, "group='Leaves' label='k specular' min=0 max=10 step=0.001");
	TwAddVarRW(controlBar, "MultiplyTranslucency",		TW_TYPE_FLOAT,		&g_leaves_MultiplyTranslucency		, "group='Leaves' label='k translucency' min=0 max=5 step=0.001");
	TwAddVarRW(controlBar, "ReduceTranslucencyInShadow", TW_TYPE_FLOAT,		&g_leaves_ReduceTranslucencyInShadow, "group='Leaves' label='transl in shadow' min=-10 max=10 step=0.001");
	TwAddVarRW(controlBar, "shadow_intensity",			TW_TYPE_FLOAT,		&g_leaves_shadow_intensity			, "group='Leaves' label='shadow intensity' min=-10 max=10 step=0.001");
	TwAddVarRW(controlBar, "LightDiffuseColor",			TW_TYPE_COLOR3F,	&g_leaves_LightDiffuseColor.data	, "group='Leaves' label='light diffuse color' ");
	
	//TwAddVarRW(controlBar, "parallax", TW_TYPE_BOOLCPP, &g_ParallaxMappingEnabled, 
	//	" help='Parallax mapping enabled' ");
	// TwAddVarRW(controlBar, "parallaxScale", TW_TYPE_FLOAT, &g_ParallaxScale, 
	// 	" help='Parallax scale value' step=0.001");
	// TwAddVarRW(controlBar, "parallaxBias", TW_TYPE_FLOAT, &g_ParallaxBias, 
	// 	" help='Parallax bias value' step=0.001");


	// camera mode
	TwEnumVal cam_mode[] = 
	{ 
		{ CameraMode::FREE					, "Free"					},
		{ CameraMode::TERRAIN_RESTRICTED	, "Terrain restricted"     },
		{ CameraMode::TERRAIN_CONNECTED		, "Terrain connected"      },
		{ CameraMode::WALK					, "Walk"					}

	};

	TwType transport_type = TwDefineEnum("Camera mode", cam_mode, 
		4);
	TwAddVarRW(controlBar, "camera mode", transport_type, &g_cameraMode, 
		" group='Camera' keyIncr=c \
		help='Change camera movement mode.' ");
	TwAddVarRO(controlBar, "LOD0_count", TW_TYPE_INT32, &(g_tree_lod0_count), " label='LOD0 instance count' group=Statistics ");
	TwAddVarRO(controlBar, "LOD1_count", TW_TYPE_INT32, &(g_tree_lod1_count), " label='LOD1 instance count' group=Statistics ");
	TwAddVarRO(controlBar, "LOD2_count", TW_TYPE_INT32, &(g_tree_lod2_count), " label='LOD2 instance count' group=Statistics ");
	TwAddVarRO(controlBar, "LOD01_count", TW_TYPE_INT32, &(g_tree_lod01_count), " label='LOD01 instance count' group=Statistics ");
	TwAddVarRO(controlBar, "LOD12_count", TW_TYPE_INT32, &(g_tree_lod12_count), " label='LOD12 instance count' group=Statistics ");
	
	
	TwAddVarRO(controlBar, "GPU_fps", TW_TYPE_FLOAT, &(g_Statistics.fps), 
		" label='GPU fps' group=Statistics help='frames per second (measured on GPU)' ");
	TwAddVarRO(controlBar, "CPU_fps", TW_TYPE_FLOAT, &(g_CPU_fps), 
		" label='CPU fps' group=Statistics help='frames per second (measured on GPU)' ");
	/* TwAddVarRO(controlBar, "primitives", TW_TYPE_INT32, &(g_Statistics.primitives), 
	" label='primitives' group=Statistics help='primitives generated' ");
	*/   
	/*
	// house 1
	TwAddVarRO(controlBar, "house1_lod", TW_TYPE_INT32, &(g_Statistics.house1_lod), 
	" label='house1_lod' group=Statistics help='house level of detail' ");
	TwAddVarRO(controlBar, "house1_samples", TW_TYPE_INT32, &(g_Statistics.house1_samples), 
	" label='house1_samples' group=Statistics help='house samples generated on bbox' ");
	// house2
	TwAddVarRO(controlBar, "house2_lod", TW_TYPE_INT32, &(g_Statistics.house2_lod), 
	" label='house2_lod' group=Statistics help='house level of detail' ");
	TwAddVarRO(controlBar, "house2_samples", TW_TYPE_INT32, &(g_Statistics.house2_samples), 
	" label='house2_samples' group=Statistics help='house samples generated on bbox' ");
	// bridge
	TwAddVarRO(controlBar, "bridge_lod", TW_TYPE_INT32, &(g_Statistics.bridge_lod), 
	" label='bridge_lod' group=Statistics help='bridge level of detail' ");
	TwAddVarRO(controlBar, "bridge_samples", TW_TYPE_INT32, &(g_Statistics.bridge_samples), 
	" label='bridge_samples' group=Statistics help='bridge samples generated on bbox' ");
	// tower 1
	TwAddVarRO(controlBar, "tower1_lod", TW_TYPE_INT32, &(g_Statistics.tower1_lod), 
	" label='tower1_lod' group=Statistics help='tower1 level of detail' ");
	TwAddVarRO(controlBar, "tower1_samples", TW_TYPE_INT32, &(g_Statistics.tower1_samples), 
	" label='tower1_samples' group=Statistics help='tower1 samples generated on bbox' ");
	// tower 2
	TwAddVarRO(controlBar, "tower2_lod", TW_TYPE_INT32, &(g_Statistics.tower2_lod), 
	" label='tower2_lod' group=Statistics help='tower2 level of detail' ");
	TwAddVarRO(controlBar, "tower2_samples", TW_TYPE_INT32, &(g_Statistics.tower2_samples), 
	" label='tower2_samples' group=Statistics help='tower2 samples generated on bbox' ");
	// eggbox
	TwAddVarRO(controlBar, "eggbox_lod", TW_TYPE_INT32, &(g_Statistics.eggbox_lod), 
	" label='eggbox_lod' group=Statistics help='eggbox level of detail' ");
	TwAddVarRO(controlBar, "eggbox_samples", TW_TYPE_INT32, &(g_Statistics.eggbox_samples), 
	" label='eggbox_samples' group=Statistics help='eggbox samples generated on bbox' ");
	// haywagon
	TwAddVarRO(controlBar, "haywagon_lod", TW_TYPE_INT32, &(g_Statistics.haywagon_lod), 
	" label='haywagon_lod' group=Statistics help='haywagon level of detail' ");
	TwAddVarRO(controlBar, "haywagon_samples", TW_TYPE_INT32, &(g_Statistics.haywagon_samples), 
	" label='haywagon_samples' group=Statistics help='haywagon samples generated on bbox' ");
	// well
	TwAddVarRO(controlBar, "well_lod", TW_TYPE_INT32, &(g_Statistics.well_lod), 
	" label='well_lod' group=Statistics help='well level of detail' ");
	TwAddVarRO(controlBar, "well_samples", TW_TYPE_INT32, &(g_Statistics.well_samples), 
	" label='well_samples' group=Statistics help='well samples generated on bbox' ");
	*/






	/*TwAddVarRW(controlBar, "x_translate", TW_TYPE_FLOAT, &(g_light_position.x), 
	" label='x' group=Light help='x translation' ");
	TwAddVarRW(controlBar, "y_translate", TW_TYPE_FLOAT, &(g_light_position.y), 
	" label='y' group=Light help='y translation' ");
	TwAddVarRW(controlBar, "z_translate", TW_TYPE_FLOAT, &(g_light_position.z), 
	" label='z' group=Light help='z translation' ");   
	*/
	TwAddVarRW(controlBar, "g_bloomDivide	"	, TW_TYPE_FLOAT, &(g_bloomDivide	), " label='g_bloomDivide'	group=Light min=-1 max=100 step=0.01"); 


	TwAddVarRW(controlBar, "g_god_expo			", TW_TYPE_FLOAT, &(g_god_expo			), " label='g_god_expo'				group=Light min=0 max=5 step=0.01");  
	TwAddVarRW(controlBar, "g_god_decay			", TW_TYPE_FLOAT, &(g_god_decay			), " label='g_god_decay'			group=Light min=0 max=5 step=0.01");  
	TwAddVarRW(controlBar, "g_god_density		", TW_TYPE_FLOAT, &(g_god_density		), " label='g_god_density'			group=Light min=0 max=5 step=0.01");  
	TwAddVarRW(controlBar, "g_god_weight		", TW_TYPE_FLOAT, &(g_god_weight		), " label='g_god_weight'			group=Light min=0 max=5 step=0.01");  
	TwAddVarRW(controlBar, "g_illuminationDecay	", TW_TYPE_FLOAT, &(g_illuminationDecay	), " label='g_illuminationDecay'	group=Light min=0 max=5 step=0.01"); 


	TwAddVarRW(controlBar, "shadows", TW_TYPE_BOOLCPP, &(g_ShadowMappingEnabled), 
		" label='Shadows enabled' group=Light help='enable/disable shadows' ");  
	TwAddVarRW(controlBar, "g_dither			", TW_TYPE_FLOAT, &(g_dither			), " label='g_dither'				group=Light min=0 max=100 step=0.01");  
	

	TwAddVarRW(controlBar, "godrays", TW_TYPE_BOOLCPP, &(g_godraysEnabled), 
		" label='God rays enabled' group=Light help='enable/disable god rays' ");  
	TwAddVarRW(controlBar, "light_direction", TW_TYPE_DIR3F, &(g_light_direction), 
		" label='light direction' group=Light help='adjust direction of light' ");  
	TwAddVarRW(controlBar, "light_position", TW_TYPE_DIR3F, &(g_light_position), 
		" label='light position' group=Light help='adjust position of light' ");  
	
	TwAddVarRW(controlBar, "light_dir", TW_TYPE_BOOLCPP, &(g_light_showDir), 
		" label='Show direction' group=Light help='enable/disable showing lightdir' ");  

	TwAddVarRW(controlBar, "fbos", TW_TYPE_BOOLCPP, &(g_showTextures), 
		" label='Show FBOs' group=Debug help='enable/disable FBO display' "); 


	TwAddVarRW(controlBar, "wind_direction",  TW_TYPE_DIR3F, & g_tree_wind_direction.data, "group='Tree'");
	TwAddVarRW(controlBar, "wind_strength",   TW_TYPE_FLOAT, & g_tree_wind_strength, " group='Tree' min=0 max=5 step=0.01 ");
	TwAddVarRW(controlBar, "wood0_frequency", TW_TYPE_FLOAT, & g_tree_wood_frequencies.x, " group='Tree' min=0 max=100 step=0.05 ");
	TwAddVarRW(controlBar, "wood0_amplitude", TW_TYPE_FLOAT, & g_tree_wood_amplitudes.x, " group='Tree' min=0 max=10 step=0.01 ");
	TwAddVarRW(controlBar, "wood1_frequency", TW_TYPE_FLOAT, & g_tree_wood_frequencies.y, " group='Tree' min=0 max=100 step=0.05 ");
	TwAddVarRW(controlBar, "wood1_amplitude", TW_TYPE_FLOAT, & g_tree_wood_amplitudes.y, " group='Tree' min=0 max=10 step=0.01 ");
	TwAddVarRW(controlBar, "wood2_frequency", TW_TYPE_FLOAT, & g_tree_wood_frequencies.z, " group='Tree' min=0 max=100 step=0.05 ");
	TwAddVarRW(controlBar, "wood2_amplitude", TW_TYPE_FLOAT, & g_tree_wood_amplitudes.z, " group='Tree' min=0 max=10 step=0.01 ");
	TwAddVarRW(controlBar, "wood3_frequency", TW_TYPE_FLOAT, & g_tree_wood_frequencies.w, " group='Tree' min=0 max=100 step=0.05 ");
	TwAddVarRW(controlBar, "wood3_amplitude", TW_TYPE_FLOAT, & g_tree_wood_amplitudes.w, " group='Tree' min=0 max=10 step=0.01 ");



	TwAddVarRW(controlBar, "leaf_frequency", TW_TYPE_FLOAT, & g_tree_leaf_frequency, " group='Tree' min=0 max=100 step=0.1 ");
	TwAddVarRW(controlBar, "leaf_amplitude", TW_TYPE_FLOAT, & g_tree_leaf_amplitude, " group='Tree' min=0 max=10 step=0.01 ");
	TwAddVarRW(controlBar, "visible_slice", TW_TYPE_INT32, & g_tree_show_slice, " group='Tree' min=0 max=3 step=1 ");
	TwAddVarRW(controlBar, "visible_sliceSet", TW_TYPE_INT32, & g_tree_show_sliceSet, " group='Tree' min=0 max=3 step=1 ");




	// TwAddVarRW(controlBar, "slice_count", TW_TYPE_INT32, & g_tree_slice_count, " group='Tree' min=0 max=10 step=1 ");


	TwAddVarCB(controlBar, "Tree 2 count", TW_TYPE_INT32, cbSetTree2Count, cbGetTree2Count, NULL, " group='Vegetation' min=0 max=10000 step=1 ");
	TwAddVarCB(controlBar, "Tree 1 count", TW_TYPE_INT32, cbSetTree1Count, cbGetTree1Count, NULL, " group='Vegetation' min=0 max=10000 step=1 ");
	TwAddVarCB(controlBar, "Grass count", TW_TYPE_INT32, cbSetGrassCount, cbGetGrassCount, NULL, " group='Vegetation' min=0 max=100000 step=1 ");

	TwAddVarRW(controlBar, "Change1", TW_TYPE_FLOAT, &g_terrain_border_values.x, " group='Surfaces' label='Snow height' min=-5 max=30 step=0.5 help='Snow height' ");
	TwAddVarRW(controlBar, "Change1a", TW_TYPE_FLOAT, &g_terrain_border_widths.x, " group='Surfaces' label='Snow-rock transition' min=0 max=5 step=0.5 help='Transition 1' ");

	TwAddVarRW(controlBar, "Change2", TW_TYPE_FLOAT, &g_terrain_border_values.y, " group='Surfaces' label='Rock height' min=-5 max=30 step=0.5 help='Rock height' ");
	TwAddVarRW(controlBar, "Change2a", TW_TYPE_FLOAT, &g_terrain_border_widths.y, " group='Surfaces' label='Rock-grass transition' min=0 max=5 step=0.5 help='Transition 2' ");

	TwAddVarRW(controlBar, "Change3", TW_TYPE_FLOAT, &g_terrain_border_values.z, " group='Surfaces' label='Grass height' min=-5 max=30 step=0.5 help='Grass height' ");
	TwAddVarRW(controlBar, "Change3a", TW_TYPE_FLOAT, &g_terrain_border_widths.z, " group='Surfaces' label='Grass-clay transition' min=0 max=5 step=0.5 help='Transition 3' ");

	TwAddVarRW(controlBar, "Change4", TW_TYPE_FLOAT, &g_terrain_border_values.w, " group='Surfaces' label='Clay height' min=-5 max=30 step=0.5 help='Clay height' ");
	TwAddVarRW(controlBar, "Change4a", TW_TYPE_FLOAT, &g_terrain_border_widths.w, " group='Surfaces' label='Clay-ground transition' min=0 max=5 step=0.5 help='Transition 4' ");

	TwAddVarCB(controlBar, "Tree2 MIN", TW_TYPE_FLOAT, cbSetTree2Min, cbGetTree2Min, NULL, " group='Levels' min=-5 max=30 step=1 ");
	TwAddVarCB(controlBar, "Tree2 MAX", TW_TYPE_FLOAT, cbSetTree2Max, cbGetTree2Max, NULL, " group='Levels' min=-5 max=30 step=1 ");

	TwAddVarCB(controlBar, "Tree1 MIN", TW_TYPE_FLOAT, cbSetTree1Min, cbGetTree1Min, NULL, " group='Levels' min=-5 max=30 step=1 ");
	TwAddVarCB(controlBar, "Tree1 MAX", TW_TYPE_FLOAT, cbSetTree1Max, cbGetTree1Max, NULL, " group='Levels' min=-5 max=30 step=1 ");

	TwAddVarCB(controlBar, "Grass MIN", TW_TYPE_FLOAT, cbSetGrassMin, cbGetGrassMin, NULL, " group='Levels' min=-5 max=30 step=1 ");
	TwAddVarCB(controlBar, "Grass MAX", TW_TYPE_FLOAT, cbSetGrassMax, cbGetGrassMax, NULL, " group='Levels' min=-5 max=30 step=1 ");


	TwDefine(" Controls/Vegetation opened=false ");
	TwDefine(" Controls/Levels opened=false ");
	TwDefine(" Controls/Surfaces opened=false ");
	TwDefine(" Controls/Surfaces opened=false ");
	TwDefine(" Controls/Debug opened=false ");
	TwDefine(" Controls/Leaves opened=false ");
	TwDefine(" Controls/LOD opened=false ");
	//TwAddVarRW(controlBar, "vertex_normals", TW_TYPE_BOOLCPP, 
	//   &g_ShowVertexNormals, " label='vertex normals' \
	//   group=Render help='Show vertex normal, tangent, binormal.' ");
	//
	//TwAddVarRW(controlBar, "face_normals", TW_TYPE_BOOLCPP, &g_FaceNormals, 
	//   " label='face normals' group=Render help='Show face normals.' ");
	//TwAddVarRW(controlBar, "transparency", TW_TYPE_BOOLCPP, &g_Transparency, 
	//   " label='transparency' group=Render \
	//   help='Render transparent meshes.'");
	//TwAddVarRW(controlBar, "wiremode", TW_TYPE_BOOLCPP, &g_WireMode,
	//   " label='wire mode' group=Render help='Toggle wire mode.' ");
	//TwAddVarRW(controlBar, "face_culling", TW_TYPE_BOOLCPP, &g_FaceCulling,
	//   " label='face culling' group=Render  help='Toggle face culling.' ");
	//TwAddVarRW(controlBar, "alpha_threshold", TW_TYPE_FLOAT, &g_AlphaThreshold,
	//   " label='alpha threshold' group=Render min=0 max=1 step=0.01 \
	//    help='Alpha test threshold.' ");
	//TwAddVarRW(controlBar, "Translate", TW_TYPE_FLOAT, &g_SceneTraZ, 
	//   " group='Scene' label='translate Z' min=1 max=1000 step=0.5 \
	//    help='Scene translation.' ");
	//TwAddVarRW(controlBar, "Scale", TW_TYPE_FLOAT, &g_SceneScale, 
	//   " group='Scene' label='scale' min=0 max=10 step=0.01 \
	//    help='Scene scale.' ");
	//TwAddVarRW(controlBar, "SceneRotation", TW_TYPE_QUAT4F, &g_SceneRot, 
	//   " group='Scene' label='Scene rotation' open \
	//   help='Change the scene orientation.' ");
#endif
}


//-----------------------------------------------------------------------------
// Name: copyStdStringToClient()
// Desc: Function called to copy the content of a std::string (souceString) 
//       handled by the AntTweakBar library to destinationClientString handled 
//       by our application 
//-----------------------------------------------------------------------------
void TW_CALL copyStdStringToClient(std::string& dst, const std::string& src)
{
	dst = src;
} 


//-----------------------------------------------------------------------------
// Name: loadNewModelCB()
// Desc: Callback function to load new model
//-----------------------------------------------------------------------------
void TW_CALL loadNewModelCB(void* clientData)
{
	/*
	const std::string* file_name = &g_ModelFileName;//(const std::string *)(clientData);
	printf("RELOAD MODEL\n");
	if (!file_name->empty())
	{
	PGR2Model* pOldModel = g_pModel;
	printf("LOAD: %s\n", (*file_name).c_str());
	g_pModel = PGR2Model::loadFromFile(file_name->c_str());
	if (g_pModel != NULL)
	{
	delete pOldModel;
	}
	else
	{
	g_pModel = pOldModel;
	}
	}
	*/
} 


//-----------------------------------------------------------------------------
// Name: cbWindowSizeChanged()
// Desc: 
//-----------------------------------------------------------------------------
void cbWindowSizeChanged(int width, int height)
{
	g_WinWidth  = width;
	g_WinHeight = height;
	g_window_sizes.x = g_WinWidth;
	g_window_sizes.y = g_WinHeight;
	p_world->windowSizeChanged(width,height);
}
void activateANTMouse()
{
	glfwEnable( GLFW_MOUSE_CURSOR );
}
void activateGLFWMouse()
{
	//glfwDisable( GLFW_MOUSE_CURSOR );
	glfwSetMousePos(g_WinWidth/2, g_WinHeight/2);
}

//-----------------------------------------------------------------------------
// Name: cbKeyboardChanged()
// Desc: 
//-----------------------------------------------------------------------------
void cbKeyboardChanged(int key, int action)
{
	if (!g_MouseModeANT){
		// apply to camera first...
		if (p_world->p_activeCamera->handleKeyDown(key, action)){
			return;
		}
	}
	switch (key)
	{
		// DA use 'z' instead of 't'
		/*
		case 'z' : g_SceneTraZ  += 0.5f;                                   break;
		case 'Z' : g_SceneTraZ  -= (g_SceneTraZ > 0.5) ? 0.5f : 0.0f;      break;
		case 's' : g_SceneScale *= 1.01;                                   break;
		case 'S' : g_SceneScale *= 0.99;                                   break;
		case 'v' : g_ShowVertexNormals = !g_ShowVertexNormals;             break;
		case 'f' : g_FaceNormals != g_FaceNormals;                         break;
		case 't' : g_Transparency = !g_Transparency;                       break;
		case 'w' : g_WireMode    = !g_WireMode;                            break;
		case 'c' : g_FaceCulling = !g_FaceCulling;                         break;
		case 'a' : if(g_AlphaThreshold < 0.99f) g_AlphaThreshold += 0.01f; break;
		case 'A' : if(g_AlphaThreshold > 0.01f) g_AlphaThreshold -= 0.01f; break;
		*/
	case ' ' : 
		g_MouseModeANT = !g_MouseModeANT;
		if (g_MouseModeANT){
			activateANTMouse();
		} else {
			activateGLFWMouse();
		}
		break;
	}
}



bool g_MouseRotationEnabled = false;

//-----------------------------------------------------------------------------
// Name: cbMouseButtonChanged()
// Desc: internal
//-----------------------------------------------------------------------------
void GLFWCALL cbMouseButtonChanged(int button, int action)
{
	g_MouseRotationEnabled = ((button == GLFW_MOUSE_BUTTON_LEFT) && 
		(action == GLFW_PRESS));
}


//-----------------------------------------------------------------------------
// Name: cbMousePositionChanged()
// Desc: 
//-----------------------------------------------------------------------------
void cbMousePositionChanged(int x, int y)
{

	p_world->p_activeCamera->handleMouseMove(x,y);
	glfwSetMousePos(g_WinWidth/2, g_WinHeight/2);
}



//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) 
{
	int output = common_main(g_WinWidth, g_WinHeight,
		"NATUREA diploma thesis project",
		cbInitGL,              // init GL callback function
		cbDeinitGL,
		cbDisplay,             // display callback function
		cbWindowSizeChanged,   // window resize callback function
		cbKeyboardChanged,     // keyboard callback function
#ifdef USE_ANTTWEAKBAR
		cbMouseButtonChanged,                  // mouse button callback function
		cbMousePositionChanged                  // mouse motion callback function
#else
		cbMouseButtonChanged,  // mouse button callback function
		cbMousePositionChanged // mouse motion callback function
#endif
		);
	//deinitApp();

	return output;
}
