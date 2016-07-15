#include "Application3D.h"
#include "CZDefine.h"
#include "CZGeometry.h"
#include "shape/CZShape.h"
#include "shape/CZCube.hpp"
#include "ModelFactory.hpp"
#include "ObjLoader.hpp"
#include "CZLog.h"
#include <ctime>
#include <vector>
#include <string>

#define DEFAULT_RENDER_SIZE 500					///< default render buffer size
#define CONFIG_FILE_PATH	"./scene.cfg"

using namespace std;

GLuint Application3D::textures[128];
std::map<CZImage*,short> Application3D::textureMap;

#if defined(__ANDROID__)
char* GetImageClass = nullptr;
char* GetImageMethod = nullptr;

char* ModelLoadCallerClass = nullptr;
char* ModelLoadCallerMethod = nullptr;

#endif

Application3D::Application3D()
{
	width = height = DEFAULT_RENDER_SIZE;
	documentDirectory = NULL;
	backgroundImage = NULL;
	backgroundTexId = -1;
	vao = -1;
    for(auto i = 0; i < 128; i ++) textures[i] = -1;
    textureMap.clear();
}

Application3D::~Application3D()
{
	for (map<ShaderType,CZShader*>::iterator itr = shaders.begin(); itr != shaders.end(); itr++)
	{
		delete itr->second;
	}
	shaders.clear();
    
#ifdef __ANDROID__
	if(GetImageClass)		{	delete [] GetImageClass; GetImageClass = nullptr;}
	if(GetImageMethod)		{	delete [] GetImageMethod; GetImageMethod = nullptr;}
	if(ModelLoadCallerClass)		{	delete [] ModelLoadCallerClass; ModelLoadCallerClass = nullptr;}
	if(ModelLoadCallerMethod)		{	delete [] ModelLoadCallerMethod; ModelLoadCallerMethod = nullptr;}
#endif
    
	if(documentDirectory)   delete [] documentDirectory;
	if (backgroundImage) {
		delete backgroundImage;
		backgroundImage = NULL;
	}
	if (backgroundTexId != -1) glDeleteTextures(1, &backgroundTexId);
	if (vao != -1) GL_DEL_VERTEXARRAY(1, &vao);
    for (auto i = 0; i < 128 && textures[i] != -1; i++)
        glDeleteTextures(1, &textures[i]);
    textureMap.clear();
}

bool Application3D::init(const char *glslDir,const char* sceneFilename /* = NULL */ )
{
# ifdef _WIN32
	/// OpenGL initialization
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel(GL_SMOOTH);					// smooth shade model

	glClearDepth(1.0f);							// set clear depth
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);



	//texture
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	//glEnable(GL_TEXTURE_2D);
# else

	glClearDepthf(1.0f);									// set clear depth
	glEnable(GL_DEPTH_TEST);
# endif

	//    glCullFace(GL_BACK);                            ///< cull back face
	//    glEnable(GL_CULL_FACE);

	CZCheckGLError();

#if	defined(__APPLE__)	|| defined(_WIN32)
    if(glslDir == nullptr)
    {
        LOG_ERROR("glslDir is nullptr!\n");
        return false;
    }
    setGLSLDirectory(glslDir);
	/// load shader
	loadShaders();
#endif


	/// config scene
	if(!parseFile(sceneFilename))
	{
		scene.eyePosition = CZPoint3D(0, 0, 100);
		scene.cameraFov = 45.f;
		scene.cameraNearPlane = 1.f;
		scene.camearFarPlane = 1000.f;
		scene.light.position = CZPoint3D(0, 0, -120);
		scene.light.intensity = CZPoint3D(1, 1, 1);
		this->setAmbientColor(50*1.56, 50*1.56, 50*1.56);
		this->setDiffuseColor(210,210,210);
		scene.directionalLight.direction = CZPoint3D(-105.351, -86.679, -133.965);

		//scene.directionalLight.direction = CZPoint3D(-105.351,-86.679,-133.965);
		scene.bgColor = CZColor(0.8f, 0.8f, 0.9f, 1.f);
		scene.mColor = CZColor(1.f, 1.f, 1.f, 1.f);
	}

	CZCheckGLError();

	return true;
}

#define DEBUG
bool Application3D::loadObjModel(const char* filename, bool quickLoad /* = true */)
{
	CZObjModel *pModel = nullptr;

#ifdef DEBUG
    quickLoad = false;
#endif
    
	bool success = false;
	string strFileName(filename);
	string tempFileName = strFileName + ".b";
	if(documentDirectory)
	{
		size_t splitLoc = tempFileName.find_last_of('/');
		size_t strLen = tempFileName.length();
		string name = tempFileName.substr(splitLoc+1,strLen-splitLoc-1);
		tempFileName = string(documentDirectory) + "/" + name;
	}

    if(quickLoad)
        pModel = ModelFactory::createObjModelFromTemp(tempFileName.c_str());
    
    if(pModel == nullptr)
	{
        pModel = ModelFactory::createObjModel(filename);
		
		if(pModel && quickLoad)
            ObjLoader::saveToTemp(pModel,tempFileName);
	}
    
    if(pModel == nullptr)
    {
        LOG_ERROR("obj model created failed!\n");
        return false;
    }

    rootNode.addSubNode(strFileName, pModel);

	reset();

	CZCheckGLError();

	modelLoadingDone();
	return true;
}

