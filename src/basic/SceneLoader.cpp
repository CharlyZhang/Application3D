//
//  SceneLoader.cpp
//  SceneLoader
//
//  Created by CharlyZhang on 16/7/25.
//  Copyright © 2016年 CharlyZhang. All rights reserved.
//

#include "SceneLoader.hpp"

using namespace std;

namespace CZ3D {
bool SceneLoader::load(CZScene *pScene, const char *path)
{
    if(pScene == nullptr)
    {
        LOG_ERROR("pScene is nullptr!\n");
        return false;
    }
    
    pCurScene = pScene;
    if(!parseFile(path)) return false;
    return true;
}
    
void SceneLoader::parseLine(ifstream& ifs, const string& ele_id)
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
        pCurScene->directionalLight.direction = CZVector3D<float>(-x,-z,y);
    }
    else if ("dl_color" == ele_id)
    {
        ifs >> r >> g >> b;
        pCurScene->directionalLight.intensity = CZVector3D<float>(r/255.0f,g/255.0f,b/255.0f);
    }
    else if ("dl_intensity" == ele_id)
    {
        float intensity;
        ifs >> intensity;
        pCurScene->directionalLight.intensity.x *= intensity;
        pCurScene->directionalLight.intensity.y *= intensity;
        pCurScene->directionalLight.intensity.z *= intensity;
    }
    
    else if ("al_color" == ele_id)
    {
        ifs >> r >> g >> b;
        pCurScene->ambientLight.intensity = CZVector3D<float>(r/255.0f,g/255.0f,b/255.0f);
    }
    else if ("al_intensity" == ele_id)
    {
        float intensity;
        ifs >> intensity;
        pCurScene->ambientLight.intensity.x *= intensity;
        pCurScene->ambientLight.intensity.y *= intensity;
        pCurScene->ambientLight.intensity.z *= intensity;
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
void SceneLoader::parseEyePosition(ifstream& ifs)
{
    float x, y, z;
    ifs >> x >> y >> z;
    pCurScene->eyePosition = CZPoint3D(x,z,-y);
}

void SceneLoader::parseCameraFov(ifstream& ifs)
{
    ifs >> pCurScene->cameraFov;
}

void SceneLoader::parseCameraNearPlane(ifstream& ifs)
{
    ifs >> pCurScene->cameraNearPlane;
}

void SceneLoader::parseCameraFarPlane(ifstream& ifs)
{
    ifs >> pCurScene->camearFarPlane;
}

void SceneLoader::parsePointLight(ifstream& ifs)
{
    float intensity;
    ifs >> pCurScene->light.position.x
    >> pCurScene->light.position.y
    >> pCurScene->light.position.z
    >> pCurScene->light.intensity.x
    >> pCurScene->light.intensity.y
    >> pCurScene->light.intensity.z
    >> intensity;
    pCurScene->light.intensity.x *= intensity;
    pCurScene->light.intensity.y *= intensity;
    pCurScene->light.intensity.z *= intensity;
}

void SceneLoader::parseDirectionalLight(ifstream& ifs)
{
    float intensity;
    ifs >> pCurScene->directionalLight.direction.x
    >> pCurScene->directionalLight.direction.y
    >> pCurScene->directionalLight.direction.z
    >> pCurScene->directionalLight.intensity.x
    >> pCurScene->directionalLight.intensity.y
    >> pCurScene->directionalLight.intensity.z
    >> intensity;
    pCurScene->directionalLight.intensity.x *= intensity;
    pCurScene->directionalLight.intensity.y *= intensity;
    pCurScene->directionalLight.intensity.z *= intensity;
}

void SceneLoader::parseBackgroundColor(ifstream& ifs)
{
    ifs >> pCurScene->bgColor.r
    >> pCurScene->bgColor.g
    >> pCurScene->bgColor.b
    >> pCurScene->bgColor.a;
}

void SceneLoader::parseMainColor(ifstream& ifs)
{
    ifs >> pCurScene->mColor.r
    >> pCurScene->mColor.g
    >> pCurScene->mColor.b
    >> pCurScene->mColor.a;
}
    
}
