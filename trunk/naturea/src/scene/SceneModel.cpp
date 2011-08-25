#include "SceneModel.h"

SceneModel::SceneModel(){
	textureManager = NULL;
	transformMatrix.setIdentity();
}
SceneModel::SceneModel(TextureManager *texManager, ShaderManager *shManager){
	textureManager = texManager;
	shaderManager  = shManager;
	transformMatrix.setIdentity();
}


SceneModel::~SceneModel()
{

}

void SceneModel::translate(v3 &movVector)
{
	transformMatrix.translate(movVector);
}

void SceneModel::rotate(v3 &axis, float angleRad)
{
	transformMatrix.rotate( axis, angleRad);
}

void SceneModel::scale(v3 &scaleVector)
{
	transformMatrix.scale(scaleVector);
}