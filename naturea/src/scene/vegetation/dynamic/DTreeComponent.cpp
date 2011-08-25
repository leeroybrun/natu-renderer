#include "DTreeComponent.h"

DTreeComponent::DTreeComponent(tc* _parent, CoordSystem &_cs, float _x):
	x(_x),
	originalCS(_cs),
	cs(_cs),
	parent(_parent)
{
	if (parent!=NULL){
		parent->children.push_back(this);
	}
	binormalID= -1;
	tangentID = -1;
	vertPtr = normalPtr = binormalPtr = NULL;
	indexPtr = NULL;

}

DTreeComponent::~DTreeComponent(void)
{
	children.clear();
}

// methods
	
		// set bending


void DTreeComponent::write(float * arr, v3 & vec3in, int index){
	arr[3*index + 0] = vec3in.x;
	arr[3*index + 1] = vec3in.y;
	arr[3*index + 2] = vec3in.z;
}
void DTreeComponent::writeTex(float * texArr, float x, float y, int index){
	texArr[2*index + 0] = x;
	texArr[2*index + 1] = y;
}

void DTreeComponent::print(float * arr, int cnt, int dataCnt){
	int i;
	for (int k=0; k<cnt*dataCnt; k+=dataCnt){
		printf("[%i]",k/dataCnt);
		for (i=0; i<dataCnt; i++){
			printf("%f ", arr[k+i]);

		}
		printf("\n");
	}
}
		
		
