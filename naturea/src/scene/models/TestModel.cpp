#include "TestModel.h"


TestModel::TestModel(void)
{
	shader				= NULL;
	vbo					= NULL;
	ebo					= NULL;
}


TestModel::~TestModel(void)
{
	SAFE_DELETE_PTR (shader					);
	SAFE_DELETE_PTR (vbo					);
	SAFE_DELETE_PTR (ebo					);
	
}


void TestModel::draw()
{
	//const float frustum_treshold = 0.70710678118654752440084436210485;
	//float frustum_treshold = camera->frustum_treshold;
	//m4 mv;
	//m4 p;
	//glGetFloatv(GL_MODELVIEW_MATRIX, mv.m);
	//glGetFloatv(GL_PROJECTION_MATRIX, p.m);
	//m4 MVP = mv*p;



	TestInstance * instance;
	float * type1Matrices = new float[16*instances.size()];
	float * type2Matrices = new float[16*instances.size()];
	float * type1Param1   = new float[instances.size()];
	float * type2Param1   = new float[instances.size()];
	int type1Index = 0;
	int type2Index = 0;
	int instanceCount = instances.size();
	TestInstance * act_instance = instances[0] ;
	TestInstance * next_instance;
	float dx = act_instance->x - g_viewer_position->x;
	float dy = act_instance->y - g_viewer_position->y;
	float dz = act_instance->z - g_viewer_position->z;

	float dist = sqrt(dx*dx + dy*dy + dz*dz);
	float act_dist = dist;
	float next_dist;
	float done_dist;

	act_instance->transformMatrix;

	v3	  act_eye_dir(dx,dy,dz); 
	//v4	  act_proj = MVP*act_instance->transformMatrix*v4(act_eye_dir);
	act_eye_dir.normalize();
	float act_disc = g_viewer_direction->dot(act_eye_dir);
	
	v3	  next_eye_dir;
	v4	  next_proj;
	float next_disc;
	float done_disc;
	v4	  done_proj;
	TestInstance* done_instance;
	// go through instances, perform one bubble sort walktrough & split into different render queues
	for (int i=1; i<instances.size(); i++){
		next_instance = instances[i];
		dx = next_instance->x - g_viewer_position->x;
		dy = next_instance->y - g_viewer_position->y;
		dz = next_instance->z - g_viewer_position->z;
		next_eye_dir = v3(dx, dy, dz);
		//next_proj = MVP*next_instance->transformMatrix*v4(next_eye_dir);
		next_eye_dir.normalize();
		next_disc = g_viewer_direction->dot(next_eye_dir);
		next_dist = sqrt(dx*dx + dy*dy + dz*dz);
		if (next_dist>act_dist){
			// swap
			instances[i-1]	= next_instance;
			instances[i]	= act_instance;
			done_instance	= next_instance;
			done_dist		= next_dist;
			done_disc		= next_disc;
			//done_proj		= next_proj;
		} else {
			done_instance	= act_instance;
			done_dist		= act_dist;
			done_disc		= act_disc;
			//done_proj		= act_proj;
			act_instance	= next_instance;
			act_dist		= next_dist;
			act_disc		= next_disc;
			act_eye_dir		= next_eye_dir;
			//act_proj		= next_proj;
		}
		//done_proj = done_proj / done_proj.w;
		//printf ("i: %i _ ", i); done_proj.printOut();

		if(done_disc>0.5){
			if (done_dist>10.0){
				memcpy( type1Matrices+type1Index*16, done_instance->transformMatrix.m, 16*sizeof(float));
				type1Param1[type1Index] = 0.3; //instance->param1;
				type1Index += 1;
			} else {
				memcpy( type2Matrices+type2Index*16, done_instance->transformMatrix.m, 16*sizeof(float));
				type2Param1[type2Index] = 0.8; //instance->param1;
				type2Index += 1;
			}
		}
	}
	//printf("T1: %i, T2: %i\n", type1Index, type2Index);
	glDisable(GL_CULL_FACE);
	glUseProgram(shader->programID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, v_indicesBuffID);
	// bind positions data
	glBindBuffer(GL_ARRAY_BUFFER, v_positionsBuffID);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(0));   //The starting point of the VBO, for the vertices
	int p1Loc = glGetAttribLocation(shader->programID, "param1");
	int tmLoc = glGetAttribLocation(shader->programID, "transformMatrix");
	int tmLoc0 = tmLoc + 0;
	int tmLoc1 = tmLoc + 1;
	int tmLoc2 = tmLoc + 2;
	int tmLoc3 = tmLoc + 3;
	glEnableVertexAttribArray(p1Loc);
	glEnableVertexAttribArray(tmLoc0);
	glEnableVertexAttribArray(tmLoc1);
	glEnableVertexAttribArray(tmLoc2);
	glEnableVertexAttribArray(tmLoc3);
	// bind instance data

	// draw type1
	glBindBuffer(GL_ARRAY_BUFFER, i_paramBuffID);
	glBufferData(GL_ARRAY_BUFFER, type1Index * 1 * sizeof(float), type1Param1, GL_STREAM_DRAW);
	glVertexAttribPointer(p1Loc, 1, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glVertexAttribDivisor(p1Loc, 1);
	glBindBuffer(GL_ARRAY_BUFFER, i_matricesBuffID);
	
	glVertexAttribPointer(tmLoc0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, (void*)(0));
	glVertexAttribPointer(tmLoc1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, (void*)(sizeof(float) * 4));
	glVertexAttribPointer(tmLoc2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, (void*)(sizeof(float) * 8));
	glVertexAttribPointer(tmLoc3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4 * 4, (void*)(sizeof(float) * 12));
	glVertexAttribDivisor(tmLoc0, 1);
	glVertexAttribDivisor(tmLoc1, 1);
	glVertexAttribDivisor(tmLoc2, 1);
	glVertexAttribDivisor(tmLoc3, 1);	

	glBufferData(GL_ARRAY_BUFFER, type1Index * 16 * sizeof(float), type1Matrices, GL_STREAM_DRAW);
	glDrawElementsInstanced(GL_QUADS, 8, GL_UNSIGNED_INT, BUFFER_OFFSET(0), type1Index);

	// draw type2
	glBufferData(GL_ARRAY_BUFFER, type2Index * 16* sizeof(float), type2Matrices, GL_STREAM_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, i_paramBuffID);
	glBufferData(GL_ARRAY_BUFFER, type2Index * 1 * sizeof(float), type2Param1, GL_STREAM_DRAW);
	glVertexAttribPointer(p1Loc, 1, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glVertexAttribDivisor(p1Loc, 1);
	glDrawElementsInstanced(GL_QUADS, 8, GL_UNSIGNED_INT, BUFFER_OFFSET(8*sizeof(unsigned int)), type2Index);  


	// disable all...
	glDisableVertexAttribArray(tmLoc0);
	glDisableVertexAttribArray(tmLoc1);
	glDisableVertexAttribArray(tmLoc2);
	glDisableVertexAttribArray(tmLoc3);
	glDisableVertexAttribArray(p1Loc);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glEnable(GL_CULL_FACE);
	glUseProgram(0);
	// free memory
	delete [] type1Matrices ;
	delete [] type2Matrices ;
	delete [] type1Param1   ;
	delete [] type2Param1   ;

}

void TestModel::drawForLOD()
{

}

void TestModel::init()
{
	// init instances
	int grid_size = g_tree_gridSize; // squared
	int instanceCount = grid_size*grid_size;
	matricesBufferData = new float[instanceCount*16];
	// create instances
	TestInstance * instance;
	float step = g_tree_mean_distance;
	float cosA, sinA;
	for (int i=0; i<grid_size; i++){
		for (int j=0; j<grid_size; j++){
			instance = new TestInstance();
			instance->x = i*step - 0.5*grid_size*step + randomf(-g_tree_dither, g_tree_dither);
			instance->z = j*step - 0.5*grid_size*step + randomf(-g_tree_dither, g_tree_dither);
			float xt	= instance->x + terrain->sz_x/2.0;
			float yt	= instance->z + terrain->sz_y/2.0;
			instance->y	= terrain->getHeightAt(xt,yt);


			instance->r = randomf(0.0, 90.0);
			cosA = cos(DEG_TO_RAD*instance->r);
			sinA = sin(DEG_TO_RAD*instance->r);

			m4 matrix(cosA			,	0			,	-sinA	,	0	,
					  0				,	1			,	0		,	0	,
					  sinA			,	0			,	cosA	,	0	,
					  instance->x	,	instance->y	,	instance->z		,	1	);

			instance->transformMatrix = matrix; // copy matrix
			instance->param1 = float(i)/float(grid_size);
			instances.push_back( instance );
		}
	}

	// init shaders

	shader = new Shader("test");
	shader->loadShader("shaders/test_instancing_vs.glsl", "shaders/test_instancing_fs.glsl");

	// init VBOs
	const float CROSS_ARRAY[3*8] = {	-1	,0	,0	,
										1	,0	,0	,
										1	,2	,0	,
										-1	,2	,0	,
										0	,0	,-1	,
										0	,0	,1	,
										0	,2	,1	,
										0	,2	,-1	,
									};
	const unsigned int INDICES[16] ={0,1,2,3,4,5,6,7,
									 7,6,5,4,3,2,1,0};
	// instance matrices
	i_matricesBuffID = 0;
	glGenBuffers(1, &i_matricesBuffID);
	glBindBuffer(GL_ARRAY_BUFFER, i_matricesBuffID);
		// load data
		glBufferData(GL_ARRAY_BUFFER, instanceCount * 16* sizeof(float), NULL, GL_STREAM_DRAW);  
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// instance params
	glGenBuffers(1, &i_paramBuffID);
	glBindBuffer(GL_ARRAY_BUFFER, i_paramBuffID);
		// load data
		glBufferData(GL_ARRAY_BUFFER, instanceCount * 1 * sizeof(float), NULL, GL_STREAM_DRAW);  
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &v_positionsBuffID);
	glBindBuffer(GL_ARRAY_BUFFER, v_positionsBuffID);
		// load data
		glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(float), CROSS_ARRAY, GL_STATIC_DRAW);  
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glGenBuffers(1, &v_indicesBuffID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, v_indicesBuffID);
		// load data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 16 * sizeof(unsigned int), INDICES, GL_STATIC_DRAW);  
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void TestModel::update(double time)
{

}
