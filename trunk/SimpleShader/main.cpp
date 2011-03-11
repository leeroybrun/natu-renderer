//-----------------------------------------------------------------------------
//  [GSY] SimpleShader - Simple GLSL framework
//  15/03/2009
//-----------------------------------------------------------------------------
//  Controls: 
//		[c]		... compile shader program
//		[ ]		... toggle fixed/programmable pipeline
//		[h]		... show/hide GUI controls
//		[Ecs]	... quit
//-----------------------------------------------------------------------------

#include "main.h"
#include "../_utils/shaders.h"
#include "../_utils/GLUT/glut.h"
#include "../_utils/Vector3.h"

#include "png.h"
#include "TestObject.h"
#include "branch.h"
#include "tree.h"

// GLOBAL DEFINITIONS__________________________________________________________

// GLOBAL CONSTANTS____________________________________________________________

// GLOBAL VARIABLES____________________________________________________________
GLuint		g_Program				= 0;	// Shader program GL id
GLuint		g_VertexShader			= 0;	// Vertex shader GL id
GLuint		g_FragmentShader		= 0;	// Fragment shader GL id
GLuint		g_GeometryShader		= 0;	// Geometry shader GL id
GLint hTimer = 0;
GLuint texId = 0;
float varA = 0.1;
float varB = 0.8;
float varC = 0.7;
float valA = 0;
float valB = 0;
float ttime = 0;

Branch *br, *br2;
Branch *xbr1, *xbr2, *xbr3;
CoordSystem* cs, *cs2;
TestOBJ *tobj;
Tree	*tree;

// FORWARD DECLARATIONS________________________________________________________

// USER DEFINED CALLBACK FUNCTIONS_____________________________________________
void callback_VariableAchanged(float value)
{
	varA = value;
	printf("variable A changed, value=%f\n", varA);
}

void callback_VariableBchanged(float value)
{
	varB = value;
	printf("variable B changed, value=%f\n", varB);
}

void callback_VariableCchanged(float value)
{
	varC = value;
	printf("variable C changed\n");
}


void callback_CompileShaders()
{
	bool bResult = createShaderProgram(	g_Program,
										&g_VertexShader,
										&g_GeometryShader,
										&g_FragmentShader,
										(g_bAttachVertexShader   == true) ? "Shaders/simple.vs" : NULL,
										(g_bAttachGeometryShader == true) ? "Shaders/simple.gs" : NULL,
										(g_bAttachFragmentShader == true) ? "Shaders/simple.fs" : NULL,
										6,
										GL_TRIANGLES,
										GL_LINE_STRIP);
										// 6 = kolik muze gs vygenerovat primitiv
										// GL_TRIANGLES ... jaka primitiva prijma
										// GL_TRIANGLE_STRIP ... jaka primitiva produkuje
	// Locate some parameters by name so we can set them later__________________
	//hTimer = glGetUniformLocation(g_Program, "timer");
	//tobj->shaderProgramID = g_Program;
	
	g_bUseShaders = true;
	callback_UseShaders(bResult);

	// prepare tree...
	tree->init();
}


// RENDER FUNCTIONS____________________________________________________________

