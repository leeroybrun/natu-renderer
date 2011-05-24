#ifndef _TREELEAF_H
#define _TREELEAF_H


#include "TreeComponent.h"
#include "TreeBranch.h"

class TreeLeaf :
	public TreeComponent
{
public:
	TreeLeaf(tc* _parent, CoordSystem &_cs, float _x, TextureManager * _texMan, float _size, v3 &_motionVector);

	~TreeLeaf(void);
	// init
	void			init();
	
	// draw
	void			draw();
	//	leaf?
	bool			isLeaf(){ return true;	}	
// variables
	float			size;
	v4				xvals;
	int				parentID;
	
};

#endif
