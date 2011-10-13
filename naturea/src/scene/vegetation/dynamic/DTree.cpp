#include "DTree.h"


DTree::DTree(TextureManager *texManager, ShaderManager *shManager):Vegetation(texManager, shManager)
{
	branchShader			= NULL;
	leafShader				= NULL;
	branchShader_sh			= NULL;
	leafShader_sh			= NULL;

	bLODShader				= NULL;
	lLODShader				= NULL;
							
	dataTexture				= NULL;
	lColorTexture			= NULL;
	frontDecalMap			= NULL;
	frontNormalMap			= NULL;
	frontTranslucencyMap	= NULL;
	frontHalfLife2Map		= NULL;
	backDecalMap			= NULL;
	backNormalMap			= NULL;
	backTranslucencyMap		= NULL;
	backHalfLife2Map		= NULL;
							
	bColorTexture			= NULL;
	bNormalTexture			= NULL;
							
	branchNoiseTexture		= NULL;
	leafNoiseTexture		= NULL;

	branchesVBO				= NULL;
	leavesVBO				= NULL;
	branchesEBO				= NULL;

	lod1shader				= NULL;
	lod1shader2				= NULL;

	lod2color1				= NULL;
	lod2color2				= NULL;
	lod2normal1				= NULL;
	lod2normal2				= NULL;
	lod2branch1				= NULL;
	lod2branch2				= NULL;
	lod2depth1				= NULL;
	lod2depth2				= NULL;
	ctr	=0;
}					
DTree::DTree(DTree* copy):
Vegetation(copy->textureManager, copy->shaderManager)
{
	//	textureId	= copy->textureId;
	//	vboId		= copy->vboId;
	//	shader		= copy->shader;
	//	shaderId	= copy->shaderId;
	//	LCmatrixLoc = copy->LCmatrixLoc;
	//	fastModeLoc	= copy->fastModeLoc;
	//	shadowMappingEnabledLoc = copy->shadowMappingEnabledLoc;
	ctr	= 0 ;
}

DTree::~DTree(void)
{
	SAFE_DELETE_PTR(	branchShader			);
	SAFE_DELETE_PTR(	leafShader				);
	SAFE_DELETE_PTR(	bLODShader				);
	SAFE_DELETE_PTR(	lLODShader				);

	SAFE_DELETE_PTR(	dataTexture				);
	SAFE_DELETE_PTR(	lColorTexture			);
	SAFE_DELETE_PTR(	frontDecalMap			);
	SAFE_DELETE_PTR(	frontNormalMap			);
	SAFE_DELETE_PTR(	frontTranslucencyMap	);
	SAFE_DELETE_PTR(	frontHalfLife2Map		);
	SAFE_DELETE_PTR(	backDecalMap			);
	SAFE_DELETE_PTR(	backNormalMap			);
	SAFE_DELETE_PTR(	backTranslucencyMap		);
	SAFE_DELETE_PTR(	backHalfLife2Map		);

	SAFE_DELETE_PTR(	bColorTexture			);
	SAFE_DELETE_PTR(	bNormalTexture			);

	SAFE_DELETE_PTR(	branchNoiseTexture		);
	SAFE_DELETE_PTR(	leafNoiseTexture		);

	SAFE_DELETE_PTR(	branchesVBO				);
	SAFE_DELETE_PTR(	leavesVBO				);
	SAFE_DELETE_PTR(	branchesEBO				);

	int i = 0;
	for (i = 0; i< slices.size(); i++){
		SAFE_DELETE_PTR( slices[i] );
	}
	slices.clear();
	for (i = 0; i< branches.size(); i++){
		SAFE_DELETE_PTR( branches[i] );
	}
	branches.clear();
	for (i = 0; i< leaves.size(); i++){
		SAFE_DELETE_PTR( leaves[i] );
	}
	leaves.clear();
	 
	// instance matrices...
	for (i=0; i< lod1_instanceMatrices.size(); i++){
		SAFE_DELETE_PTR( lod1_instanceMatrices[i] );
	}
	lod1_instanceMatrices.clear();
	// render queues
	for (i=0; i< instancesInRenderQueues.size(); i++){
		SAFE_DELETE_ARRAY_PTR( instancesInRenderQueues[i] );
	}
	instancesInRenderQueues.clear();

	SAFE_DELETE_PTR(	lod1shader		);
	SAFE_DELETE_PTR(	lod1shader2		);


	SAFE_DELETE_PTR(	lod2color1		);	
	SAFE_DELETE_PTR(	lod2color2		);
	SAFE_DELETE_PTR(	lod2normal1		);
	SAFE_DELETE_PTR(	lod2normal2		);
	SAFE_DELETE_PTR(	lod2branch1		);
	SAFE_DELETE_PTR(	lod2branch2		);
	SAFE_DELETE_PTR(	lod2depth1		);
	SAFE_DELETE_PTR(	lod2depth2		);

}

bool DTree::loadOBJT(string filename)
{
	printf(" DYN_TREE load %s\n", filename.c_str());
	OBJTfile file;
	file.loadFromFile(filename.c_str());

	// setup bbox 
	//TODO: get proper coords of bbox from OBJT / model itself
	bbox = getBBox();

	// process branches
	map <int, DTreeBranch*> m_branches;
	map <int, StEntity>::iterator iter;
	DTreeBranch *parent = NULL;
	CoordSystem cs;
	CoordSystem objectCS;
	float x;
	float length, r1, r2, c2=0.3326, c4=0.398924;
	int divT=2, divR=4;
	v3 motionVector(1.0, 1.0, 1.0);
	StEntity entity;
	// get trunk (id = parentId)
	int id=0;
	branchCount = 0;
	for ( iter=file.branches.begin() ; iter != file.branches.end(); iter++ ){
		parent	= NULL;
		id		= (*iter).first;
		entity	= (*iter).second;
		branchCount++;

		// parent
		if (entity.isSetParent && entity.id!=entity.parentId){
			parent = m_branches[entity.parentId];
		} else {
			parent = NULL;
		}
		// length
		if (entity.isSetLength){
			length = entity.length / file.maxLength;
		} else {
			if (entity.isSetLevel){
				length = 1.f/(entity.level*3.f+1.f);
			} else {
				length = 1.0;
			}
		}
		// x
		if (entity.isSetX){
			x = entity.x;
		} else {
			x = 0.5;
		}
		// base vectors
		if (entity.isSetBase){
			cs.r = entity.base[0];
			cs.s = entity.base[1];	
			cs.t = entity.base[2];
			objectCS.r = entity.base[0];
			objectCS.s = entity.base[1];
			objectCS.t = entity.base[2];
			cs.normalize();
			cs.repair();
		} else {
			cs.r = v3(1.0, 0.0, 0.0);
			cs.s = v3(0.0, 1.0, 0.0);	
			cs.t = v3(0.0, 0.0, 1.0);
			objectCS.r = v3(1.0, 0.0, 0.0);
			objectCS.s = v3(0.0, 1.0, 0.0);
			objectCS.t = v3(0.0, 0.0, 1.0);
		}

		// origin
		if (entity.isSetOrigin){
			cs.origin = entity.origin;
			// recalculate origin
			if (parent!=NULL){
				// TODO!!!! get position of origin{branch space} from origin{obj space} 
			}
		} else {
			if (parent!=NULL){
				cs.origin = v3(0.0, 0.0, 1.0) * x * parent->L;
			} else {
				cs.origin = v3(0.f, 0.f, 0.f);
			}
		}
		// r1
		if (entity.isSetR1){
			r1 = entity.r1;
		} else {		
			r1 = 0.02*length;
		}
		// r2
		if (entity.isSetR2){
			r2 = entity.r2;
		} else {		
			r2 = 0.0001;
		}
		// add branch...	
		m_branches[id] = new DTreeBranch(parent, cs, x, length, r1, r2);  
		m_branches[id]->id = id;
	} // for each entity in map
	// set trunk
	trunk = m_branches[0];
	trunk->level = 0;
	int leafCnt = 0;
	// process leaves
	for ( iter=file.leaves.begin() ; iter != file.leaves.end(); iter++ ){
		id		= (*iter).first;
		entity	= (*iter).second;
		leafCnt++;
		// parent
		if (entity.isSetParent){
			parent = m_branches[entity.parentId];
		} else {
			parent = NULL;
		}

		// leaf X position
		if (entity.isSetX){
			x = entity.x;
		} else {
			x = 0.5f;
		}

		// leaf size
		float size;
		if (entity.isSetLength){
			size = entity.length;
		} else {
			size = DYN_TREE::LEAF_SIZE;
		}

		// base vectors
		if (entity.isSetBase){
			cs.r = entity.base[0];
			cs.s = entity.base[1];	
			cs.t = entity.base[2];
			objectCS.r = entity.base[0];
			objectCS.s = entity.base[1];
			objectCS.t = entity.base[2];
			cs.normalize();
			cs.repair();

		} else {
			cs.r = v3(1.0, 0.0, 0.0);
			cs.s = v3(0.0, 1.0, 0.0);	
			cs.t = v3(0.0, 0.0, 1.0);
			objectCS.r = v3(1.0, 0.0, 0.0);
			objectCS.s = v3(0.0, 1.0, 0.0);
			objectCS.t = v3(0.0, 0.0, 1.0);
		}

		// create leaf
		DTreeLeaf * leaf = new DTreeLeaf(parent, cs, x, size);
		// add to container
		leaves.push_back(leaf);
	}

	printf("DYN_TREE load successfull (branches: %d, leaves: %d)\n", branchCount, leafCnt);
	return true;
}

void DTree::createVBOs()
{
	// TODO:
	// create VBO for branch vertices
	createBranchesVBO();

	// create VBO for leaf vertices
	createLeavesVBO();
}

void DTree::createBranchesVBO()
{
	int i, sizeB = branches.size();
	int vCnt = 0, iCnt = 0;
	// count total vertices & indices
	for (i=0; i<sizeB; i++){
		vCnt += branches[i]->getVertexCount();
		iCnt += branches[i]->getIndicesCount();
	}
	int * sizes = new int[DYN_TREE::VERTEX_ATTRIBUTES::COUNT];
	// get sizes
	for (i = 0; i < DYN_TREE::VERTEX_ATTRIBUTES::COUNT; i++){
		sizes[i]=0;
	}
	sizes[DYN_TREE::VERTEX_ATTRIBUTES::POSITION]	= 3;
	sizes[DYN_TREE::VERTEX_ATTRIBUTES::NORMAL]		= 3;
	sizes[DYN_TREE::VERTEX_ATTRIBUTES::TANGENT]		= 3;
	sizes[DYN_TREE::VERTEX_ATTRIBUTES::TEXCOORD0]	= 2;
	//sizes[DYN_TREE::VERTEX_ATTRIBUTES::TEXCOORD1]	= 2;
	sizes[DYN_TREE::VERTEX_ATTRIBUTES::BRANCH_INDEX]= 1;
	sizes[DYN_TREE::VERTEX_ATTRIBUTES::WEIGHT]		= 4;
	int totalVertexSize = 0;
	for (i = 0; i < DYN_TREE::VERTEX_ATTRIBUTES::COUNT; i++){
		vbo_data[i]= new GLfloat[vCnt*sizes[i]];
		totalVertexSize+=sizes[i];
	}
	// ebo
	ebo_data = new GLuint	[iCnt];

	// fill arrays
	int k, offset;
	int indexPtr = 0, dataPtr=0, v=0, id=0;
	int indexOffset = 0;
	Vertex *vertex;
	int index = 0;
	for (i=0; i<sizeB; i++)// each branch 
	{
		id = branches[i]->id;
		offset = dataPtr;
		for (k=0; k<branches[i]->vertices.size(); k++){
			vertex = branches[i]->vertices[k];
			// position
			index = DYN_TREE::VERTEX_ATTRIBUTES::POSITION;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->bPos.data[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->bPos.data[ 1 ];
			vbo_data[index][dataPtr*sizes[index] + 2] = vertex->bPos.data[ 2 ];

			// normal
			index = DYN_TREE::VERTEX_ATTRIBUTES::NORMAL;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->normal.data[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->normal.data[ 1 ];
			vbo_data[index][dataPtr*sizes[index] + 2] = vertex->normal.data[ 2 ];

			// tangent
			index = DYN_TREE::VERTEX_ATTRIBUTES::TANGENT;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->tangent.data[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->tangent.data[ 1 ];
			vbo_data[index][dataPtr*sizes[index] + 2] = vertex->tangent.data[ 2 ];

			// texcoord0
			index = DYN_TREE::VERTEX_ATTRIBUTES::TEXCOORD0;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->textureCoords.data[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->textureCoords.data[ 1 ];

			// xvals
			index = DYN_TREE::VERTEX_ATTRIBUTES::WEIGHT;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->x[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->x[ 1 ];
			vbo_data[index][dataPtr*sizes[index] + 2] = vertex->x[ 2 ];
			vbo_data[index][dataPtr*sizes[index] + 3] = vertex->x[ 3 ];

			// branch index
			index = DYN_TREE::VERTEX_ATTRIBUTES::BRANCH_INDEX;
			vbo_data[index][dataPtr + 0] = id+0.5f;
			dataPtr++;
		}
		// copy branch indices...
		int * branchIndices = branches[i]->indexPtr;
		int branchIndexCnt = branches[i]->indicesCount;

		for (k=0; k<branchIndexCnt; k++){
			ebo_data[indexPtr] = branchIndices[k]+offset; 
			indexPtr++;
		}
	}
	// create Buffers...

	// VBO
	branchesVBO = new VBO();
	branchesVBO->setVertexCount(vCnt);
	// position
	VBODataSet * dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::POSITION],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::POSITION]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::POSITION,
		true
		);
	branchesVBO->addVertexAttribute( dataSet );
	// normal
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::NORMAL],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::NORMAL]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::NORMAL,
		false
		);
	branchesVBO->addVertexAttribute( dataSet );

	// tangent
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::TANGENT],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::TANGENT]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TANGENT,
		false
		);
	branchesVBO->addVertexAttribute( dataSet );

	// texCoord0
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::TEXCOORD0],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::TEXCOORD0]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TEXCOORD0,
		false
		);
	branchesVBO->addVertexAttribute( dataSet );

	// weights
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::WEIGHT],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::WEIGHT]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::WEIGHT,
		false
		);	
	branchesVBO->addVertexAttribute( dataSet );

	// branch index
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::BRANCH_INDEX],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::BRANCH_INDEX]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::BRANCH_ID,
		false
		);	
	branchesVBO->addVertexAttribute( dataSet );

	branchesVBO->compileData(GL_STATIC_DRAW);

	// element buffer object
	branchesEBO = new EBO();
	branchesEBO->create(GL_UNSIGNED_INT, GL_TRIANGLES, iCnt, ebo_data, GL_STATIC_DRAW);
	branchesEBO->linkVBO(branchesVBO);

}

void DTree::createLeavesVBO()
{
	// recalc coord systems...
	int i, sizeL = leaves.size();

	DTreeLeaf * leaf;
	for (i=0; i<sizeL; i++){
		leaf = leaves[i];
		leaf->init();
		if (leaf->parent!=NULL){
			leaf->cs = leaf->parent->originalCS.getSystemInThisSystem(leaf->originalCS);
		}
	}
	int vCnt = 0;
	// count total vertices & indices
	for (i=0; i<sizeL; i++){
		vCnt += leaves[i]->getVertexCount();
	}
	//leafVerticesCount = vCnt;
	// get sizes
	int * sizes = new int [DYN_TREE::COUNT];
	for (i = 0; i < DYN_TREE::COUNT; i++){
		sizes[i]=0;
	}
	sizes[DYN_TREE::POSITION	]	= 3;
	sizes[DYN_TREE::NORMAL		]	= 3;
	sizes[DYN_TREE::TANGENT		]	= 3;
	sizes[DYN_TREE::TEXCOORD0	]	= 2;
	sizes[DYN_TREE::BRANCH_INDEX]	= 1;
	sizes[DYN_TREE::WEIGHT		]	= 4;

	int totalVertexSize = 0;

	for (i = 0; i < DYN_TREE::COUNT; i++){
		vbo_data[i]= new GLfloat[vCnt*sizes[i]];
		totalVertexSize+=sizes[i];
	}

	// fill arrays
	int k;
	int indexPtr = 0, dataPtr=0, v=0, id=0;
	int indexOffset = 0;
	int index = 0;
	Vertex *vertex;
	for (i=0; i<sizeL; i++) // each leaf 
	{
		leaf = leaves[i];
		id = leaf->parentID;
		for (k=0; k<leaf->vertices.size(); k++){
			vertex = leaf->vertices[k];
			// position
			index = DYN_TREE::VERTEX_ATTRIBUTES::POSITION;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->bPos.data[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->bPos.data[ 1 ];
			vbo_data[index][dataPtr*sizes[index] + 2] = vertex->bPos.data[ 2 ];

			// normal
			index = DYN_TREE::VERTEX_ATTRIBUTES::NORMAL;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->normal.data[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->normal.data[ 1 ];
			vbo_data[index][dataPtr*sizes[index] + 2] = vertex->normal.data[ 2 ];

			// tangent
			index = DYN_TREE::VERTEX_ATTRIBUTES::TANGENT;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->tangent.data[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->tangent.data[ 1 ];
			vbo_data[index][dataPtr*sizes[index] + 2] = vertex->tangent.data[ 2 ];

			// texcoord0
			index = DYN_TREE::VERTEX_ATTRIBUTES::TEXCOORD0;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->textureCoords.data[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->textureCoords.data[ 1 ];

			// xvals
			index = DYN_TREE::VERTEX_ATTRIBUTES::WEIGHT;
			vbo_data[index][dataPtr*sizes[index] + 0] = vertex->x[ 0 ];
			vbo_data[index][dataPtr*sizes[index] + 1] = vertex->x[ 1 ];
			vbo_data[index][dataPtr*sizes[index] + 2] = vertex->x[ 2 ];
			vbo_data[index][dataPtr*sizes[index] + 3] = vertex->x[ 3 ];

			// branch index
			index = DYN_TREE::VERTEX_ATTRIBUTES::BRANCH_INDEX;
			vbo_data[index][dataPtr + 0] = id+0.5f;

			dataPtr++;
		}
	}

	// create VBO
	leavesVBO = new VBO();
	leavesVBO->setVertexCount(vCnt);
	// position
	VBODataSet * dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::POSITION],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::POSITION]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::POSITION,
		true
		);
	leavesVBO->addVertexAttribute( dataSet );
	// normal
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::NORMAL],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::NORMAL]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::NORMAL,
		false
		);
	leavesVBO->addVertexAttribute( dataSet );

	// tangent
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::TANGENT],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::TANGENT]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TANGENT,
		false
		);
	leavesVBO->addVertexAttribute( dataSet );

	// texCoord0
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::TEXCOORD0],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::TEXCOORD0]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TEXCOORD0,
		false
		);
	leavesVBO->addVertexAttribute( dataSet );

	// weights
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::WEIGHT],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::WEIGHT]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::WEIGHT,
		false
		);	
	leavesVBO->addVertexAttribute( dataSet );

	// branch index
	dataSet = new VBODataSet(
		vbo_data[DYN_TREE::VERTEX_ATTRIBUTES::BRANCH_INDEX],
		sizes[DYN_TREE::VERTEX_ATTRIBUTES::BRANCH_INDEX]*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::BRANCH_ID,
		false
		);	
	leavesVBO->addVertexAttribute( dataSet );

	leavesVBO->compileData(GL_STATIC_DRAW);

}

void DTree::createDataTexture()
{
	// linearize structure
	texDimX = linearizeHierarchy();
	int branch_count = texDimX;
	this->branchCountF = branch_count;
	texDimY = 18; // num of data rows  [4 floats]

	recalcCoordSystems();

	fillParentDataForEachBranch();

	DTreeBranch * b;
	int channels = 4, ch, bh;
	float *data = new float[texDimX*texDimY*channels];
	int k;
	for (int i=0; i<texDimX*texDimY*channels; i=i+texDimY*channels){
		k=0;
		b = branches[i/(texDimY*channels)];
		for (ch = 0; ch<channels; ch++){
			data[i + k*channels + ch] = 0.0f;
		}
		// motion vectors
		k = 0;
		data[i + k*channels + 0] = b->motionVectors[0].x;
		data[i + k*channels + 1] = b->motionVectors[0].y;
		data[i + k*channels + 2] = b->motionVectors[1].x;
		data[i + k*channels + 3] = b->motionVectors[1].y;
		k = 1;
		data[i + k*channels + 0] = b->motionVectors[2].x;
		data[i + k*channels + 1] = b->motionVectors[2].y;
		data[i + k*channels + 2] = b->motionVectors[3].x;
		data[i + k*channels + 3] = b->motionVectors[3].y;

		// up0_l0 - up2_l2
		k = 2;
		for (bh = 0;bh < DYN_TREE::MAX_HIERARCHY_DEPTH; bh++){
			data[i + k*channels + 0] = b->upVectors[bh].x;
			data[i + k*channels + 1] = b->upVectors[bh].y;
			data[i + k*channels + 2] = b->upVectors[bh].z;
			data[i + k*channels + 3] = b->lengths[bh];
			k++;
		}
		// right0_x0 - right2_x2
		for (bh = 0;bh < DYN_TREE::MAX_HIERARCHY_DEPTH; bh++){
			data[i + k*channels + 0] = b->rightVectors[bh].x;
			data[i + k*channels + 1] = b->rightVectors[bh].y;
			data[i + k*channels + 2] = b->rightVectors[bh].z;
			data[i + k*channels + 3] = b->xvals[bh];
			k++;
		}
		// t0_x0 - t2_x2
		for (bh = 0;bh < DYN_TREE::MAX_HIERARCHY_DEPTH; bh++){
			data[i + k*channels + 0] = b->tVectors[bh].x;
			data[i + k*channels + 1] = b->tVectors[bh].y;
			data[i + k*channels + 2] = b->tVectors[bh].z;
			data[i + k*channels + 3] = 1.0f;
			k++;
		}
		// center1-center3
		for (bh = 0;bh < DYN_TREE::MAX_HIERARCHY_DEPTH; bh++){
			data[i + k*channels + 0] = b->origins[bh].x;
			data[i + k*channels + 1] = b->origins[bh].y;
			data[i + k*channels + 2] = b->origins[bh].z;
			data[i + k*channels + 3] = 1.0f;
			k++;
		}
	}
	dataTexture = new Texture(  
		GL_TEXTURE_2D,
		GL_RGBA32F,
		GL_RGBA,
		GL_FLOAT,
		data,
		texDimY, 
		texDimX,
		DYN_TREE::DATA_TEXTURE_NAME
		);
	delete [] data;
	data = NULL;
}

