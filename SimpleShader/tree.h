#ifndef _TREE_H
#define _TREE_H

#include "../_utils/GLEE/glee.h" 
#include "../_utils/GLUT/glut.h"
#include "../_utils/shaders.h"
#include "branch.h"

using namespace std;

extern bool		g_bShadersSupported;
extern bool		g_bUseShaders;

class Tree{
public:
	Tree();
	~Tree();
	
	// create texture & set-up for drawing...
	void init();
	
	void update(float time, float var=1.f);
	void draw();

	void load();
	void save();

	void setTime(float _time);

	Branch *trunk;
	
private:
	// create texture containing tree data
	void createDataTexture();
	int texDimX, texDimY;
	GLuint dataTextureID;
	float *dataTexture;

	int getBranchCount();
	int linearizeHierarchy();
	vector<Branch*> branches;

	GLuint hvd_shaderProgram;
	GLuint hvd_vs;
	GLuint hvd_gs;
	GLuint hvd_fs;
	float time;
	bool makeShader(const char* vsfilename,const char* gsfilename,const char* fsfilename, GLuint &programID, GLuint &vertexShader, GLuint &geometryShader,  GLuint &fragmentShader);
};



#endif