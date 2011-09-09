#include "TestModel.h"


TestModel::TestModel(void)
{
	tree_time_offset = 0;
	shader				= NULL;
	vbo					= NULL;
	colorMap 			= NULL;
	colorMap2 			= NULL;
	displacementMap 	= NULL;
	displacement2Map 	= NULL;
	weightMap 			= NULL;
	dataMap				= NULL;
						
	frontDecalMap 		= NULL;
	frontNormalMap 		= NULL;
	frontTranslucencyMap= NULL;
	frontHalfLife2Map 	= NULL;
	backDecalMap 		= NULL;
	backNormalMap 		= NULL;
	backTranslucencyMap = NULL;
	backHalfLife2Map 	= NULL;
}


TestModel::~TestModel(void)
{
	SAFE_DELETE_PTR (shader					);
	SAFE_DELETE_PTR (vbo					);
	SAFE_DELETE_PTR (colorMap 				);
	SAFE_DELETE_PTR (colorMap2 				);
	SAFE_DELETE_PTR (displacementMap 		);
	SAFE_DELETE_PTR (displacement2Map 		);
	SAFE_DELETE_PTR (weightMap 				);
	SAFE_DELETE_PTR (dataMap				);

	SAFE_DELETE_PTR (frontDecalMap 			);
	SAFE_DELETE_PTR (frontNormalMap 		);
	SAFE_DELETE_PTR (frontTranslucencyMap	);
	SAFE_DELETE_PTR (frontHalfLife2Map 		);
	SAFE_DELETE_PTR (backDecalMap 			);
	SAFE_DELETE_PTR (backNormalMap 			);
	SAFE_DELETE_PTR (backTranslucencyMap	);
	SAFE_DELETE_PTR (backHalfLife2Map 		);
	/*
	for (int i = 0; i<slices.size(); i++ ){
		SAFE_DELETE_PTR ( slices[i]);
	}
	*/
	slices.clear();
}

void TestModel::processTree(DTree * tree, v3 &dir)
{
	// create slices from tree

	float res = 256;
	win_resolution = v2 (res, res);
	tree->createSlices( dir, 5, win_resolution.x,  win_resolution.y, false);

	slices = tree->slices;	
}

void TestModel::draw()
{
	int i, count = slices.size();
	Texture * colorTexture, * dataTexture, *displacementTexture, *displacement2Texture, *normalTexture;
	displacementTexture		= displacementMap;
	displacement2Texture	= displacement2Map;
	for (i=0; i<count; i++){
		colorTexture			= slices[i]->colormap;
		normalTexture			= slices[i]->normalmap;
		dataTexture				= slices[i]->datamap;

		glPushMatrix();	
		
			//l3dBillboardCheatCylindricalBegin();
			
			//glRotatef(90, 0.0, 1.0, 0.0);
			glTranslatef((i-(float(count)/2.f) + 0.5), 0.0, 0.0);
			glScalef(10.0,10.0,10.0);
			//glRotatef(90, 0.0, 0.0, 1.0);
			glRotatef(90, 0.0, 1.0, 0.0);
			

		
			glDisable(GL_CULL_FACE);
			colorTexture		->bind(GL_TEXTURE0);
			displacementTexture	->bind(GL_TEXTURE1);
			displacement2Texture	->bind(GL_TEXTURE4);
			dataTexture			->bind(GL_TEXTURE2);
			normalTexture		->bind(GL_TEXTURE3);

			u_time_offset->data = &	g_tree_time_offset_1;
			// turn on shader
			shader->use(true);
			shader->setTexture(l_color	, colorTexture			->textureUnitNumber	);
			shader->setTexture(l_displ	, displacementTexture	->textureUnitNumber	);
			shader->setTexture(l_displ2	, displacement2Texture	->textureUnitNumber	);
			shader->setTexture(l_data	, dataTexture			->textureUnitNumber	);
			shader->setTexture(l_normal	, normalTexture			->textureUnitNumber	);
			vbo->draw(shader, GL_QUADS, 0);

			colorTexture		->unbind();
			displacementTexture	->unbind();
			displacement2Texture	->unbind();
			dataTexture			->unbind();
			normalTexture		->unbind();
			// turn off shader
			shader->use(false);
			glEnable(GL_CULL_FACE);

		glPopMatrix();
	}
}

