#include "Texture.h"

Texture::Texture(){
	data = NULL;
}

Texture::Texture(GLuint _texType, GLuint _inFormat, GLenum _dataFormat, GLenum _dataType, GLvoid * _data, GLsizei _width, GLsizei _height)
{
	type = _texType;
	data = _data;
	dataFormat = _dataFormat;
	width = _width;
	height = _height;
	glEnable(type);
	glGenTextures(1, &glID);	
	glBindTexture(type, glID);
		glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP); 
		glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(type, 0, _inFormat, _width, _height, 0, _dataFormat, _dataType, _data);
	glBindTexture(type, 0);
	glDisable(type);
}


Texture::~Texture(void)
{
	glDeleteTextures(1, &glID);
	if (data!=NULL){
		delete [] data;
	}

}
bool Texture::loadTexture(const char * filename){
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
	decoder.decode(image, buffer);
	width  = int(decoder.infoPng.width); 
	height = int(decoder.infoPng.height);
	int channels = int(decoder.getChannels());
	unsigned char* dataC = new (unsigned char[width*height*channels]);
	int x,y,c;
	for(x=0; x<width; x++){
		for(y=0; y<height; y++){
			for(c=0; c<channels; c++){
				dataC[channels * y * width + channels * x + c] = image[4 * y * width + 4 * x + c];
			}
		}
	}
	data = dataC;
	type = GL_TEXTURE_2D;
	switch(channels){
		case 3:
			dataFormat = GL_RGB;
			break;
		case 4:
			dataFormat = GL_RGBA;
			break;
	}
	glEnable(type);
	glGenTextures(1, &glID);	
	glBindTexture(type, glID);
		glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP); 
		glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(type, 0, dataFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
	glBindTexture(type, 0);
	glDisable(type);
	image.clear();
	buffer.clear();
	if(decoder.hasError())
	  {
		return false;
	  }
	return true;

}


void Texture::bindTexture(GLenum texUnit){
		//glEnable(type);
		glActiveTexture(texUnit);
		//glClientActiveTexture(texUnit);
		glBindTexture(type, glID);	
}

void Texture::unbindTexture(GLenum texUnit){
		glActiveTexture(texUnit);	
		glBindTexture(type, 0);
}