#pragma once
#include "..\Vegetation.h"
#include "..\..\..\IO\OBJTfile.h"
#include "DTreeBranch.h"
#include "DTreeLeaf.h"
#include "settings.h"
#include "../../../utility/Texture.h"
#include "../../../utility/EBO.h"
#include "../../../utility/VBO.h"

class DTreeSlice{
public:
	DTreeSlice(){
		colormap	= NULL;
		normalmap	= NULL;
		depthmap	= NULL;
		branchmap	= NULL;
		datamap		= NULL;
	}
	~DTreeSlice(){
		printf("deleting slice\n");
		SAFE_DELETE_PTR(	colormap	);
		SAFE_DELETE_PTR(	normalmap	);
		SAFE_DELETE_PTR(	depthmap	);
		SAFE_DELETE_PTR(	branchmap	);
		SAFE_DELETE_PTR(	datamap		);
		printf("deleted slice\n");
	}
	Texture * colormap;
	Texture * normalmap;
	Texture * depthmap;
	Texture * branchmap;
	Texture * datamap;
};


class DTree :
	public Vegetation
{
public:
	DTree(TextureManager *texManager, ShaderManager *shManager);
	DTree(DTree* copy);
	~DTree(void);

	bool loadOBJT(string filename);

	void createVBOs();
	void createDataTexture();

	bool loadDataTexture(string filename);
	bool saveDataTexture(string filename);

	bool loadVBO(string filename);
	bool saveVBO(string filename);

	//Texture * renderToTexture()

	Vegetation* getCopy();

	void draw();

	void drawForLOD();

	void init();

	void update(double time);

	void bakeToVBO(void);

	void fixTexType(void);
	v3	 transformTexCoords(v3 &origTexCoords);


	BBox * getBBox(void);
	void createSlices(v3 & direction, int num=2, int resolution_x=DYN_TREE::SLICE_RESOLUTION_X, int resolution_y=DYN_TREE::SLICE_RESOLUTION_Y, bool half=true);

	vector<DTreeSlice*>	slices;

private:
	DTreeBranch			*trunk;
	vector<DTreeBranch*> branches;
	vector<DTreeLeaf*>	 leaves;



	Shader *			branchShader;
	Shader *			leafShader;

	Texture*			dataTexture;
	Texture*			lColorTexture;
	Texture*			frontDecalMap;
	Texture*			frontNormalMap;
	Texture*			frontTranslucencyMap;
	Texture*			frontHalfLife2Map;
	Texture*			backDecalMap;
	Texture*			backNormalMap;
	Texture*			backTranslucencyMap;
	Texture*			backHalfLife2Map;
	
	Texture*			bColorTexture;
	Texture*			bNormalTexture;

	Texture*			branchNoiseTexture;
	Texture*			leafNoiseTexture;

	int					texDimX, texDimY;
	int					branchCount;
	float				branchCountF;

	VBO *				branchesVBO;
	VBO	*				leavesVBO;
	EBO *				branchesEBO;

	GLfloat				*vbo_data[DYN_TREE::COUNT];
	GLuint 				*ebo_data;

	int linearizeHierarchy();
	void recalcCoordSystems();
	void fillParentDataForEachBranch();

	void createBranchesVBO();
	void createLeavesVBO();

};

