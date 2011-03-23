#ifndef _TREE_H
#define _TREE_H

#include "../_utils/GLEE/glee.h" 
#include "../_utils/GLUT/glut.h"
#include "../_utils/shaders.h"
#include "branch.h"
#include "TreeBranch.h"
#include "TreeLeaf.h"
#include "Shader.h"
#include "TextureManager.h"

#include "settings.h"

// #include "TreeLeaf.h"

using namespace std;

extern bool		g_bShadersSupported;
extern bool		g_bUseShaders;

class Tree{
public:
	Tree();
	~Tree();
	
	// create texture & set-up for drawing...
	void init();
	void init2();
	
	void update(float time, float var=1.f);
	void draw();
	void draw2();

	void load();
	void save();

	void setTime(float _time);

	Branch *trunk;


	TreeComponent*		trunk2;
	vector<TreeBranch*> branches2;
	vector<TreeLeaf*>	leaves;
	Texture*			dataTexture2;
	Texture*			leafTexture1;
	void				createDataTexture2();
	int					linearizeHierarchy2();
	GLuint				branchShaderID;
	GLuint				leafShaderID;



private:
	// create texture containing tree data
	void createDataTexture();
	int texDimX, texDimY;
	GLuint dataTextureID;
	float *dataTexture;

	int getBranchCount();
	int linearizeHierarchy();
	vector<Branch*> branches;
	//vector<TreeLeaf*>	leaves2;
	
	GLuint hvd_shaderProgram;
	GLuint hvd_vs;
	GLuint hvd_gs;
	GLuint hvd_fs;
	float time;
	bool makeShader(const char* vsfilename,const char* gsfilename,const char* fsfilename, GLuint &programID, GLuint &vertexShader, GLuint &geometryShader,  GLuint &fragmentShader);
};



#endif