bool DTree::loadDataTexture(string filename)
{
	return false;
}

bool DTree::saveDataTexture(string filename)
{
	return false;
}

bool DTree::loadVBO(string filename)
{
	return false;
}

bool DTree::saveVBO(string filename)
{
	return false;
}


int DTree::linearizeHierarchy(){

	stack<DTreeBranch*> bStack;	
	bStack.push(trunk);
	DTreeBranch * branch, *actBranch;
	int cnt = 0, i;
	while( !bStack.empty() ){
		branch = bStack.top();
		branch->id = cnt;
		cnt++;
		bStack.pop();
		branches.push_back(branch);
		for (i=0; i<branch->children.size(); i++){
			if (! branch->children[i]->isLeaf()){
				actBranch = (DTreeBranch*)(branch->children[i]);
				bStack.push(actBranch);
			} else {
				//leaves.push_back((TreeLeaf*) branch->children[i] );
			}
		}		
	}
	return cnt;
};

void DTree::recalcCoordSystems()

{
	/*
	*	Express original coord system in terms of parent coord system...
	*/

	int i, size=branches.size();
	DTreeBranch * branch;
	for (i=0; i<size; i++){
		branch = branches[i];
		if (branch->parent!=NULL){
			branch->cs = branch->parent->originalCS.getSystemInThisSystem(branch->originalCS);
		}
	}

}

void DTree::fillParentDataForEachBranch()
{
	/*
	*	Propagate data from parents to all children, grandchildren, etc.
	*/
	// and copy from parent...
	DTreeBranch* actBranch, *branch;
	CoordSystem *cSys;
	int i, size=branches.size();
	for (i=0; i<size; i++){
		branch = branches[i];
		actBranch = branch;
		while (actBranch!=NULL){
			cSys = &(actBranch->cs);
			branch->xvals.data		[actBranch->level] = actBranch->x;
			branch->lengths.data	[actBranch->level] = actBranch->L;
			branch->motionVectors[actBranch->level]= actBranch->motionVector;
			if (actBranch->parent!=NULL){
				branch->origins		[actBranch->level] = cSys->origin;
				branch->upVectors	[actBranch->level] = cSys->r;
				branch->rightVectors[actBranch->level] = cSys->s;
				branch->tVectors    [actBranch->level] = cSys->t;
				actBranch = (DTreeBranch*)(actBranch->parent);

			} else {
				branch->upVectors	[actBranch->level] = cSys->r;
				branch->rightVectors[actBranch->level] = cSys->s;
				branch->tVectors    [actBranch->level] = cSys->t;
				actBranch = NULL;
			}
		}
	}
}


Vegetation* DTree::getCopy(){
	DTree * copy = new DTree(this);
	return copy;
}

void DTree::draw_instance_LOD0(DTreeInstanceData * instance, float alpha){
	//drawNormals(instance);
	
	if (g_draw_lod0){
		if (g_Draw2Shadowmap){
			time_offset = instance->time_offset;
			glColor4f(1.0,1.0,1.0, alpha);
		
			glDisable(GL_CULL_FACE);
			glPushMatrix();
			glTranslatef(instance->position.x, instance->position.y, instance->position.z);
			glRotatef(instance->rotation_y+180, 0.0, 1.0, 0.0);
			glScalef( 10.f , -10.f, 10.f);
	
			// draw bbox
			//bbox->draw();


			// bind textures
			
			dataTexture->bind(GL_TEXTURE1);
			branchNoiseTexture->bind(GL_TEXTURE2);
			bColorTexture->bind(GL_TEXTURE4);

			// TODO: use positions
			// draw branches
			branchesEBO->draw(branchShader_sh);

			bColorTexture->unbind();

			// bind textures
			leafNoiseTexture	->bind(GL_TEXTURE3);
			frontDecalMap		->bind(GL_TEXTURE4);
			seasonMap			->bind(GL_TEXTURE12);
		
			// TODO: send instance attributes
			leafShader_sh->use(true);
			// draw leaves
			leavesVBO->draw(leafShader_sh, GL_QUADS, 0);
			leafShader_sh->use(false);
			leafNoiseTexture	->unbind();
			frontDecalMap		->unbind();
			branchNoiseTexture	->unbind();
			dataTexture			->unbind();
			seasonMap			->unbind();

			glPopMatrix();
			glEnable(GL_CULL_FACE);
		
		} else {
			time_offset = instance->time_offset;
			glColor4f(1.0,1.0,1.0, alpha);
		
			glDisable(GL_CULL_FACE);
			glPushMatrix();
			glTranslatef(instance->position.x, instance->position.y, instance->position.z);
			glRotatef(instance->rotation_y+180, 0.0, 1.0, 0.0);
			glScalef( 10.f , -10.f, 10.f);
	
			// draw bbox
			//bbox->draw();


			// bind textures
			g_shadowmap1->bind(GL_TEXTURE0);
			dataTexture->bind(GL_TEXTURE1);
			branchNoiseTexture->bind(GL_TEXTURE2);
			bColorTexture->bind(GL_TEXTURE4);

			// TODO: use positions
			// draw branches
			branchesEBO->draw(branchShader);

			bColorTexture->unbind();

			// bind textures
			leafNoiseTexture->bind(GL_TEXTURE3);
			frontDecalMap		->bind(GL_TEXTURE4);
			seasonMap			->bind(GL_TEXTURE12);
			frontNormalMap		->bind(GL_TEXTURE5);
			frontTranslucencyMap->bind(GL_TEXTURE6);
			frontHalfLife2Map	->bind(GL_TEXTURE7);
			backDecalMap		->bind(GL_TEXTURE8);
			backNormalMap		->bind(GL_TEXTURE9);
			backTranslucencyMap	->bind(GL_TEXTURE10);
			backHalfLife2Map	->bind(GL_TEXTURE11);
		
			// TODO: send instance attributes
			leafShader->use(true);
			leafShader->setUniform3f(iu0Loc1, instance->colorVariance.r,instance->colorVariance.g, instance->colorVariance.b);

			// draw leaves
			leavesVBO->draw(leafShader, GL_QUADS, 0);

			frontNormalMap		->unbind();
			frontTranslucencyMap->unbind();
			frontHalfLife2Map	->unbind();
			backDecalMap		->unbind();
			backNormalMap		->unbind();
			backTranslucencyMap	->unbind();
			backHalfLife2Map	->unbind();
		
			leafNoiseTexture	->unbind();
			frontDecalMap		->unbind();
			branchNoiseTexture	->unbind();
			dataTexture			->unbind();
			seasonMap			->unbind();
			g_shadowmap1		->unbind();
			leafShader->use(false);
			glPopMatrix();
			glEnable(GL_CULL_FACE);
			
		}
	}
}

void DTree::draw_instance_LOD1(DTreeInstanceData * instance, float alpha){
	if (g_draw_lod1){
		if (g_Draw2Shadowmap){
			time_offset = instance->time_offset;
			//printf("draw LOD1 instance\n");
			glColor4f(1.0,1.0,1.0, alpha);
			glPushMatrix();
				glTranslatef(instance->position.x, instance->position.y, instance->position.z);
				glRotatef(instance->rotation_y, 0.0, 1.0, 0.0);	
				glDisable(GL_CULL_FACE);
				glDisable(GL_LIGHTING);

				jColorMap			->bind(GL_TEXTURE1);
				leafNoiseTexture	->bind(GL_TEXTURE2);
				branchNoiseTexture  ->bind(GL_TEXTURE3);
				jDataMap			->bind(GL_TEXTURE4);
				jNormalMap			->bind(GL_TEXTURE5);
				jDepthMap			->bind(GL_TEXTURE7);
				seasonMap			->bind(GL_TEXTURE6);
				lod1shader_shadow->use(true);
				lod1vbo2->bind(lod1shader_shadow);
				// set attribute
				// draw EBO...
				eboLOD1->bind();
			
					int off = instance->offset*(3*3*4*sizeof(unsigned int));
					// draw ebo
					eboLOD1->draw(GL_UNSIGNED_INT, GL_QUADS, 3*3*4, BUFFER_OFFSET(off));  

					// disable all...
					eboLOD1->unbind();
					lod1vbo2->unbind(lod1shader_shadow);
			
				jColorMap			->unbind();
				leafNoiseTexture	->unbind();
				branchNoiseTexture	->unbind();
				jDataMap			->unbind();
				jNormalMap			->unbind();
				jDepthMap			->unbind();
				seasonMap			->unbind();
					// turn shader off
				lod1shader_shadow->use(false);
				glUseProgram(0);	
				glEnable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
			
			glPopMatrix();


		} else {
			time_offset = instance->time_offset;
			//printf("draw LOD1 instance\n");
			glColor4f(1.0,1.0,1.0, alpha);
			glPushMatrix();
				glTranslatef(instance->position.x, instance->position.y, instance->position.z);
				glRotatef(instance->rotation_y, 0.0, 1.0, 0.0);

				Texture * colorTexture, * dataTexture, *displacementTexture, *displacement2Texture, *normalTexture;
				displacementTexture		=	leafNoiseTexture;
				displacement2Texture	=	branchNoiseTexture;
					
				glDisable(GL_CULL_FACE);
				glDisable(GL_LIGHTING);

				g_shadowmap1		->bind(GL_TEXTURE0);
				jColorMap			->bind(GL_TEXTURE1);
				displacementTexture	->bind(GL_TEXTURE2);
				displacement2Texture->bind(GL_TEXTURE3);
				jDataMap			->bind(GL_TEXTURE4);
				jNormalMap			->bind(GL_TEXTURE5);
				jDepthMap			->bind(GL_TEXTURE7);
				seasonMap			->bind(GL_TEXTURE6);
				lod1shader2->use(true);
			
			

				lod1shader2->setTexture(l2_season	, seasonMap				->textureUnitNumber	);
			
				lod1shader2->setTexture(l2_color	, jColorMap				->textureUnitNumber	);
				lod1shader2->setTexture(l2_displ	, displacementTexture	->textureUnitNumber	);			
				lod1shader2->setTexture(l2_displ2	, displacement2Texture	->textureUnitNumber	);
				lod1shader2->setTexture(l2_data		, jDataMap				->textureUnitNumber	);
				lod1shader2->setTexture(l2_normal	, jNormalMap			->textureUnitNumber	);
				lod1shader2->setTexture(l2_depth	, jDepthMap				->textureUnitNumber	);
			
				lod1shader2->setUniform3f(iu1Loc1, instance->colorVariance.r, instance->colorVariance.g, instance->colorVariance.b);
				lod1vbo2->bind(lod1shader2);
				// set attribute
				// draw EBO...
				eboLOD1->bind();
			
					int off = instance->offset*(3*3*4*sizeof(unsigned int));
					// draw ebo
					eboLOD1->draw(GL_UNSIGNED_INT, GL_QUADS, 3*3*4, BUFFER_OFFSET(off));  

					// disable all...
					eboLOD1->unbind();
					lod1vbo2->unbind(lod1shader2);
			
				jColorMap			->unbind();
				displacementTexture	->unbind();
				displacement2Texture->unbind();
				jDataMap			->unbind();
				jNormalMap			->unbind();
				jDepthMap			->unbind();
				seasonMap			->unbind();
				g_shadowmap1		->unbind();
					// turn shader off
				lod1shader2->use(false);
				glUseProgram(0);	
				glEnable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
			
			glPopMatrix();
		} // if not draw to shadow map...
	}
}

void DTree::draw_all_instances_LOD1(){
	g_transitionControl = 1.0;
	if (g_draw_lod1){
		if (g_Draw2Shadowmap){			
			glColor4f(1.0, 1.0, 1.0, 1.0);	
			int i, j, sliceCount, setCount=sliceSets2.size();

			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
				jColorMap			->bind(GL_TEXTURE1);
				leafNoiseTexture	->bind(GL_TEXTURE2);
				branchNoiseTexture	->bind(GL_TEXTURE3);
				jDataMap			->bind(GL_TEXTURE4);
				jNormalMap			->bind(GL_TEXTURE5);
				seasonMap			->bind(GL_TEXTURE6);
				jDepthMap			->bind(GL_TEXTURE7);

			lod1shader_shadow->use(true);
			
				//lod1shader2->setTexture(l2_color	, jColorMap				->textureUnitNumber	);
				//lod1shader2->setTexture(l2_displ	, displacementTexture	->textureUnitNumber	);			
				//lod1shader2->setTexture(l2_displ2	, displacement2Texture	->textureUnitNumber	);
				//lod1shader2->setTexture(l2_data		, jDataMap				->textureUnitNumber	);
				//lod1shader2->setTexture(l2_normal	, jNormalMap			->textureUnitNumber	);
				//lod1shader2->setTexture(l2_season	, seasonMap				->textureUnitNumber	);
				//lod1shader2->setTexture(l2_depth	, jDepthMap				->textureUnitNumber	);
			
				// bind element buffer
				eboLOD1->bind();
				// bind vertex attribute buffer
				lod1vbo2->bind(lod1shader_shadow);	

				// instance matrices VBO


				// bind instance data
				glBindBuffer(GL_ARRAY_BUFFER, i_matricesBuffID);
				glEnableVertexAttribArray(tmLoc0_shadow);
				glEnableVertexAttribArray(tmLoc0_shadow + 1);
				glEnableVertexAttribArray(tmLoc0_shadow + 2);
				glEnableVertexAttribArray(tmLoc0_shadow + 3);
				
				if (iaLoc1_shadow>=0){
					glEnableVertexAttribArray(iaLoc1_shadow);
					glVertexAttribPointer(iaLoc1_shadow, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 16));
					glVertexAttribDivisor(iaLoc1_shadow, 1);
				}
				
				glVertexAttribPointer(tmLoc0_shadow, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(0));
				glVertexAttribPointer(tmLoc0_shadow + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 4));
				glVertexAttribPointer(tmLoc0_shadow + 2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 8));
				glVertexAttribPointer(tmLoc0_shadow + 3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 12));
				// color variations, time_offset
			
				glVertexAttribDivisor(tmLoc0_shadow, 1);
				glVertexAttribDivisor(tmLoc0_shadow + 1, 1);
				glVertexAttribDivisor(tmLoc0_shadow + 2, 1);
				glVertexAttribDivisor(tmLoc0_shadow + 3, 1);
			
				for (int i=0; i<lod1_instanceMatrices.size(); i++){
					// transfer data to buffer
					glBufferData(GL_ARRAY_BUFFER, lod1_typeIndices[i] * instanceFloatCount * sizeof(float), lod1_instanceMatrices[i], GL_STREAM_DRAW);
					int off = i*(3*3*4*sizeof(unsigned int));
					void * offset = BUFFER_OFFSET(off);
					// draw instanced
					eboLOD1->drawInstanced(GL_QUADS, 3*3*4, GL_UNSIGNED_INT, offset, lod1_typeIndices[i]);  

				} // for each configuration
			
				glVertexAttribDivisor(tmLoc0_shadow, 0);
				glVertexAttribDivisor(tmLoc0_shadow + 1, 0);
				glVertexAttribDivisor(tmLoc0_shadow + 2, 0);
				glVertexAttribDivisor(tmLoc0_shadow + 3, 0);
				// disable all...
				
				if (iaLoc1_shadow>=0){
					glVertexAttribDivisor(iaLoc1_shadow, 0);
					glDisableVertexAttribArray(iaLoc1_shadow);
				}
				
				glDisableVertexAttribArray(tmLoc0_shadow);
				glDisableVertexAttribArray(tmLoc0_shadow + 1);
				glDisableVertexAttribArray(tmLoc0_shadow + 2);
				glDisableVertexAttribArray(tmLoc0_shadow + 3);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				eboLOD1->unbind();
				lod1vbo2->unbind(lod1shader_shadow);
			
				jColorMap			->unbind();
				leafNoiseTexture	->unbind();
				branchNoiseTexture	->unbind();
				jDataMap			->unbind();
				jNormalMap			->unbind();
				jDepthMap			->unbind();
				seasonMap			->unbind();
				// turn shader off
			lod1shader_shadow->use(false);
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
		} else {
			glColor4f(1.0, 1.0, 1.0, 1.0);	
			int i, j, sliceCount, setCount=sliceSets2.size();
		
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);

				g_shadowmap1		->bind(GL_TEXTURE0);
				jColorMap			->bind(GL_TEXTURE1);
				leafNoiseTexture	->bind(GL_TEXTURE2);
				branchNoiseTexture	->bind(GL_TEXTURE3);
				jDataMap			->bind(GL_TEXTURE4);
				jNormalMap			->bind(GL_TEXTURE5);
				seasonMap			->bind(GL_TEXTURE6);
				jDepthMap			->bind(GL_TEXTURE7);

			lod1shader2->use(true);
			
				lod1shader2->setTexture(l2_color	, jColorMap				->textureUnitNumber	);
				lod1shader2->setTexture(l2_displ	, leafNoiseTexture		->textureUnitNumber	);			
				lod1shader2->setTexture(l2_displ2	, branchNoiseTexture	->textureUnitNumber	);
				lod1shader2->setTexture(l2_data		, jDataMap				->textureUnitNumber	);
				lod1shader2->setTexture(l2_normal	, jNormalMap			->textureUnitNumber	);
				lod1shader2->setTexture(l2_season	, seasonMap				->textureUnitNumber	);
				lod1shader2->setTexture(l2_depth	, jDepthMap				->textureUnitNumber	);
			
				// bind element buffer
				eboLOD1->bind();
				// bind vertex attribute buffer
				lod1vbo2->bind(lod1shader2);	

				// instance matrices VBO


				// bind instance data
				glBindBuffer(GL_ARRAY_BUFFER, i_matricesBuffID);
				glEnableVertexAttribArray(tmLoc0);
				glEnableVertexAttribArray(tmLoc0 + 1);
				glEnableVertexAttribArray(tmLoc0 + 2);
				glEnableVertexAttribArray(tmLoc0 + 3);
				if (iaLoc1>=0){
					glEnableVertexAttribArray(iaLoc1);
					glVertexAttribPointer(iaLoc1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 16));
					glVertexAttribDivisor(iaLoc1, 1);
				}
				glVertexAttribPointer(tmLoc0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(0));
				glVertexAttribPointer(tmLoc0 + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 4));
				glVertexAttribPointer(tmLoc0 + 2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 8));
				glVertexAttribPointer(tmLoc0 + 3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 12));
				// color variations, time_offset
			
				glVertexAttribDivisor(tmLoc0, 1);
				glVertexAttribDivisor(tmLoc0 + 1, 1);
				glVertexAttribDivisor(tmLoc0 + 2, 1);
				glVertexAttribDivisor(tmLoc0 + 3, 1);
			
				for (int i=0; i<lod1_instanceMatrices.size(); i++){
					// transfer data to buffer
					glBufferData(GL_ARRAY_BUFFER, lod1_typeIndices[i] * instanceFloatCount * sizeof(float), lod1_instanceMatrices[i], GL_STREAM_DRAW);
					int off = i*(3*3*4*sizeof(unsigned int));
					void * offset = BUFFER_OFFSET(off);
					// draw instanced
					eboLOD1->drawInstanced(GL_QUADS, 3*3*4, GL_UNSIGNED_INT, offset, lod1_typeIndices[i]);  

				} // for each configuration
			
				glVertexAttribDivisor(tmLoc0, 0);
				glVertexAttribDivisor(tmLoc0 + 1, 0);
				glVertexAttribDivisor(tmLoc0 + 2, 0);
				glVertexAttribDivisor(tmLoc0 + 3, 0);
				// disable all...
				if (iaLoc1>=0){
					glVertexAttribDivisor(iaLoc1, 0);
					glDisableVertexAttribArray(iaLoc1);
				}
				glDisableVertexAttribArray(tmLoc0);
				glDisableVertexAttribArray(tmLoc0 + 1);
				glDisableVertexAttribArray(tmLoc0 + 2);
				glDisableVertexAttribArray(tmLoc0 + 3);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				eboLOD1->unbind();
				lod1vbo2->unbind(lod1shader2);
			
				jColorMap			->unbind();
				leafNoiseTexture	->unbind();
				branchNoiseTexture	->unbind();
				jDataMap			->unbind();
				jNormalMap			->unbind();
				jDepthMap			->unbind();
				seasonMap			->unbind();
				g_shadowmap1        ->unbind();
				// turn shader off
			lod1shader2->use(false);
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
		} // if not drawing to shadow map
	}
}

