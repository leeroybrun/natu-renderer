//-----------------------------------------------------------------------------
//  [PGR2] Common function definitions
//  27/02/2008
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#include "GLEW/glew.h"
#include "GLFW/glfw.h"

#include <assert.h>
#ifdef USE_ANTTWEAKBAR
#  include "../common/AntTweakBar/AntTweakBar.h"
#else
#  include <stdio.h>
#  include <stdlib.h>
#  define TW_CALL
#endif

#include "tools.h"

extern bool g_MouseModeANT;


// FUNCTION POINTER TYPES______________________________________________________
/* Function pointer types */
typedef void (* TInitGLCallback)(void);
typedef void (* TDisplayCallback)(void);
typedef void (GLFWCALL * TWindowSizeChangedCallback)(int,int);
typedef void (GLFWCALL * TWindowRefreshCallback)(void);
typedef void (GLFWCALL * TMouseButtonChangedCallback)(int,int);
typedef void (GLFWCALL * TMousePositionChangedCallback)(int,int);
typedef void (GLFWCALL * TMouseWheelChangedCallback)(int);
typedef void (GLFWCALL * TKeyboardChangedCallback)(int,int);
typedef void (GLFWCALL * TThreadCallback)(void *);
//typedef int  (GLFWCALL * TWindowClosedCallback)(void);
//typedef void (GLFWCALL * TCharacterChangedCallback)(int,int);


// INTERNAL USER CALLBACK FUNCTION POINTERS____________________________________
TDisplayCallback              g_cbUserDisplay               = NULL;
TWindowSizeChangedCallback    g_cbUserWindowSizeChanged     = NULL;
TMouseButtonChangedCallback   g_cbUserMouseButtonChanged    = NULL;
TMousePositionChangedCallback g_cbUserMousePositionChanged  = NULL;
TKeyboardChangedCallback      g_cbUserKeyboardChanged       = NULL;


// INTERNAL CALLBACK FUNCTION DEFINITIONS______________________________________

//-----------------------------------------------------------------------------
// Name: _cbWindowSizeChanged()
// Desc: internal
//-----------------------------------------------------------------------------
void GLFWCALL _cbWindowSizeChanged(int width, int height)
{
   if (height < 1) 
   {
      height = 1;
   }

   if (g_cbUserWindowSizeChanged)
   {
      g_cbUserWindowSizeChanged(width, height);
   }

   // Send the new window size to AntTweakBar
#ifdef USE_ANTTWEAKBAR
   TwWindowSize(width, height);
#endif
}

//-----------------------------------------------------------------------------
// Name: _cbKeyboardChanged()
// Desc: internal
//-----------------------------------------------------------------------------
void GLFWCALL _cbKeyboardChanged(int key, int action)
{
//   TwEventKeyGLFW(key, action);
#ifdef USE_ANTTWEAKBAR
   if (!TwEventCharGLFW(key, action))
#endif
   if (action == GLFW_PRESS)
   {
      if (key == GLFW_KEY_ESC)
      {
         exit(0);
      }
      
      if (g_cbUserKeyboardChanged)
      {
         g_cbUserKeyboardChanged(key, action);
      }
   }
}


//-----------------------------------------------------------------------------
// Name: _cbMouseButtonChanged()
// Desc: internal
//-----------------------------------------------------------------------------
void GLFWCALL _cbMouseButtonChanged(int button, int action)
{
#ifdef USE_ANTTWEAKBAR
	if (g_MouseModeANT){
		TwEventMouseButtonGLFW(button, action);
	} else {
#endif  
		if (g_cbUserMouseButtonChanged)
		{
			g_cbUserMouseButtonChanged(button, action);
		}
#ifdef USE_ANTTWEAKBAR
	}
#endif
}


//-----------------------------------------------------------------------------
// Name: _cbMousePositionChanged()
// Desc: internal
//-----------------------------------------------------------------------------
void GLFWCALL _cbMousePositionChanged(int x, int y)
{
  
#ifdef USE_ANTTWEAKBAR
  if (g_MouseModeANT){
   TwEventMousePosGLFW(x, y);
  } else {
#endif  
  if (g_cbUserMousePositionChanged)
   {
      g_cbUserMousePositionChanged(x, y);
   }
#ifdef USE_ANTTWEAKBAR
  }
#endif
}


//-----------------------------------------------------------------------------
// Name: common_main()
// Desc: 
//-----------------------------------------------------------------------------
int common_main(int window_width, int window_height, 
                const char* window_title,
                TInitGLCallback               cbUserInitGL,
                TDisplayCallback              cbUserDisplay,
                TWindowSizeChangedCallback    cbUserWindowSizeChanged,
                TKeyboardChangedCallback      cbUserKeyboardChanged,
                TMouseButtonChangedCallback   cbUserMouseButtonChanged,
                TMousePositionChangedCallback cbUserMousePositionChanged) 
{
   // Setup user callback functions
   assert(cbUserDisplay && cbUserInitGL);
   g_cbUserDisplay               = cbUserDisplay;
   g_cbUserWindowSizeChanged     = cbUserWindowSizeChanged;
   g_cbUserKeyboardChanged       = cbUserKeyboardChanged;
   g_cbUserMouseButtonChanged    = cbUserMouseButtonChanged;
   g_cbUserMousePositionChanged  = cbUserMousePositionChanged;

   // Intialize GLFW   
   glfwInit();

   // Create a window
   glfwOpenWindow(window_width, window_height, 0, 0, 0, 0, 32, 0, GLFW_WINDOW);
   glfwSetWindowTitle(window_title);
   glfwSetWindowPos(100, 100);

   glfwEnable(GLFW_MOUSE_CURSOR);
   glfwEnable(GLFW_KEY_REPEAT);

   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
      printf("Error: %s\n", glewGetErrorString(err));
      assert(0);
      return 1;
   }

   // Init GL
   if (cbUserInitGL)
   {
      cbUserInitGL();
   }

   // Set GLFW event callbacks
   glfwSetWindowSizeCallback(_cbWindowSizeChanged);
   glfwSetCharCallback(_cbKeyboardChanged);
   glfwSetMouseButtonCallback(_cbMouseButtonChanged);
   glfwSetMousePosCallback(_cbMousePositionChanged);
//   glfwSetKeyCallback(cbKeyboardChanged);
  
   // Main loop
   while(glfwGetWindowParam(GLFW_OPENED) && !glfwGetKey(GLFW_KEY_ESC))
   {
      if (g_cbUserDisplay)
      {
         g_cbUserDisplay();
      }

#ifdef USE_ANTTWEAKBAR
      // Draw tweak bars
      TwDraw();
#endif
      // Present frame buffer
      glfwSwapBuffers();
   }
 
   // Terminate AntTweakBar and GLFW
#ifdef USE_ANTTWEAKBAR
   TwTerminate();
#endif
   glfwTerminate();    // Terminate GLFW

   return 0;
} 
