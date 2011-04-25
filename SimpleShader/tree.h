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
#include "OBJTfile.h"

#include "settings.h"

// #include "TreeLeaf.h"
enum VERTEX_ATTRIBUTES{
	 POSITION             ,
	 NORMAL		          ,
	 BINORMAL	          ,
	 TANGENT	          ,
	 COLOR0		          ,
	 COLOR1		          ,
	 TEXCOORD0	          ,
	 TEXCOORD1	          ,
	 XVALS		          ,
	 TIME				  ,
	 BRANCH_INDEX         ,
	 BRANCH_DATA_TEXTURE  ,
	 BRANCH_TEXTURE0	  ,

	 VBO_VERTEX_COMPONENTS
};


using namespace std;

extern bool		g_bShadersSupported;
extern bool		g_bUseShaders;

class Tree{
public:
	Tree();
	~Tree();
	
	// create texture & set-up for drawing...
	void init();
	
	void draw();

	void load(string filename, TextureManager *texManager);
	void save(string filename);

	void setTime(float _time);

	

	GLuint				branchShaderID;
	GLuint				leafShaderID;

	void				createDataTexture();
	int					linearizeHierarchy();
	void				createBranchesVBO();
	void				createLeavesVBO();

	TreeBranch			*trunk;
	vector<TreeBranch*>	branches;
	vector<TreeLeaf*>	leaves;

	Texture*			dataTexture;
	Texture*			leafTexture1;

	GLuint				branchVBOid;
	GLuint				branchEBOid;
	GLuint				branchEBOcount;

	int					offsets[VBO_VERTEX_COMPONENTS];
	int					sizes[VBO_VERTEX_COMPONENTS];
	GLint				locations[VBO_VERTEX_COMPONENTS];
	GLfloat				*vbo_data[VBO_VERTEX_COMPONENTS];
	GLuint 				*ebo_data;
	
	//GLuint				leavesVBOid;
	//void				*leavesData;

	GLuint				dataTextureID;

	

private:
	// create texture containing tree data
	
	int texDimX, texDimY;
	

	//vector<TreeLeaf*>	leaves2;
	
	GLuint hvd_shaderProgram;
	GLuint hvd_vs;
	GLuint hvd_gs;
	GLuint hvd_fs;
	float time;
	bool makeShader(const char* vsfilename,const char* gsfilename,const char* fsfilename, GLuint &programID, GLuint &vertexShader, GLuint &geometryShader,  GLuint &fragmentShader);
};



#endif