void DTree::draw_instance_LOD2(DTreeInstanceData * instance, float alpha){
	if (g_draw_lod2){
		
		if (g_Draw2Shadowmap){
			time_offset = instance->time_offset;
			//printf("draw LOD1 instance\n");
			glColor4f(1.0,1.0,1.0, alpha);
			glPushMatrix();
				glTranslatef(instance->position.x, instance->position.y, instance->position.z);
				glRotatef(instance->rotation_y, 0.0, 1.0, 0.0);

				glDisable(GL_CULL_FACE);
				glDisable(GL_LIGHTING);	
				leafNoiseTexture	->bind(GL_TEXTURE7);
				branchNoiseTexture  ->bind(GL_TEXTURE8);
				seasonMap			->bind(GL_TEXTURE6);
				lod2color1			->bind(GL_TEXTURE0);
				lod2color2			->bind(GL_TEXTURE1);
				lod2normal1			->bind(GL_TEXTURE2);
				lod2normal2			->bind(GL_TEXTURE3);
				lod2branch1			->bind(GL_TEXTURE4);
				lod2branch2			->bind(GL_TEXTURE5);
				lod2depth1			->bind(GL_TEXTURE10);
				lod2depth2			->bind(GL_TEXTURE11);
			
				lod2shader_shadow->use(true);
				lod2vbo->bind(lod2shader_shadow);
				// set attribute
				// draw EBO...
				lod2ebo->bind();
			
					int off = instance->offset*(2*4*sizeof(unsigned int));
					// draw ebo
					lod2ebo->draw(GL_UNSIGNED_INT, GL_QUADS, 2*4, BUFFER_OFFSET(off));  

					// disable all...
					lod2ebo->unbind();
					lod2vbo->unbind(lod2shader_shadow);
				leafNoiseTexture	->unbind();
				branchNoiseTexture	->unbind();
				lod2color1			->unbind();
				lod2color2			->unbind();
				lod2normal1			->unbind();
				lod2normal2			->unbind();
				lod2branch1			->unbind();
				lod2branch2			->unbind();
					// turn shader off
				lod2shader_shadow->use(false);
				glUseProgram(0);	
				glEnable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);			
			glPopMatrix();
		} else {
			time_offset = instance->time_offset;
			//printf("draw LOD1 instance\n");
			glColor4f(1.0,1.0,1.0, alpha);
			glPushMatrix();
				glTranslatef(instance->position.x, instance->position.y, instance->position.z);
				glRotatef(instance->rotation_y, 0.0, 1.0, 0.0);

				glDisable(GL_CULL_FACE);
				glDisable(GL_LIGHTING);
				g_shadowmap1		->bind(GL_TEXTURE9);			
				leafNoiseTexture	->bind(GL_TEXTURE7);
				branchNoiseTexture  ->bind(GL_TEXTURE8);
				seasonMap			->bind(GL_TEXTURE6);
				lod2color1			->bind(GL_TEXTURE0);
				lod2color2			->bind(GL_TEXTURE1);
				lod2normal1			->bind(GL_TEXTURE2);
				lod2normal2			->bind(GL_TEXTURE3);
				lod2branch1			->bind(GL_TEXTURE4);
				lod2branch2			->bind(GL_TEXTURE5);
				lod2depth1			->bind(GL_TEXTURE10);
				lod2depth2			->bind(GL_TEXTURE11);
			
				lod2shader->use(true);
				lod2shader->setUniform3f(lod2loc_colorVariance, instance->colorVariance.r, instance->colorVariance.g, instance->colorVariance.b);
				lod2vbo->bind(lod2shader);
				// set attribute
				// draw EBO...
				lod2ebo->bind();
			
					int off = instance->offset*(2*4*sizeof(unsigned int));
					// draw ebo
					lod2ebo->draw(GL_UNSIGNED_INT, GL_QUADS, 2*4, BUFFER_OFFSET(off));  

					// disable all...
					lod2ebo->unbind();
					lod2vbo->unbind(lod2shader);
				leafNoiseTexture	->unbind();
				branchNoiseTexture	->unbind();
				lod2color1			->unbind();
				lod2color2			->unbind();
				lod2normal1			->unbind();
				lod2normal2			->unbind();
				lod2branch1			->unbind();
				lod2branch2			->unbind();
				g_shadowmap1		->unbind();
					// turn shader off
				lod2shader->use(false);
				glUseProgram(0);	
				glEnable(GL_CULL_FACE);
				glEnable(GL_LIGHTING);
			
			glPopMatrix();
		} // if not drawing to shadowmap
	}

}

void DTree::draw_all_instances_LOD2(){
	if (g_draw_lod2){		
		if (g_Draw2Shadowmap){
			glColor4f(1.0, 1.0, 1.0, 1.0);	
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
				leafNoiseTexture	->bind(GL_TEXTURE7);
				branchNoiseTexture  ->bind(GL_TEXTURE8);
				seasonMap			->bind(GL_TEXTURE6);
				lod2color1			->bind(GL_TEXTURE0);
				lod2color2			->bind(GL_TEXTURE1);
				lod2normal1			->bind(GL_TEXTURE2);
				lod2normal2			->bind(GL_TEXTURE3);
				lod2branch1			->bind(GL_TEXTURE4);
				lod2branch2			->bind(GL_TEXTURE5);
				lod2depth1			->bind(GL_TEXTURE10);
				lod2depth2			->bind(GL_TEXTURE11);

			lod2shader_shadow->use(true);
			

				//lod2shader->setTexture(lod2loc_color_tex_1		, lod2color1			->textureUnitNumber	);
				//lod2shader->setTexture(lod2loc_color_tex_2		, lod2color2			->textureUnitNumber	);
				//lod2shader->setTexture(lod2loc_normal_tex_1		, lod2normal1			->textureUnitNumber	);
				//lod2shader->setTexture(lod2loc_normal_tex_2		, lod2normal2			->textureUnitNumber	);
				//lod2shader->setTexture(lod2loc_branch_tex_1		, lod2branch1			->textureUnitNumber	);
				//lod2shader->setTexture(lod2loc_branch_tex_2		, lod2branch2			->textureUnitNumber	);
				//
				//
				//lod2shader->setTexture(lod2loc_leaf_tex		, leafNoiseTexture		->textureUnitNumber	);	
				//lod2shader->setTexture(lod2loc_branch_tex	, branchNoiseTexture	->textureUnitNumber	);
				//lod2shader->setTexture(lod2loc_season_tex	, seasonMap				->textureUnitNumber	);
				// bind element buffer
				lod2ebo->bind();
				// bind vertex attribute buffer
				lod2vbo->bind(lod2shader_shadow);	

				// instance matrices VBO


				// bind instance data
				glBindBuffer(GL_ARRAY_BUFFER, i_matricesBuffID);
				glEnableVertexAttribArray(tm2Loc0_shadow);
				glEnableVertexAttribArray(tm2Loc0_shadow + 1);
				glEnableVertexAttribArray(tm2Loc0_shadow + 2);
				glEnableVertexAttribArray(tm2Loc0_shadow + 3);
				glEnableVertexAttribArray(ia2Loc1_shadow);
				glVertexAttribPointer(tm2Loc0_shadow, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(0));
				glVertexAttribPointer(tm2Loc0_shadow + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 4));
				glVertexAttribPointer(tm2Loc0_shadow + 2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 8));
				glVertexAttribPointer(tm2Loc0_shadow + 3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 12));
				// color variations, time_offset
				glVertexAttribPointer(ia2Loc1_shadow, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 16));

				glVertexAttribDivisor(tm2Loc0_shadow, 1);
				glVertexAttribDivisor(tm2Loc0_shadow + 1, 1);
				glVertexAttribDivisor(tm2Loc0_shadow + 2, 1);
				glVertexAttribDivisor(tm2Loc0_shadow + 3, 1);
				glVertexAttribDivisor(ia2Loc1_shadow, 1);
				for (int i=0; i<lod2_instanceMatrices.size(); i++){
					// transfer data to buffer
					glBufferData(GL_ARRAY_BUFFER, lod2_typeIndices[i] * instanceFloatCount * sizeof(float), lod2_instanceMatrices[i], GL_STREAM_DRAW);
					int off = i*(2*4*sizeof(unsigned int));
					void * offset = BUFFER_OFFSET(off);
					// draw instanced
					lod2ebo->drawInstanced(GL_QUADS, 2*4, GL_UNSIGNED_INT, offset, lod2_typeIndices[i]);  

				} // for each configuration
				glVertexAttribDivisor(ia2Loc1_shadow, 0);
				glVertexAttribDivisor(tm2Loc0_shadow, 0);
				glVertexAttribDivisor(tm2Loc0_shadow + 1, 0);
				glVertexAttribDivisor(tm2Loc0_shadow + 2, 0);
				glVertexAttribDivisor(tm2Loc0_shadow + 3, 0);
				// disable all...
				glDisableVertexAttribArray(ia2Loc1_shadow);
				glDisableVertexAttribArray(tm2Loc0_shadow);
				glDisableVertexAttribArray(tm2Loc0_shadow + 1);
				glDisableVertexAttribArray(tm2Loc0_shadow + 2);
				glDisableVertexAttribArray(tm2Loc0_shadow + 3);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				lod2ebo->unbind();
				lod2vbo->unbind(lod2shader_shadow);
			
				leafNoiseTexture	->unbind();
				branchNoiseTexture  ->unbind();
				seasonMap			->unbind();
				lod2color1			->unbind();
				lod2color2			->unbind();
				lod2normal1			->unbind();
				lod2normal2			->unbind();
				lod2branch1			->unbind();
				lod2branch2			->unbind();
				lod2depth1			->unbind();
				lod2depth2			->unbind();
				// turn shader off
			lod2shader_shadow->use(false);
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
		} else {
		
			glColor4f(1.0, 1.0, 1.0, 1.0);	
		
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);

				leafNoiseTexture	->bind(GL_TEXTURE7);
				branchNoiseTexture  ->bind(GL_TEXTURE8);
				seasonMap			->bind(GL_TEXTURE6);
				lod2color1			->bind(GL_TEXTURE0);
				lod2color2			->bind(GL_TEXTURE1);
				lod2normal1			->bind(GL_TEXTURE2);
				lod2normal2			->bind(GL_TEXTURE3);
				lod2branch1			->bind(GL_TEXTURE4);
				lod2branch2			->bind(GL_TEXTURE5);
				g_shadowmap1		->bind(GL_TEXTURE9);

			lod2shader->use(true);
			

				lod2shader->setTexture(lod2loc_color_tex_1		, lod2color1			->textureUnitNumber	);
				lod2shader->setTexture(lod2loc_color_tex_2		, lod2color2			->textureUnitNumber	);
				lod2shader->setTexture(lod2loc_normal_tex_1		, lod2normal1			->textureUnitNumber	);
				lod2shader->setTexture(lod2loc_normal_tex_2		, lod2normal2			->textureUnitNumber	);
				lod2shader->setTexture(lod2loc_branch_tex_1		, lod2branch1			->textureUnitNumber	);
				lod2shader->setTexture(lod2loc_branch_tex_2		, lod2branch2			->textureUnitNumber	);


				lod2shader->setTexture(lod2loc_leaf_tex		, leafNoiseTexture		->textureUnitNumber	);	
				lod2shader->setTexture(lod2loc_branch_tex	, branchNoiseTexture	->textureUnitNumber	);
				lod2shader->setTexture(lod2loc_season_tex	, seasonMap				->textureUnitNumber	);
				// bind element buffer
				lod2ebo->bind();
				// bind vertex attribute buffer
				lod2vbo->bind(lod2shader);	

				// instance matrices VBO


				// bind instance data
				glBindBuffer(GL_ARRAY_BUFFER, i_matricesBuffID);
				glEnableVertexAttribArray(tm2Loc0);
				glEnableVertexAttribArray(tm2Loc0 + 1);
				glEnableVertexAttribArray(tm2Loc0 + 2);
				glEnableVertexAttribArray(tm2Loc0 + 3);
				glEnableVertexAttribArray(ia2Loc1);
				glVertexAttribPointer(tm2Loc0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(0));
				glVertexAttribPointer(tm2Loc0 + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 4));
				glVertexAttribPointer(tm2Loc0 + 2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 8));
				glVertexAttribPointer(tm2Loc0 + 3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 12));
				// color variations, time_offset
				glVertexAttribPointer(ia2Loc1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * instanceFloatCount, (void*)(sizeof(float) * 16));

				glVertexAttribDivisor(tm2Loc0, 1);
				glVertexAttribDivisor(tm2Loc0 + 1, 1);
				glVertexAttribDivisor(tm2Loc0 + 2, 1);
				glVertexAttribDivisor(tm2Loc0 + 3, 1);
				glVertexAttribDivisor(ia2Loc1, 1);
				for (int i=0; i<lod2_instanceMatrices.size(); i++){
					// transfer data to buffer
					glBufferData(GL_ARRAY_BUFFER, lod2_typeIndices[i] * instanceFloatCount * sizeof(float), lod2_instanceMatrices[i], GL_STREAM_DRAW);
					int off = i*(2*4*sizeof(unsigned int));
					void * offset = BUFFER_OFFSET(off);
					// draw instanced
					lod2ebo->drawInstanced(GL_QUADS, 2*4, GL_UNSIGNED_INT, offset, lod2_typeIndices[i]);  

				} // for each configuration
				glVertexAttribDivisor(ia2Loc1, 0);
				glVertexAttribDivisor(tm2Loc0, 0);
				glVertexAttribDivisor(tm2Loc0 + 1, 0);
				glVertexAttribDivisor(tm2Loc0 + 2, 0);
				glVertexAttribDivisor(tm2Loc0 + 3, 0);
				// disable all...
				glDisableVertexAttribArray(ia2Loc1);
				glDisableVertexAttribArray(tm2Loc0);
				glDisableVertexAttribArray(tm2Loc0 + 1);
				glDisableVertexAttribArray(tm2Loc0 + 2);
				glDisableVertexAttribArray(tm2Loc0 + 3);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				lod2ebo->unbind();
				lod2vbo->unbind(lod2shader);
			
				leafNoiseTexture	->unbind();
				branchNoiseTexture  ->unbind();
				seasonMap			->unbind();
				lod2color1			->unbind();
				lod2color2			->unbind();
				lod2normal1			->unbind();
				lod2normal2			->unbind();
				lod2branch1			->unbind();
				lod2branch2			->unbind();
				g_shadowmap1		->unbind();
				// turn shader off
			lod2shader->use(false);
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
		} // if not drawing to shadowmap
	}
}


void DTree::drawLOD0()
{
	if (g_draw_lod0){
		glColor4f(0.0, 0.0,0.0, alpha_c);
		
		glDisable(GL_CULL_FACE);
		glPushMatrix();
		glScalef( 10.f , -10.f, 10.f);
	
		// draw bbox
		//bbox->draw();


		// bind textures
		dataTexture->bind(GL_TEXTURE1);
		branchNoiseTexture->bind(GL_TEXTURE2);
		bColorTexture->bind(GL_TEXTURE4);

		// TODO: use positions

		// draw branches
		branchesEBO->draw(branchShader);

		bColorTexture->unbind();

		// bind textures
		leafNoiseTexture->bind(GL_TEXTURE3);
		frontDecalMap		->bind(GL_TEXTURE4);
		frontNormalMap		->bind(GL_TEXTURE5);
		frontTranslucencyMap->bind(GL_TEXTURE6);
		frontHalfLife2Map	->bind(GL_TEXTURE7);
		backDecalMap		->bind(GL_TEXTURE8);
		backNormalMap		->bind(GL_TEXTURE9);
		backTranslucencyMap	->bind(GL_TEXTURE10);
		backHalfLife2Map	->bind(GL_TEXTURE11);

		// TODO: use positions

		// draw leaves
		leavesVBO->draw(leafShader, GL_QUADS, 0);


		leafNoiseTexture->unbind();
		frontDecalMap		->unbind();
		frontNormalMap		->unbind();
		frontTranslucencyMap->unbind();
		frontHalfLife2Map	->unbind();
		backDecalMap		->unbind();
		backNormalMap		->unbind();
		backTranslucencyMap	->unbind();
		backHalfLife2Map	->unbind();
		branchNoiseTexture->unbind();
		dataTexture->unbind();
		glPopMatrix();
		glEnable(GL_CULL_FACE);
	}
}

void DTree::drawLOD1()
{
	if (g_draw_lod1){
		glColor4f(0.0, 0.0,0.0, alpha_c);
		
		int i, j, sliceCount, setCount=sliceSets.size();
		Texture * colorTexture, * dataTexture, *displacementTexture, *displacement2Texture, *normalTexture;
			displacementTexture		= leafNoiseTexture;
			displacement2Texture	= branchNoiseTexture;
			DTreeSlice * slice;
		for (j = 0; j< setCount; j++){
			sliceCount = sliceSets[j]->size();
			// if discrepacy angle under treshold, DO NOT write in z-buffer
			v3 p = position;
			v3 v = *viewer_position;
			p.y = 0.0;
			v.y = 0.0;
			v3 eyeDir = (p-v).getNormalized();
			v3 normalDir = sliceSets[j]->getNormal();
			normalDir.rotateY(rotationY*DEG_TO_RAD);
			float discrepacy = abs(normalDir.dot(eyeDir));
			//printf("sliceset[%i] DISC: %f\n",j, discrepacy);
			for (i=0; i<sliceCount; i++){
				slice					= sliceSets[j]->getSlice(i);
				colorTexture			= slice->colormap;
				//printf("sliceset: %i, slice: %i is using: glTextureID %i\n", j,i, colorTexture->id);
				normalTexture			= slice->normalmap;
				dataTexture				= slice->datamap;

				glPushMatrix();
					
					//l3dBillboardCheatCylindricalBegin();
			
					glRotatef(sliceSets[j]->rotation_y, 0.0, 1.0, 0.0);
					glTranslatef((i-(float(sliceCount)/2.f) + 0.5), 0.0, 0.0);
					glScalef(10.0,10.0,-10.0);
					//glRotatef(90, 0.0, 0.0, 1.0);
					glRotatef(-90.0, 0.0, 1.0, 0.0);
					glDisable(GL_CULL_FACE);
					glDisable(GL_LIGHTING);
					colorTexture		->bind(GL_TEXTURE0);
					displacementTexture	->bind(GL_TEXTURE1);
					displacement2Texture	->bind(GL_TEXTURE4);
					dataTexture			->bind(GL_TEXTURE2);
					normalTexture		->bind(GL_TEXTURE3);

					u_time_offset->data = &	g_tree_time_offset_1;
					// turn on shader
					lod1shader->use(true);
					lod1shader->setTexture(l_color	, colorTexture			->textureUnitNumber	);
					lod1shader->setTexture(l_displ	, displacementTexture	->textureUnitNumber	);
					lod1shader->setTexture(l_displ2	, displacement2Texture	->textureUnitNumber	);
					lod1shader->setTexture(l_data	, dataTexture			->textureUnitNumber	);
					lod1shader->setTexture(l_normal	, normalTexture			->textureUnitNumber	);
			
				

					// TODO: draw at all positions
					if (discrepacy<0.8){
						glDepthMask(GL_FALSE);
						lod1vbo->draw(lod1shader, GL_QUADS, 0);
						glDepthMask(GL_TRUE);
					} else {
						lod1vbo->draw(lod1shader, GL_QUADS, 0);
					}
				
					colorTexture		->unbind();
					displacementTexture	->unbind();
					displacement2Texture	->unbind();
					dataTexture			->unbind();
					normalTexture		->unbind();
					// turn off shader
					lod1shader->use(false);
					glEnable(GL_CULL_FACE);
					glEnable(GL_LIGHTING);
				glPopMatrix();
			} // for i - each slice
		} // for j - sliceSets
	}
}


