#pragma once
#include "scene\scenemodel.h"
#include "models\elephant.h"
#include "../../utility/VBO.h"

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

	Shader				*shader;
	VBO					*vbo;

	Texture*			colorMap;
	Texture*			displacementMap;
	Texture*			weightMap;

	Texture*			frontDecalMap;
	Texture*			frontNormalMap;
	Texture*			frontTranslucencyMap;
	Texture*			frontHalfLife2Map;
	Texture*			backDecalMap;
	Texture*			backNormalMap;
	Texture*			backTranslucencyMap;
	Texture*			backHalfLife2Map;
};

