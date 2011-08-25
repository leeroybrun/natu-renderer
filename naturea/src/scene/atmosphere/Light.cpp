#include "Light.h"


Light::Light(TextureManager * tm)
{
	positionFixedToSkybox = v3(0.0,0.0,0.0);
	textureManager = tm;
}


Light::~Light(void)
{

}
void Light::init()
{

}

void Light::update(double time)
{

}

void Light::translate(v3 &movVector)
{
	position += v4(movVector, 0.0);
}

void Light::rotate(v3 &axis, float angleRad)
{
	//direction->rotate(angleRad, axis);
}

void Light::scale(v3 &scaleVector)
{

}

void Light::showTextures(){
	show_texture(cb_shad_ID, 0,200,200,200);
	show_texture(db_shad_ID, 200,200,200,200);
}

void Light::initShadowMapping(Camera *_cam, int resolution)
{
	camera = _cam;
	resolution_x = resolution;
	resolution_y = resolution;

	glGenTextures(1, &db_shad_ID );
		glBindTexture(GL_TEXTURE_2D, db_shad_ID );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOWMAP_RESOLUTION_X, SHADOWMAP_RESOLUTION_Y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glGenTextures(1, &cb_shad_ID );
		glBindTexture(GL_TEXTURE_2D, cb_shad_ID );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SHADOWMAP_RESOLUTION_X, SHADOWMAP_RESOLUTION_Y, 0, GL_RGBA, GL_FLOAT, 0);
	 glGenFramebuffersEXT(1, &fb_shad_ID);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb_shad_ID);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, db_shad_ID, 0);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, cb_shad_ID, 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	Texture * shadowMapTexture = new Texture();
	shadowMapTexture->id = db_shad_ID;
	shadowMapTexture->inShaderName = "shadowMap";
	shadowMapTexture->textureUnitNumber = 7; // last texture (0-7)
	shadowMapTMID = textureManager->addTexture(shadowMapTexture);
	textureManager->shadowMapID = shadowMapTMID;
}


void Light::beginShadowMap(){
	g_ShadowMappingEnabled = false;
	g_fastMode = true;
	// set camera to light position and set its params...
	float fov = 100.f;
	float near = 100.f;
	float far = 1000.f;
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb_shad_ID);
    // prenastavit viewport
		glViewport(0,0,resolution_x, resolution_y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glLoadIdentity();
		glOrtho(-TERRAIN_SIZE_X*0.5, TERRAIN_SIZE_X*0.5, -TERRAIN_SIZE_Y*0.5, TERRAIN_SIZE_Y*0.5, near ,far);
		//gluPerspective(fov, (GLfloat)resolution_x/(GLfloat)resolution_y, near, far);  
		glGetFloatv(GL_MODELVIEW_MATRIX, &Pmatrix.m[0]);
		glLoadIdentity();
		v3 p(LIGHT_POSITION);
		gluLookAt(	p.x	, p.y , p.z	,
					0.0	, 0.0 , 0.0	,
					0.0	, 1.0 , 0.0	);	
		glGetFloatv(GL_MODELVIEW_MATRIX, &MVmatrix.m[0]);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixf(Pmatrix.m);
	glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadMatrixf(MVmatrix.m);

	MVPmatrix = Pmatrix * MVmatrix;
	
	// redirect renderign to framebuffer
	
	// set viewport
	//glViewport(0, 0, SHADOWMAP_RESOLUTION_X, SHADOWMAP_RESOLUTION_Y);
	// ... render scene at fastest settings
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

}
void Light::endShadowMap(){
	g_ShadowMappingEnabled = true;
	g_fastMode = false;
	// restore rendering settings
	glPopAttrib();

	// redirect rendering back to back screen buffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	// reset viewport
	glViewport(0, 0, g_WinWidth, g_WinHeight);
	// set camera...
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	
	// SHOW SHADOW MAP
	showTextures();
}

void Light::drawForLOD()
{
	draw();
}

void Light::draw()
{
	float lightLineSize = 50.f;
	// draw a line from origin to the direction
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	//glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glTranslatef(0.0,8.0,0.0);
		glColor3f(1.0,0.0,0.0);
		
		glBegin(GL_LINES);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(lightLineSize*position.x, lightLineSize*position.y, lightLineSize*position.z);
		glEnd();




	glPopMatrix();
	glEnable(GL_LIGHTING);
	glPopAttrib();



	// draw a circle on the screen where the light is...

	/*
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glTranslate(positionFixedToSkybox);
		glColor3f(1.0,0.0,0.0);
		glutSolidSphere(10.0, 5, 5);
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glPopAttrib();
	*/
}

void Light::turnOn()
{
	glEnable(GL_LIGHTING);
	glEnable(lightId);
	//position.printOut();
	position = -g_light_position;
	glLightfv(lightId, GL_POSITION      , position.data);
	//glLightfv(lightId, GL_SPOT_DIRECTION, (*direction).data);
	//direction->printOut();
	
}

void Light::turnOff()
{
	glDisable(lightId);
}

void Light::setup(GLuint lid, v4 &pos, v4 *dir, v4 &ambi, v4 &diff, v4 &spec, float cutOff, float exp)
{
	lightId = lid;
	position = pos;
	direction = dir;
	glLightfv(lightId, GL_POSITION      , v4(position).data);
	glLightfv(lightId, GL_SPOT_DIRECTION, direction->data);
	glLightfv(lightId, GL_AMBIENT       , ambi.data);
	glLightfv(lightId, GL_DIFFUSE       , diff.data);
	glLightfv(lightId, GL_SPECULAR	    , spec.data);
	//glLightf (lightId, GL_SPOT_CUTOFF	, cutOff);
	//glLightf (lightId, GL_SPOT_EXPONENT , exp);
}
