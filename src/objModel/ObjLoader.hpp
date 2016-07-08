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
#include "CZObjModel.h"

#include <string>

class ObjLoader : public CZObjFileParser
{
public:
    static bool load(CZObjModel *objModel, std::string &filename);
    static bool loadFromTemp(CZObjModel *objModel, std::string &filename);
};

#endif /* ObjLoader_hpp */
