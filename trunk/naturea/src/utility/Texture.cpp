#include "Texture.h"


Texture::Texture(void)
{
	id = 0;
	textureUnitNumber = 0;
}
Texture::Texture(string _inShaderName)
{
	id =0;
	inShaderName = _inShaderName;
	textureUnitNumber = 0;
}

Texture::Texture(GLuint _texType, GLuint _inFormat, GLenum _dataFormat, GLenum _dataType, GLvoid * _data, GLsizei _width, GLsizei _height, string _inShaderName)
{
	inShaderName = _inShaderName;
	textureUnitNumber = 0;
	dataFormat = _dataFormat;
	width = _width;
	height = _height;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &id);	
	glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	 glTexImage2D(GL_TEXTURE_2D, 0, _inFormat, _width, _height, 0, _dataFormat, _dataType, _data);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

Texture::~Texture(void)
{
	glDeleteTextures(1, &id);
	printf("texture: %s deleted.\n", inShaderName.c_str());
}

void Texture::show(GLint x,GLint y, GLsizei width, GLsizei height){
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, g_WinWidth, 0, g_WinHeight, -1,1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);
	glColor4f(1.f,1.f,1.f,1.f);

	glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.f); glVertex2i(x,y);
		glTexCoord2f(1.f, 0.f); glVertex2i(x+width,y);
		glTexCoord2f(1.f, 1.f); glVertex2i(x+width,y+height);
		glTexCoord2f(0.f, 1.f); glVertex2i(x,y+height);
	glEnd();

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void Texture::activate()
{
	glActiveTexture(textureUnit);	
	glClientActiveTexture(textureUnit);
	glEnable(GL_TEXTURE_2D);
}

void Texture::deactivate()
{
	glActiveTexture(textureUnit);	
	glDisable(GL_TEXTURE_2D);
	glClientActiveTexture(GL_TEXTURE0);
}


bool Texture::load(string filename, bool buildMipmaps, bool makeFloat,  GLint wrapMode, GLint magFilter, GLint minFilter )
{
	GLint type = GL_TEXTURE_2D;
	vector<unsigned char> buffer;
	vector<unsigned char> image;
	LodePNG::loadFile(buffer, filename); //load the image file with given filename
	LodePNG::Decoder decoder;
	decoder.inspect(buffer);
	if (decoder.isGreyscaleType()){
		decoder.getInfoRaw().color.colorType = 0;
		decoder.getInfoRaw().color.bitDepth = 16;
		// is grayscale

	}
	void * data;
	decoder.decode(image, buffer);
	width  = int(decoder.infoPng.width); 
	height = int(decoder.infoPng.height);
	int channels = int(decoder.getChannels());
	unsigned char* dataC = new (unsigned char[width*height*channels]);
	float* dataF = new (float[width*height*channels]);
	int x,y,c;
	for(x=0; x<width; x++){
		for(y=0; y<height; y++){
			for(c=0; c<channels; c++){
				if (makeFloat){
					dataF[channels * y * width + channels * x + c] = float(image[4 * y * width + 4 * x + c])/255.f;
				} else {
					dataC[channels * y * width + channels * x + c] = image[4 * y * width + 4 * x + c];
				}
			}
		}
	}
	if (makeFloat){
		data = dataF;
	} else {
		data = dataC;
	}
	
	switch(channels){
		case 3:
			dataFormat = GL_RGB;
			break;
		case 4:
			dataFormat = GL_RGBA;
			break;
	}
	glEnable(type);
	glGenTextures(1, &id);	
	glBindTexture(type, id);
		glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapMode); 
		glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(type, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, minFilter);
		if (makeFloat){
			int iformat = (dataFormat==GL_RGBA) ? GL_RGBA16F : GL_RGB16F;
			glTexImage2D(type, 0, iformat, width, height, 0, dataFormat, GL_FLOAT, data);
		} else {
			glTexImage2D(type, 0, dataFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		}
		if (buildMipmaps){
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	glBindTexture(type, 0);
	glDisable(type);
	image.clear();
	buffer.clear();
	delete [] data;
	data = NULL;
	if(decoder.hasError())
	  {
		return false;
	  }
	return true;

}
	

void Texture::load(string filename, GLint unitNumber, bool buildMipmaps, GLint wrapMode, GLint filterMode, bool makeFloat)
{
	// pre-set texture unit
	textureUnit = GL_TEXTURE0 + unitNumber;
	textureUnitNumber = unitNumber;
	// load image
	PNG png;
	png.load(filename);
	width	= png.width;
	height	= png.height;
	if (!makeFloat){
		unsigned char * data = png.getData();
		// generate
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		if (buildMipmaps){
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
	} else {
		// float
		float * data = png.getFloatData();
		// generate
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		if (buildMipmaps){
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA32F, width, height, GL_RGBA, GL_FLOAT, data);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	png.clear();
}

unsigned char * Texture::snapshot(){
	int pixelCount = width * height;
	unsigned char * pixels = new unsigned char [4*sizeof(unsigned char)*pixelCount];
	glEnable(GL_TEXTURE_2D);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, this->id);

	//glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0,0, width, height, 0);

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	return pixels;
}

void Texture::save(string filename)
{
	/*GLuint fboID = 0;
	glGenFramebuffersEXT(1, &fboID);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboID);
		
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, , 0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		*/


	printf("trying to get texture id: %i\n", this->id);
	
	unsigned char * pixels = snapshot();

	PNG png;
	png.create(width, height, pixels);
	if (png.save(filename)){
		printf("\"%s\" successfully saved...", filename.c_str());
	} else {
		printf("Failed to save \"%s\" !!!");
	}
}



void Texture::bind(GLenum texUnit)
{
	textureUnit = texUnit;
	textureUnitNumber = texUnitToNumber(texUnit);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(texUnit);
	//glClientActiveTexture(texUnit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::unbind()
{
	glActiveTexture(textureUnit);
	//glClientActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}