void DTree::drawLOD1b()
{
	if (g_draw_lod1){
		glColor4f(0.0, 0.0,0.0, alpha_c);
		g_tree_lod1_count++;
		int i, j, sliceCount, setCount=sliceSets2.size();
		Texture * colorTexture, * dataTexture, *displacementTexture, *displacement2Texture, *normalTexture;
		displacementTexture		=	leafNoiseTexture;
		displacement2Texture	=	branchNoiseTexture;
		
		colorTexture			=	jColorMap;
		dataTexture				=	jDataMap;
		normalTexture			=	jNormalMap;

		glPushMatrix();
					
			//l3dBillboardCheatCylindricalBegin();
			
			//glRotatef(90, 0.0, 1.0, 0.0);
			glDisable(GL_CULL_FACE);
			glDisable(GL_LIGHTING);
			
			/*
			v3 dirA = v3( -1.0, 0.0, 0.0);									// A
			v3 rightA = dirA.getRotated( 90*DEG_TO_RAD, v3(0.0, 1.0, 0.0));
			v3 dirB = dirA.getRotated(  60*DEG_TO_RAD, v3(0.0, 1.0, 0.0));	// B
			v3 rightB = dirB.getRotated( 90*DEG_TO_RAD, v3(0.0, 1.0, 0.0));
			v3 dirC = dirA.getRotated( -60*DEG_TO_RAD, v3(0.0, 1.0, 0.0));	// C
			v3 rightC = dirC.getRotated( 90*DEG_TO_RAD, v3(0.0, 1.0, 0.0));
			float short_l = 0.1, long_l = 0.3;
			glPushMatrix();
				glTranslatef(0.0, 0.1, 0.0);
				glBegin(GL_LINE_STRIP);
					glColor4f(1.0, 0.0,0.0, alpha_c);
					glVertex3f(short_l*rightA.x, short_l*rightA.y, short_l*rightA.z); glVertex3f(0.0, .0, .0); glVertex3f(long_l*dirA.x, long_l*dirA.y, long_l*dirA.z);
				glEnd();
				glBegin(GL_LINE_STRIP);
					glColor4f(0.0, 1.0,0.0, alpha_c);
					glVertex3f(short_l*rightB.x, short_l*rightB.y, short_l*rightB.z); glVertex3f(0.0, .0, .0); glVertex3f(long_l*dirB.x, long_l*dirB.y, long_l*dirB.z);
				glEnd();
				glBegin(GL_LINE_STRIP);
					glColor4f(0.0, 0.0, 1.0, alpha_c);
					glVertex3f(short_l*rightC.x, short_l*rightC.y, short_l*rightC.z); glVertex3f(0.0, .0, .0); glVertex3f(long_l*dirC.x, long_l*dirC.y, long_l*dirC.z);
				glEnd();

				glBegin(GL_LINES);
					glColor4f(1.0, 1.0, 1.0, alpha_c);
					glVertex3f(0.0, 0.0, 0.0); glVertex3f(eyeDir.x,eyeDir.y, eyeDir.z);
				glEnd();
			glPopMatrix();
			*/
			lod1shader2->use(true);
			jColorMap			->bind(GL_TEXTURE1);
			displacementTexture	->bind(GL_TEXTURE2);
			displacement2Texture->bind(GL_TEXTURE3);
			jDataMap			->bind(GL_TEXTURE4);
			jNormalMap			->bind(GL_TEXTURE5);

			//u_time_offset->data = &	g_tree_time_offset_1;
	
			//printf("SETTING TEXTURE[%s] loc:%i, unit id: %i\n", colorTexture->inShaderName.c_str(),l2_color, colorTexture->textureUnitNumber);
			lod1shader2->setTexture(l2_color	, jColorMap				->textureUnitNumber	);
			//printf("SETTING TEXTURE[%s] loc:%i, unit id: %i\n", displacementTexture->inShaderName.c_str(),l2_displ, displacementTexture->textureUnitNumber);
			lod1shader2->setTexture(l2_displ	, displacementTexture	->textureUnitNumber	);			
			//printf("SETTING TEXTURE[%s] loc:%i, unit id: %i\n", displacement2Texture->inShaderName.c_str(),l2_displ2, displacement2Texture->textureUnitNumber);
			lod1shader2->setTexture(l2_displ2	, displacement2Texture	->textureUnitNumber	);
			//printf("SETTING TEXTURE[%s] loc:%i, unit id: %i\n", dataTexture->inShaderName.c_str(),l2_data, dataTexture->textureUnitNumber);
			lod1shader2->setTexture(l2_data		, jDataMap				->textureUnitNumber	);
			//printf("SETTING TEXTURE[%s] loc:%i, unit id: %i\n", normalTexture->inShaderName.c_str(),l2_normal, normalTexture->textureUnitNumber);
			lod1shader2->setTexture(l2_normal	, jNormalMap			->textureUnitNumber	);
			//glDepthMask(GL_FALSE);
			lod1vbo2->bind(lod1shader2);

			// draw EBO...
			eboLOD1->bind();

			// for each instance

			for (int i = 0; i < tree_instances.size(); i++){
				// determine LOD configuration
				position = tree_instances[i]->position;
				rotationY= tree_instances[i]->rotation_y;
				glPushMatrix();
					glTranslate(position);
					glRotatef(rotationY, 0.0, 1.0 ,0.0);
					glScalef(10.0, 10.0, 10.0);
					v3 p = position;
					v3 v = *viewer_position;
					p.y = 0.0;
					v.y = 0.0;
					v3 eyeDir = -(p-v).getNormalized();	
					v3 dirA = v3( -1.0, 0.0, 0.0).getRotated( (rotationY)*DEG_TO_RAD, v3(0.0, 1.0, 0.0));	// A									// A
					v3 dirB = dirA.getRotated( 60 * DEG_TO_RAD, v3(0.0, 1.0, 0.0));							// B
					//v3 dirC = dirA.getRotated( -60 * DEG_TO_RAD, v3(0.0, 1.0, 0.0));						// C
				
					float cosA = dirA.dot(eyeDir);
					float cosB = dirB.dot(eyeDir);
					//float cosC = dirC.dot(eyeDir);
					int o = 1; 
					if (cosA<= -0.86 || cosA>=0.86){
						o = 0;
					} else if (cosB<= -0.86 || cosB>=0.86){
						o = 2;
					}
					int off = o*(4*3*3*sizeof(unsigned int));
					void * offset = BUFFER_OFFSET(off);
					eboLOD1->draw(GL_UNSIGNED_INT, GL_QUADS, 3*3*4, offset);
				glPopMatrix();
			} // for each position
			
			eboLOD1->unbind();

			lod1vbo2->unbind(lod1shader2);
			//lod1vbo2->draw(lod1shader2, GL_QUADS, 0);
			//glDepthMask(GL_TRUE);
			colorTexture		->unbind();
			displacementTexture	->unbind();
			displacement2Texture->unbind();
			dataTexture			->unbind();
			normalTexture		->unbind();
			// turn off shader
			lod1shader2->use(false);
			
			glEnable(GL_CULL_FACE);
			glEnable(GL_LIGHTING);
		
		glPopMatrix();
	}
}


void DTree::drawLOD2()
{
	if (g_draw_lod2){
		glColor4f(0.0, 0.0,0.0, alpha_c);
		g_tree_lod2_count++;
		int i, j, sliceCount, setCount=sliceSets.size();
		Texture * colorTexture, * dataTexture, *displacementTexture, *displacement2Texture, *normalTexture;
			displacementTexture		= leafNoiseTexture;
			displacement2Texture	= branchNoiseTexture;
			DTreeSlice * slice;
		for (j = 0; j< setCount; j++){
			// if discrepacy angle under treshold, DO NOT write in z-buffer
			v3 p = position;
			v3 v = *viewer_position;
			p.y = 0.0;
			v.y = 0.0;
			v3 eyeDir = (p-v).getNormalized();
			v3 normalDir = sliceSets[j]->getNormal();
			normalDir.rotateY(rotationY*DEG_TO_RAD);
			float discrepacy = abs(normalDir.dot(eyeDir));

			sliceCount = sliceSets[j]->size();
			i = sliceCount/2;
				slice					= sliceSets[j]->getSlice(i);
				colorTexture			= slice->colormap;
				normalTexture			= slice->normalmap;
				dataTexture				= slice->datamap;
				glPushMatrix();	
					//l3dBillboardCheatCylindricalBegin();
			
					glRotatef(j*90, 0.0, 1.0, 0.0);
					glTranslatef((i-(float(sliceCount)/2.f) + 0.5), 0.0, 0.0);
					glScalef(10.0,10.0,-10.0);
					//glRotatef(90, 0.0, 0.0, 1.0);
					glRotatef(-90, 0.0, 1.0, 0.0);
					glDisable(GL_CULL_FACE);
					colorTexture		->bind(GL_TEXTURE0);
					displacementTexture	->bind(GL_TEXTURE1);
					displacement2Texture->bind(GL_TEXTURE4);
					dataTexture			->bind(GL_TEXTURE2);
					normalTexture		->bind(GL_TEXTURE3);

					u_time_offset->data = &	g_tree_time_offset_1;
					// turn on shader
					lod1shader->use(true);
					lod1shader->setTexture(l_color	, colorTexture			->textureUnitNumber	);
					lod1shader->setTexture(l_displ	, displacementTexture	->textureUnitNumber	);
					lod1shader->setTexture(l_displ2	, displacement2Texture	->textureUnitNumber	);
					lod1shader->setTexture(l_data	, dataTexture			->textureUnitNumber	);
					lod1shader->setTexture(l_normal	, normalTexture			->textureUnitNumber	);
			
					// TODO: draw at all positions
					if (discrepacy<0.8){
						glDepthMask(GL_FALSE);
						lod1vbo->draw(lod1shader, GL_QUADS, 0);
						glDepthMask(GL_TRUE);
					} else {
						lod1vbo->draw(lod1shader, GL_QUADS, 0);
					}

					colorTexture		->unbind();
					displacementTexture	->unbind();
					displacement2Texture->unbind();
					dataTexture			->unbind();
					normalTexture		->unbind();
					// turn off shader
					lod1shader->use(false);
					glEnable(GL_CULL_FACE);

				glPopMatrix();
		} // for j - sliceSets
	}
}

void DTree::drawNormals(DTreeInstanceData* instance){
	glColor4f(1.0,1.0,1.0, instance->alpha);
		
	glDisable(GL_CULL_FACE);
	glPushMatrix();
	glTranslatef(instance->position.x, instance->position.y, instance->position.z);
	glRotatef(instance->rotation_y+180, 0.0, 1.0, 0.0);
	glScalef( 10.f , -10.f, 10.f);
	
	// draw bbox
	//bbox->draw();


	// bind textures
	dataTexture->bind(GL_TEXTURE1);
	branchNoiseTexture->bind(GL_TEXTURE2);
	bColorTexture->bind(GL_TEXTURE4);

	// TODO: use positions

	// draw branches
	//branchesEBO->draw(n_branchShader);

	
	leafNoiseTexture->bind(GL_TEXTURE3);
	frontDecalMap		->bind(GL_TEXTURE4);
	frontNormalMap		->bind(GL_TEXTURE5);
	frontTranslucencyMap->bind(GL_TEXTURE6);
	frontHalfLife2Map	->bind(GL_TEXTURE7);
	backDecalMap		->bind(GL_TEXTURE8);
	backNormalMap		->bind(GL_TEXTURE9);
	backTranslucencyMap	->bind(GL_TEXTURE10);
	backHalfLife2Map	->bind(GL_TEXTURE11);

	// TODO: use positions

	// draw leaves
	leavesVBO->draw(n_leafShader, GL_QUADS, 0);


	leafNoiseTexture->unbind();
	frontDecalMap		->unbind();
	frontNormalMap		->unbind();
	frontTranslucencyMap->unbind();
	frontHalfLife2Map	->unbind();
	backDecalMap		->unbind();
	backNormalMap		->unbind();
	backTranslucencyMap	->unbind();
	backHalfLife2Map	->unbind();
	branchNoiseTexture->unbind();
	dataTexture->unbind();
	
	glPopMatrix();
	glEnable(GL_CULL_FACE);
}

void DTree::draw(){
	/*
	if (g_draw_lod1_method){
		drawLOD1b();
		return;
	}
	for (int i = 0; i < tree_instances.size(); i++){
		// determine LOD configuration
		position = tree_instances[i]->position;
		rotationY= tree_instances[i]->rotation_y;
		glPushMatrix();
			glTranslate(position);
			glRotatef(rotationY, 0.0, 1.0 ,0.0);
			glScalef(10.0, 10.0, 10.0);
			drawLOD1();
		glPopMatrix();
	} // for each position
	return;
	// sort positions depending on the distance
	/*
	vector<DTreeInstanceDrawData*> lod0;

	vector<DTreeInstanceDrawData*> lod1;

	vector<DTreeInstanceDrawData*> lod2;
	*/
	/*
	if (tree_instances.size()>1){
		DTreeInstanceData* act = tree_instances[0];
		DTreeInstanceData* toDraw;
		float toDraw_dist = 0.0;
		float act_dist	= act->position.distanceTo(*viewer_position);
		for (int i=1; i<tree_instances.size(); i++){
			//v4* act			= tree_instances[i-1];
			DTreeInstanceData* next		= tree_instances[i];
			//float act_dist	= act	->distanceTo(*viewer_position);
			float next_dist	= next->position.distanceTo(*viewer_position);

			if (act_dist < next_dist){
				// swap 
				tree_instances[i-1]	= next;
				tree_instances[i]	= act;
				toDraw				= next;
				toDraw_dist			= next_dist;
			} else {
				// 
				toDraw		= act;
				toDraw_dist = act_dist;
				act			= next;
				act_dist	= next_dist;
			}
			toDraw->index = i-1;
			// draw the rigth LOD
			alpha_c = 1.0;
			// if LOD vorbidden
			if (!g_draw_dtree_lod){
				drawLOD0();
				return;
			}
			// TODO: check if is in front of camera...

			float alpha=0.0;
			
			// manage array of LOD0, LOD1, and LODs in transition: LOD0_LOD1...

			if (act_dist >= g_lodTresholds.w){
				alpha_c = 1.0;

				//lod2.push_back(drawData;


				drawLOD2();
			} else if (act_dist >= g_lodTresholds.z){
				alpha = (act_dist - g_lodTresholds.z) / (g_lodTresholds.w - g_lodTresholds.z);
			
				// // show LOD 1
				// 	alpha_c = 1.0-alpha;
				// 	drawLOD1();
				// 	glDepthMask(GL_FALSE);				
				// 	// show LOD 2 
				// 	alpha_c = alpha;
				// 	drawLOD2();
				// 	glDepthMask(GL_TRUE);
			
			
				if (alpha<0.5){
					// show LOD 1
					alpha_c = 1.0;
					drawLOD1();
					glDepthMask(GL_FALSE);
					// show LOD 2 
					alpha_c = 2*alpha;
					drawLOD2();
					glDepthMask(GL_TRUE);

				} else {
					// show LOD 2 
					alpha_c = 1.0;
					drawLOD2();
					// show LOD 1 
					glDepthMask(GL_FALSE);
					alpha_c = 2*(1-alpha);
					drawLOD1();
					glDepthMask(GL_TRUE);
				
				}
			
			} else if (act_dist >= g_lodTresholds.y){
				alpha_c = 1.0;
				drawLOD1();
			} else if (act_dist >=g_lodTresholds.x){
				alpha = (act_dist - g_lodTresholds.x) / (g_lodTresholds.y - g_lodTresholds.x);
			
				//glColor4f(0.0, 0.0, 0.0, 1.0-alpha);
				//	drawLOD0();
				//	glDepthMask(GL_FALSE);
				//	// show LOD 1 
				//	glColor4f(0.0, 0.0, 0.0, alpha);
				//	drawLOD1();
				//	glDepthMask(GL_TRUE);
			
				if (alpha<0.5){
					// show LOD 0
					alpha_c = 1.0;
					drawLOD0();
					glDepthMask(GL_FALSE);
					// show LOD 1 
					alpha_c = 2*alpha;
					drawLOD1();
					glDepthMask(GL_TRUE);
				} else {
					// show LOD 0
					glDepthMask(GL_FALSE);
					alpha_c = 2*(1-alpha);
					drawLOD0();
					glDepthMask(GL_TRUE);
					// show LOD 1 
					alpha_c = 1.0;
					drawLOD1();				
				}
			
			} else {
				alpha_c = 1.0;
				drawLOD0();
			}


		} // for
	} // if positions > 1



	alpha_c = 1.0;

	if (!g_draw_dtree_lod){
		drawLOD0();
		return;
	}
	*/
	// get distance to viewer
	v3		toViewDir = *viewer_position - position;
	float distance = toViewDir.length();
	toViewDir.normalize();
	float discrepacy = toViewDir.dot(*viewer_direction);
	//v4 lodTresholds = v4 (12.0, 13.0, 50.0, 55.0);
	//v4 g_lodTresholds = v4 (15.0, 18.0, 50.0, 55.0);
	if (discrepacy<0.0){ // if is infront of the camera

		float alpha=0.0;
		
		if (distance >= g_lodTresholds.w){
			alpha_c = 1.0;
			//drawLOD2();
		} else if (distance >= g_lodTresholds.z){
			alpha = (distance - g_lodTresholds.z) / (g_lodTresholds.w - g_lodTresholds.z);
			g_tree_lod1_count++;
			/*
			// // show LOD 1
			// 	alpha_c = 1.0-alpha;
			// 	drawLOD1();
			// 	glDepthMask(GL_FALSE);				
			// 	// show LOD 2 
			// 	alpha_c = alpha;
			// 	drawLOD2();
			// 	glDepthMask(GL_TRUE);
			
			
			if (alpha<0.5){
				// show LOD 1
				alpha_c = 1.0;
				drawLOD1();
				glDepthMask(GL_FALSE);
				// show LOD 2 
				alpha_c = 2*alpha;
				drawLOD2();
				glDepthMask(GL_TRUE);
				
			} else {
				// show LOD 2 
				alpha_c = 1.0;
				drawLOD2();
				// show LOD 1 
				glDepthMask(GL_FALSE);
				alpha_c = 2*(1-alpha);
				drawLOD1();
				glDepthMask(GL_TRUE);
				
			}
			*/
		} else if (distance >=g_lodTresholds.y){
			alpha_c = 1.0;
			drawLOD1();
		} else if (distance >=g_lodTresholds.x){
			alpha = (distance - g_lodTresholds.x) / (g_lodTresholds.y - g_lodTresholds.x);
			g_tree_lod0_count++;
			/*
			if (alpha<0.5){
				alpha_c = 1.0 - alpha;
				drawLOD0();
				//glDepthMask(GL_FALSE);
				// show LOD 1 
				alpha_c = alpha;
				drawLOD1();
				//glDepthMask(GL_TRUE);
			} else {
					
				// show LOD 0
				glDepthMask(GL_FALSE);
				alpha_c = 1.0-alpha;
				drawLOD0();
				glDepthMask(GL_TRUE);
				// show LOD 1 
				alpha_c = alpha;
				drawLOD1();	
				
			}

			//glColor4f(0.0, 0.0, 0.0, 1.0-alpha);
			//	drawLOD0();
			//	glDepthMask(GL_FALSE);
			//	// show LOD 1 
			//	glColor4f(0.0, 0.0, 0.0, alpha);
			//	drawLOD1();
			//	glDepthMask(GL_TRUE);
			/*/
			if (alpha<0.5){
				// show LOD 0
				alpha_c = 1.0;
				drawLOD0();
				glDepthMask(GL_FALSE);
				// show LOD 1 
				alpha_c = 2*alpha;
				drawLOD1();
				glDepthMask(GL_TRUE);
			} else {
				// show LOD 0
				glDepthMask(GL_FALSE);
				alpha_c = 2*(1-alpha);
				drawLOD0();
				glDepthMask(GL_TRUE);
				// show LOD 1 
				alpha_c = 1.0;
				drawLOD1();				
			}
		
		} else {
			alpha_c = 1.0;
			drawLOD0();
		}

	} // if in front of camera plane
}

