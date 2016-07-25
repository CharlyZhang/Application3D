#include "Application3D.h"
#include "CZDefine.h"
#include "CZGeometry.h"
#include "shape/CZShape.h"
#include "shape/CZCube.hpp"
#include "ModelFactory.hpp"
#include "ObjLoader.hpp"
#include "SceneLoader.hpp"

#include "CZLog.h"
#include <ctime>


#define CONFIG_FILE_PATH	"./scene.cfg"

using namespace std;

namespace CZ3D {
    

#if defined(__ANDROID__)
char* GetImageClass = nullptr;
char* GetImageMethod = nullptr;

char* ModelLoadCallerClass = nullptr;
char* ModelLoadCallerMethod = nullptr;

#endif

Application3D::Application3D()
{
	documentDirectory = nullptr;
    backgroundImage = nullptr;
    sceneFilePath = nullptr;
}

Application3D::~Application3D()
{
#ifdef __ANDROID__
	if(GetImageClass)		{	delete [] GetImageClass; GetImageClass = nullptr;}
	if(GetImageMethod)		{	delete [] GetImageMethod; GetImageMethod = nullptr;}
	if(ModelLoadCallerClass)		{	delete [] ModelLoadCallerClass; ModelLoadCallerClass = nullptr;}
	if(ModelLoadCallerMethod)		{	delete [] ModelLoadCallerMethod; ModelLoadCallerMethod = nullptr;}
#endif
    
	if(documentDirectory)   delete [] documentDirectory;
    if (backgroundImage) {
        delete backgroundImage;
        backgroundImage = nullptr;
    }
    if(sceneFilePath) delete [] sceneFilePath;
}

bool Application3D::init(const char *glslDir,const char* sceneFilename /* = NULL */ )
{

#if	defined(__APPLE__)	|| defined(_WIN32)
    if(glslDir == nullptr)
    {
        LOG_ERROR("glslDir is nullptr!\n");
        return false;
    }
    render.setGLSLDirectory(glslDir);
#endif

    render.init();
    
	/// config scene
    SceneLoader sceneLoader;
	if(!sceneLoader.load(&scene,sceneFilename))
	{
		scene.eyePosition = CZPoint3D(0, 0, 100);
		scene.cameraFov = 45.f;
		scene.cameraNearPlane = 1.f;
		scene.camearFarPlane = 1000.f;
		scene.light.position = CZPoint3D(0, 0, -120);
		scene.light.intensity = CZPoint3D(1, 1, 1);
        scene.ambientLight.intensity = CZVector3D<float>(50*1.56 /255.f , 50*1.56 /255.f, 50*1.56 /255.f);
        scene.directionalLight.intensity = CZVector3D<float>(210 /255.f , 210 /255.f, 210 /255.f);
		//scene.directionalLight.direction = CZPoint3D(-105.351,-86.679,-133.965);
		scene.bgColor = CZColor(0.8f, 0.8f, 0.9f, 1.f);
		scene.mColor = CZColor(1.f, 1.f, 1.f, 1.f);
	}

    // reserve scene file path
    size_t len = strlen(sceneFilename);
    sceneFilePath = new char[len+1];
    strcpy(sceneFilePath, sceneFilename);
    sceneFilePath[len] = '\0';
    
	return true;
}

bool Application3D::loadObjModel(const char* filename, const char* filepath, bool quickLoad /* = true */)
{
	Model *pModel = nullptr;
    
	bool success = false;
	string strFilePath(filepath);
	string tempFilePath = strFilePath + ".b";
	if(documentDirectory)
	{
		size_t splitLoc = tempFilePath.find_last_of('/');
		size_t strLen = tempFilePath.length();
		string name = tempFilePath.substr(splitLoc+1,strLen-splitLoc-1);
		tempFilePath = string(documentDirectory) + "/" + name;
	}

    if(quickLoad)
        pModel = ModelFactory::createObjModelFromTemp(tempFilePath.c_str());
    
    if(pModel == nullptr)
	{
        pModel = ModelFactory::createObjModel(filepath);
		
		if(pModel && quickLoad)
            ObjLoader::saveToTemp(pModel,tempFilePath);
	}
    
    if(pModel == nullptr)
    {
        LOG_ERROR("obj model created failed!\n");
        return false;
    }

    string strFileName(filename);
    rootNode.addSubNode(strFileName, pModel);

	reset();

	modelLoadingDone();
	return true;
}

bool Application3D::setNodeVisible(const char* nodeName, bool visible)
{
    if(nodeName == nullptr)
    {
        LOG_ERROR("nodeName is nullptr!\n");
        return false;
    }
    
    string strNodeName(nodeName);
    CZNode *pNode = rootNode.getNode(strNodeName);
    
    if(pNode == nullptr)
    {
        LOG_ERROR("node(%s) doesnot exist!\n",nodeName);
        return false;
    }
    
    pNode->isVisible = visible;
    
    return true;
}
    
bool Application3D::clearObjModel()
{
    return rootNode.removeAllSubNodesOfType(CZNode::kObjModel);
}

bool Application3D::setRenderBufferSize(int w, int h)
{
    render.setSize(w, h);
	return true;
}

void Application3D::frame()
{
    clock_t nowTime = clock();
    animationManager.update(nowTime);
    
#ifdef SHOW_RENDER_TIME
	static clock_t start, finish;
	start = clock();
#endif

    if(backgroundImage)
        render.blitBackground(backgroundImage,true);
    
    if(backgroundImage)
        render.frame(scene,&rootNode,false);
    else
        render.frame(scene, &rootNode, true);

#ifdef SHOW_RENDER_TIME
	finish = clock();
	double totalTime = (double)(finish - start) / CLOCKS_PER_SEC;
	LOG_INFO("rendering time is %0.6f, FPS = %0.1f\n",totalTime, 1.0f/totalTime);
#endif
}

bool Application3D::createShape(const char* shapeFileName, bool contentInParam /*= false*/)
{
    CZCube *cube = new CZCube;
    CZPoint3D p(0,0,0);
    cube->create(p,1,1,1);
    string strShapeName(shapeFileName);
    rootNode.addSubNode(strShapeName, cube);
    
    CZPoint3D p1(15,0,0);
    CZCube *cube1 = new CZCube;
    cube1->create(p1, 10, 10, 10);
    string shape1Name("cube1");
    cube->addSubNode(shape1Name, cube1);
    return true;
}

bool Application3D::clearShapes()
{
    return rootNode.removeAllSubNodesOfType(CZNode::kShape);
}

void Application3D::reset()
{
    rootNode.resetMatrix();
    
    SceneLoader sceneLoader;
    sceneLoader.load(&scene, sceneFilePath);
}

#ifdef	__ANDROID__
bool Application3D::createShader(ShaderType type,const char* vertFile, const char* fragFile,std::vector<std::string> &attributes,std::vector<std::string> &uniforms)
{
	if(vertFile == nullptr || fragFile == nullptr)
	{
		LOG_ERROR("vertFile or fragFile is NULL\n");
		return false;
	}

	CZShader *pShader = new CZShader(vertFile,fragFile,attributes,uniforms,true);
	if (pShader->isReady() == false)
	{
		LOG_ERROR("Error: Creating Shaders failed\n");
		return false;
	}

	shaders.insert(make_pair(type,pShader));

	CZCheckGLError();

	return true;
}

void Application3D::setImageLoader(const char * cls, const char * method)
{
	if(cls == nullptr || method == nullptr)
	{
		LOG_ERROR("parameters contains nullptr\n");
		return;
	}

	if(GetImageClass)	delete[] GetImageClass;
	GetImageClass = new char[strlen(cls)+1];
	strcpy(GetImageClass,cls);

	if(GetImageMethod)	delete[] GetImageMethod;
	GetImageMethod = new char[strlen(method)+1];
	strcpy(GetImageMethod,method);
}

void Application3D::setModelLoadCallBack(const char * cls, const char *method)
{
	if(cls == nullptr || method == nullptr)
	{
		LOG_ERROR("parameters contains nullptr\n");
		return;
	}

	if(ModelLoadCallerClass)	delete[] ModelLoadCallerClass;
	ModelLoadCallerClass = new char[strlen(cls)+1];
	strcpy(ModelLoadCallerClass,cls);

	if(ModelLoadCallerMethod)	delete[] ModelLoadCallerMethod;
	ModelLoadCallerMethod = new char[strlen(method)+1];
	strcpy(ModelLoadCallerMethod,method);
}

#endif

// document directory
//  /note : default as the same of model's location;
//          should be set in ios platform to utilize binary data
void Application3D::setDocDirectory(const char* docDir)
{
	if (docDir == NULL)
	{
		LOG_WARN("docDir is NULL\n");
		return;
	}

	delete documentDirectory;
	size_t len = strlen(docDir);
	documentDirectory = new char[len+1];
	strcpy(documentDirectory, docDir);
	documentDirectory[len] = '\0';
	LOG_INFO("document diretory is %s\n", documentDirectory);
}

// control
void Application3D::rotate(float deltaX, float deltaY, const char *nodeName /*= nullptr*/)
{
	CZMat4 tempMat;
	if (nodeName == nullptr)    // rotate all nodes
	{
        tempMat.SetRotationY(deltaX);
        rootNode.rotateMat = tempMat * rootNode.rotateMat;
        tempMat.SetRotationX(-deltaY);
        rootNode.rotateMat = tempMat * rootNode.rotateMat;

	}
    
	else
	{
        string strNodeName(nodeName);
        CZNode *node = rootNode.getNode(strNodeName);
        if(node == nullptr) return;
        
        CZShape *shape = dynamic_cast<CZShape*>(node);
        if(shape && shape->isAnimating)
        {
            LOG_WARN("the shape is animating and cannot be controlled!\n");
            return;
        }
        
        tempMat.SetRotationY(deltaX);
		node->rotateMat = tempMat * node->rotateMat;
		tempMat.SetRotationX(-deltaY);
		node->rotateMat = tempMat * node->rotateMat;
	}

}

void Application3D::translate(float deltaX, float deltaY,  const char *nodeName /*= nullptr*/)
{
	CZMat4 tempMat;
	tempMat.SetTranslation(-deltaX, -deltaY, 0);
	if (nodeName == nullptr)
	{
        rootNode.translateMat = tempMat * rootNode.translateMat;
	}
	else
	{
        string strNodeName(nodeName);
        CZNode *node = rootNode.getNode(strNodeName);
        if(node == nullptr) return;
        
        CZShape *shape = dynamic_cast<CZShape*>(node);
        if(shape && shape->isAnimating)
        {
            LOG_WARN("the shape is animating and cannot be controlled!\n");
            return;
        }
        
		node->translateMat = tempMat * node->translateMat;
	}
	
}
void Application3D::scale(float s,  const char *nodeName /*= nullptr*/)
{
	CZMat4 tempMat;
	tempMat.SetScale(s);
	if (nodeName == nullptr)
	{
        rootNode.scaleMat = tempMat * rootNode.scaleMat;
	}
	else
	{
        string strNodeName(nodeName);
        CZNode *node = rootNode.getNode(strNodeName);
        if(node == nullptr) return;

        
        CZShape *shape = dynamic_cast<CZShape*>(node);
        if(shape && shape->isAnimating)
        {
            LOG_WARN("the shape is animating and cannot be controlled!\n");
            return;
        }
        
		node->scaleMat = tempMat * node->scaleMat;
	}
}

// custom config
void Application3D::setBackgroundColor(float r, float g, float b, float a)
{
	scene.bgColor = CZColor(r,g,b,a);
}
void Application3D::setBackgroundImage(CZImage *img)
{
	if(img == NULL || img->data == NULL)
	{
		LOG_WARN("img is illegal\n");
		return ;
	}

	// clear the old
	if (backgroundImage) delete backgroundImage;
	backgroundImage = img;
}

// camera
void Application3D::setCameraPosition(float x, float y, float z)
{
	scene.eyePosition = CZVector3D<float>(x,y,z);
}

// light
void Application3D::setLightPosition(float x, float y, float z)
{
	scene.light.position = CZPoint3D(x, y, z);
}
void Application3D::setLigthDirection(float x, float y, float z)
{
	scene.directionalLight.direction = CZVector3D<float>(x,y,z);
}
void Application3D::setAmbientColor(unsigned char r, unsigned char g, unsigned char b)
{
	scene.ambientLight.intensity = CZVector3D<float>(r/255.0f,g/255.0f,b/255.0f);
}
void Application3D::setDiffuseColor(unsigned char r, unsigned char g, unsigned char b)
{
	scene.directionalLight.intensity = CZVector3D<float>(r/255.0f,g/255.0f,b/255.0f);
}

}