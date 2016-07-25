#ifndef _CZAPPLICATION3D_H_
#define _CZAPPLICATION3D_H_

#include <string>
#include <vector>
#include "CZBasic.h"
#include "CZNode.h"
#include "CZAnimaitonManager.hpp"

namespace CZ3D {

class Render;
    
class Application3D 
{
public:
	Application3D();
	~Application3D();

	bool init(const char *glslDir, const char* sceneFilename = NULL);
	bool loadObjModel(const char* nodeName, const char* filepath, bool quickLoad = true);
    bool setNodeVisible(const char* nodeName, bool visible);
    bool clearObjModel();
	bool setRenderBufferSize(int w, int h);
	void frame();
	void reset();
    void freeGraphicResources();
    
    // shape
    bool createShape(const char* shapeFileName, bool contentInParam = false);
    bool clearShapes();
    void animateShape();

#ifdef	__ANDROID__
	bool createShader(ShaderType type,const char* vertFile, const char* fragFile, std::vector<std::string> &attributes,std::vector<std::string> &uniforms);
    void setImageLoader(const char * cls, const char * method);
    void setModelLoadCallBack(const char * cls, const char *method);
#endif
    
	// control
	//	/note : (deltaX,deltaY) is in the screen coordinate system
	void rotate(float deltaX, float deltaY, const char *nodeName = nullptr);
	void translate(float deltaX, float deltaY, const char *nodeName = nullptr);
	void scale(float s, const char *nodeName = nullptr);

    // document directory
    //  /note : default as the same of model's location;
    //          should be set in ios platform to utilize binary data
    void setDocDirectory(const char* docDir);
    
	// custom config
	void setBackgroundColor(float r, float g, float b, float a);
    void setBackgroundImage(CZImage *img);
    // camera
    void setCameraPosition(float x, float y, float z);
    
	// light
	void setLightPosition(float x, float y, float z);   ///< TO DEPRECATED
    void setLigthDirection(float x, float y, float z);
    void setAmbientColor(unsigned char r, unsigned char g, unsigned char b);
    void setDiffuseColor(unsigned char r, unsigned char g, unsigned char b);

private:
	CZScene scene;
    CZNode rootNode;
    CZAnimationManager animationManager;
    Render *pRender;
    CZImage *backgroundImage;
    char *documentDirectory;                          ///< to store the binary data of model
    char *sceneFilePath;                              ///< to store the scene file path
};

}
#endif