void DTree::prepareForRender(){
	countRenderQueues[0] = 0;
	countRenderQueues[1] = 0;
	countRenderQueues[2] = 0;
	countRenderQueues[3] = 0;
	countRenderQueues[4] = 0;
	// go through instances and put them in proper render pipelines
	// TODO: this part can be PARALLEL to the rendering thread...
	DTreeInstanceData * instance;
	int i;
	for (i=0; i<lod1_typeIndices.size(); i++){
		lod1_typeIndices[i] = 0;
	}
	for (i=0; i<lod2_typeIndices.size(); i++){
		lod2_typeIndices[i] = 0;
	}
	int instanceCount = tree_instances.size();
	DTreeInstanceData * act_instance = tree_instances[0] ;
	DTreeInstanceData * next_instance;
	act_instance->eye_dir =  act_instance->position - *g_viewer_position;
	act_instance->distance = act_instance->eye_dir.length();
	act_instance->eye_dir.normalize();
	act_instance->discrepacy = g_viewer_direction->dot(act_instance->eye_dir);
	
	DTreeInstanceData* done_instance;
	// go through instances, perform one bubble sort walktrough & split into different render queues
	DTreeInstanceData* rest_instance = act_instance;
	//int i=1;
	for (int i=1; i<tree_instances.size(); i++){
		next_instance = tree_instances[i];
		next_instance->eye_dir = next_instance->position - *g_viewer_position;
		next_instance->distance= next_instance->eye_dir.length();
		next_instance->eye_dir.normalize();
		next_instance->discrepacy = g_viewer_direction->dot(next_instance->eye_dir);
		if (next_instance->distance > act_instance->distance){
			// swap
			tree_instances[i-1]	= next_instance;
			tree_instances[i-1]	->index = i-1;
			tree_instances[i]	= act_instance;
			tree_instances[i]	->index = i;
			done_instance		= next_instance;
			//if (i==1){ done_instance=NULL; }
			rest_instance		= act_instance;
		} else {
			done_instance	= act_instance;
			act_instance	= next_instance;
			rest_instance	= act_instance;
		}
		// draw only instances near the view angle
		enqueueInRenderList(done_instance);
	} // loop over instances
	if (rest_instance!=NULL){
		enqueueInRenderList(rest_instance);
	}
}
void DTree::makeTransition(float control, bool maskOld, DTreeInstanceData* instance, void (DTree::*drawLODA)(DTreeInstanceData*, float),  void (DTree::*drawLODB)(DTreeInstanceData*, float)){
	float shift;
	float c;
	float a1;
	float a2;
	g_transitionControl = control;
	switch (g_lodTransition){
			case LODTransitionMethod::HARD_SWITCH:
				if (control<0.5){
					(this->*drawLODA)(instance, 1.0);
				} else {
					(this->*drawLODB)(instance, 1.0);
				}
				break;
			case LODTransitionMethod::CROSS_FADE:
				if (control<0.5){
					// show LOD 1 
					glDepthMask(GL_FALSE);

					(this->*drawLODB)(instance, control);
					glDepthMask(GL_TRUE);

					// show LOD 0
					(this->*drawLODA)(instance, 1.0-control);			
				} else {			
					// show LOD 0
					glDepthMask(GL_FALSE);
					//instance->alpha = 1.0-control;
					(this->*drawLODA)(instance, 1.0-control);
					glDepthMask(GL_TRUE);

					// show LOD 1 
					//instance->alpha = control;
					(this->*drawLODB)(instance, control);			
				}
				break;
			case LODTransitionMethod::FADE_IN_BACKGROUND:
				if (control<0.5){
					// show LOD 1 
					glDepthMask(GL_FALSE);
					//instance->alpha = 2*control;
					(this->*drawLODB)(instance, 2*control);
					glDepthMask(GL_TRUE);

					// show LOD 0
					//instance->alpha = 1.0;
					(this->*drawLODA)(instance,1.0);			
				} else {			
					// show LOD 0
					glDepthMask(GL_FALSE);
					//instance->alpha = 2.0*(1.0-control);
					(this->*drawLODA)(instance, 2.0*(1.0-control));
					glDepthMask(GL_TRUE);

					// show LOD 1 
					//instance->alpha = 1.0;
					(this->*drawLODB)(instance, 1.0);			
				}
				break;
			case LODTransitionMethod::SHIFTED_CROSS_FADE:
				shift = g_transitionShift;
				c = 1.0f/(1.f - shift);
				a1 = min(1.f, max(0.f , control*c));
				a2 = min(1.f, max(0.f, 1.f-(control-shift)*c));
				if (control<0.5){
					if (maskOld){
						// show LOD 0
						glDepthMask(GL_FALSE);
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);	
						glDepthMask(GL_TRUE);

						// show LOD 1 
						//instance->alpha = a1;
						(this->*drawLODB)(instance, a1);
					} else {
						// show LOD 0						
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);	
						
						// show LOD 1 
						glDepthMask(GL_FALSE);
						//instance->alpha = a1;
						(this->*drawLODB)(instance, a1);
						glDepthMask(GL_TRUE);
					}

				} else {			
					if (maskOld){
						// show LOD 1 
						glDepthMask(GL_FALSE);
						//instance->alpha = a1;
						(this->*drawLODB)(instance, a1);	
						glDepthMask(GL_TRUE);

						// show LOD 0
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);
					} else {
						// show LOD 1 
						
						//instance->alpha = a1;
						(this->*drawLODB)(instance, a1);	
						

						// show LOD 0
						glDepthMask(GL_FALSE);
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);
						glDepthMask(GL_TRUE);
					}
					
				}
				break;
			case LODTransitionMethod::SHIFTED_SOFT_FADE:
				shift = g_transitionShift;
				c = 1.0f/(1.f - shift);
				a1 = 1.f - (smoothTransitionCos(c*(control)*PI)*0.5f + 0.5f);
				a2 = smoothTransitionCos(c*(control-shift)*PI)*0.5f + 0.5f;

				//printf("LODA: %f, LODB: %f\n", a2, a1);
				if (maskOld){
					if (maskOld){
						// show LOD 0
						glDepthMask(GL_FALSE);
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);	
						glDepthMask(GL_TRUE);

						// show LOD 1 
						//instance->alpha = a1;
						(this->*drawLODB)(instance, a1);
					} else {
						// show LOD 0						
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);	
						
						// show LOD 1 
						glDepthMask(GL_FALSE);
						//instance->alpha = a1;
						(this->*drawLODB)(instance, a1);
						glDepthMask(GL_TRUE);
					}
				} else {
					// LOD 01
					if (g_Draw2Shadowmap){
						//glDepthMask(GL_FALSE);
						// show LOD 1 
						//instance->alpha = a1;						
						(this->*drawLODB)(instance, a1);
						//glDepthMask(GL_TRUE);
						
						// show LOD 0	
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);
					} else {
						glDepthMask(GL_FALSE);
						// show LOD 1 
						//instance->alpha = a1;						
						(this->*drawLODB)(instance, a1);
						glDepthMask(GL_TRUE);

						// show LOD 0	
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);
					}
					/*
					if (control<0.5){
						//glDepthMask(GL_FALSE);
						// show LOD 1 
						//instance->alpha = a1;						
						(this->*drawLODB)(instance, a1);
						//glDepthMask(GL_TRUE);
						
						// show LOD 0	
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);
					} else {
						//glDepthMask(GL_FALSE);
						// show LOD 1 
						//instance->alpha = a1;						
						(this->*drawLODB)(instance, a1);
						//glDepthMask(GL_TRUE);

						// show LOD 0	
						//instance->alpha = a2;
						(this->*drawLODA)(instance, a2);
					}
					*/
				}
				break;
		}
}


void DTree::render(){
	int i;
	DTreeInstanceData* instance;
	// render LOD2 instances
	isInstancingEnabled = true;
	draw_all_instances_LOD2();
	// render LOD1-LOD2 transitioning instances
	isInstancingEnabled = false;
	for (i=0; i<countRenderQueues[3]; i++){
		instance = instancesInRenderQueues[3][i];
		float alpha = instance->alpha;
		makeTransition(alpha,true, instance, &DTree::draw_instance_LOD1, &DTree::draw_instance_LOD2);
	}
	// render LOD1 instances
	isInstancingEnabled = true;
	draw_all_instances_LOD1();

	// render LOD0-LOD1 transitioning instances
	isInstancingEnabled = false;
	for (i=0; i<countRenderQueues[1]; i++){
		instance = instancesInRenderQueues[1][i];
		float alpha = instance->alpha;
		makeTransition(alpha,false, instance, &DTree::draw_instance_LOD0, &DTree::draw_instance_LOD1);
	}

	// render LOD0 instances
	isInstancingEnabled = false;
	for (i=0; i<countRenderQueues[0]; i++){
		instance = instancesInRenderQueues[0][i];
		// show LOD 0
		g_transitionControl = 0.0;
		draw_instance_LOD0(instance, 1.0);	
	}

	g_tree_lod0_count	= countRenderQueues[0];
	g_tree_lod01_count	= countRenderQueues[1];	 
	g_tree_lod1_count	= countRenderQueues[2];
	g_tree_lod12_count	= countRenderQueues[3];	 
	g_tree_lod2_count	= countRenderQueues[4];
}

void DTree::draw2(){
	//prepareForRender();
	render();	
}

void DTree::enqueueInRenderList(DTreeInstanceData * instance){
	if (instance!=NULL){
		int LODindex = 0;
		if (instance->distance<=g_lodTresholds.x){
			//	LOD0
			g_tree_lod0_count++;
			instance->alpha = 1.0;
			instancesInRenderQueues[LODindex][countRenderQueues[LODindex]] = instance;
			countRenderQueues[LODindex]+=1;
			return;
		}
	
		if(instance->discrepacy>0.5){
			instance->dirA.y = 0.0;
			instance->dirB.y = 0.0;
			instance->eye_dir.y = 0.0;
			instance->eye_dir.normalize();
			// determine LOD pipeline
		
			if (instance->distance>g_lodTresholds.w){
				// LOD 2
				// TODO: instanced drawing of LOD2
				g_tree_lod2_count++;
				LODindex = 4;
				instance->offset = 1;
				
				float cosA = instance->dirA.dot(instance->eye_dir);
				// 45° treshold
				if (cosA<= -0.70 || cosA>= 0.70){
					// varianta A
					instance->offset = 0;
				}
				memcpy( lod2_instanceMatrices[instance->offset] + lod2_typeIndices[instance->offset]*instanceFloatCount,
						instance->transformMatrix.m,
						16*sizeof(float)
					  );
					// instance attribs
				memcpy( lod2_instanceMatrices[instance->offset] + lod2_typeIndices[instance->offset]*instanceFloatCount + 16,
						instance->colorVariance.data,
						3*sizeof(float)
					  );
				lod2_instanceMatrices[instance->offset][lod2_typeIndices[instance->offset]*instanceFloatCount + 19] = instance->time_offset;
				lod2_typeIndices[instance->offset] += 1;
			} else if (instance->distance>g_lodTresholds.z){
				// transition between LOD1-LOD2
				g_tree_lod12_count++;
				// TODO: direct drawing of LOD2 & LOD1
				LODindex = 3;
				float cosA = instance->dirA.dot(instance->eye_dir);
				// 45° treshold
				if (cosA<= -0.70 || cosA>= 0.70){
					// varianta A
					instance->offset = 0;
				} else {
					// varianta B
					instance->offset = 1;
				}
				instance->alpha = (instance->distance - g_lodTresholds.z) / (g_lodTresholds.w - g_lodTresholds.z);

			} else
			if (instance->distance>g_lodTresholds.y){
				g_tree_lod1_count++;
				LODindex = 2;
				// LOD1
				// determine the order of slices to render
				float cosA = instance->dirA.dot(instance->eye_dir);
				float cosB = instance->dirB.dot(instance->eye_dir);
				instance->offset = 1;
				if (cosA<= -0.86 || cosA>=0.86){
					// B
					instance->offset = 0;
				} else if (cosB<= -0.86 || cosB>=0.86){
					// C
					instance->offset = 2;
				} 
				// copy matrices (and other instance attributes to array for VBO)
					// matrices
				memcpy( lod1_instanceMatrices[instance->offset] + lod1_typeIndices[instance->offset]*instanceFloatCount,
						instance->transformMatrix.m,
						16*sizeof(float)
					  );
					// instance attribs
				memcpy( lod1_instanceMatrices[instance->offset] + lod1_typeIndices[instance->offset]*instanceFloatCount + 16,
						instance->colorVariance.data,
						3*sizeof(float)
					  );
				lod1_instanceMatrices[instance->offset][lod1_typeIndices[instance->offset]*instanceFloatCount + 19] = instance->time_offset;

				lod1_typeIndices[instance->offset] += 1;
			} else
			if (instance->distance>g_lodTresholds.x){
				//	transition between LOD0-LOD1
				g_tree_lod01_count++;
				LODindex = 1;
				// determine the order of slices to render
				float cosA = instance->dirA.dot(instance->eye_dir);
				float cosB = instance->dirB.dot(instance->eye_dir);
				instance->offset = 1;
				if (cosA<= -0.86 || cosA>=0.86){
					// B
					instance->offset = 0;
				} else if (cosB<= -0.86 || cosB>=0.86){
					// C
					instance->offset = 2;
				}
				// TODO: direct drawing of LOD0 & LOD1
				instance->alpha = (instance->distance - g_lodTresholds.x) / (g_lodTresholds.y - g_lodTresholds.x);
			}
			/*
			else {
				// LOD0
				LODindex = 0;
				instance->alpha = 1.0;
				//draw_instance_LOD0(instance);
			}*/
			instancesInRenderQueues[LODindex][countRenderQueues[LODindex]] = instance;
			countRenderQueues[LODindex]+=1;
		}
	} // not NULL instance
}

void DTree::drawForLOD(){
	// TODO: optimize rendering for LOD slice generation...
	glDisable(GL_CULL_FACE);
	glPushMatrix();
	glScalef( 1.f , -1.f, 1.f);
	
	// draw bbox
	//bbox->draw();

	glDisable(GL_LIGHTING);
	// bind textures
	dataTexture->bind(GL_TEXTURE1);
	branchNoiseTexture->bind(GL_TEXTURE2);
	bColorTexture->bind(GL_TEXTURE4);

	// draw branches
	branchesEBO->draw(bLODShader);

	bColorTexture->unbind();

	// bind textures
	leafNoiseTexture->bind(GL_TEXTURE3);
	frontDecalMap		->bind(GL_TEXTURE4);
	frontNormalMap		->bind(GL_TEXTURE5);
	frontTranslucencyMap->bind(GL_TEXTURE6);
	frontHalfLife2Map	->bind(GL_TEXTURE7);
	backDecalMap		->bind(GL_TEXTURE8);
	backNormalMap		->bind(GL_TEXTURE9);
	backTranslucencyMap	->bind(GL_TEXTURE10);
	backHalfLife2Map	->bind(GL_TEXTURE11);



	// draw leaves
	leavesVBO->draw(lLODShader, GL_QUADS, 0);


	leafNoiseTexture->unbind();
	frontDecalMap		->unbind();
	frontNormalMap		->unbind();
	frontTranslucencyMap->unbind();
	frontHalfLife2Map	->unbind();
	backDecalMap		->unbind();
	backNormalMap		->unbind();
	backTranslucencyMap	->unbind();
	backHalfLife2Map	->unbind();
	branchNoiseTexture->unbind();
	dataTexture->unbind();
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);

}