bool Application3D::clearObjModel()
{
    return rootNode.removeAllSubNodesOfType(CZNode::kObjModel);
}

bool Application3D::setRenderBufferSize(int w, int h)
{
	width = w;	height = h;

	glViewport(0,0,width,height);
# ifdef _WIN32
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(scene.cameraFov,(GLfloat)width/(GLfloat)height, scene.cameraNearPlane, scene.camearFarPlane);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

# endif

	//    projMat.SetPerspective(60.0,(GLfloat)width/(GLfloat)height, 0.5f, 500.0f);
	projMat.SetPerspective(scene.cameraFov,(GLfloat)width/(GLfloat)height, scene.cameraNearPlane, scene.camearFarPlane);

	if (backgroundImage) {
		/// build vao
		const GLfloat vertices[] =
		{
			0.0, 0.0, 0.0, 0.0,
			(GLfloat)width, 0.0, 1.0, 0.0,
			0.0, (GLfloat)height, 0.0, 1.0,
			(GLfloat)width, (GLfloat)height, 1.0, 1.0,
		};

		if (vao != -1) GL_DEL_VERTEXARRAY(1, &vao);
		GL_GEN_VERTEXARRAY(1, &vao);
		GL_BIND_VERTEXARRAY(vao);
		// create, bind, and populate VBO
		glGenBuffers(1, &vao);
		glBindBuffer(GL_ARRAY_BUFFER, vao);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16, vertices, GL_STATIC_DRAW);

		// set up attrib pointers
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 4, (void*)8);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER,0);

		GL_BIND_VERTEXARRAY(0);
		CZCheckGLError();
	}

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

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

# ifdef _WIN32
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(scene.eyePosition.x,scene.eyePosition.y,scene.eyePosition.z, 0,0,0,0,1,0);
# endif

	/// blit background image
	//    glEnable(GL_BLEND);
	if (backgroundImage && !blitBackgroundImage()) return;

	glClear(GL_DEPTH_BUFFER_BIT);
	CZMat4 viewMat,viewProjMat;
	viewMat.SetLookAt(scene.eyePosition.x, scene.eyePosition.y, scene.eyePosition.z, 0, 0, 0, 0, 1, 0);
    viewProjMat = projMat * viewMat;
    
	CZShader *pShader = getShader(kDirectionalLightShading);

	if (pShader == NULL)
	{
		LOG_ERROR("there's no shader designated\n");
		return;
	}
	pShader->begin();
	CZCheckGLError();

	// common uniforms
	glUniform3f(pShader->getUniformLocation("ambientLight.intensities"),
		scene.ambientLight.intensity.x,
		scene.ambientLight.intensity.y,
		scene.ambientLight.intensity.z);

	glUniform3f(pShader->getUniformLocation("directionalLight.direction"),
		scene.directionalLight.direction.x,scene.directionalLight.direction.y,scene.directionalLight.direction.z);

	glUniform3f(pShader->getUniformLocation("eyePosition"),scene.eyePosition.x,scene.eyePosition.y,scene.eyePosition.z);

	glUniform3f(pShader->getUniformLocation("directionalLight.intensities"),
		scene.directionalLight.intensity.x,
		scene.directionalLight.intensity.y, 
		scene.directionalLight.intensity.z);
	CZCheckGLError();

    rootNode.draw(pShader,viewProjMat);
    CZCheckGLError();

	pShader->end();
#ifdef USE_OPENGL
	glColor3f(1.0,0.0,0.0);
	glPushMatrix();
	glTranslatef(scene.light.position.x, scene.light.position.y, scene.light.position.z);
	glDisable(GL_TEXTURE_2D);
	glutSolidSphere(2, 100, 100);
	glPopMatrix();
