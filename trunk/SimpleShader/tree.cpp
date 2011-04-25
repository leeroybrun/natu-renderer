#include "tree.h"
#include <stack>

/* Constructor */
Tree::Tree(){
	dataTexture		= NULL;
	leafTexture1	= NULL;

	trunk			= NULL;


	hvd_shaderProgram = 0;
	hvd_vs			= 0;
	hvd_gs			= 0;
	hvd_fs			= 0;

	branchVBOid		= 0;
	texDimX			= 0;
	texDimY			= 0;

}

/* Destructor */
Tree::~Tree(){
	if (hvd_shaderProgram!=NULL){
		destroyShaderProgram(hvd_shaderProgram, &hvd_vs, &hvd_gs, &hvd_fs);
	}
			
	if (dataTexture!=NULL){
		delete [] dataTexture;
	}
}


void Tree::init(){
	// get loactions of proper attributes
	locations[NORMAL]  = glGetAttribLocation(branchShaderID, "normal");
	locations[TANGENT] = glGetAttribLocation(branchShaderID, "tangent");
	locations[XVALS]   = glGetAttribLocation(branchShaderID, "x_vals");
	locations[BRANCH_INDEX]        = glGetAttribLocation(branchShaderID, "branch_index");
	locations[BRANCH_DATA_TEXTURE] = glGetUniformLocation(branchShaderID, "branch_data_tex");
	locations[TIME]	   = glGetUniformLocation(branchShaderID, "time");


	// make branch data texture
	createDataTexture();
	// create VBO for branch vertices
	createBranchesVBO();

	// add leaf color texture
	//leafTexture1 = new Texture();
	//leafTexture1->loadTexture(TEX_LEAF1);

}

