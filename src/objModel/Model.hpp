//
//  Model.hpp
//  Application3D
//
//  Created by CharlyZhang on 16/7/25.
//  Copyright © 2016年 CharlyZhang. All rights reserved.
//

#ifndef Model_hpp
#define Model_hpp

#include "CZNode.h"
#include "ObjModel.hpp"

namespace CZ3D {
    
    class Model : public ObjModel, public CZNode
    {
    public:
        Model();
        ~Model();
    };
    
}


#endif /* Model_hpp */
