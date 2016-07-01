#ifndef _CZMATERIAL_H_
#define _CZMATERIAL_H_

#include <string>
#include "CZDefine.h"
#include "CZBasic.h"

class CZMaterial
{
public:
	CZMaterial();
    ~CZMaterial();

    void setTextureImage(CZImage *img);
    bool use() const;

public:
	float	Ns;		///<	shininess
    float   Ke[4];
	float	Ka[4];	///<	ambient color
	float	Kd[4];	///<	diffuse color
	float	Ks[4];	///<	specular color
    CZImage *texImage;

	bool hasTexture;
	unsigned int texId;

private:
	std::string	 dirPath;
};

#endif