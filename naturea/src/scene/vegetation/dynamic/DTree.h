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



	Texture *	colormap;
	Texture *	normalmap;
	Texture *	depthmap;
	Texture *	branchmap;
	Texture *	datamap;
	
	float		getSortValue(v3 &viewer_position) {
					return (controlPoint-viewer_position).length();
				}

	//			transformed direction of the slice normal
	
private:
	v3			controlPoint;


};

class DTreeSliceSet{
public :
	DTreeSliceSet(){}
	~DTreeSliceSet(){}
	v3			getNormal(){
					v3 n = v3(0.0, 0.0, -1.0);
					n.rotateY((90+rotation_y)*DEG_TO_RAD);
					return n;
				}
	void		setRotMatrix(m3 &_rotMatrix){
					rotMatrix = _rotMatrix;
				}
	void		setSlices(vector<DTreeSlice*> _slices){
					slices = _slices;
				}
	int			size(){
					return slices.size();
				}
	DTreeSlice*	getSlice(int id){
					return slices[id];
				}
	int			getSliceCnt(){
					return slices.size();
				}
	float		rotation_y; // degrees
private:
	vector<DTreeSlice*> slices;
	v3			normal;
	m3			rotMatrix;
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

	void drawLOD0();
	void drawLOD1();
	void drawLOD1b();
	void drawLOD2();

	void init();
	void initLOD0();
	void initLOD1();
	void initLOD1b();
	void initLOD2();

	void update(double time);

	void bakeToVBO(void);

	void fixTexType(void);
	v3	 transformTexCoords(v3 &origTexCoords);


	BBox * getBBox(void);
	void createSlices(v3 & direction, v3 & right,  bool half=true);
	int resolution_x, resolution_y;
	int	slice_count;


	void joinSliceSetsTextures();
	Texture*			jColorMap;
	Texture*			jDataMap;
	Texture*			jNormalMap;
	Texture*			jDepthMap;


	vector<DTreeSlice*>	slices;
	v3					position;
	v3*					viewer_position;
	v3*					viewer_direction;

private:
	DTreeBranch			*trunk;
	vector<DTreeBranch*> branches;
	vector<DTreeLeaf*>	 leaves;



	Shader *			branchShader;
	Shader *			leafShader;

	Shader*				bLODShader;
	Shader*				lLODShader;


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

	// LOD 1
	float					tree_time_offset;
	Uniform*				u_time_offset;	 			

	int						l_color	  ;
	int						l_displ	  ;
	int						l_displ2  ;
	int						l_data	  ;
	int						l_normal  ;
	v2						win_resolution;

	Texture*				colorMap;
	Texture*				colorMap2;
	Texture*				weightMap;
	Texture*				dataMap;
	vector<DTreeSliceSet*>	sliceSets;
	Shader	*				lod1shader;
	VBO		*				lod1vbo;



	int						l2_color	;
	int						l2_displ	;
	int						l2_displ2	;
	int						l2_data		;
	int						l2_normal	;
	vector<DTreeSliceSet*>	sliceSets2	;
	Shader	*				lod1shader2	;
	VBO		*				lod1vbo2	;

	int						ctr;
	float					alpha_c;

};

