#include "TreeLeaf.h"


TreeLeaf::TreeLeaf(
		tc* _parent,
		CoordSystem &_cs,
		float _x,
		TextureManager * _texMan, 
		float _size,
		v3 &_motionVector         ):
	TreeComponent(_parent, _cs, _x, _texMan)
{
	type			= ComponentType::LEAF;
	size			= _size;
	parentID		= ((TreeBranch*)_parent)->id;

	vertPtr				= new float[4*3];
	normalPtr			= new float[4*3];
	binormalPtr			= new float[4*3];
	tangentPtr			= new float[4*3];
	indexPtr			= new int [4];
	dataTextureCoords	= new float [4*2];
	col1TextureCoords	= new float [4*2];
}


TreeLeaf::~TreeLeaf(void)
{
	delete [] vertPtr;
	delete [] normalPtr;
	delete [] binormalPtr;
	delete [] indexPtr;
	delete [] dataTextureCoords;
}

void TreeLeaf::init()
{
	// create quad...

	v3 normal	= originalCS.t;
	v3 binormal = originalCS.s;
	v3 tangent	= originalCS.r;

	v3 p1 = v3(0.0, 0.0,  -size/2.f);
	v3 p2 = v3(0.0, 0.0,   size/2.f);
	v3 p3 = v3(0.0, size,  size/2.f);
	v3 p4 = v3(0.0, size, -size/2.f);
	// TODO
	float size = 1.0;
	int i=0;
	writeTex(dataTextureCoords, this->x, float(((TreeBranch*)this->parent)->id), i);
	writeTex(col1TextureCoords, 0.f, 0.f, i);	
	write(normalPtr,normal,i);
	write(vertPtr,p1,i);
	write(binormalPtr,binormal,i);
	write(tangentPtr,tangent,i);
	indexPtr[i] = i;
	i++;
	writeTex(dataTextureCoords, this->x, float(((TreeBranch*)this->parent)->id), i);
	writeTex(col1TextureCoords, size, 0.f, i);	
	write(normalPtr,normal,i);
	write(vertPtr,p2,i);
	write(binormalPtr,binormal,i);
	write(tangentPtr,tangent,i);
	indexPtr[i] = i;
	i++;
	writeTex(dataTextureCoords, this->x, float(((TreeBranch*)this->parent)->id), i);
	writeTex(col1TextureCoords, size, size, i);	
	write(normalPtr,normal,i);
	write(vertPtr,p3,i);
	write(binormalPtr,binormal,i);
	write(tangentPtr,tangent,i);
	indexPtr[i] = i;
	i++;
	writeTex(dataTextureCoords, this->x, float(((TreeBranch*)this->parent)->id), i);
	writeTex(col1TextureCoords, 0.f, size, i);	
	write(normalPtr,normal,i);
	write(vertPtr,p4,i);
	write(binormalPtr,binormal,i);
	write(tangentPtr,tangent,i);
	indexPtr[i] = i;
	i++;
	printf("NORMALS:\n");
	print(normalPtr, i, 3);
	printf("BINORMALS:\n");
	print(binormalPtr, i, 3);
	printf("TANGENTS:\n");
	print(tangentPtr, i, 3);
}


void TreeLeaf::draw()
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
		//glActiveTexture(DATA_TEX_UNIT);
		glClientActiveTexture(DATA_TEX_UNIT);
		glTexCoordPointer(2, GL_FLOAT, 0, dataTextureCoords);

		//glActiveTexture(COL1_TEX_UNIT);
		glClientActiveTexture(COL1_TEX_UNIT);
		glTexCoordPointer(2, GL_FLOAT, 0, col1TextureCoords);
		
		glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, this->indexPtr);
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