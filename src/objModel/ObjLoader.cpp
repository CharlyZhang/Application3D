//
//  ObjLoader.cpp
//  Application3D
//
//  Created by CharlyZhang on 16/7/8.
//  Copyright © 2016年 CharlyZhang. All rights reserved.
//

#include "ObjLoader.hpp"
#include "CZDefine.h"

using namespace std;

bool ObjLoader::load(CZObjModel *objModel, std::string &filename)
{
    if (objModel == nullptr)
    {
        LOG_ERROR("objModel is nullptr!\n");
        return false;
    }
    
    return objModel->parseFile(filename);
}

bool ObjLoader::loadFromTemp(CZObjModel *objModel, std::string &filename)
{
    if (objModel == nullptr)
    {
        LOG_ERROR("objModel is nullptr!\n");
        return false;
    }
    
    return objModel->loadBinary(filename);
}
