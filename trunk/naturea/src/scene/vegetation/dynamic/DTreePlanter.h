#ifndef _DTREEPLANTER_H
#define _DTREEPLANTER_H

#include "settings.h"
#include "scene\terrain\Terrain.h"
#include "Vector2.h"
#include "scene\vegetation\dynamic\DTree.h"
#include <ctime>

class DTreePlanter
{
public:
	DTreePlanter();
	~DTreePlanter(void);
	void init(	Terrain *_terrain,
				DTree * _tree				
				);

	void createCandidates(float _height_min,
				float _height_max,
				float _dither,
				float _distance);

	void setInstanceCount(int _count);

	vector<v4> occupiedCandidates;
	int count;

private:
	vector<v4> freeCandidates;	
	float height_min;
	float height_max;
	int res_x;
	int res_y;
	float dither;
	float distance;
	DTree	*tree;
	Terrain *terrain;	
};

#endif