void TestModel::drawForLOD()
{

}

void TestModel::init()
{

	// init textures...
	if (slices.size()<1){
		// error!!!
		printf("Slices for LOD not attached to LOD model!!! Fatal error, terminating...\n");
		pauseAndExit();

	}

	displacementMap		=new Texture("displacementMap");
	displacementMap		->load(DYN_TREE::LEAF_NOISE_TEXTURE, true, false, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	
	displacement2Map	=new Texture("displacement2Map");
	displacement2Map	->load(DYN_TREE::BRANCH_NOISE_TEXTURE, true, false, GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	

	// init shaders

	shader = new Shader("test");
	shader->loadShader("shaders/test2_vs.glsl", "shaders/test2_fs.glsl");
	// link textures to shader
	//shader->linkTexture(colorMap			);
	//shader->linkTexture(displacementMap		);
	//shader->linkTexture(dataMap				);

	l_color		 = shader->getGLLocation("colorMap"			);
	l_displ		 = shader->getGLLocation("displacementMap"	);
	l_displ2	 = shader->getGLLocation("displacement2Map"	);
	l_data		 = shader->getGLLocation("dataMap"			);
	l_normal	 = shader->getGLLocation("normalMap"		);

	shader->registerUniform("time", UniformType::F1, & g_float_time);

	shader->registerUniform("wave_amplitude"		, UniformType::F1, & g_tree_wave_amplitude		);
	shader->registerUniform("wave_frequency"		, UniformType::F1, & g_tree_wave_frequency		);
	shader->registerUniform("movementVectorA"		, UniformType::F2, & g_tree_movementVectorA		);
	shader->registerUniform("movementVectorB"		, UniformType::F2, & g_tree_movementVectorB		);
	shader->registerUniform("wave_y_offset"			, UniformType::F1, & g_tree_wave_y_offset			);
	shader->registerUniform("wave_increase_factor"	, UniformType::F1, & g_tree_wave_increase_factor	);
	shader->registerUniform("window_size"			, UniformType::F2, & win_resolution				);


	shader->registerUniform("wood_amplitudes"		, UniformType::F4, & g_tree_wood_amplitudes.data	);
	shader->registerUniform("wood_frequencies"		, UniformType::F4, & g_tree_wood_frequencies.data	);
	shader->registerUniform("leaf_amplitude"		, UniformType::F1, & g_tree_leaf_amplitude	);
	shader->registerUniform("leaf_frequency"		, UniformType::F1, & g_tree_leaf_frequency	);

	int i = shader->registerUniform("time_offset"	, UniformType::F1, & tree_time_offset);
	u_time_offset = shader->getUniform(i);
	/*
	shader = new Shader("test");
	shader->loadShader("shaders/test_vs.glsl", "shaders/test_fs.glsl");
	// link textures to shader
	shader->linkTexture(frontDecalMap		);
	shader->linkTexture(frontNormalMap		);
	shader->linkTexture(frontTranslucencyMap);
	shader->linkTexture(frontHalfLife2Map	);
	shader->linkTexture(backDecalMap		);
	shader->linkTexture(backNormalMap		);
	shader->linkTexture(backTranslucencyMap	);
	shader->linkTexture(backHalfLife2Map	);
	*/

	// init VBO
	int count = 4;
	vbo = new VBO();
	vbo->setVertexCount(count);
	// position
	VBODataSet * dataSet = new VBODataSet(
		PLANE_VERTEX_ARRAY2,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::POSITION,
		true
	);
	vbo->addVertexAttribute( dataSet );
	// normal
	dataSet = new VBODataSet(
		PLANE_NORMAL_ARRAY2,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::NORMAL,
		false
	);
	vbo->addVertexAttribute( dataSet );
	// tangent
	dataSet = new VBODataSet(
		PLANE_TANGENT_ARRAY,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TANGENT,
		false
	);
	vbo->addVertexAttribute( dataSet );
	// texture coordinates
	dataSet = new VBODataSet(
		PLANE_TEX_COORD_ARRAY2,
		2*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TEXCOORD0,
		false
	);
	vbo->addVertexAttribute( dataSet );

	// link vbo and shaders
	vbo->compileData(GL_STATIC_DRAW);
	vbo->compileWithShader(shader);

}

void TestModel::update(double time)
{

}
