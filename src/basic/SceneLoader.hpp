//
//  SceneLoader.hpp
//  Application3D
//
//  Created by CharlyZhang on 16/7/25.
//  Copyright © 2016年 CharlyZhang. All rights reserved.
//

#ifndef SceneLoader_hpp
#define SceneLoader_hpp

#include "CZObjFileParser.h"
#include "CZBasic.h"

namespace CZ3D {
    
class SceneLoader : private CZObjFileParser
{
public:
    SceneLoader(){};
    ~SceneLoader(){};
    bool load(CZScene *pScene, const char *path);
    
private:
    void parseLine(std::ifstream& ifs, const std::string& ele_id) override;
    void parseEyePosition(std::ifstream& ifs);
    void parseCameraFov(std::ifstream& ifs);
    void parseCameraNearPlane(std::ifstream& ifs);
    void parseCameraFarPlane(std::ifstream& ifs);
    void parsePointLight(std::ifstream& ifs);
    void parseDirectionalLight(std::ifstream& ifs);
    void parseBackgroundColor(std::ifstream& ifs);
    void parseMainColor(std::ifstream& ifs);
    
    CZScene *pCurScene;
};
    
}

#endif /* SceneLoader_hpp */