#endif

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
    
	/// color
	modelColor = scene.mColor;
	glClearColor(scene.bgColor.r, scene.bgColor.g, scene.bgColor.b, scene.bgColor.a);
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
	glClearColor(r, g, b, a);
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
	if (backgroundTexId != -1) glDeleteTextures(1, &backgroundTexId);

	backgroundImage = img;

	//generate an OpenGL texture ID for this texture
	glGenTextures(1, &backgroundTexId);
	//bind to the new texture ID
	glBindTexture(GL_TEXTURE_2D, backgroundTexId);
	//store the texture data for OpenGL use
	if (img->colorSpace == CZImage::RGBA) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)img->width , (GLsizei)img->height,
			0, GL_RGBA, GL_UNSIGNED_BYTE, img->data);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)img->width , (GLsizei)img->height,
			0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img->data);
	}

	//	gluBuild2DMipmaps(GL_TEXTURE_2D, components, width, height, texFormat, GL_UNSIGNED_BYTE, bits);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	CZCheckGLError();
}

void Application3D::setModelColor(float r, float g, float b, float a)
{
	modelColor = CZColor(r, g, b, a);
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

//////////////////////////////////////////////////////////////////////////
bool Application3D::loadShaders()
{
	//
	vector<string> attributes;
	attributes.push_back("vert");
	attributes.push_back("vertNormal");
	attributes.push_back("vertTexCoord");
	vector<string> uniforms;
	uniforms.push_back("mvpMat");
	uniforms.push_back("modelMat");
	uniforms.push_back("modelInverseTransposeMat");
	uniforms.push_back("ambientLight.intensities");
	uniforms.push_back("directionalLight.direction");
	uniforms.push_back("directionalLight.intensities");
	uniforms.push_back("eyePosition");
	uniforms.push_back("tex");
	uniforms.push_back("hasTex");
	uniforms.push_back("material.kd");
	uniforms.push_back("material.ka");
	uniforms.push_back("material.ke");
	uniforms.push_back("material.ks");
	uniforms.push_back("material.Ns");

	CZShader *pShader = new CZShader("standard","directionalLight",attributes,uniforms);
	shaders.insert(make_pair(kDirectionalLightShading,pShader));

	//
	attributes.clear();
	attributes.push_back("inPosition");
	attributes.push_back("inTexcoord");
	uniforms.clear();
	uniforms.push_back("mvpMat");
	uniforms.push_back("texture");
	uniforms.push_back("opacity");

	pShader = new CZShader("blit","blit",attributes,uniforms);
	shaders.insert(make_pair(kBlitImage,pShader));

    //
    attributes.clear();
    attributes.push_back("inPosition");
    uniforms.clear();
    uniforms.push_back("mvpMat");
    uniforms.push_back("inColor");
    
    pShader = new CZShader("blitColor","blitColor",attributes,uniforms);
    shaders.insert(make_pair(kBlitColor,pShader));

	CZCheckGLError();

	return true;
}

void Application3D::setGLSLDirectory(const char* glslDir)
{
    if (glslDir == NULL)
    {
        LOG_WARN("glslDir is NULL\n");
        return;
    }
    
    CZShader::glslDirectory = string(glslDir);
}

CZShader* Application3D::getShader(ShaderType type)
{
	ShaderMap::iterator itr = shaders.find(type);

	return itr != shaders.end() ?	itr->second : NULL;
}

void Application3D::parseLine(ifstream& ifs, const string& ele_id)
{
	float x,y,z;
	int r,g,b;
	if ("camera_position" == ele_id)
		parseEyePosition(ifs);
	else if ("camera_fov" == ele_id)
		parseCameraFov(ifs);
	else if ("camera_near_clip" == ele_id)
		parseCameraNearPlane(ifs);
	else if ("camera_far_clip" == ele_id)
		parseCameraFarPlane(ifs);

	else if ("pl" == ele_id)
		parsePointLight(ifs);

	else if ("dl" == ele_id)
		parseDirectionalLight(ifs);
	else if ("dl_direction" == ele_id)
	{
		ifs >> x >> y >> z;
		setLigthDirection(-x, -z, y);
	}
	else if ("dl_color" == ele_id)
	{
		ifs >> r >> g >> b;
		setDiffuseColor((unsigned char)r, (unsigned char)g, (unsigned char)b);
	}
	else if ("dl_intensity" == ele_id)
	{
		float intensity;
		ifs >> intensity;
		scene.directionalLight.intensity.x *= intensity;
		scene.directionalLight.intensity.y *= intensity;
		scene.directionalLight.intensity.z *= intensity;
	}

	else if ("al_color" == ele_id)
	{
		ifs >> r >> g >> b;
		setAmbientColor((unsigned char)r, (unsigned char)g, (unsigned char)b);
	}
	else if ("al_intensity" == ele_id)
	{
		float intensity;
		ifs >> intensity;
		scene.ambientLight.intensity.x *= intensity;
		scene.ambientLight.intensity.y *= intensity;
		scene.ambientLight.intensity.z *= intensity;
	}

	else if ("background_color" == ele_id)
		parseBackgroundColor(ifs);
	else if ("render_color" == ele_id)
		parseMainColor(ifs);
	else
		skipLine(ifs);
}

// \note
// the coordinate (x,y,z) should be converted to (x,z,-y), for the 3dMax is diffrent
void Application3D::parseEyePosition(ifstream& ifs)
{
	float x, y, z;
	ifs >> x >> y >> z;
	scene.eyePosition = CZPoint3D(x,z,-y);
}

void Application3D::parseCameraFov(ifstream& ifs)
{
	ifs >> scene.cameraFov;
}

void Application3D::parseCameraNearPlane(ifstream& ifs)
{
	ifs >> scene.cameraNearPlane;
}

void Application3D::parseCameraFarPlane(ifstream& ifs)
{
	ifs >> scene.camearFarPlane;
}

void Application3D::parsePointLight(ifstream& ifs)
{
	float intensity;
	ifs >> scene.light.position.x
		>> scene.light.position.y
		>> scene.light.position.z
		>> scene.light.intensity.x
		>> scene.light.intensity.y
		>> scene.light.intensity.z
		>> intensity;
	scene.light.intensity.x *= intensity;
	scene.light.intensity.y *= intensity;
	scene.light.intensity.z *= intensity;
}

void Application3D::parseDirectionalLight(ifstream& ifs)
{
	float intensity;
	ifs >> scene.directionalLight.direction.x
		>> scene.directionalLight.direction.y
		>> scene.directionalLight.direction.z
		>> scene.directionalLight.intensity.x
		>> scene.directionalLight.intensity.y
		>> scene.directionalLight.intensity.z
		>> intensity;
	scene.directionalLight.intensity.x *= intensity;
	scene.directionalLight.intensity.y *= intensity;
	scene.directionalLight.intensity.z *= intensity;
}

void Application3D::parseBackgroundColor(ifstream& ifs)
{
	ifs >> scene.bgColor.r
		>> scene.bgColor.g
		>> scene.bgColor.b
		>> scene.bgColor.a;
}

void Application3D::parseMainColor(ifstream& ifs)
{
	ifs >> scene.mColor.r
		>> scene.mColor.g
		>> scene.mColor.b
		>> scene.mColor.a;
}

bool Application3D::blitBackgroundImage()
{
	CZShader *pShader = getShader(kBlitImage);

	if (pShader == NULL)
	{
		LOG_ERROR("there's no shader for blitting background image\n");
		return false;
	}

	CZMat4 mvpMat;
	mvpMat.SetOrtho(0,width,0,height,-1.0f,1.0f);

	pShader->begin();

	glUniformMatrix4fv(pShader->getUniformLocation("mvpMat"),1,GL_FALSE,mvpMat);
	glUniform1i(pShader->getUniformLocation("texture"), (GLuint) 0);
	glUniform1f(pShader->getUniformLocation("opacity"), 1.0f); // fully opaque

	glActiveTexture(GL_TEXTURE0);
	// Bind the texture to be used
	glBindTexture(GL_TEXTURE_2D, backgroundTexId);

	// clear the buffer to get a transparent background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT );

	// set up premultiplied normal blend
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	GL_BIND_VERTEXARRAY(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	GL_BIND_VERTEXARRAY(0);

	pShader->end();

	CZCheckGLError();

	return true;
}

bool Application3D::enableTexture(CZImage* image)
{
    if(image == NULL || image->data == NULL)
    {
        LOG_WARN("image is illegal\n");
        return false;
    }
    
    map<CZImage*, short>::iterator itr = textureMap.find(image);
    
    short texInd;
    if(itr == textureMap.end())
    {
        if (itr == textureMap.begin()) texInd = 0;
        else    texInd = (--itr)->second + 1;
        
        if(texInd >= 128)
        {
            LOG_ERROR("texture resources exceed!\n");
            return false;
        }
        else
        {
            //generate an OpenGL texture ID for this texture
            glGenTextures(1, &textures[texInd]);
            //bind to the new texture ID
            glBindTexture(GL_TEXTURE_2D, textures[texInd]);
            //store the texture data for OpenGL use
            if (image->colorSpace == CZImage::RGBA) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)image->width , (GLsizei)image->height,
                             0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
            }
            else if (image->colorSpace == CZImage::RGB) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)image->width , (GLsizei)image->height,
                             0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
            }
            else if (image->colorSpace == CZImage::GRAY) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, (GLsizei)image->width , (GLsizei)image->height,
                             0, GL_LUMINANCE, GL_UNSIGNED_BYTE, image->data);
            }
            
            //	gluBuild2DMipmaps(GL_TEXTURE_2D, components, width, height, texFormat, GL_UNSIGNED_BYTE, bits);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            CZCheckGLError();
            textureMap[image] = texInd;
        }
    }
    else
    {
        texInd = itr->second;
    }
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[texInd]);
    //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#if !defined(__APPLE__)
    glEnable(GL_TEXTURE_2D);
#endif
    
    return true;
}