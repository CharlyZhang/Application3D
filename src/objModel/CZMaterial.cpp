#include "CZMaterial.h"

#include "CZLog.h"
#include "CZDefine.h"

using namespace std;

CZMaterial::CZMaterial()
{
    Ns = 10;						//	shininess
    Ka[0] = Ka[1] = Ka[2] = 0.2f;	//	ambient color
    Ka[3] = 0;
    Kd[0] = Kd[1] = Kd[2] = 0.8f;	// diffuse color
    Kd[3] = 0;
    Ks[0] = Ks[1] = Ks[2] = Ks[3] = 0;	//	specular color
    Ke[0] = Ke[1] = Ke[2] = Ke[3] = 0;	//	specular color
    
    hasTexture = false;
    texId = -1;
    texImage = NULL;
}

CZMaterial::~CZMaterial()
{
    if (texImage)   delete texImage;
    if (texId != -1) glDeleteTextures(1, &texId);
}

void CZMaterial::setTextureImage(CZImage *img)
{
    if(img == NULL || img->data == NULL)
    {
        LOG_WARN("image is illegal\n");
        return;
    }
    
    // clear the old
    if (texImage)   delete texImage;
    if (texId != -1) glDeleteTextures(1, &texId);
    
    texImage = img;
    
    //generate an OpenGL texture ID for this texture
    glGenTextures(1, &texId);
    //bind to the new texture ID
    glBindTexture(GL_TEXTURE_2D, texId);
    //store the texture data for OpenGL use
    if (img->colorSpace == CZImage::RGBA) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)img->width , (GLsizei)img->height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
    }
    else if (img->colorSpace == CZImage::RGB) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)img->width , (GLsizei)img->height,
			0, GL_RGB, GL_UNSIGNED_BYTE, img->data);
    }
	else if (img->colorSpace == CZImage::GRAY) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)img->width , (GLsizei)img->height,
                     0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img->data);
    }
    
    //	gluBuild2DMipmaps(GL_TEXTURE_2D, components, width, height, texFormat, GL_UNSIGNED_BYTE, bits);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CZCheckGLError();
    
    hasTexture = true;
}

bool CZMaterial::use() const
{
    if (texId != -1)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texId);
        //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#if !defined(__APPLE__)
		glEnable(GL_TEXTURE_2D);
#endif
        return true;
    }
    return false;
}
