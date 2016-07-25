//
//  ModelFactory.hpp
//  Application3D
//
//  Created by CharlyZhang on 16/7/8.
//  Copyright © 2016年 CharlyZhang. All rights reserved.
//

#ifndef ModelFactory_hpp
#define ModelFactory_hpp

namespace CZ3D {
    
class Model;

class ModelFactory
{
public:
    static Model* createObjModel(const char* filename);
    static Model* createObjModelFromTemp(const char* filename);
};

}
#endif /* ModelFactory_hpp */