void DTree::initLOD0()
{
	printf("DYN_TREE init\n");
	// load textures
	// SEASON MAP
	seasonMap = new Texture("seasonMap");
	seasonMap->load(DYN_TREE::SEASON_MAP, false, false, GL_CLAMP, GL_LINEAR, GL_LINEAR);
	seasonMap->textureUnitNumber = 6;
	seasonMap->textureUnit = GL_TEXTURE0 + 6;

	// NOISE
	branchNoiseTexture = new Texture("branch_noise_tex"); 
	branchNoiseTexture->load(DYN_TREE::BRANCH_NOISE_TEXTURE, false, true, GL_REPEAT, GL_LINEAR, GL_LINEAR);
	leafNoiseTexture = new Texture("leaf_noise_tex");
	leafNoiseTexture->load(DYN_TREE::LEAF_NOISE_TEXTURE, false, true, GL_REPEAT, GL_LINEAR, GL_LINEAR);

	//  COLOR
	bColorTexture = new Texture("color_texture");
	bColorTexture->load(DYN_TREE::TEX_WOOD1, true, false , GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
	//lColorTexture = new Texture("color_texture");
	//lColorTexture->load(DYN_TREE::TEX_LEAF1, true, false , GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

	//frontDecalMap		=new Texture("color_texture");
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
	n_branchShader = new Shader("n_branch");
	n_branchShader->loadShader(DYN_TREE::SHADER_BRANCH_VN,DYN_TREE::SHADER_BRANCH_FN, DYN_TREE::SHADER_BRANCH_GN, 3, GL_TRIANGLES, GL_LINE_STRIP);  
	n_leafShader = new Shader("n_leaf");
	n_leafShader->loadShader(DYN_TREE::SHADER_LEAF_VN,DYN_TREE::SHADER_LEAF_FN, DYN_TREE::SHADER_LEAF_GN, 3*4+4, GL_TRIANGLES, GL_LINE_STRIP);  

	branchShader = new Shader("branch");
	branchShader->loadShader(DYN_TREE::SHADER_BRANCH_V,DYN_TREE::SHADER_BRANCH_F); 
	leafShader = new Shader("leaf");
	leafShader->loadShader(DYN_TREE::SHADER_LEAF_V,DYN_TREE::SHADER_LEAF_F); 
	// shadow shaders
	branchShader_sh = new Shader("branch");
	branchShader_sh->loadShader(DYN_TREE::SHADER_BRANCH_SH_V,DYN_TREE::SHADER_BRANCH_SH_F); 
	leafShader_sh = new Shader("leaf");
	leafShader_sh->loadShader(DYN_TREE::SHADER_LEAF_SH_V,DYN_TREE::SHADER_LEAF_SH_F); 
		
	
	// LOD shaders
	bLODShader = new Shader("LODbranch");
	bLODShader->loadShader(DYN_TREE::SHADER_BRANCH_LOD_V,DYN_TREE::SHADER_BRANCH_LOD_F); 
	lLODShader = new Shader("LODleaf");
	lLODShader->loadShader(DYN_TREE::SHADER_LEAF_LOD_V,DYN_TREE::SHADER_LEAF_LOD_F); 
	 

	// connect textures with shader
	branchShader->linkTexture(branchNoiseTexture);
	branchShader->linkTexture(bColorTexture);
	branchShader->linkTexture(g_shadowmap1);
	branchShader_sh->linkTexture(branchNoiseTexture	);
	branchShader_sh->linkTexture(bColorTexture		);


	n_branchShader->linkTexture(branchNoiseTexture);
	n_branchShader->linkTexture(bColorTexture);

	bLODShader->linkTexture(branchNoiseTexture);
	bLODShader->linkTexture(bColorTexture);

	leafShader->linkTexture(branchNoiseTexture);
	leafShader->linkTexture(leafNoiseTexture);
	//leafShader->linkTexture(lColorTexture);
	leafShader->linkTexture(g_shadowmap1);
	leafShader->linkTexture(frontDecalMap		);
	leafShader->linkTexture(frontNormalMap		);
	leafShader->linkTexture(frontTranslucencyMap);
	leafShader->linkTexture(frontHalfLife2Map	);
	leafShader->linkTexture(backDecalMap		);
	leafShader->linkTexture(backNormalMap		);
	leafShader->linkTexture(backTranslucencyMap	);
	leafShader->linkTexture(backHalfLife2Map	);
	leafShader->linkTexture(seasonMap			);

	leafShader_sh->linkTexture(frontDecalMap		);
	leafShader_sh->linkTexture(seasonMap			);
	leafShader_sh->linkTexture(branchNoiseTexture	);
	leafShader_sh->linkTexture(leafNoiseTexture		);


	n_leafShader->linkTexture(branchNoiseTexture);
	n_leafShader->linkTexture(leafNoiseTexture);
	//n_leafShader->linkTexture(lColorTexture);
	n_leafShader->linkTexture(frontDecalMap		);
	n_leafShader->linkTexture(frontNormalMap		);
	n_leafShader->linkTexture(frontTranslucencyMap);
	n_leafShader->linkTexture(frontHalfLife2Map	);
	n_leafShader->linkTexture(backDecalMap		);
	n_leafShader->linkTexture(backNormalMap		);
	n_leafShader->linkTexture(backTranslucencyMap	);
	n_leafShader->linkTexture(backHalfLife2Map	);



	lLODShader->linkTexture(branchNoiseTexture	);
	lLODShader->linkTexture(leafNoiseTexture	);
	//lLODShader->linkTexture(lColorTexture);
	lLODShader->linkTexture(frontDecalMap		);
	lLODShader->linkTexture(frontNormalMap		);
	lLODShader->linkTexture(frontTranslucencyMap);
	lLODShader->linkTexture(frontHalfLife2Map	);
	lLODShader->linkTexture(backDecalMap		);
	lLODShader->linkTexture(backNormalMap		);
	lLODShader->linkTexture(backTranslucencyMap	);
	lLODShader->linkTexture(backHalfLife2Map	);
	lLODShader->linkTexture(seasonMap			);


	// register uniforms
	this->branchCountF = 0;
	branchShader->registerUniform("branch_count",			UniformType::F1,	& this->branchCountF);
	branchShader->registerUniform("time",					UniformType::F1,	& g_float_time);
	branchShader->registerUniform("time_offset",			UniformType::F1,	& time_offset);	
	branchShader->registerUniform("wind_direction",			UniformType::F3,	& g_tree_wind_direction.data);
	branchShader->registerUniform("wind_strength",			UniformType::F1,	& g_tree_wind_strength);
	branchShader->registerUniform("wood_amplitudes",		UniformType::F4,	& g_tree_wood_amplitudes.data);
	branchShader->registerUniform("wood_frequencies",		UniformType::F4,	& g_tree_wood_frequencies.data);
	branchShader->registerUniform("window_size",			UniformType::F2,	& g_window_sizes.data);		
	branchShader->registerUniform("LightMVPCameraVInverseMatrix",			UniformType::M4,	g_LightMVPCameraVInverseMatrix);
	branchShader->registerUniform("shadowMappingEnabled",	UniformType::B1,	& g_ShadowMappingEnabled);
	branchShader->registerUniform("transition_control",		UniformType::F1,	& g_transitionControl);		
	branchShader_sh->registerUniform("branch_count",		UniformType::F1,	& this->branchCountF);
	branchShader_sh->registerUniform("time",				UniformType::F1,	& g_float_time);
	branchShader_sh->registerUniform("time_offset",			UniformType::F1,	& time_offset);	
	branchShader_sh->registerUniform("wind_direction",		UniformType::F3,	& g_tree_wind_direction.data);
	branchShader_sh->registerUniform("wind_strength",		UniformType::F1,	& g_tree_wind_strength);
	branchShader_sh->registerUniform("wood_amplitudes",		UniformType::F4,	& g_tree_wood_amplitudes.data);
	branchShader_sh->registerUniform("wood_frequencies",	UniformType::F4,	& g_tree_wood_frequencies.data);
	branchShader_sh->registerUniform("window_size",			UniformType::F2,	& g_window_sizes.data);		
	branchShader_sh->registerUniform("transition_control",	UniformType::F1,	& g_transitionControl);		
	


	n_branchShader->registerUniform("branch_count",			UniformType::F1,	& this->branchCountF);
	n_branchShader->registerUniform("time",					UniformType::F1,	& g_float_time);
	n_branchShader->registerUniform("time_offset",			UniformType::F1,	& time_offset);	
	n_branchShader->registerUniform("wind_direction",		UniformType::F3,	& g_tree_wind_direction.data);
	n_branchShader->registerUniform("wind_strength",		UniformType::F1,	& g_tree_wind_strength);
	n_branchShader->registerUniform("wood_amplitudes",		UniformType::F4,	& g_tree_wood_amplitudes.data);
	n_branchShader->registerUniform("wood_frequencies",		UniformType::F4,	& g_tree_wood_frequencies.data);
	n_branchShader->registerUniform("window_size",			UniformType::F2,	& g_window_sizes.data);		



	bLODShader->registerUniform("branch_count",				UniformType::F1,	& this->branchCountF);
	bLODShader->registerUniform("time",						UniformType::F1,	& g_float_time);
	bLODShader->registerUniform("time_offset",				UniformType::F1,	& time_offset);
	bLODShader->registerUniform("wind_direction",			UniformType::F3,	& g_tree_wind_direction.data);
	bLODShader->registerUniform("wind_strength",			UniformType::F1,	& g_tree_wind_strength);
	bLODShader->registerUniform("wood_amplitudes",			UniformType::F4,	& g_tree_wood_amplitudes.data);
	bLODShader->registerUniform("wood_frequencies",			UniformType::F4,	& g_tree_wood_frequencies.data);
	bLODShader->registerUniform("window_size",				UniformType::F2,	& g_window_sizes.data);		

	//branchShader->registerUniform("leaf_amplitude", UniformType::F1, & g_tree_wood_amplitudes.data);
	//branchShader->registerUniform("leaf_frequency", UniformType::F1, & g_tree_wood_amplitudes.data);

	leafShader->registerUniform("branch_count",				UniformType::F1,	& this->branchCountF);
	leafShader->registerUniform("time",						UniformType::F1,	& g_float_time);
	leafShader->registerUniform("time_offset",				UniformType::F1,	& time_offset);
	leafShader->registerUniform("wind_direction",			UniformType::F3,	& g_tree_wind_direction.data);
	leafShader->registerUniform("wind_strength",			UniformType::F1,	& g_tree_wind_strength);
	leafShader->registerUniform("wood_amplitudes",			UniformType::F4,	& g_tree_wood_amplitudes.data);
	leafShader->registerUniform("wood_frequencies",			UniformType::F4,	& g_tree_wood_frequencies.data);
	leafShader->registerUniform("leaf_amplitude",			UniformType::F1,	& g_tree_leaf_amplitude);
	leafShader->registerUniform("leaf_frequency",			UniformType::F1,	& g_tree_leaf_frequency);
	leafShader->registerUniform("MultiplyAmbient",			UniformType::F1,	& g_leaves_MultiplyAmbient);
	leafShader->registerUniform("MultiplyDiffuse",			UniformType::F1,	& g_leaves_MultiplyDiffuse);
	leafShader->registerUniform("MultiplySpecular",			UniformType::F1,	& g_leaves_MultiplySpecular);
	leafShader->registerUniform("MultiplyTranslucency",		UniformType::F1,	& g_leaves_MultiplyTranslucency);
	leafShader->registerUniform("ReduceTranslucencyInShadow", UniformType::F1,	& g_leaves_ReduceTranslucencyInShadow);
	leafShader->registerUniform("shadow_intensity",			UniformType::F1,	& g_leaves_shadow_intensity);
	leafShader->registerUniform("LightDiffuseColor",		UniformType::F3,	& g_leaves_LightDiffuseColor.data);
	leafShader->registerUniform("window_size",				UniformType::F2,	& g_window_sizes.data);		
	leafShader->registerUniform("season",					UniformType::F1,	& g_season);		
	leafShader->registerUniform("transition_control",		UniformType::F1,	& g_transitionControl);		
	leafShader->registerUniform("LightMVPCameraVInverseMatrix",			UniformType::M4,	g_LightMVPCameraVInverseMatrix);
	leafShader->registerUniform("shadowMappingEnabled",		UniformType::B1,	& g_ShadowMappingEnabled);


	leafShader->registerUniform("gauss_shift",				UniformType::F1,	& g_gauss_shift);		
	leafShader->registerUniform("gauss_steep",				UniformType::F1,	& g_gauss_steep);		
	leafShader->registerUniform("gauss_weight",				UniformType::F1,	& g_gauss_weight);		
	


	iu0Loc1 = leafShader->getLocation("colorVariance");

	leafShader_sh->registerUniform("branch_count",				UniformType::F1,	& this->branchCountF);
	leafShader_sh->registerUniform("time",						UniformType::F1,	& g_float_time);
	leafShader_sh->registerUniform("time_offset",				UniformType::F1,	& time_offset);
	leafShader_sh->registerUniform("wind_direction",			UniformType::F3,	& g_tree_wind_direction.data);
	leafShader_sh->registerUniform("wind_strength",				UniformType::F1,	& g_tree_wind_strength);
	leafShader_sh->registerUniform("wood_amplitudes",			UniformType::F4,	& g_tree_wood_amplitudes.data);
	leafShader_sh->registerUniform("wood_frequencies",			UniformType::F4,	& g_tree_wood_frequencies.data);
	leafShader_sh->registerUniform("leaf_amplitude",			UniformType::F1,	& g_tree_leaf_amplitude);
	leafShader_sh->registerUniform("leaf_frequency",			UniformType::F1,	& g_tree_leaf_frequency);
	leafShader_sh->registerUniform("window_size",				UniformType::F2,	& g_window_sizes.data);		
	leafShader_sh->registerUniform("season",					UniformType::F1,	& g_season);		
	leafShader_sh->registerUniform("transition_control",		UniformType::F1,	& g_transitionControl);		
	leafShader_sh->registerUniform("dither",					UniformType::F1,	& g_dither);

	n_leafShader->registerUniform("branch_count",			UniformType::F1,	& this->branchCountF);
	n_leafShader->registerUniform("time",					UniformType::F1,	& g_float_time);
	n_leafShader->registerUniform("time_offset",			UniformType::F1,	& time_offset);	
	n_leafShader->registerUniform("wind_direction",			UniformType::F3,	& g_tree_wind_direction.data);
	n_leafShader->registerUniform("wind_strength",			UniformType::F1,	& g_tree_wind_strength);
	n_leafShader->registerUniform("wood_amplitudes",		UniformType::F4,	& g_tree_wood_amplitudes.data);
	n_leafShader->registerUniform("wood_frequencies",		UniformType::F4,	& g_tree_wood_frequencies.data);
	n_leafShader->registerUniform("leaf_amplitude",			UniformType::F1,	& g_tree_leaf_amplitude);
	n_leafShader->registerUniform("leaf_frequency",			UniformType::F1,	& g_tree_leaf_frequency);
	n_leafShader->registerUniform("MultiplyAmbient",		UniformType::F1,	& g_leaves_MultiplyAmbient);
	n_leafShader->registerUniform("MultiplyDiffuse",		UniformType::F1,	& g_leaves_MultiplyDiffuse);
	n_leafShader->registerUniform("MultiplySpecular",		UniformType::F1,	& g_leaves_MultiplySpecular);
	n_leafShader->registerUniform("MultiplyTranslucency",	UniformType::F1,	& g_leaves_MultiplyTranslucency);
	n_leafShader->registerUniform("ReduceTranslucencyInShadow", UniformType::F1,& g_leaves_ReduceTranslucencyInShadow);
	n_leafShader->registerUniform("shadow_intensity",		UniformType::F1,	& g_leaves_shadow_intensity);
	n_leafShader->registerUniform("LightDiffuseColor",		UniformType::F3,	& g_leaves_LightDiffuseColor.data);
	n_leafShader->registerUniform("window_size",			UniformType::F2,	& g_window_sizes.data);		


	lLODShader->registerUniform("branch_count",				UniformType::F1,	& this->branchCountF);
	lLODShader->registerUniform("time",						UniformType::F1,	& g_float_time);
	lLODShader->registerUniform("time_offset",				UniformType::F1,	& time_offset);	
	lLODShader->registerUniform("wind_direction",			UniformType::F3,	& g_tree_wind_direction.data);
	lLODShader->registerUniform("wind_strength",			UniformType::F1,	& g_tree_wind_strength);
	lLODShader->registerUniform("wood_amplitudes",			UniformType::F4,	& g_tree_wood_amplitudes.data);
	lLODShader->registerUniform("wood_frequencies",			UniformType::F4,	& g_tree_wood_frequencies.data);
	lLODShader->registerUniform("leaf_amplitude",			UniformType::F1,	& g_tree_leaf_amplitude);
	lLODShader->registerUniform("leaf_frequency",			UniformType::F1,	& g_tree_leaf_frequency);
	lLODShader->registerUniform("MultiplyAmbient",			UniformType::F1,	& g_leaves_MultiplyAmbient);
	lLODShader->registerUniform("MultiplyDiffuse",			UniformType::F1,	& g_leaves_MultiplyDiffuse);
	lLODShader->registerUniform("MultiplySpecular",			UniformType::F1,	& g_leaves_MultiplySpecular);
	lLODShader->registerUniform("MultiplyTranslucency",		UniformType::F1,	& g_leaves_MultiplyTranslucency);
	lLODShader->registerUniform("ReduceTranslucencyInShadow", UniformType::F1,	& g_leaves_ReduceTranslucencyInShadow);
	lLODShader->registerUniform("shadow_intensity",			UniformType::F1,	& g_leaves_shadow_intensity);
	lLODShader->registerUniform("LightDiffuseColor",		UniformType::F3,	& g_leaves_LightDiffuseColor.data);
	lLODShader->registerUniform("window_size",				UniformType::F2,	& g_window_sizes.data);		


	// create branch data texture
	createDataTexture();
	// link data texture
	//dataTexture->save("dataTexture.png");
	branchShader->linkTexture(dataTexture);
	branchShader_sh->linkTexture(dataTexture);
	n_branchShader->linkTexture(dataTexture);
	leafShader	->linkTexture(dataTexture);
	leafShader_sh->linkTexture(dataTexture);
	n_leafShader->linkTexture(dataTexture);
	bLODShader	->linkTexture(dataTexture);
	lLODShader	->linkTexture(dataTexture);


	// create VBO & EBO with geometry...
	createVBOs();

	// link vbos & shaders
	branchesVBO	->compileWithShader(branchShader);
	branchesVBO	->compileWithShader(branchShader_sh);
	branchesVBO ->compileWithShader(n_branchShader);
	leavesVBO	->compileWithShader(leafShader);
	leavesVBO	->compileWithShader(leafShader_sh);
	leavesVBO	->compileWithShader(n_leafShader);

	branchesVBO	->compileWithShader(bLODShader);
	leavesVBO	->compileWithShader(lLODShader);

	//printf("branch count = %i\n", branchCount);
	//printf("DYN_TREE done (branch VBOid:%i, branchEBOid:%i, leafVBOid:%i)\n", branchesVBO->getID(), branchesEBO->getID(), leavesVBO->getID());


}

void DTree::initLOD1b()
{
	
	// create slices

	// create 2 sliceSets (cross, double sided)
	v3 dir = v3(-1.0, 0.0, 0.0);
	v3 right = v3(0.0, 0.0, -1.0);
	float res = 512;
	lod1_win_resolution = v2 (res, res);
	
	DTreeSliceSet * set;
	slice_count = 3;
	resolution_x = lod1_win_resolution.x;
	resolution_y = lod1_win_resolution.y;
	
	set = new DTreeSliceSet();
	set->rotation_y = 0.0f;	
	this->createSlices(dir, right, false);
	set->setSlices(this->slices);
	sliceSets2.push_back(set);
	slices.clear();	
	
	dir.rotateY(60*DEG_TO_RAD);
	right.rotateY(60*DEG_TO_RAD);
	set = new DTreeSliceSet();
	set->rotation_y = 60;
	this->createSlices(dir, right, false);
	set->setSlices(this->slices);
	//set->createFromDir(this, dir);
	sliceSets2.push_back(set);
	slices.clear();	

	dir = v3(-1.0, 0.0, 0.0);
	right = v3(0.0, 0.0, -1.0);
	dir.rotateY(-60*DEG_TO_RAD);
	right.rotateY(-60*DEG_TO_RAD);
	set = new DTreeSliceSet();	
	this->createSlices(dir, right, false);
	set->rotation_y = -60;
	set->setSlices(this->slices);
	//set->createFromDir(this, dir);
	sliceSets2.push_back(set);
	slices.clear();	
	// join textures to one...
	joinSliceSetsTextures();
	
	// create mipmaps
	jColorMap->generateMipmaps();
	jNormalMap->generateMipmaps();
	jNormalMap->setParameterI(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//jDepthMap->generateMipmaps();

	// init shaders
	lod1shader2 = new Shader("lod1");
	lod1shader2->loadShader("shaders/test3_vs.glsl", "shaders/test3_fs.glsl");
	
	lod1shader_shadow = new Shader("lod1_shadow");
	lod1shader_shadow->loadShader("shaders/test5_shadow_vs.glsl", "shaders/test5_shadow_fs.glsl");
	
	jColorMap			 ->textureUnitNumber = 0;
	jNormalMap			 ->textureUnitNumber = 0;
	branchNoiseTexture	 ->textureUnitNumber = 0;
	leafNoiseTexture	 ->textureUnitNumber = 0;
	jDataMap			 ->textureUnitNumber = 0;
	jDepthMap			 ->textureUnitNumber = 0;
	seasonMap			 ->textureUnitNumber = 0;
	lod1shader_shadow->linkTexture(jColorMap);
	lod1shader_shadow->linkTexture(jNormalMap);
	lod1shader_shadow->linkTexture(branchNoiseTexture);
	lod1shader_shadow->linkTexture(leafNoiseTexture);
	lod1shader_shadow->linkTexture(jDataMap);
	lod1shader_shadow->linkTexture(jDepthMap);
	lod1shader_shadow->linkTexture(seasonMap);
	// link textures to shader
	//shader->linkTexture(colorMap			);
	//shader->linkTexture(displacementMap	);
	//shader->linkTexture(dataMap			);

	l2_color	= lod1shader2->getGLLocation("colorMap"			);
	l2_displ	= lod1shader2->getGLLocation("leaf_noise_tex"	);
	l2_displ2	= lod1shader2->getGLLocation("branch_noise_tex"	);
	l2_data		= lod1shader2->getGLLocation("dataMap"			);
	l2_normal	= lod1shader2->getGLLocation("normalMap"		);
	l2_depth	= lod1shader2->getGLLocation("depthMap"			);
	
	l2_season	= lod1shader2->getGLLocation("seasonMap"		);
	lod1shader2->linkTexture(g_shadowmap1);

	lod1shader2->registerUniform("time"					, UniformType::F1, & g_float_time	);
	lod1shader2->registerUniform("time_offset"			, UniformType::F1, & time_offset	);
	lod1shader2->registerUniform("season"				, UniformType::F1, & g_season		);
	lod1shader2->registerUniform("instancing"			, UniformType::I1, & isInstancingEnabled);
	lod1shader2->registerUniform("shift"				, UniformType::F1, & g_transitionShift);
	lod1shader2->registerUniform("transition_control"	, UniformType::F1, & g_transitionControl);
	lod1shader2->registerUniform("movementVectorA"		, UniformType::F2, & g_tree_movementVectorA			);
	lod1shader2->registerUniform("movementVectorB"		, UniformType::F2, & g_tree_movementVectorB			);
	lod1shader2->registerUniform("window_size"			, UniformType::F2, & lod1_win_resolution			);
	lod1shader2->registerUniform("wood_amplitudes"		, UniformType::F4, & g_tree_wood_amplitudes.data	);
	lod1shader2->registerUniform("wood_frequencies"		, UniformType::F4, & g_tree_wood_frequencies.data	);
	lod1shader2->registerUniform("leaf_amplitude"		, UniformType::F1, & g_tree_leaf_amplitude			);
	lod1shader2->registerUniform("leaf_frequency"		, UniformType::F1, & g_tree_leaf_frequency			);
	//lod1shader2->registerUniform("varA"					, UniformType::F1, & g_varA							);
	//lod1shader2->registerUniform("scale"				, UniformType::F1, & g_ParallaxScale				);
	//lod1shader2->registerUniform("bias"					, UniformType::F1, & g_ParallaxBias					);
	lod1shader2->registerUniform("MultiplyAmbient",				UniformType::F1,	& g_leaves_MultiplyAmbient);
	lod1shader2->registerUniform("MultiplyDiffuse",				UniformType::F1,	& g_leaves_MultiplyDiffuse);
	lod1shader2->registerUniform("MultiplySpecular",			UniformType::F1,	& g_leaves_MultiplySpecular);
	lod1shader2->registerUniform("MultiplyTranslucency",		UniformType::F1,	& g_leaves_MultiplyTranslucency);
	lod1shader2->registerUniform("ReduceTranslucencyInShadow",	UniformType::F1,	& g_leaves_ReduceTranslucencyInShadow);
	lod1shader2->registerUniform("shadow_intensity",			UniformType::F1,	& g_leaves_shadow_intensity);
	lod1shader2->registerUniform("LightDiffuseColor",			UniformType::F3,	& g_leaves_LightDiffuseColor.data);
	
	lod1shader2->registerUniform("near",	UniformType::F1,	& g_ShadowNear);
	lod1shader2->registerUniform("far",		UniformType::F1,	& g_ShadowFar);

	lod1shader2->registerUniform("LightMVPCameraVInverseMatrix",			UniformType::M4,	g_LightMVPCameraVInverseMatrix);
	lod1shader2->registerUniform("shadowMappingEnabled",	UniformType::B1,	& g_ShadowMappingEnabled);

	// shadow shader
	lod1shader_shadow->registerUniform("time"						, UniformType::F1, & g_float_time					);
	lod1shader_shadow->registerUniform("time_offset"				, UniformType::F1, & time_offset					);
	lod1shader_shadow->registerUniform("season"						, UniformType::F1, & g_season						);
	lod1shader_shadow->registerUniform("instancing"					, UniformType::I1, & isInstancingEnabled			);
	lod1shader_shadow->registerUniform("shift"						, UniformType::F1, & g_transitionShift				);
	lod1shader_shadow->registerUniform("transition_control"			, UniformType::F1, & g_transitionControl			);
	lod1shader_shadow->registerUniform("movementVectorA"			, UniformType::F2, & g_tree_movementVectorA			);
	lod1shader_shadow->registerUniform("movementVectorB"			, UniformType::F2, & g_tree_movementVectorB			);
	lod1shader_shadow->registerUniform("window_size"				, UniformType::F2, & lod1_win_resolution			);
	lod1shader_shadow->registerUniform("wood_amplitudes"			, UniformType::F4, & g_tree_wood_amplitudes.data	);
	lod1shader_shadow->registerUniform("wood_frequencies"			, UniformType::F4, & g_tree_wood_frequencies.data	);
	lod1shader_shadow->registerUniform("leaf_amplitude"				, UniformType::F1, & g_tree_leaf_amplitude			);
	lod1shader_shadow->registerUniform("leaf_frequency"				, UniformType::F1, & g_tree_leaf_frequency			);
	lod1shader_shadow->registerUniform("dither"						, UniformType::F1, & g_dither						);
	lod1shader_shadow->registerUniform("near"						, UniformType::F1, & g_ShadowNear);
	lod1shader_shadow->registerUniform("far"						, UniformType::F1, & g_ShadowFar);

	iu1Loc1 = lod1shader2->getLocation("u_colorVariance");
	
	tmLoc0 = lod1shader2->getAttributeLocation("transformMatrix");
	iaLoc1 = lod1shader2->getAttributeLocation("colorVariance");
	tmLoc0_shadow = lod1shader_shadow->getAttributeLocation("transformMatrix");
	iaLoc1_shadow = lod1shader_shadow->getAttributeLocation("colorVariance");

	int i;
	// int i = lod1shader2->registerUniform("time_offset"	, UniformType::F1, & tree_time_offset);
	// u_time_offset = lod1shader2->getUniform(i);
	
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
	vector<v3> vertexArr0;
	vector<v3> normalArr0;
	vector<v3> tangentArr0;
	vector<v2> texCoordArr0;
	vector<v2> sliceAtrArr0;

	vertexArr0.push_back(v3( 0.0,  0.0, -0.5 ));
	vertexArr0.push_back(v3( 0.0,  0.0,  0.5 ));
	vertexArr0.push_back(v3( 0.0,  1.0,  0.5 ));
	vertexArr0.push_back(v3( 0.0,  1.0, -0.5 ));
	vertexArr0.push_back(v3(-0.1,  0.0, -0.5 ));
	vertexArr0.push_back(v3(-0.1,  0.0,  0.5 ));
	vertexArr0.push_back(v3(-0.1,  1.0,  0.5 ));
	vertexArr0.push_back(v3(-0.1,  1.0, -0.5 ));
	vertexArr0.push_back(v3( 0.1,  0.0, -0.5 ));
	vertexArr0.push_back(v3( 0.1,  0.0,  0.5 ));
	vertexArr0.push_back(v3( 0.1,  1.0,  0.5 ));
	vertexArr0.push_back(v3( 0.1,  1.0, -0.5 ));

	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));

	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	
	float y_max = 0.97; 

	texCoordArr0.push_back(v2( 0.0,  0.0	));
	texCoordArr0.push_back(v2( 1.0,  0.0	));
	texCoordArr0.push_back(v2( 1.0,  y_max	));
	texCoordArr0.push_back(v2( 0.0,  y_max	));
	texCoordArr0.push_back(v2( 0.0,  0.0	));
	texCoordArr0.push_back(v2( 1.0,  0.0	));
	texCoordArr0.push_back(v2( 1.0,  y_max	));
	texCoordArr0.push_back(v2( 0.0,  y_max	));
	texCoordArr0.push_back(v2( 0.0,  0.0	));
	texCoordArr0.push_back(v2( 1.0,  0.0	));
	texCoordArr0.push_back(v2( 1.0,  y_max	));
	texCoordArr0.push_back(v2( 0.0,  y_max	));

	// first slice number, second sliceSet number
	sliceAtrArr0.push_back(v2( 1.0,  0.0));
	sliceAtrArr0.push_back(v2( 1.0,  0.0));
	sliceAtrArr0.push_back(v2( 1.0,  0.0));
	sliceAtrArr0.push_back(v2( 1.0,  0.0));
	sliceAtrArr0.push_back(v2( 0.0,  0.0));
	sliceAtrArr0.push_back(v2( 0.0,  0.0));
	sliceAtrArr0.push_back(v2( 0.0,  0.0));
	sliceAtrArr0.push_back(v2( 0.0,  0.0));
	sliceAtrArr0.push_back(v2( 2.0,  0.0));
	sliceAtrArr0.push_back(v2( 2.0,  0.0));
	sliceAtrArr0.push_back(v2( 2.0,  0.0));
	sliceAtrArr0.push_back(v2( 2.0,  0.0));
	
	// A 

	int sliceCount = 3;
	v3 axisY = v3(0.0, 1.0, 0.0);
	// B
	float angle= 60.0;
	for (i=0; i<(sliceCount*4); i++){
		vertexArr0.push_back(vertexArr0[i].getRotated(angle*DEG_TO_RAD, axisY));
		normalArr0.push_back(normalArr0[i].getRotated(angle*DEG_TO_RAD, axisY));
		tangentArr0.push_back(tangentArr0[i].getRotated(angle*DEG_TO_RAD, axisY));
		texCoordArr0.push_back(texCoordArr0[i]);
		sliceAtrArr0.push_back(v2(sliceAtrArr0[i].x, 1.0));
	}
	// C
	angle= -60.0;
	for (i=0; i<(sliceCount*4); i++){
		vertexArr0.push_back(vertexArr0[i].getRotated(angle*DEG_TO_RAD, axisY));
		normalArr0.push_back(normalArr0[i].getRotated(angle*DEG_TO_RAD, axisY));
		tangentArr0.push_back(tangentArr0[i].getRotated(angle*DEG_TO_RAD, axisY));
		texCoordArr0.push_back(texCoordArr0[i]);
		sliceAtrArr0.push_back(v2(sliceAtrArr0[i].x, 2.0));
	}
	// copy to buffer
	GLfloat* vertices	= new GLfloat[3*vertexArr0.size()]; 
	GLfloat* normals	= new GLfloat[3*normalArr0.size()]; 
	GLfloat* tangents	= new GLfloat[3*tangentArr0.size()]; 
	GLfloat* texCoords	= new GLfloat[2*texCoordArr0.size()]; 
	GLfloat* sliceAttr	= new GLfloat[2*sliceAtrArr0.size()]; 
	int vertexCount = vertexArr0.size();
	for (i=0; i<vertexCount; i++){
		vertices[i*3 + 0] = vertexArr0[i].x;
		vertices[i*3 + 1] = vertexArr0[i].y;
		vertices[i*3 + 2] = vertexArr0[i].z;
		normals	[i*3 + 0] = normalArr0[i].x;
		normals	[i*3 + 1] = normalArr0[i].y;
		normals	[i*3 + 2] = normalArr0[i].z;
		tangents[i*3 + 0] = tangentArr0[i].x;
		tangents[i*3 + 1] = tangentArr0[i].y;
		tangents[i*3 + 2] = tangentArr0[i].z;
		texCoords[i*2 + 0] = texCoordArr0[i].x;
		texCoords[i*2 + 1] = texCoordArr0[i].y;
		sliceAttr[i*2 + 0] = sliceAtrArr0[i].x;
		sliceAttr[i*2 + 1] = sliceAtrArr0[i].y;
	}
	// delete vector 'vertexArr0'

	vertexArr0.clear();
	normalArr0.clear();
	tangentArr0.clear();
	texCoordArr0.clear();
	sliceAtrArr0.clear();


	// init VBO
	int count = vertexCount;
	lod1vbo2 = new VBO();
	lod1vbo2->setVertexCount(count);
	// position
	VBODataSet * dataSet = new VBODataSet(
		vertices,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::POSITION,
		true
	);
	lod1vbo2->addVertexAttribute( dataSet );
	// normal
	dataSet = new VBODataSet(
		normals,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::NORMAL,
		false
	);
	lod1vbo2->addVertexAttribute( dataSet );
	// tangent
	dataSet = new VBODataSet(
		tangents,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TANGENT,
		false
	);
	lod1vbo2->addVertexAttribute( dataSet );
	// texture coordinates
	dataSet = new VBODataSet(
		texCoords,
		2*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TEXCOORD0,
		false
	);
	lod1vbo2->addVertexAttribute( dataSet );
	// slice attributes
	dataSet = new VBODataSet(
		sliceAttr,
		2*sizeof(GLfloat),
		GL_FLOAT, 
		"sliceDescription",
		false
	);
	lod1vbo2->addVertexAttribute( dataSet );
	// link vbo and shaders
	lod1vbo2->compileData(GL_STATIC_DRAW);
	lod1vbo2->compileWithShader(lod1shader2);
	lod1vbo2->compileWithShader(lod1shader_shadow);

	/*	
	*	CREATE EBO
	*		
	*/
	unsigned int ebo_data[3*36] = {	 0, 1, 2, 3,	 4, 5, 6, 7,	 8, 9,10,11, // A
									24,25,26,27,	28,29,30,31,	32,33,34,35, // C
									12,13,14,15,	16,17,18,19,	20,21,22,23, // B
									
									24,25,26,27,	28,29,30,31,	32,33,34,35, // C
									12,13,14,15,	16,17,18,19,	20,21,22,23, // B
									 0, 1, 2, 3,	 4, 5, 6, 7,	 8, 9,10,11, // A

									12,13,14,15,	16,17,18,19,	20,21,22,23, // B
									 0, 1, 2, 3,	 4, 5, 6, 7,	 8, 9,10,11, // A
									24,25,26,27,	28,29,30,31,	32,33,34,35  // C
									
								};
	int iCnt = 3 * 4 * sliceCount * 3; // 4 = vertexCnt of one slice, 3 = number of slice sets
	
	eboLOD1 = new EBO();
	eboLOD1->create(GL_UNSIGNED_INT, GL_QUADS, iCnt, ebo_data, GL_STATIC_DRAW);
}

