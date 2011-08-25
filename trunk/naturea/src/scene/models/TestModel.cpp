#include "TestModel.h"


TestModel::TestModel(void)
{
}


TestModel::~TestModel(void)
{
}

void TestModel::draw()
{
	glPushMatrix();
	glTranslatef(0.0, 8.0, 0.0);
	glScalef(3.0,3.0,3.0);
	glDisable(GL_CULL_FACE);

	frontDecalMap		->bind(GL_TEXTURE0);
	frontNormalMap		->bind(GL_TEXTURE1);
	frontTranslucencyMap->bind(GL_TEXTURE2);
	frontHalfLife2Map	->bind(GL_TEXTURE3);
	backDecalMap		->bind(GL_TEXTURE4);
	backNormalMap		->bind(GL_TEXTURE5);
	backTranslucencyMap	->bind(GL_TEXTURE6);
	backHalfLife2Map	->bind(GL_TEXTURE7);

	vbo->draw(shader, GL_QUADS, 0);

	frontDecalMap		->unbind();
	frontNormalMap		->unbind();
	frontTranslucencyMap->unbind();
	frontHalfLife2Map	->unbind();
	backDecalMap		->unbind();
	backNormalMap		->unbind();
	backTranslucencyMap	->unbind();
	backHalfLife2Map	->unbind();

	glEnable(GL_CULL_FACE);

	glPopMatrix();
}

void TestModel::drawForLOD()
{

}

void TestModel::init()
{
	// init textures...
	frontDecalMap		=new Texture("frontDecalMap");
	frontNormalMap		=new Texture("frontNormalMap");
	frontTranslucencyMap=new Texture("frontTranslucencyMap");
	frontHalfLife2Map	=new Texture("frontHalfLife2Map");
	backDecalMap		=new Texture("backDecalMap");
	backNormalMap		=new Texture("backNormalMap");
	backTranslucencyMap	=new Texture("backTranslucencyMap");
	backHalfLife2Map	=new Texture("backHalfLife2Map");
	frontDecalMap		->load(DYN_TREE::TEX_FDM, true, false, GL_CLAMP, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	frontNormalMap		->load(DYN_TREE::TEX_FNM, true, false, GL_CLAMP, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	frontTranslucencyMap->load(DYN_TREE::TEX_FTM, true, false, GL_CLAMP, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	frontHalfLife2Map	->load(DYN_TREE::TEX_FHM, true, false, GL_CLAMP, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	backDecalMap		->load(DYN_TREE::TEX_BDM, true, false, GL_CLAMP, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	backNormalMap		->load(DYN_TREE::TEX_BNM, true, false, GL_CLAMP, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	backTranslucencyMap	->load(DYN_TREE::TEX_BTM, true, false, GL_CLAMP, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	backHalfLife2Map	->load(DYN_TREE::TEX_BHM, true, false, GL_CLAMP, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

	// init shaders
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
		PLANE_NORMAL_ARRAY,
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
