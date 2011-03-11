#include "tree.h"
#include <stack>

/* Constructor */
Tree::Tree(){
	dataTexture		= NULL;
	trunk			= NULL;
	hvd_shaderProgram = 0;
	hvd_vs			= 0;
	hvd_gs			= 0;
	hvd_fs			= 0;
	texDimX			= 4;
	texDimY			= 5;
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
	if (!g_bShadersSupported){
		return;
	}
	// init shader program
	if (!makeShader("Shaders/hvd_vs.glsl", NULL, "Shaders/hvd_fs.glsl", hvd_shaderProgram, hvd_vs, hvd_gs, hvd_fs)){
		// fatal error!
		printf("!cannot create shader! ->EXIT, BYE\n");
		system("PAUSE");
		exit(1);
	}
	// reset branch data...
	for (int i=0; i<branches.size(); i++){
		branches[i]->init();
	}

	// generate texture 
	createDataTexture();
	
	// init GL texture
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	//glClientActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &dataTextureID);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, dataTextureID);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP); 
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB32F, texDimY, texDimX, 0, GL_RGB, GL_FLOAT, dataTexture);

		glTexEnvi(GL_TEXTURE_ENV, GL_PRIMARY_COLOR,GL_SRC_COLOR);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
}

bool Tree::makeShader(const char* vsfilename,const char* gsfilename,const char* fsfilename, GLuint &programID, GLuint &vertexShader, GLuint &geometryShader,  GLuint &fragmentShader){

	bool bResult = createShaderProgram(	programID,
										&vertexShader,
										&geometryShader,
										&fragmentShader,
										vsfilename,
										gsfilename,
										fsfilename,
										6,
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
	texDimY = 7; // num of data triplets  [floats]
	Branch * b;
	dataTexture = new float[texDimX*texDimY*3];
	int k,l;
	for (int i=0; i<texDimX*texDimY*3; i=i+texDimY*3){
		k=0;
		b = branches[i/(texDimY*3)];
		// if not trunk...
		if (b->parent!=NULL){
			/*
			// origin
			dataTexture[i + k + 0]= b->originalCS.origin.dot(b->parent->originalCS.r);
			dataTexture[i + k + 1]= b->originalCS.origin.dot(b->parent->originalCS.s);
			dataTexture[i + k + 2]= b->originalCS.origin.dot(b->parent->originalCS.t);
			k+=3;				
			*/
			// origin
			dataTexture[i + k + 0]= b->originalCS.origin.x;
			dataTexture[i + k + 1]= b->originalCS.origin.y;
			dataTexture[i + k + 2]= b->originalCS.origin.z;
			k+=3;

			// precompute
			// r vector
			dataTexture[i + k + 0]= b->originalCS.r.dot(b->parent->originalCS.r);
			dataTexture[i + k + 1]= b->originalCS.r.dot(b->parent->originalCS.s);
			dataTexture[i + k + 2]= b->originalCS.r.dot(b->parent->originalCS.t);
			k+=3;

			// s vector
			dataTexture[i + k + 0]= b->originalCS.s.dot(b->parent->originalCS.r);
			dataTexture[i + k + 1]= b->originalCS.s.dot(b->parent->originalCS.s);
			dataTexture[i + k + 2]= b->originalCS.s.dot(b->parent->originalCS.t);
			k+=3;

			// t vector
			dataTexture[i + k + 0]= b->originalCS.t.dot(b->parent->originalCS.r);
			dataTexture[i + k + 1]= b->originalCS.t.dot(b->parent->originalCS.s);
			dataTexture[i + k + 2]= b->originalCS.t.dot(b->parent->originalCS.t);
			k+=3;
		} else {
			// origin
			dataTexture[i + k + 0]= b->originalCS.origin.x;
			dataTexture[i + k + 1]= b->originalCS.origin.y;
			dataTexture[i + k + 2]= b->originalCS.origin.z;
			k+=3;

			// r vector
			dataTexture[i + k + 0]= b->originalCS.r.x;
			dataTexture[i + k + 1]= b->originalCS.r.y;
			dataTexture[i + k + 2]= b->originalCS.r.z;
			k+=3;

			// s vector
			dataTexture[i + k + 0]= b->originalCS.s.x;
			dataTexture[i + k + 1]= b->originalCS.s.y;
			dataTexture[i + k + 2]= b->originalCS.s.z;
			k+=3;

			// t vector
			dataTexture[i + k + 0]= b->originalCS.t.x;
			dataTexture[i + k + 1]= b->originalCS.t.y;
			dataTexture[i + k + 2]= b->originalCS.t.z;
			k+=3;
		}

		// x-value, parentBranchIndex, level
		dataTexture[i + k + 0]= b->x;
		dataTexture[i + k + 1]= (b->parent!=NULL?float(b->parent->id):-10.0f);
		dataTexture[i + k + 2]= 0;
		k+=3;

		// c2, c4, L
		dataTexture[i + k + 0]= b->c2;
		dataTexture[i + k + 1]= b->c4;
		dataTexture[i + k + 2]= b->L;
		k+=3;

		// motion texture vector x,y
		dataTexture[i + k + 0]= b->Ar;
		dataTexture[i + k + 1]= b->As;
		dataTexture[i + k + 2]= 0.0;
		k+=3;
		
		for(l=0; l<texDimY*3; l++){
			printf("dataTexture[%i]= %f\n",i + l, dataTexture[i + l] );
		}
	}
}

void Tree::setTime(float _time){
	time = _time;
}

int Tree::getBranchCount(){
	return trunk->childCnt();
};

int Tree::linearizeHierarchy(){
	stack<Branch*> bStack;
	bStack.push(trunk);
	Branch * b;
	int cnt = 0, i;
	while( !bStack.empty() ){
		b = bStack.top();
		b->id = cnt;
		cnt++;
		bStack.pop();
		branches.push_back(b);
		for (i=0; i<b->childs.size(); i++){
			bStack.push(b->childs[i]);
		}		
	}
	return cnt;
};

void Tree::draw(){
	/*if (!( g_bShadersSupported && g_bUseShaders)){
		return;
	}*/
	glDisable(GL_LIGHTING);
	glPushMatrix();
	//glTranslatef(-1.f, 0.f, 0.f);

	glEnable(GL_TEXTURE_RECTANGLE_ARB);
		
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, dataTextureID);
	//glClientActiveTexture(GL_TEXTURE0);
	glActiveTexture(GL_TEXTURE0);
	if (g_bShadersSupported && g_bUseShaders)
	{
		glUseProgram(hvd_shaderProgram);
		GLint loc = glGetUniformLocation(hvd_shaderProgram, "data_tex");
		//GLint loc2 = glGetUniformLocation(hvd_shaderProgram, "dataTexture_steps");
		glUniform1i(loc,0);
		GLint locTime = glGetUniformLocation(hvd_shaderProgram, "time");
		glUniform1f(locTime,this->time);
	}
	// DRAW
	// each branch
	int i;
	for (i=0; i<branches.size(); i++){
		branches[i]->draw();
	}
	if (g_bShadersSupported && g_bUseShaders)
	{
		glUseProgram(NULL);
	}
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
	glPopMatrix();
	//glDisable(GL_TEXTURE_RECTANGLE_ARB);
	glEnable(GL_LIGHTING);
}

void Tree::update(float time, float var){
	int i;
	float valA = sin(time);
	float val = valA*var;
	for (i=0; i<branches.size(); i++){
		branches[i]->setBending(val, 0);
		branches[i]->update();
	}
}

void Tree::load(){}

void Tree::save(){}