void DTree::initLOD1()
{
	
	// create slices

	// create 2 sliceSets (cross, double sided)
	v3 dir = v3(-1.0, 0.0, 0.0);
	v3 right = v3(0.0, 0.0, -1.0);
	float res = 512;
	lod1_win_resolution = v2 (res, res);
	
	DTreeSliceSet * set;
	slice_count = 3;
	resolution_x = lod1_win_resolution.x;
	resolution_y = lod1_win_resolution.y;
	
	set = new DTreeSliceSet();
	set->rotation_y = 180.0f;	
	this->createSlices(-dir, right, false);
	set->setSlices(this->slices);
	sliceSets.push_back(set);
	
	
	//dir.rotateY(-30*DEG_TO_RAD);
	//right.rotateY(-30*DEG_TO_RAD);
	dir.rotateY(60*DEG_TO_RAD);
	right.rotateY(60*DEG_TO_RAD);
	set = new DTreeSliceSet();
	set->rotation_y = 60;
	this->createSlices(dir, right, false);
	set->setSlices(this->slices);
	//set->createFromDir(this, dir);
	sliceSets.push_back(set);

	dir = v3(-1.0, 0.0, 0.0);
	right = v3(0.0, 0.0, -1.0);
	//dir.rotateY(+30*DEG_TO_RAD);

	dir.rotateY(-60*DEG_TO_RAD);	
	right.rotateY(-60*DEG_TO_RAD);
	set = new DTreeSliceSet();	
	this->createSlices(dir, right, false);
	set->rotation_y = -60;
	set->setSlices(this->slices);
	//set->createFromDir(this, dir);
	sliceSets.push_back(set);

	// join textures to one...
	//joinSliceSetsTextures();
	
	
	
	// init shaders




	lod1shader = new Shader("test");
	lod1shader->loadShader("shaders/test2_vs.glsl", "shaders/test2_fs.glsl");
	// link textures to shader
	//shader->linkTexture(colorMap			);
	//shader->linkTexture(displacementMap		);
	//shader->linkTexture(dataMap				);

	l_color		 = lod1shader->getGLLocation("colorMap"			);
	l_displ		 = lod1shader->getGLLocation("leaf_noise_tex"	);
	l_displ2	 = lod1shader->getGLLocation("branch_noise_tex"	);
	l_data		 = lod1shader->getGLLocation("dataMap"			);
	l_normal	 = lod1shader->getGLLocation("normalMap"		);

	lod1shader->registerUniform("time"					, UniformType::F1, & g_float_time					);
	
	lod1shader->registerUniform("wave_amplitude"		, UniformType::F1, & g_tree_wave_amplitude			);
	lod1shader->registerUniform("wave_frequency"		, UniformType::F1, & g_tree_wave_frequency			);
	lod1shader->registerUniform("movementVectorA"		, UniformType::F2, & g_tree_movementVectorA			);
	lod1shader->registerUniform("movementVectorB"		, UniformType::F2, & g_tree_movementVectorB			);
	lod1shader->registerUniform("wave_y_offset"			, UniformType::F1, & g_tree_wave_y_offset			);
	lod1shader->registerUniform("wave_increase_factor"	, UniformType::F1, & g_tree_wave_increase_factor	);
	lod1shader->registerUniform("window_size"			, UniformType::F2, & lod1_win_resolution					);


	lod1shader->registerUniform("wood_amplitudes"		, UniformType::F4, & g_tree_wood_amplitudes.data	);
	lod1shader->registerUniform("wood_frequencies"		, UniformType::F4, & g_tree_wood_frequencies.data	);
	lod1shader->registerUniform("leaf_amplitude"		, UniformType::F1, & g_tree_leaf_amplitude			);
	lod1shader->registerUniform("leaf_frequency"		, UniformType::F1, & g_tree_leaf_frequency			);

	//int i = lod1shader->registerUniform("time_offset"	, UniformType::F1, & tree_time_offset);
	//u_time_offset = lod1shader->getUniform(i);
	
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
	lod1vbo = new VBO();
	lod1vbo->setVertexCount(count);
	// position
	VBODataSet * dataSet = new VBODataSet(
		PLANE_VERTEX_ARRAY2,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::POSITION,
		true
	);
	lod1vbo->addVertexAttribute( dataSet );
	// normal
	dataSet = new VBODataSet(
		PLANE_NORMAL_ARRAY2,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::NORMAL,
		false
	);
	lod1vbo->addVertexAttribute( dataSet );
	// tangent
	dataSet = new VBODataSet(
		PLANE_TANGENT_ARRAY,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TANGENT,
		false
	);
	lod1vbo->addVertexAttribute( dataSet );
	// texture coordinates
	dataSet = new VBODataSet(
		PLANE_TEX_COORD_ARRAY2,
		2*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TEXCOORD0,
		false
	);
	lod1vbo->addVertexAttribute( dataSet );



	// link vbo and shaders
	lod1vbo->compileData(GL_STATIC_DRAW);
	lod1vbo->compileWithShader(lod1shader);
	
}

void DTree::initLOD2()
{
	// create slices

	// create 2 sliceSets (cross, double sided)
	v3 dir = v3(-1.0, 0.0, 0.0);
	v3 right = v3(0.0, 0.0, -1.0);
	float res = 256;
	lod2_win_resolution = v2 (res, res);
	
	DTreeSliceSet * set;
	slice_count = 1;
	resolution_x = lod2_win_resolution.x;
	resolution_y = lod2_win_resolution.y;
	
	set = new DTreeSliceSet();
	set->rotation_y = 0.0f;	
	this->createSlices(dir, right, false);
	set->setSlices(this->slices);
	lod2color1	= set->getSlice(0)->colormap;
	lod2normal1 = set->getSlice(0)->normalmap;
	lod2branch1 = set->getSlice(0)->datamap;
	lod2depth1	= set->getSlice(0)->depthmap;
	slices.clear();	

	dir.rotateY(90*DEG_TO_RAD);
	right.rotateY(90*DEG_TO_RAD);
	set = new DTreeSliceSet();
	set->rotation_y = 90;
	this->createSlices(dir, right, false);
	set->setSlices(this->slices);	
	lod2color2	= set->getSlice(0)->colormap;	
	lod2normal2 = set->getSlice(0)->normalmap;
	lod2branch2 = set->getSlice(0)->datamap;
	lod2depth2	= set->getSlice(0)->depthmap;
	slices.clear();	

	// join textures to one...
	//joinSliceSetsTextures();
	
	// create mipmaps
	//lod2color1->generateMipmaps();
	//lod2color2->generateMipmaps();
	//lod2branch1->generateMipmaps();
	//lod2branch2->generateMipmaps();
	//lod2normal1->generateMipmaps();
	//lod2normal2->generateMipmaps();
	//lod2normal1->setParameterI(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//lod2normal2->setParameterI(GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//lod2depth1->generateMipmaps();
	//lod2depth2->generateMipmaps();

	//jColorMap->setParameterI(GL_TEXTURE_SAMPLES, GL_LINEAR_MIPMAP_LINEAR);
	
	// init shaders
	lod2shader = new Shader("lod2");
	lod2shader->loadShader("shaders/test4_vs.glsl", "shaders/test4_fs.glsl");
	lod2shader->linkTexture(g_shadowmap1);
	lod2shader_shadow = new Shader("lod2_shadow");
	lod2shader_shadow->loadShader("shaders/test6_shadow_vs.glsl", "shaders/test6_shadow_fs.glsl");
	
	// link textures to shader
	lod2color1 ->inShaderName = "color_tex_1";
	lod2color2 ->inShaderName = "color_tex_2";
	lod2normal1->inShaderName = "normal_tex_1";
	lod2normal2->inShaderName = "normal_tex_2";
	lod2branch1->inShaderName = "branch_tex_1";
	lod2branch2->inShaderName = "branch_tex_2";
	lod2depth1->inShaderName = "depth_tex_1";
	lod2depth2->inShaderName = "depth_tex_2";

	lod2color1 ->textureUnitNumber = 0;
	lod2color2 ->textureUnitNumber = 0;
	lod2normal1->textureUnitNumber = 0;
	lod2normal2->textureUnitNumber = 0;
	lod2branch1->textureUnitNumber = 0;
	lod2branch2->textureUnitNumber = 0;
	lod2depth1->textureUnitNumber = 0;
	lod2depth2->textureUnitNumber = 0;

	lod2shader_shadow->linkTexture(seasonMap);
	lod2shader_shadow->linkTexture(leafNoiseTexture);
	lod2shader_shadow->linkTexture(branchNoiseTexture);
	lod2shader_shadow->linkTexture(lod2color1);
	lod2shader_shadow->linkTexture(lod2color2);
	lod2shader_shadow->linkTexture(lod2normal1);
	lod2shader_shadow->linkTexture(lod2normal2);
	lod2shader_shadow->linkTexture(lod2branch1);
	lod2shader_shadow->linkTexture(lod2branch2);
	lod2shader_shadow->linkTexture(lod2depth1);
	lod2shader_shadow->linkTexture(lod2depth2);


	// lod2loc_color_tex_1	 = lod2shader->getGLLocation("color_tex_1"		);
	// lod2loc_color_tex_2	 = lod2shader->getGLLocation("color_tex_2"		);
	// lod2loc_normal_tex_1 = lod2shader->getGLLocation("normal_tex_1"		);
	// lod2loc_normal_tex_2 = lod2shader->getGLLocation("normal_tex_2"		);
	// lod2loc_branch_tex_1 = lod2shader->getGLLocation("branch_tex_1"		);
	// lod2loc_branch_tex_2 = lod2shader->getGLLocation("branch_tex_2"		);


	lod2shader->linkTexture(seasonMap);
	lod2shader->linkTexture(leafNoiseTexture);
	lod2shader->linkTexture(branchNoiseTexture);
	lod2shader->linkTexture(lod2color1);
	lod2shader->linkTexture(lod2color2);
	lod2shader->linkTexture(lod2normal1);
	lod2shader->linkTexture(lod2normal2);
	lod2shader->linkTexture(lod2branch1);
	lod2shader->linkTexture(lod2branch2);




	//lod2loc_season_tex	= lod2shader->getGLLocation("seasonMap"		);
	//lod2loc_leaf_tex	= lod2shader->getGLLocation("leaf_noise_tex"	);
	//lod2loc_branch_tex	= lod2shader->getGLLocation("branch_noise_tex"	);


	lod2shader->registerUniform("time"						, UniformType::F1, & g_float_time	);
	lod2shader->registerUniform("time_offset"				, UniformType::F1, & time_offset	);
	lod2shader->registerUniform("season"					, UniformType::F1, & g_season		);
	lod2shader->registerUniform("instancing"				, UniformType::I1, & isInstancingEnabled);
	lod2shader->registerUniform("movementVectorA"			, UniformType::F2, & g_tree_movementVectorA			);
	lod2shader->registerUniform("movementVectorB"			, UniformType::F2, & g_tree_movementVectorB			);
	lod2shader->registerUniform("window_size"				, UniformType::F2, & lod2_win_resolution			);
	lod2shader->registerUniform("wood_amplitudes"			, UniformType::F4, & g_tree_wood_amplitudes.data	);
	lod2shader->registerUniform("wood_frequencies"			, UniformType::F4, & g_tree_wood_frequencies.data	);
	lod2shader->registerUniform("leaf_amplitude"			, UniformType::F1, & g_tree_leaf_amplitude			);
	lod2shader->registerUniform("leaf_frequency"			, UniformType::F1, & g_tree_leaf_frequency			);
	lod2shader->registerUniform("MultiplyAmbient",				UniformType::F1,	& g_leaves_MultiplyAmbient);
	lod2shader->registerUniform("MultiplyDiffuse",				UniformType::F1,	& g_leaves_MultiplyDiffuse);
	lod2shader->registerUniform("MultiplySpecular",				UniformType::F1,	& g_leaves_MultiplySpecular);
	lod2shader->registerUniform("MultiplyTranslucency",			UniformType::F1,	& g_leaves_MultiplyTranslucency);
	lod2shader->registerUniform("ReduceTranslucencyInShadow",	UniformType::F1,	& g_leaves_ReduceTranslucencyInShadow);
	lod2shader->registerUniform("shadow_intensity",				UniformType::F1,	& g_leaves_shadow_intensity);
	lod2shader->registerUniform("LightDiffuseColor",			UniformType::F3,	& g_leaves_LightDiffuseColor.data);
	lod2loc_colorVariance = lod2shader->getLocation("u_colorVariance");
	lod2shader->registerUniform("LightMVPCameraVInverseMatrix"	, UniformType::M4,	g_LightMVPCameraVInverseMatrix);
	lod2shader->registerUniform("shadowMappingEnabled"			, UniformType::B1, & g_ShadowMappingEnabled);
	lod2shader->registerUniform("near"							, UniformType::F1, & g_ShadowNear);
	lod2shader->registerUniform("far"							, UniformType::F1, & g_ShadowFar);

	// shadow
	lod2shader_shadow->registerUniform("time"					, UniformType::F1, & g_float_time	);
	lod2shader_shadow->registerUniform("time_offset"			, UniformType::F1, & time_offset	);
	lod2shader_shadow->registerUniform("season"					, UniformType::F1, & g_season		);
	lod2shader_shadow->registerUniform("instancing"				, UniformType::I1, & isInstancingEnabled);
	lod2shader_shadow->registerUniform("movementVectorA"		, UniformType::F2, & g_tree_movementVectorA			);
	lod2shader_shadow->registerUniform("movementVectorB"		, UniformType::F2, & g_tree_movementVectorB			);
	lod2shader_shadow->registerUniform("window_size"			, UniformType::F2, & lod2_win_resolution			);
	lod2shader_shadow->registerUniform("wood_amplitudes"		, UniformType::F4, & g_tree_wood_amplitudes.data	);
	lod2shader_shadow->registerUniform("wood_frequencies"		, UniformType::F4, & g_tree_wood_frequencies.data	);
	lod2shader_shadow->registerUniform("leaf_amplitude"			, UniformType::F1, & g_tree_leaf_amplitude			);
	lod2shader_shadow->registerUniform("leaf_frequency"			, UniformType::F1, & g_tree_leaf_frequency			);
	lod2shader_shadow->registerUniform("near"					, UniformType::F1, & g_ShadowNear);
	lod2shader_shadow->registerUniform("far"					, UniformType::F1, & g_ShadowFar);

	tm2Loc0 = lod2shader->getAttributeLocation("transformMatrix");
	ia2Loc1 = lod2shader->getAttributeLocation("colorVariance");
	tm2Loc0_shadow = lod2shader_shadow->getAttributeLocation("transformMatrix");
	ia2Loc1_shadow = lod2shader_shadow->getAttributeLocation("colorVariance");

	int i;
	
	vector<v3> vertexArr0;
	vector<v3> normalArr0;
	vector<v3> tangentArr0;
	vector<v2> texCoordArr0;
	vector<v2> sliceAtrArr0;

	vertexArr0.push_back(v3( 0.0,  0.0, -0.5 ));
	vertexArr0.push_back(v3( 0.0,  0.0,  0.5 ));
	vertexArr0.push_back(v3( 0.0,  1.0,  0.5 ));
	vertexArr0.push_back(v3( 0.0,  1.0, -0.5 ));
	
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));
	normalArr0.push_back(v3( 1.0,  0.0,  0.0));

	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	tangentArr0.push_back(v3( 0.0,  0.0,  1.0));
	
	float y_max = 1.0; 

	texCoordArr0.push_back(v2( 0.0,  0.0	));
	texCoordArr0.push_back(v2( 1.0,  0.0	));
	texCoordArr0.push_back(v2( 1.0,  y_max	));
	texCoordArr0.push_back(v2( 0.0,  y_max	));

	// first slice number, second sliceSet number
	sliceAtrArr0.push_back(v2( 0.0,  0.0));
	sliceAtrArr0.push_back(v2( 0.0,  0.0));
	sliceAtrArr0.push_back(v2( 0.0,  0.0));
	sliceAtrArr0.push_back(v2( 0.0,  0.0));
	
	// A 

	int sliceCount = 3;
	v3 axisY = v3(0.0, 1.0, 0.0);
	// B
	float angle= 90.0;
	for (i=0; i<4; i++){
		vertexArr0.push_back(vertexArr0[i].getRotated(angle*DEG_TO_RAD, axisY));
		normalArr0.push_back(normalArr0[i].getRotated(angle*DEG_TO_RAD, axisY));
		tangentArr0.push_back(tangentArr0[i].getRotated(angle*DEG_TO_RAD, axisY));
		texCoordArr0.push_back(texCoordArr0[i]);
		sliceAtrArr0.push_back(v2(sliceAtrArr0[i].x, 1.0));
	}
	// copy to buffer
	GLfloat* vertices	= new GLfloat[3*vertexArr0.size()]; 
	GLfloat* normals	= new GLfloat[3*normalArr0.size()]; 
	GLfloat* tangents	= new GLfloat[3*tangentArr0.size()]; 
	GLfloat* texCoords	= new GLfloat[2*texCoordArr0.size()]; 
	GLfloat* sliceAttr	= new GLfloat[2*sliceAtrArr0.size()]; 
	int vertexCount = vertexArr0.size();
	for (i=0; i<vertexCount; i++){
		vertices[i*3 + 0] = vertexArr0[i].x;
		vertices[i*3 + 1] = vertexArr0[i].y;
		vertices[i*3 + 2] = vertexArr0[i].z;
		normals	[i*3 + 0] = normalArr0[i].x;
		normals	[i*3 + 1] = normalArr0[i].y;
		normals	[i*3 + 2] = normalArr0[i].z;
		tangents[i*3 + 0] = tangentArr0[i].x;
		tangents[i*3 + 1] = tangentArr0[i].y;
		tangents[i*3 + 2] = tangentArr0[i].z;
		texCoords[i*2 + 0] = texCoordArr0[i].x;
		texCoords[i*2 + 1] = texCoordArr0[i].y;
		sliceAttr[i*2 + 0] = sliceAtrArr0[i].x;
		sliceAttr[i*2 + 1] = sliceAtrArr0[i].y;
	}
	// delete vector 'vertexArr0'

	vertexArr0.clear();
	normalArr0.clear();
	tangentArr0.clear();
	texCoordArr0.clear();
	sliceAtrArr0.clear();


	// init VBO
	int count = vertexCount;
	lod2vbo = new VBO();
	lod2vbo->setVertexCount(count);
	// position
	VBODataSet * dataSet = new VBODataSet(
		vertices,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::POSITION,
		true
	);
	lod2vbo->addVertexAttribute( dataSet );
	// normal
	dataSet = new VBODataSet(
		normals,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::NORMAL,
		false
	);
	lod2vbo->addVertexAttribute( dataSet );
	// tangent
	dataSet = new VBODataSet(
		tangents,
		3*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TANGENT,
		false
	);
	lod2vbo->addVertexAttribute( dataSet );
	// texture coordinates
	dataSet = new VBODataSet(
		texCoords,
		2*sizeof(GLfloat),
		GL_FLOAT, 
		ATTRIB_NAMES::TEXCOORD0,
		false
	);
	lod2vbo->addVertexAttribute( dataSet );
	// slice attributes
	dataSet = new VBODataSet(
		sliceAttr,
		2*sizeof(GLfloat),
		GL_FLOAT, 
		"sliceDescription",
		false
	);
	lod2vbo->addVertexAttribute( dataSet );
	// link vbo and shaders
	lod2vbo->compileData(GL_STATIC_DRAW);
	lod2vbo->compileWithShader(lod2shader);
	lod2vbo->compileWithShader(lod2shader_shadow);

	/*	
	*	CREATE EBO
	*		
	*/
	const int iCnt = 2 * 8;
	unsigned int ebo_data[iCnt] = {	 0, 1, 2, 3,	 4, 5, 6, 7,
									 4, 5, 6, 7,	 0, 1, 2, 3,
									 
									};
	
	
	lod2ebo = new EBO();
	lod2ebo->create(GL_UNSIGNED_INT, GL_QUADS, iCnt, ebo_data, GL_STATIC_DRAW);

}

