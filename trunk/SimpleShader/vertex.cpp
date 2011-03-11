#include "vertex.h"

Vertex::Vertex(v3 _oPos, v3 _bPos, v3 _normal, v3 _tangent, float _x){
	oPos = _oPos;
	bPos = _bPos;
	normal = _normal;
	tangent = _tangent;
	x = _x;
}

Vertex::Vertex(){
}

Vertex::~Vertex(){
}

void Vertex::printOut(void){
	printf ( "Vertex [%f, %f, %f], x=%f\n", oPos.x, oPos.y, oPos.z, x);
	
}