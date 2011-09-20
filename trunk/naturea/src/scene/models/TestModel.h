#pragma once
#include "scene\scenemodel.h"
#include "models\elephant.h"
#include "../../utility/VBO.h"
#include "../../scene/vegetation/dynamic/DTree.h"
#include "../../scene/cameras/Camera.h"


class TestInstance{
public:
	TestInstance(){}
	~TestInstance(){}

	m4	transformMatrix;
	float param1;
	float x,y,z,r;

};



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

	GLuint				i_matricesBuffID;
	GLuint				i_paramBuffID;
	GLuint				v_positionsBuffID;
	GLuint				v_indicesBuffID;
	float				*matricesBufferData;
	vector<TestInstance*> instances;
	Shader				*shader;
	VBO					*vbo;
	EBO					*ebo;
	Terrain				*terrain;
	Camera				*camera;
};