void Tree::createBranchesVBO(){
	int i, sizeB = branches.size(), size;
	int vCnt = 0, iCnt = 0;
	// count total vertices & indices
	for (i=0; i<sizeB; i++){
		vCnt += branches[i]->getVertexCount();
		iCnt += branches[i]->getIndicesCount();
	}

	// get sizes
	for (i = 0; i < VBO_VERTEX_COMPONENTS; i++){
		sizes[i]=0;
	}
	sizes[POSITION] = 3;
	sizes[NORMAL]	= 3;
	sizes[TANGENT]	= 3;
	sizes[TEXCOORD0]= 2;
	//sizes[TEXCOORD1]= 2;
	sizes[BRANCH_INDEX]=1;
	sizes[XVALS]	= 4;
	int totalVertexSize = 0;
	for (i = 0; i < VBO_VERTEX_COMPONENTS; i++){
		vbo_data[i]= new GLfloat[vCnt*sizes[i]];
		totalVertexSize+=sizes[i];
	}
	// ebo
	ebo_data = new GLuint	[iCnt];

	// get offsets
	offsets[POSITION] = 0;
	offsets[NORMAL]   = offsets[POSITION]  + vCnt * sizes[POSITION]	* sizeof(GLfloat);
	offsets[TANGENT]  = offsets[NORMAL]    + vCnt * sizes[NORMAL]	* sizeof(GLfloat);
	offsets[TEXCOORD0]= offsets[TANGENT]   + vCnt * sizes[TANGENT]	* sizeof(GLfloat);
	offsets[XVALS]	  =	offsets[TEXCOORD0] + vCnt * sizes[TEXCOORD0]* sizeof(GLfloat);
	offsets[BRANCH_INDEX]= offsets[XVALS]  + vCnt * sizes[XVALS]	* sizeof(GLfloat);
	
	// fill arrays
	int k,l, indicesCnt, vertexCnt, offset, ch;
	int indexPtr = 0, dataPtr=0, v=0, id=0;
	int indexOffset = 0;
	Vertex *vertex;
	for (i=0; i<sizeB; i++)// each branch 
	{
		id = branches[i]->id;
		offset = dataPtr;
		for (k=0; k<branches[i]->vertices.size(); k++){
			vertex = branches[i]->vertices[k];
			// position
			vbo_data[POSITION][dataPtr*sizes[POSITION] + 0] = vertex->bPos.data[ 0 ];
			vbo_data[POSITION][dataPtr*sizes[POSITION] + 1] = vertex->bPos.data[ 1 ];
			vbo_data[POSITION][dataPtr*sizes[POSITION] + 2] = vertex->bPos.data[ 2 ];

			// normal
			vbo_data[NORMAL][dataPtr*sizes[NORMAL] + 0] = vertex->normal.data[ 0 ];
			vbo_data[NORMAL][dataPtr*sizes[NORMAL] + 1] = vertex->normal.data[ 1 ];
			vbo_data[NORMAL][dataPtr*sizes[NORMAL] + 2] = vertex->normal.data[ 2 ];

			// tangent
			vbo_data[TANGENT][dataPtr*sizes[TANGENT] + 0] = vertex->tangent.data[ 0 ];
			vbo_data[TANGENT][dataPtr*sizes[TANGENT] + 1] = vertex->tangent.data[ 1 ];
			vbo_data[TANGENT][dataPtr*sizes[TANGENT] + 2] = vertex->tangent.data[ 2 ];

			// texcoord0
			vbo_data[TEXCOORD0][dataPtr*sizes[TEXCOORD0] + 0] = vertex->textureCoords.data[ 0 ];
			vbo_data[TEXCOORD0][dataPtr*sizes[TEXCOORD0] + 1] = vertex->textureCoords.data[ 1 ];

			// xvals
			vbo_data[XVALS][dataPtr*sizes[XVALS] + 0] = vertex->x[ 0 ];
			vbo_data[XVALS][dataPtr*sizes[XVALS] + 1] = vertex->x[ 1 ];
			vbo_data[XVALS][dataPtr*sizes[XVALS] + 2] = vertex->x[ 2 ];
			vbo_data[XVALS][dataPtr*sizes[XVALS] + 3] = vertex->x[ 3 ];

			// branch index
			vbo_data[BRANCH_INDEX][dataPtr + 0] = id;

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

	// print out vbo_data
	float x,y,z,w;
	for (i=0; i<dataPtr; i++){
		x = vbo_data[POSITION][i*sizes[POSITION] + 0];
		y = vbo_data[POSITION][i*sizes[POSITION] + 1];
		z = vbo_data[POSITION][i*sizes[POSITION] + 2];
		printf("POS[%i]= %f, %f, %f\n", i, x,y,z);
		// normal
		x = vbo_data[NORMAL][i*sizes[NORMAL] + 0];
		y = vbo_data[NORMAL][i*sizes[NORMAL] + 1];
		z = vbo_data[NORMAL][i*sizes[NORMAL] + 2];
		printf("NOR[%i]= %f, %f, %f\n", i, x,y,z);
		// tangent
		x = vbo_data[TANGENT][i*sizes[TANGENT] + 0];
		y = vbo_data[TANGENT][i*sizes[TANGENT] + 1];
		z = vbo_data[TANGENT][i*sizes[TANGENT] + 2];
		printf("TAN[%i]= %f, %f, %f\n", i, x,y,z);
		// texcoord0
		x = vbo_data[TEXCOORD0][i*sizes[TEXCOORD0] + 0];
		y = vbo_data[TEXCOORD0][i*sizes[TEXCOORD0] + 1];
		printf("TEX0[%i]= %f, %f\n", i, x,y);
		// xvals
		x = vbo_data[XVALS][i*sizes[XVALS] + 0];
		y = vbo_data[XVALS][i*sizes[XVALS] + 1];
		z = vbo_data[XVALS][i*sizes[XVALS] + 2];
		w = vbo_data[XVALS][i*sizes[XVALS] + 3];
		printf("XVAL[%i]= %f, %f, %f, %f\n", i, x,y,z,w);
		// branch index
		x = vbo_data[BRANCH_INDEX][i + 0];
		printf("BRI[%i]= %f\n", i, x);
	}

	// create vbos...
	glGenBuffers(1, &branchEBOid);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, branchEBOid); // vytvoreni indexoveho bufferu
     glBufferData(GL_ELEMENT_ARRAY_BUFFER, iCnt*sizeof(GLuint), ebo_data, GL_STATIC_DRAW);
	 branchEBOcount = iCnt;	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenBuffers(1, &branchVBOid);
	glBindBuffer(GL_ARRAY_BUFFER, branchVBOid);
		// empty init
		int vboSize = totalVertexSize * vCnt * sizeof(GLfloat);
		glBufferData(GL_ARRAY_BUFFER, vboSize, 0, GL_STATIC_DRAW);
		
		// fill by parts
		// position
		glBufferSubData(GL_ARRAY_BUFFER,
						offsets   [POSITION],
						vCnt*sizes[POSITION]*sizeof(GLfloat),
						vbo_data  [POSITION]
						);
		// normal
		glBufferSubData(GL_ARRAY_BUFFER,
						offsets   [NORMAL],
						vCnt*sizes[NORMAL]*sizeof(GLfloat),
						vbo_data  [NORMAL]
						);
		// tangent
		glBufferSubData(GL_ARRAY_BUFFER,
						offsets   [TANGENT],
						vCnt*sizes[TANGENT]*sizeof(GLfloat),
						vbo_data  [TANGENT]
						);
		// xvals
		glBufferSubData(GL_ARRAY_BUFFER,
						offsets   [XVALS],
						vCnt*sizes[XVALS]*sizeof(GLfloat),
						vbo_data  [XVALS]
						);
		// branch index
		glBufferSubData(GL_ARRAY_BUFFER,
						offsets   [BRANCH_INDEX],
						vCnt*sizes[BRANCH_INDEX]*sizeof(GLfloat),
						vbo_data  [BRANCH_INDEX]
						);
		// texcoord0
		glBufferSubData(GL_ARRAY_BUFFER,
						offsets   [TEXCOORD0],
						vCnt*sizes[TEXCOORD0]*sizeof(GLfloat),
						vbo_data  [TEXCOORD0]
						);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool Tree::makeShader(const char* vsfilename,const char* gsfilename,const char* fsfilename, GLuint &programID, GLuint &vertexShader, GLuint &geometryShader,  GLuint &fragmentShader){

	bool bResult = createShaderProgram(	programID,
										&vertexShader,
										&geometryShader,
										&fragmentShader,
										vsfilename,
										gsfilename,
										fsfilename,
										3,
										GL_TRIANGLES,
										GL_LINE_STRIP);
										// 6 = kolik muze gs vygenerovat primitiv
										// GL_TRIANGLES ... jaka primitiva prijma
										// GL_TRIANGLE_STRIP ... jaka primitiva produkuje
	return bResult;
}


void Tree::createDataTexture(){
	// linearize structure
	texDimX = linearizeHierarchy();
	texDimY = 10; // num of data rows  [4 floats]
	TreeBranch * b;
	int channels = 4, ch, bh, z;
	float *data = new float[texDimX*texDimY*channels];
	int k,l;
	for (int i=0; i<texDimX*texDimY*channels; i=i+texDimY*channels){
		k=0;
		b = branches[i/(texDimY*channels)];
		for (ch = 0; ch<channels; ch++){
			data[i + k + ch] = 0.f;
		}
		// k = 0 
		// empty ...

		// phases
		k = 1;
		data[i + k + 0] = b->phases.x;
		data[i + k + 1] = b->phases.y;
		data[i + k + 2] = b->phases.z;
		data[i + k + 3] = b->phases.w;

		// up0_l0 - up2_l2
		k = 2;
		for (bh = 0;bh < MAX_HIERARCHY_DEPTH; bh++){
			data[i + k + 0] = b->upVectors[bh].x;
			data[i + k + 1] = b->upVectors[bh].y;
			data[i + k + 2] = b->upVectors[bh].z;
			data[i + k + 3] = b->lengths[bh];
			k++;
		}
		// right0_x0 - right2_x2
		for (bh = 0;bh < MAX_HIERARCHY_DEPTH; bh++){
			data[i + k + 0] = b->rightVectors[bh].x;
			data[i + k + 1] = b->rightVectors[bh].y;
			data[i + k + 2] = b->rightVectors[bh].z;
			data[i + k + 3] = b->xvals[bh];
			k++;
		}
	}
	dataTexture = new Texture(GL_TEXTURE_RECTANGLE_ARB, GL_RGBA32F, GL_RGBA, GL_FLOAT, data, texDimY, texDimX);
	
}

void Tree::setTime(float _time){
	time = _time;
}

int Tree::linearizeHierarchy(){
	stack<TreeBranch*> bStack;	
	bStack.push(trunk);
	TreeBranch * branch, *actBranch;
	int cnt = 0, i;
	while( !bStack.empty() ){
		branch = bStack.top();
		branch->id = cnt;
		cnt++;
		bStack.pop();
		branches.push_back(branch);
		
		// and copy from parent...
		actBranch = branch;
		while (actBranch!=NULL){
			branch->upVectors	[actBranch->level] = actBranch->upVector;
			branch->rightVectors[actBranch->level] = actBranch->rightVector;
			branch->xvals.d		[actBranch->level] = actBranch->x;
			branch->lengths.d	[actBranch->level] = actBranch->L;
			branch->phases.d	[actBranch->level] = actBranch->phase;
			if (actBranch->parent!=NULL){
				actBranch = (TreeBranch*)(actBranch->parent);
			} else {
				actBranch = NULL;
			}
		}
		for (i=0; i<branch->children.size(); i++){
			if (! branch->children[i]->isLeaf()){
				actBranch = (TreeBranch*)(branch->children[i]);
				bStack.push(actBranch);
			}
		}		
	}
	return cnt;
};

void Tree::draw(){
	glDisable(GL_LIGHTING);
	glPushMatrix();
	//glTranslatef(-1.f, 0.f, 0.f);

	dataTexture->bindTexture(GL_TEXTURE0);

	glUseProgram(branchShaderID);
		// set time
		glUniform1f(locations[TIME], time);
		// set data_texture
		glUniform1i(locations[BRANCH_DATA_TEXTURE], dataTexture->unitId);
			
		// bind index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, branchEBOid);
		glBindBuffer(GL_ARRAY_BUFFER, branchVBOid); 
		   // enable states
		   glEnableClientState(GL_VERTEX_ARRAY);
		   glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		   glEnableVertexAttribArray(GLuint(locations[BRANCH_INDEX]	));
		   glEnableVertexAttribArray(GLuint(locations[NORMAL]		));
		   glEnableVertexAttribArray(GLuint(locations[TANGENT]		));
		   glEnableVertexAttribArray(GLuint(locations[XVALS]		));
		   // draw VBOs...
			glVertexPointer(3, GL_FLOAT, 0, BUFFER_OFFSET(offsets[POSITION]));
			
			glVertexAttribPointer(locations[BRANCH_INDEX], sizes[BRANCH_INDEX], GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offsets[BRANCH_INDEX]));
			
			glVertexAttribPointer(locations[XVALS], sizes[XVALS], GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offsets[XVALS]));
			
			glVertexAttribPointer(locations[NORMAL], sizes[NORMAL], GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offsets[NORMAL]));
			
			glVertexAttribPointer(locations[TANGENT], sizes[TANGENT], GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offsets[TANGENT]));
			
			//glClientActiveTexture(dataTexture->unitId);
			//glTexCoordPointer(2, GL_FLOAT, 0, BUFFER_OFFSET());
			
			glDrawElements(GL_TRIANGLES, branchEBOcount, GL_UNSIGNED_INT, BUFFER_OFFSET(0));
			
		   // disable
		   glDisableVertexAttribArray(locations[BRANCH_INDEX]);
		   glDisableVertexAttribArray(locations[NORMAL]);
		   glDisableVertexAttribArray(locations[TANGENT]);
		   glDisableVertexAttribArray(locations[XVALS]);
		   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		   glDisableClientState(GL_VERTEX_ARRAY);
		// unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	// turn off shader
	glUseProgram(0);
	// unbind data texture
	dataTexture->unbindTexture();
	glPopMatrix();
	//glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glEnable(GL_LIGHTING);
	
}