//-----------------------------------------------------------------------------
// Name: display()
// Desc: 
//-----------------------------------------------------------------------------
void display()
{
	static float rotY = 0.0;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0f, 0.0f, g_CameraZ );
    glRotatef( g_RotObject[0], 1.0f, 0.0f, 0.0f );
    glRotatef( g_RotObject[1] + rotY, 0.0f, 1.0f, 0.0f );
	
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, texId);
	
	
	if (g_bShadersSupported)
	{
		glUseProgram(g_Program);	// Active programmable pipeline
		//glUniform1f(hTimer, (GetTickCount()%2000)/2000.0f);
		//glUniform1i(glGetUniformLocation(g_Program, "my_texture"),0);
		//glUniform1f(glGetUniformLocation(g_Program, "size"),varA);
	}
	
	// glMultiTexCoord2f(GL_TEXTURE1, 1.0, 1.0);
	// VA: glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//     glTexCoordPointer(2,GL_FLOAT, ptr)...
	//glutSolidTeapot(1);


	glUseProgram(NULL);
	glDisable(GL_LIGHTING);
	//tobj->draw();
	//tree->draw();
	/*glColor3f(0.f, 1.f, 0.f);
	glBegin(GL_LINE_STRIP);
		glVertex3f(0.f, 0.f,0.f);
		glVertex3f(1.f,1.f,0.f);
		glColor3f(1.f, 0.f, 0.f);
		glVertex3f(1.f,1.f,0.f);
		glVertex3f(1.5f, 1.5f,0.f);
	glEnd();
	*/
	/*glColor3f(0.f, 0.f, 5.f);
	for (int i=0; i<20; i++){

		glBegin(GL_LINE_STRIP);
		glVertex3f(0.f, i*0.1f,0.f);
		glVertex3f(1.f, i*0.1f,0.f);
		glEnd();
	}*/
	glEnable(GL_LIGHTING);
	glColor3f(1.f, 1.f, 1.f);
	// draw tree/branch
	//xbr1->draw();
	//xbr2->draw();
	//xbr3->draw();
	tree->draw();
	// Return to fixed-functional GL pipeline
	

	// Render GUI controls
	GUIManager::display();			// Render gui components

	glutSwapBuffers();

	if (g_bAutoRotationEnabled)
	{
		rotY += 0.75f;
	}
}
void initApp(void){
	

	// init coord system of branch
	Vector3 org(0.f,0.f, 0.f);
	Vector3 org2(0.f,0.f, 0.f);
	Vector3 r(1.f,0.f, 0.f);
	Vector3 s(0.f,0.f, 1.f);
	Vector3 t(0.f,1.f, 0.f);
	cs = new CoordSystem(org, r,s,t);
	CoordSystem	xcs = cs->getRotated(v3(0.f, 0.f, 1.f), 1);
	xbr1 = new Branch(NULL, 0, *cs, 2, 0.1,0.1,10,1,0.3326, 0.398924);
	xbr1->setBending(1,0);
	
	xcs.origin.t = varC*xbr1->L;
	xbr2 = new Branch(xbr1, varC, xcs, 2, 0.1,0.1,2,4,0.3326, 0.398924);
	xbr2->setBending(0.1,0);
	
	tree = new Tree();
	tree->trunk = xbr1;
	tree->init();

	
	/*
	CoordSystem xcs3 = xcs.getRotated(v3(0.f, 0.f, 1.f), 1);
	
	xcs3.origin.z=1.0f;
	xbr3 = new Branch(xbr2, 0.5, xcs3, 2, 0.1,0.1,2,4,0.3326, 0.398924);
	*/
	//tree = new Tree();
	//tree->trunk = br;
	//tree->init();
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
}
void deinitApp(){
	delete br;
	delete br2;
	delete cs;
	delete cs2;
}
void onTimer(int value)
{
	ttime += PI/100;
	valA = sin(ttime);
	valB = sin(ttime*0.8);
	tree->setTime(ttime);
	tree->update(ttime);
	//xbr1->setBending(valA*varA,0);
	//xbr1->update();
	/*
	xbr2->x = varC;
	xbr2->originalCS.origin.t = varC*xbr1->L;
	xbr2->setBending(valB*varB,0);
	xbr2->update();
	xbr3->setBending(0.1,0);
	xbr3->update();
	*/
	//printf("%f\n",time );
	glutTimerFunc(10, onTimer, value);
}


