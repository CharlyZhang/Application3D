
#ifndef _CZOBJMODEL_H_
#define _CZOBJMODEL_H_

#include <string>
#include <vector>
#include "CZVector.h"
#include "CZObjFileParser.h"
#include "CZGeometry.h"
#include "CZMaterialLib.h"
#include "CZShader.h"
#include "CZMat4.h"
#include "CZNode.h"
#include "ObjModel.hpp"

/// CZObjModel
class CZObjModel : public ObjModel, public CZObjFileParser, public CZNode
{
public:
	CZObjModel();
	~CZObjModel();

	bool load(const std::string& path) override;
    bool loadBinary(const std::string& path, const char *originalPath = NULL);
    bool saveAsBinary(const std::string& path);

	bool draw(CZShader *pShader, CZMat4 &viewProjMat) override;

private:
	void parseLine(std::ifstream& ifs, const std::string& ele_id) override;
	void parseMaterialLib(std::ifstream &ifs);		//mtllib <material lib name>
	void parseUseMaterial(std::ifstream &ifs);		//usemtl <material name>
	void parseVertex(std::ifstream &ifs);			//v <x> <y> <z>
	void parseVertexNormal(std::ifstream &ifs);		//vn <x> <y> <z>
	void parseVertexTexCoord(std::ifstream &ifs);	//vt <u> <v>
	void parseFace(std::ifstream &ifs);				//f <v/vt/vn <v/vt/vn> <v/vt/vn> 

    void transform2GCard();
    
	CZGeometry *pCurGeometry;
};
#endif