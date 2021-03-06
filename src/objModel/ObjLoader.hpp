//
//  ObjLoader.hpp
//  Application3D
//
//  Created by CharlyZhang on 16/7/8.
//  Copyright © 2016年 CharlyZhang. All rights reserved.
//

#ifndef ObjLoader_hpp
#define ObjLoader_hpp

#include "CZObjFileParser.h"
#include "Model.hpp"

#include <string>

namespace CZ3D {
    
class ObjLoader : public CZObjFileParser
{
public:
    bool load(Model *objModel, std::string &path);
    bool loadFromTemp(Model *objModel, std::string &path);
    
    bool saveToTemp(Model *objModel, const std::string& path);
    
    bool parseFile(const std::string& path) override;
    
private:
    void parseLine(std::ifstream& ifs, const std::string& ele_id) override;
    void parseMaterialLib(std::ifstream &ifs);		//mtllib <material lib name>
    void parseUseMaterial(std::ifstream &ifs);		//usemtl <material name>
    void parseVertex(std::ifstream &ifs);			//v <x> <y> <z>
    void parseVertexNormal(std::ifstream &ifs);		//vn <x> <y> <z>
    void parseVertexTexCoord(std::ifstream &ifs);	//vt <u> <v>
    void parseFace(std::ifstream &ifs);				//f <v/vt/vn <v/vt/vn> <v/vt/vn>
    
    CZGeometry *pCurGeometry;
    Model *pCurModel;
};
    
}

#endif /* ObjLoader_hpp */