//-----------------------------------------------------------------------------
// Name: initGL()
// Desc: 
//-----------------------------------------------------------------------------
void initGL()
{
	glClearColor(0.5f, .5f, .5f, 0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);

	float lightPos[4]		= {0.0f, 3.0f, 3.0f, 1.0f};
	float lightAmbient[4]	= {0.4f, 0.4f, 0.4f, 1.0f};
	float lightDiffuse[4]	= {0.0f, 0.0f, 0.0f, 1.0f};
	float lightSpecular[4]	= {0.5f, 0.5f, 0.5f, 1.0f};
	float lightDirection[4]	= {0.0f,-1.5f,-3.0f, 1.0f};
	
	float matGlobal[4]	= {0.1f, 0.1f, 0.1f, 1.0f};
	float matSpecular[4]= {0.8f, 0.8f, 0.8f, 1.0f};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matGlobal);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matGlobal);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 256);

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_POSITION      , lightPos);
	glLightfv(GL_LIGHT1, GL_AMBIENT       , lightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE       , lightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR	  , lightSpecular);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, lightDirection);
	glLightf (GL_LIGHT1, GL_SPOT_CUTOFF	  , 24);
	glLightf (GL_LIGHT1, GL_SPOT_EXPONENT , 512);
	
	g_bShadersSupported = (	GLEE_ARB_shader_objects && GLEE_ARB_shading_language_100 &&
							GLEE_ARB_vertex_shader  && GLEE_ARB_fragment_shader);
	texId = createSimpleTexture(256,256);

}


//-----------------------------------------------------------------------------
// Name: reshape()
// Desc: 
//-----------------------------------------------------------------------------
void reshape (int w, int h)
{
	if ((w > 1) && (h > 1))
	{
		g_WindowWidth  = w;
		g_WindowHeight = h;

		glViewport(0, 0, g_WindowWidth, g_WindowHeight);

		glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(60, GLfloat(g_WindowWidth)/ g_WindowHeight, 0.1f, 100.0f);
		glMatrixMode(GL_MODELVIEW);
	}
}


// GLUT FUNCTIONS______________________________________________________________

//-----------------------------------------------------------------------------
// Name: keyboard()
// Desc: 
//-----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 27 : 
			// Destroy shaders
			destroyShaderProgram(g_Program, &g_VertexShader, &g_GeometryShader, &g_FragmentShader);
			exit(0);
		break;
		case ' ': 
			callback_UseShaders(!g_bUseShaders);
		break;
		case 'c': 
			callback_CompileShaders();
		break;
		case 'h':
			GUIManager::visible(!GUIManager::isVisible());
		break;
	}

	glutPostRedisplay();
}


//-----------------------------------------------------------------------------
// Name: mouse()
// Desc: 
//-----------------------------------------------------------------------------
void mouse(int button, int state, int x, int y)
{
	if (!guiActive){
		if (state == GLUT_DOWN)
		{
			g_LastMouseX = x;
			g_LastMouseY = y;

			g_LastMouseButton = button;
			
			GUIManager::checkClickEvent(x, y);
		}
	}
}


//-----------------------------------------------------------------------------
// Name: mouseMotion()
// Desc: 
//-----------------------------------------------------------------------------
void mouseMotion(int x, int y)
{
	if (!guiActive){
		switch(g_LastMouseButton)
		{
			case GLUT_LEFT_BUTTON:
				g_RotObject[0] += ROTATION_SPEED*(y - g_LastMouseY);
				g_RotObject[1] += ROTATION_SPEED*(x - g_LastMouseX);
			break;

			case GLUT_RIGHT_BUTTON:
				g_CameraZ += TRANSLATION_SPEED*(x - g_LastMouseX);
			break;
		}
	}
	g_LastMouseX = x;
	g_LastMouseY = y;
}

	
//-----------------------------------------------------------------------------
// Name: mousePassiveMotion()
// Desc: 
//-----------------------------------------------------------------------------
void mousePassiveMotion(int x, int y)
{
	GUIManager::checkMoveEvent(x, y);
}


//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main (int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	
	glutInitWindowPosition(800, 100);
	glutInitWindowSize(g_WindowWidth, g_WindowHeight);
	glutCreateWindow("NATUrenderer");

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(display);

    glutTimerFunc(1, onTimer, 1234);


	glutMotionFunc(mouseMotion);
	glutPassiveMotionFunc(mousePassiveMotion);
	glutMouseFunc(mouse);

	initGL();
	initGUI();
	
	initApp();
	
	glutMainLoop();
	
	deinitApp();
	return 0;
}