void DTree::init2(v4 ** positions_rotations, int count){
	swapCnt = 0;
	instanceFloatCount = 20;
	/*****
	* LOD1 is made of 3 slice sets... due to blending problems we need to 
	* switch the rendering order of the sliceSets -> there are 3 types of
	* meshes [A,B,C] (just different EBO), from which the best is chosen on the fly...
	*/
	// init instances
	DTreeInstanceData * instance;
	v4 * pos_rot;
	float cosA, sinA;
	for (int i=0; i< count; i++){
		pos_rot = positions_rotations[i];
		instance = new DTreeInstanceData();
		instance->position	= pos_rot->xyz();
		instance->rotation_y= pos_rot->w;
		instance->index = i;
		cosA = cos(DEG_TO_RAD*instance->rotation_y);
		sinA = sin(DEG_TO_RAD*instance->rotation_y);
		// this matrix is passed to each instanced geometry and vertex shader 
		// ensures the proper positioning...
		// note, that by changing the order of matrices passed to VS,
		// we can influence the order of rendered instances
		m4 matrix(cosA			,	0			,	-sinA	,	0	,
				  0				,	1			,	0		,	0	,
				  sinA			,	0			,	cosA	,	0	,
				  instance->position.x,	instance->position.y, instance->position.z,	1);
		instance->transformMatrix = matrix; // copy matrix

		// to be able to determine the proper type to display,
		// we need to know from which angle are we looking at the geometry
		// so we precalculate all we can now, to spare time in draw calls
		instance->dirA = v3( -1.0, 0.0, 0.0).getRotated( instance->rotation_y*DEG_TO_RAD, v3(0.0, 1.0, 0.0));	// A									// A
		instance->dirB = instance->dirA.getRotated( 60 * DEG_TO_RAD, v3(0.0, 1.0, 0.0));				// B

		// add some instance attributes
		instance->colorVariance = v3( randomf(.8f, 1.f), randomf(.8f, 1.f), randomf(.9f, 1.f)) *  randomf(.8f, 1.f);
		instance->time_offset = randomf(0.f, 1000.f); 

		instance->alpha = 1.0;
		instance->index = i;
		tree_instances.push_back(instance);
	}
	// init instance matrices
	// LOD2
	lod2_instanceMatrices.push_back( new float[ instanceFloatCount * count]);	// A
	lod2_instanceMatrices.push_back( new float[ instanceFloatCount * count]);	// B
	// LOD1
	lod1_instanceMatrices.push_back( new float[ instanceFloatCount * count]);	// A
	lod1_instanceMatrices.push_back( new float[ instanceFloatCount * count]);	// B
	lod1_instanceMatrices.push_back( new float[ instanceFloatCount * count]);	// C
	instancesInRenderQueues.push_back ( new DTreeInstanceData * [count] );		// LOD0
	instancesInRenderQueues.push_back ( new DTreeInstanceData * [count] );		// LOD0-LOD1
	instancesInRenderQueues.push_back ( new DTreeInstanceData * [count] );		// LOD1
	instancesInRenderQueues.push_back ( new DTreeInstanceData * [count] );		// LOD1-LOD2
	instancesInRenderQueues.push_back ( new DTreeInstanceData * [count] );		// LOD2
	countRenderQueues.push_back(0);							// LOD0
	countRenderQueues.push_back(0);							// LOD0-LOD1
	countRenderQueues.push_back(0);							// LOD1
	countRenderQueues.push_back(0);							// LOD1-LOD2
	countRenderQueues.push_back(0);							// LOD2

	
	lod2_typeIndices.push_back(0);
	lod2_typeIndices.push_back(1);

	lod1_typeIndices.push_back(0);
	lod1_typeIndices.push_back(1);
	lod1_typeIndices.push_back(2);

	initLOD0();
	//initLOD1();
	initLOD1b();
	initLOD2();
	
	
	
	// init instance matrices VBO
	i_matricesBuffID = 0;
	glGenBuffers(1, &i_matricesBuffID);
	glBindBuffer(GL_ARRAY_BUFFER, i_matricesBuffID);
		// load data
		glBufferData(GL_ARRAY_BUFFER, count * instanceFloatCount* sizeof(float), NULL, GL_STREAM_DRAW);  
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DTree::init(){
	initLOD0();
	initLOD1();
	//initLOD1b();
	initLOD2();
}

void DTree::update(double time){
	prepareForRender();
}

void DTree::bakeToVBO(){

}

void DTree::fixTexType(){

}

v3	 DTree::transformTexCoords(v3 &origTexCoords){
	return origTexCoords;
}

BBox * DTree::getBBox()
{
	if (bbox==NULL){
		bbox = new BBox(v3(-0.5, -1.0, -0.5),v3(0.5, 0.0, 0.5), v3(0.0, 1.0, 0.0));
	}
	return bbox;
}

void DTree::createSlices(v3 & direction, v3 & rightVector, bool half){
	// init data pre-processing shader
	time_offset = 0.0;
	Shader * dataProcessShader = new Shader("data_pre-processor");
	dataProcessShader->loadShader(DYN_TREE::SHADER_PREPROCESS_V, DYN_TREE::SHADER_PREPROCESS_F);
	GLint	gl_location		= dataProcessShader->getGLLocation("branchMap");
	int		loc_win_size	= dataProcessShader->getLocation("window_size");
	//int		loc_cam_dir		= dataProcessShader->getLocation("cam_dir");
	v3 d = direction.getRotated(90*DEG_TO_RAD, v3(0.0, 1.0, 0.0));
	v3 r = rightVector.getRotated(90*DEG_TO_RAD, v3(0.0, 1.0, 0.0));
	bLODShader->registerUniform("cam_dir", UniformType::F3, & d);
	lLODShader->registerUniform("cam_dir", UniformType::F3, & d);
	bLODShader->registerUniform("cam_right", UniformType::F3, & r);
	lLODShader->registerUniform("cam_right", UniformType::F3, & r);
	// dummy depth map
	Texture * depthmap = new Texture(GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, resolution_x, resolution_y, "dummy_depthMap");
	depthmap->textureUnit = GL_TEXTURE7;
	depthmap->textureUnitNumber = 7;
	// dummy color map

	//Texture * colormap = new Texture(GL_TEXTURE_2D, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, NULL, resolution_x, resolution_y, "dummy_colorMap");
	//colormap->textureUnit = GL_TEXTURE8;
	//colormap->textureUnitNumber = 8;

// get "slice thickness"
	BBox * box = getBBox();
	float distance = -10.f;
	v3 position = direction * distance;
	float diameter = 0.7*box->getMinSize();
	float radius = diameter*0.5f;
	float thickness = diameter/(float(slice_count));
	if (half) thickness*=0.5;
	float left = -0.5, right= 0.5, bottom= 0.0, top= 1.0, near, far;
	float positionDist = position.length();
	DTreeSlice * slice;
	int i;

	// clear previous slices
	/*
	for ( i = 0; i < slices.size(); i++){
			delete slices[i];
		}
	*/
	slices.clear();
	GLuint fbo = 0;
		

	for ( i = 0; i< slice_count; i++){
		// create FBO
			glGenFramebuffersEXT(1, &fbo);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
		// each slice
			slice = new DTreeSlice();
		// add slice
			slices.push_back(slice);
		// create & setup textures
			slice->colormap = new Texture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, resolution_x, resolution_y, "colorMap");
			slice->colormap->textureUnit = GL_TEXTURE0;
			slice->colormap->textureUnitNumber = 0;
			slice->colormap->setParameterI(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			slice->colormap->setParameterI(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			char buff[100];
			ctr++;
			sprintf(buff, "color texture %i", ctr);
			slice->colormap->description = buff;


			slice->depthmap = new Texture(GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, resolution_x, resolution_y, "depthMap");
			slice->depthmap->textureUnit = GL_TEXTURE2;
			slice->depthmap->textureUnitNumber = 2;


			slice->normalmap = new Texture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, resolution_x, resolution_y, "normalMap");
			slice->normalmap->textureUnit = GL_TEXTURE3;
			slice->normalmap->textureUnitNumber = 3;
			
			slice->branchmap = new Texture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, resolution_x, resolution_y, "branchMap");
			slice->branchmap->textureUnit = GL_TEXTURE4;
			slice->branchmap->textureUnitNumber = 4;

		// attach textures to FBO attachments

			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, slice->colormap->id  , 0);
			//glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, colormap->id , 0);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, slice->normalmap->id , 0);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, slice->branchmap->id , 0);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT , GL_TEXTURE_2D, slice->depthmap->id  , 0);
			//glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
			

			glClearColor(0.f, 0.f, 0.f, 0.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//GLenum buffers[4] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT};
			//glDrawBuffersARB(4, buffers);
			GLenum buffers[3] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT};
			glDrawBuffersARB(3, buffers);
			assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)==GL_FRAMEBUFFER_COMPLETE_EXT);
			assert( glGetError() == GL_NO_ERROR );
			printf("TREE_SLICE %i framebuffer initialized successfully\n", i);
			/*
			GLenum buffers[3] = {GL_COLOR_ATTACHMENT0_EXT,GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT};
			glDrawBuffersARB(3, buffers);
			*/

		// setup near & far plane
			near = positionDist-radius + i * thickness;
			far = near + thickness;
			// last interval must be to infinity if half slicing
			if (half && i==slice_count-1)
			{
				far = positionDist-radius + 2*(i+1) * thickness;
			}
		// setup camera to orthoprojection with respect to slice interval
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
				glLoadIdentity();
				g_window_sizes.x = resolution_x;
				g_window_sizes.y = resolution_y;
				glViewport(0, 0, g_window_sizes.x, g_window_sizes.y);
				//system("PAUSE");
				glOrtho(left, right, bottom, top, near, far);
				
			glMatrixMode(GL_MODELVIEW);
				glPushMatrix();
				glLoadIdentity();
				gluLookAt( position.x, position.y, position.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		// render offscreen
		// draw tree now...
			drawForLOD();
			glFinish();
		glMatrixMode(GL_PROJECTION);
			glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
		//
	

		// generate mipmaps where needed
			//slice->colormap->generateMipmaps();
			//slice->colormap->setParameterI(GL_TEXTURE_MIN_LOD, 0);
			//slice->colormap->setParameterI(GL_TEXTURE_BASE_LEVEL, 0);
			
			//slice->normalmap->generateMipmaps();
			//slice->depthmap->generateMipmaps();


		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		// prepare data texture from branch texture
		glDeleteFramebuffersEXT(1, &fbo);
		
		glGenFramebuffersEXT(1, &fbo);
		glDisable(GL_DEPTH_TEST);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
			slice->datamap = new Texture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, resolution_x, resolution_y, "dataMap");
			slice->datamap->textureUnit = GL_TEXTURE5;
			slice->datamap->textureUnitNumber = 5;
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, slice->datamap->id , 0);
			//glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT , GL_TEXTURE_2D, depthmap->id , 0);
			assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)==GL_FRAMEBUFFER_COMPLETE_EXT);
			assert( glGetError() == GL_NO_ERROR );
			printf("TREE_SLICE %i data framebuffer initialized successfully\n", i);

			glClearColor(1.f, 0.f, 0.f, 1.f);
			glClear(GL_COLOR_BUFFER_BIT);
			//glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
			GLenum buffers2[1] = {GL_COLOR_ATTACHMENT0_EXT};
			glDrawBuffersARB(1, buffers2);
			
			// activate shader
			//slice->branchmap->bind(GL_TEXTURE0);
			dataProcessShader->use(true);
			slice->branchmap->bind(slice->branchmap->textureUnit);
			dataProcessShader->setTexture(gl_location, slice->branchmap->textureUnitNumber);			
			dataProcessShader->setUniform2f(loc_win_size,	g_window_sizes.x, g_window_sizes.y);
			slice->branchmap->show(0,0, g_window_sizes.x, g_window_sizes.y);
			dataProcessShader->use(false);
			//slice->branchmap->unbind();
			glFinish();
			glEnable(GL_DEPTH_TEST);
		// return to normal screen rendering	
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		// branchmap not useful anymore, delete it
		SAFE_DELETE_PTR(slice->branchmap);
		
		glDeleteFramebuffersEXT(1, &fbo);
		//*/
		glDrawBuffer(GL_BACK);

		g_window_sizes.x = g_WinWidth;
		g_window_sizes.y = g_WinHeight;
		glViewport(0, 0, g_window_sizes.x, g_window_sizes.y);
		
		// delete branch texture
		SAFE_DELETE_PTR ( slice->branchmap );

	} // for each slice
	SAFE_DELETE_PTR ( depthmap );
	SAFE_DELETE_PTR ( dataProcessShader );

	//system("PAUSE");
}

void DTree::joinSliceSetsTextures(){

	Shader * joinShader = new Shader("join");
	joinShader->loadShader(DYN_TREE::SHADER_JOIN_V, DYN_TREE::SHADER_JOIN_F);
	int loc_colormap	= joinShader->getGLLocation("colorMap");	
	int loc_datamap		= joinShader->getGLLocation("dataMap");			
	int loc_normalmap	= joinShader->getGLLocation("normalMap");		
	int loc_depthmap	= joinShader->getGLLocation("depthMap");		
	int sliceSetCnt = sliceSets2.size();
	GLuint fbo = 0;
	int jResX = slice_count * resolution_x;
	int jResY = sliceSetCnt * resolution_y;



	// render offscreen
	g_window_sizes.x = jResX;
	g_window_sizes.y = jResX;
	glViewport(0, 0, g_window_sizes.x, g_window_sizes.y);

	glGenFramebuffersEXT(1, &fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	
		jColorMap	= new Texture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, jResX, jResY, "colorMap");
		jColorMap->setParameterI(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//jColorMap->setParameterI(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		jColorMap->textureUnit =  GL_TEXTURE1;
		jColorMap->textureUnitNumber =		1;
		jDataMap	= new Texture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, jResX, jResY, "dataMap");
		jDataMap->textureUnit =	  GL_TEXTURE2;
		jDataMap->textureUnitNumber =		2;
		jDepthMap	= new Texture(GL_TEXTURE_2D, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, NULL, jResX, jResY, "depthMap");
		jDepthMap->textureUnit =  GL_TEXTURE3;
		jDepthMap->textureUnitNumber =		3;
		jNormalMap	= new Texture(GL_TEXTURE_2D, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, jResX, jResY, "normalMap");
		jNormalMap->setParameterI(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//jNormalMap->setParameterI(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		jNormalMap->textureUnit = GL_TEXTURE4;
		jNormalMap->textureUnitNumber =     4;


		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, jColorMap->id , 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, jNormalMap->id , 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, jDataMap->id , 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, jDepthMap->id , 0);
		assert(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)==GL_FRAMEBUFFER_COMPLETE_EXT);
		assert( glGetError() == GL_NO_ERROR );

		glClearColor(0.f, 0.f, 0.f, 0.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		GLenum buffers[3] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT};
		glDrawBuffersARB(3, buffers);
		DTreeSlice* slice;
		// draw slices
		//glDisable(GL_DEPTH_TEST);

		joinShader->use(true);
		int x,y,width,height;
		int j=0;
		for (int i=0; i<slice_count; i++){
			// draw slice sets
			for (j=0; j<sliceSetCnt; j++){
				// draw on proper position
				slice	= sliceSets2[j]->getSlice(i); 
				x		= i*resolution_x;
				y		= j*resolution_y;
				width	= resolution_x;
				height	= resolution_y;
				joinShader->setTexture(loc_colormap,	slice->colormap->	textureUnitNumber);
				joinShader->setTexture(loc_datamap,		slice->datamap->	textureUnitNumber);
				joinShader->setTexture(loc_normalmap,	slice->normalmap->	textureUnitNumber);
				joinShader->setTexture(loc_depthmap,	slice->depthmap->	textureUnitNumber);

				slice->colormap->	bind(slice->colormap->textureUnit);
				slice->datamap->	bind(slice->datamap->textureUnit);
				slice->normalmap->	bind(slice->normalmap->textureUnit);
				slice->depthmap->	bind(slice->depthmap->textureUnit);
				
				slice->colormap->show(i*resolution_x, j*resolution_y, resolution_x, resolution_y, false);	

				slice->colormap->	unbind();
				slice->datamap->	unbind();
				slice->normalmap->	unbind();
				slice->depthmap->	unbind();
			}
		}
		joinShader->use(false);
		glFinish();
		//glEnable(GL_DEPTH_TEST);
	/*glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
		glPopMatrix();*/
		
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteFramebuffersEXT(1, &fbo);
	glDrawBuffer(GL_BACK);

	g_window_sizes.x = g_WinWidth;
	g_window_sizes.y = g_WinHeight;
	glViewport(0, 0, g_window_sizes.x, g_window_sizes.y);

	// system("PAUSE");
}
