//
//  Render.hpp
//  Application3D
//
//  Created by CharlyZhang on 16/7/15.
//  Copyright © 2016年 CharlyZhang. All rights reserved.
//

#ifndef Render_hpp
#define Render_hpp

#include "../basic/CZMat4.h"
#include "../basic/CZBasic.h"
#include "CZNode.h"
#include <map>

class CZShader;
class Application3D;            ///< for developing
namespace CZ3D {
    
// shader type
typedef enum _ShaderType {
    kDirectionalLightShading,		///< directional light shadding mode
    kBlitImage,                      ///< blit image to the renderbuffer
    kBlitColor                      ///< blit color
} ShaderType;

typedef enum RenderMode_ {
    kDirectionalLight
} RenderMode;
    
typedef std::map<ShaderType,CZShader*> ShaderMap;
typedef std::map<void*,GLuint> GLResourceMap;
    
class Render
{
public:
    Render();
    ~Render();
    friend class ::Application3D;
    
    bool init();
    bool blitBackground(CZImage *bgImg, bool clearColor = true);
    bool frame(CZScene &scene,CZNode *pNode, bool clearColor = false);
    
    bool setMode(RenderMode mode);
    void setSize(int w, int h);
    void setGLSLDirectory(const char* glslDir);
    
private:
    bool draw(CZNode *pNode, CZMat4 &viewProjMat);
    bool drawObjModel(CZNode *pNode, CZMat4 &viewProjMat);
    bool transform2Gcard(CZNode *pNode);
    bool prepareBgImage(CZImage *bgImg);
    bool prepareBgVAO();
    
    bool enableTexture(CZImage *img);
    
    // shader
    CZShader* getShader(ShaderType type);
    bool loadShaders();
    
    ShaderMap shaders;
    CZMat4 projMat;
    RenderMode mode_;
    CZShader *curShader;
    
    uint32_t m_vao;
    GLuint m_vboPos;
    GLuint m_vboNorm;
    GLuint m_vboTexCoord;
    bool hasTransformed;

    // background image
    GLuint vao_bgImg, vbo_bgImg;
    ::CZImage *ptrBgImage;
    
    // textures
    static GLuint textures[128];
    static std::map<::CZImage*,short> textureMap;              ///< model textures mapping
    GLResourceMap backgroundImage;
    GLResourceMap vaoMap;                                      ///< use node memory address to index
    
    int width,height;
};

}

#endif /* Render_hpp */
