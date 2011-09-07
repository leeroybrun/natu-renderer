#pragma once
#include "scene\scenemodel.h"
#include "models\elephant.h"
#include "../../utility/VBO.h"
#include "../../scene/vegetation/dynamic/DTree.h"

class TestModel :
	public SceneModel
{
public:
	TestModel(void);
	~TestModel(void);

	virtual void draw();

	virtual void drawForLOD();

	virtual void init();

	virtual void update(double time);

	void		 processTree(DTree * tree);

	Shader				*shader;
	VBO					*vbo;

	float				tree_time_offset;
	Uniform*			u_time_offset;	 			

	Texture*			colorMap;
	Texture*			colorMap2;
	Texture*			displacementMap;
	Texture*			weightMap;
	Texture*			dataMap;

	Texture*			frontDecalMap;
	Texture*			frontNormalMap;
	Texture*			frontTranslucencyMap;
	Texture*			frontHalfLife2Map;
	Texture*			backDecalMap;
	Texture*			backNormalMap;
	Texture*			backTranslucencyMap;
	Texture*			backHalfLife2Map;

	vector<DTreeSlice*>	slices;

	int					l_color	  ;
	int					l_displ	  ;
	int					l_data	  ;
	int					l_normal  ;

	v2					win_resolution;
};

