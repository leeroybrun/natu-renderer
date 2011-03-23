#include "TreeBranch.h"


TreeBranch::TreeBranch(	tc* _parent,
						CoordSystem &_cs,
						float _x,
						TextureManager * _texMan,
						float length,
						float radiusAtBegin,
						float radiusAtEnd,
						int _divT,
						int _divR,
						float _c2,
						float _c4,
						v3 &_motionVector):
	TreeComponent(_parent, _cs, _x, _texMan)
{
	type	= ComponentType::BRANCH;
	L		= length;
	r1		= radiusAtBegin;
	r2		= radiusAtEnd;
	c2		= _c2;
	c4		= _c4;
	divT	= _divT;
	divR	= _divR;
	motionVector	= _motionVector;

	vertPtr				= new float[(divT+2)*divR*3];
	normalPtr			= new float[(divT+2)*divR*3];
	binormalPtr			= new float[(divT+2)*divR*3];
	tangentPtr			= new float[(divT+2)*divR*3];
	indexPtr			= new int [4*(divR)*(divT+1)];
	dataTextureCoords	= new float [(divT+2)*divR*2];
}


TreeBranch::~TreeBranch(void)
{
	delete [] vertPtr;
	delete [] normalPtr;
	delete [] binormalPtr;
	delete [] indexPtr;
	delete [] dataTextureCoords;
}

void TreeBranch::init()
{
	// generate vertices
	int i,j;
	int vi=0;
	int ii=0;
	float t = 0;
	float stepL = L/(divT+1);
	float angle = 0;
	float r = 0, s=0, R=0;
	
	// each ring:
	for (i=0; i<divT+2; i++){
		// position along the branch
		t = i*stepL;
		// interpolate radius along the branch
		R = (1-t/L)*(r1 - r2)+r2;

		// each vertex on ring:
		for (j=0; j<divR; j++){
		
			angle = TWO_PI/divR * j;
			v3 pos, oTangent, oNormal, oBinormal;
			r = R*cos(angle);
			s = R*sin(angle);
			pos+= (originalCS.r * r); // r
			pos+= (originalCS.s * s); // s
			pos+= (originalCS.t * t); // t
		
			oTangent	= originalCS.t.getNormalized();
			oNormal		= originalCS.r.getNormalized();
			oBinormal	= oTangent.cross(oNormal);
			pos+=  originalCS.origin;
			v3 bPos(r,s,t);

			// add in vertices array
			Vertex v(pos, bPos, oNormal, oTangent, t/L);
			// set texture coords (color texture)
			v.textureCoords.x = angle/TWO_PI;	//[0..1]
			v.textureCoords.y = v.x;			//[0..1]
			vertices.push_back(v);

			// fill texture coords [x-vals, branch ids]
			dataTextureCoords[2*vi/3] = v.x;
			dataTextureCoords[2*vi/3+1] = float(this->id);
			normalPtr[vi]	 = oNormal.x;	
			vertPtr[vi]		 = bPos.x;
			binormalPtr[vi]	 = oBinormal.x;
			tangentPtr[vi]	 = oTangent.x;
			vi++;
			normalPtr[vi]	 = oNormal.y;
			vertPtr[vi]		 = bPos.y;
			binormalPtr[vi]	 = oBinormal.y;
			tangentPtr[vi]	 = oTangent.y;
			vi++;
			normalPtr[vi]	 = oNormal.z;
			vertPtr[vi]		 = bPos.z;
			binormalPtr[vi]	 = oBinormal.z;
			tangentPtr[vi]	 = oTangent.z;
			vi++;

			// fill indices array
			if (i<divT+1){
				// not for the last ring
				if (j!=0){
					indexPtr[ii] = ((i+1)*divR+j);
					ii++;

					indexPtr[ii] = ((i)*divR+j);
					ii++;

				} 
				indexPtr[ii] = (i*divR+j); 
				ii++;

				indexPtr[ii] = ((i+1)*divR+j); 
				ii++;
			}

		}
		if (i<divT+1){
			indexPtr[ii] = ((i+1)*divR); 
			ii++;

			indexPtr[ii] = ((i)*divR);
			ii++;
		}
	}			
	indicesCount = ii;
}

// draw
void TreeBranch::draw()
{			
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (binormalID!=-1){
			// binormal present in shader...
			glVertexAttribPointer( binormalID, 3, GL_FLOAT, false, 0, binormalPtr );
			glEnableVertexAttribArray( binormalID );
			
		}
		if (tangentID!=-1){
			// binormal present in shader...
			glVertexAttribPointer( tangentID, 3, GL_FLOAT, false, 0, tangentPtr );
			glEnableVertexAttribArray( tangentID );
			
		}		
		glVertexPointer(3, GL_FLOAT, 0, vertPtr);
		glNormalPointer(GL_FLOAT, 0, normalPtr);

		// for each texture
		// color texture
		//glClientActiveTexture(GL_TEXTURE0);
		//glTexCoordPointer(2, GL_FLOAT, 0, dataTextureCoords); //TODO

		// data texture
		//glEnable(GL_TEXTURE_RECTANGLE_ARB);
		//glActiveTexture(DATA_TEX_UNIT);
		glClientActiveTexture(DATA_TEX_UNIT);
		glTexCoordPointer(2, GL_FLOAT, 0, dataTextureCoords);
				
		glDrawElements(GL_LINE_STRIP, indicesCount, GL_UNSIGNED_INT, this->indexPtr);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	if (binormalID!=-1){
		glDisableVertexAttribArray( binormalID );
	}
	if (tangentID!=-1){
		glDisableVertexAttribArray( tangentID );
	}
	GLenum errCode;
	const GLubyte *errString;

	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		//fprintf (stderr, "OpenGL Error: %s\n", errString);
	}
	
}