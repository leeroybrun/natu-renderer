#include "SkyBox.h"
SkyBox::SkyBox():
	SceneModel(NULL, NULL)
{
}

SkyBox::SkyBox(TextureManager *texManager, ShaderManager *shManager, string filename):
	SceneModel(texManager, shManager)
{
	// load textures...
	char fname[100];
	char shname[100];
	int id;
	for (int i=0; i<6; i++)
	{
		sprintf(fname, filename.c_str(),appendices[i].c_str()); 
		sprintf(shname, "skybox_tex_%s", appendices[i].c_str());
		id = textureManager->loadTexture(string(fname), string(shname), false, false, GL_REPEAT, GL_LINEAR, GL_LINEAR);
		textureIds.push_back(id);
	}
}


SkyBox::~SkyBox(void)
{
	// delete plane
}

void SkyBox::drawForLOD()
{
}

void SkyBox::draw()
{
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
    // move with user...
	glTranslate(p_activeCamera->getPosition());
	//if (!g_drawingReflection){
	//if (g_godraysEnabled && !g_drawingReflection){
		p_light->positionFixedToSkybox = p_light->position.xyz() + p_activeCamera->getPosition();
		//p_light->draw();
	//}
	glColor4f(1.0,1.0,1.0, 1.0);
	glScale(v3(SKYBOX_SIZE));
	
	// down 
	glPushMatrix();
	glTranslatef(0,-0.499,0);
	glRotatef(180, 1,0,0);
	textureManager->bindTexture(textureIds[5], GL_TEXTURE0);
	drawPlane();
	textureManager->unbindTexture(textureIds[5]);
	glPopMatrix();
	
	// up 
	glPushMatrix();	
	glTranslatef(0,0.499,0);	
	textureManager->bindTexture(textureIds[4], GL_TEXTURE0);
	drawPlane();
	textureManager->unbindTexture(textureIds[4]);
	glPopMatrix();
	

	// north
	glPushMatrix();
	glTranslatef(0, 0, -0.499);
	glRotatef(-90, 1,0,0);
	glScale(v3(-1));
	textureManager->bindTexture(textureIds[0], GL_TEXTURE0);
	drawPlane();
	textureManager->unbindTexture(textureIds[0]);
	glPopMatrix();
	
	// south
	glPushMatrix();
	glTranslatef(0, 0, 0.499);
	glRotatef(90, 1,0,0);
	textureManager->bindTexture(textureIds[3], GL_TEXTURE0);
	drawPlane();
	textureManager->unbindTexture(textureIds[3]);
	glPopMatrix();
	
	// east 
	glPushMatrix();
	glTranslatef(-0.499, 0,0);
	glRotatef(90, 0,0,1);
	textureManager->bindTexture(textureIds[2], GL_TEXTURE0);
	drawPlane();
	textureManager->unbindTexture(textureIds[2]);
	glPopMatrix();
	
	// west 
	glPushMatrix();
	glTranslatef(0.499,0,0);
	glRotatef(-90, 0,0,1);
	textureManager->bindTexture(textureIds[1], GL_TEXTURE0);
	drawPlane();	
	textureManager->unbindTexture(textureIds[1]);
	glPopMatrix();
   glPopMatrix();	
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
}

void SkyBox::init()
{
}

void SkyBox::update(double time)
{

}

void SkyBox::loadFromFiles(string filename)
{

}
