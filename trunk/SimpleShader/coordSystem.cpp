#include "coordSystem.h"

// contructor
CoordSystem::CoordSystem(v3 _origin, v3 _x, v3 _y, v3 _z):origin(_origin),r(_x),s(_y),t(_z){
	//origin.normalize();
	r.normalize();
	s.normalize();
	t.normalize();
};
//copy contructor
CoordSystem::CoordSystem(const CoordSystem& copy){
	origin = copy.origin;
	r = copy.r;
	s = copy.s;
	t = copy.t;
};
// rotate
void CoordSystem::rotate(v3 &axis, float angle){
	r.rotate(angle, axis);
	s.rotate(angle, axis);
	t.rotate(angle, axis);
};
CoordSystem CoordSystem::getRotated(v3 &axis, float angle){
	CoordSystem out;
	out.origin = origin;
	out.r = r.getRotated(angle, axis);
	out.s = s.getRotated(angle, axis);
	out.t = t.getRotated(angle, axis);
	return out;
}


// translate
void CoordSystem::translate(v3 &trans){
	origin+=trans;
};
// normalize axis vectors
void CoordSystem::normalize(){
	r.normalize();
	s.normalize();
	t.normalize();
};

void CoordSystem::draw(){
	// draw axis
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);

		glColor3f(1.f, 0.f, 0.f);
		glVertex3f(origin.x, origin.y, origin.z);
		glVertex3f(origin.x+r.x, origin.y+r.y, origin.z+r.z);
	glEnd();
	glBegin(GL_LINES);
		glColor3f(0.f, 1.f, 0.f);
		glVertex3f(origin.x, origin.y, origin.z);
		glVertex3f(origin.x+s.x, origin.y+s.y, origin.z+s.z);
	glEnd();
	glBegin(GL_LINES);

		glColor3f(0.f, 0.f, 1.f);
		glVertex3f(origin.x, origin.y, origin.z);
		glVertex3f(origin.x+t.x, origin.y+t.y, origin.z+t.z);
	glEnd();
	glEnable(GL_LIGHTING);
};

// destructor
CoordSystem::~CoordSystem(void){

};