void Tree::load(string filename, TextureManager *texManager){
	OBJTfile file;
	file.loadFromFile(filename.c_str());
	
	// process branches
	map <int, TreeBranch*> branches;
	map <int, StEntity>::iterator iter;
	TreeBranch *parent = NULL;
	CoordSystem cs;
	CoordSystem objectCS;
	float x;
	float length, r1, r2, c2=0.3326, c4=0.398924;
	int divT=2, divR=4;
	v3 motionVector(1.0, 1.0, 1.0);
	StEntity entity;
	// get trunk (id = parentId)
	int id=0;
	for ( iter=file.branches.begin() ; iter != file.branches.end(); iter++ ){
		parent	= NULL;
		id		= (*iter).first;
		entity	= (*iter).second;
		// parent
		if (entity.isSetParent && entity.id!=entity.parentId){
			parent = branches[entity.parentId];
		} else {
			parent = NULL;
		}
		// length
		if (entity.isSetLength){
			length = entity.length;
		} else {
			if (entity.isSetLevel){
				length = 1.f/(entity.level+1.f);
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

		} else {
			cs.r = v3(1.0, 0.0, 0.0);
			cs.s = v3(0.0, 1.0, 0.0);	
			cs.t = v3(0.0, 0.0, 1.0);
			objectCS.r = v3(1.0, 0.0, 0.0);
			objectCS.s = v3(0.0, 1.0, 0.0);
			objectCS.t = v3(0.0, 0.0, 1.0);
		}
		// recalculate base
		/*if (parent!=NULL){
			cs.r = parent->objectCS.getCoordsInThisSystem(objectCS.r);
			cs.s = parent->objectCS.getCoordsInThisSystem(objectCS.s);
			cs.t = parent->objectCS.getCoordsInThisSystem(objectCS.t);
		}*/
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
			r2 = 0.01*length;
		}
		// add branch...	
		branches[id] = new TreeBranch(parent, cs, objectCS, x, texManager, length, r1, r2, divT, divR, c2, c4, motionVector);  
		branches[id]->setBending(0.5,0.0);
	} // for each entity in map
	
	// set trunk
	trunk = branches[0];



}

void Tree::save(string filename){}
