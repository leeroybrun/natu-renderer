#ifndef _TREEBRANCH_H
#define _TREEBRANCH_H

#include "TreeComponent.h"

class TreeBranch: public TreeComponent
{
public:
	TreeBranch(tc* _parent, CoordSystem &_cs, float _x, TextureManager * _texMan, float length, float radiusAtBegin, float radiusAtEnd, int _divT, int _divR, float _c2, float _c4, v3 &_motionVector);

	~TreeBranch(void);

	// init
	void			init();
	
	// draw
	void			draw();
		
// variables
	int				divT;
	int				divR;
	float			r1, r2;
	float			c2, c4;
	float			L;
	int				id;
	int				level;

	v3				motionVector;

	int				motionTextureID;


	GLuint			indicesCount;
};

#endif
