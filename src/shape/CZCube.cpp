//
//  CZCube.cpp
//  Application3D
//
//  Created by CharlyZhang on 16/6/29.
//  Copyright © 2016年 CharlyZhang. All rights reserved.
//

#include "CZCube.hpp"
#include "../basic/CZLog.h"

using namespace std;

unsigned char CZCube::indices[] = {0,1,2,3,
                                    0,2,4,6,
                                    2,3,6,7,
                                    0,1,4,5,
                                    4,5,6,7,
                                    1,3,5,7};

CZCube::CZCube()
{
}

CZCube::~CZCube()
{
}

void CZCube::create(CZPoint3D &origin, float width, float length, float height)
{
    /// create original data
    vector<CZPoint3D> positions,normals;
    for(int i = 0; i < 8; i ++)
    {
        int w = i & 1;
        int l = (i & 2) >> 1;
        int h = (i & 4) >> 2;
        
        // points' position and normal
        CZPoint3D offset(width*((float)w-0.5f),length*((float)l-0.5f),height*((float)h-0.5f));
        CZPoint3D p = origin + offset;
        positions.push_back(p);
    
        offset.normalize();
        normals.push_back(offset);
    }
    
    for(auto i = 0; i < 6; i++)
    {
        kd[i][0] = 1.0f * rand() / RAND_MAX;
        kd[i][1] = 1.0f * rand() / RAND_MAX;
        kd[i][2] = 1.0f * rand() / RAND_MAX;
        kd[i][3] = 1.0f;
    }
    /// transfer to graphic card
    // vao
    GL_GEN_VERTEXARRAY(1, &m_vao);
    GL_BIND_VERTEXARRAY(m_vao);
    
    // vertex
    glGenBuffers(1, &m_vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboPos);
    glBufferData(GL_ARRAY_BUFFER,positions.size() * 3 * sizeof(GLfloat), positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    CZCheckGLError();
    
    // normal
    glGenBuffers(1, &m_vboNorm);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboNorm);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * 3 * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    CZCheckGLError();
    
    GL_BIND_VERTEXARRAY(0);
    
    positions.clear();
    normals.clear();
    
}

bool CZCube::draw(CZShader *pShader, CZMat4 &viewProjMat)
{
    if(CZNode::draw(pShader, viewProjMat) != true) return false;
    
    CZMat4 modelMat = getTransformMat();
    glUniformMatrix4fv(pShader->getUniformLocation("mvpMat"), 1, GL_FALSE, viewProjMat * modelMat);
    glUniformMatrix4fv(pShader->getUniformLocation("modelMat"), 1, GL_FALSE, modelMat);
    glUniformMatrix4fv(pShader->getUniformLocation("modelInverseTransposeMat"), 1, GL_FALSE, modelMat.GetInverseTranspose());
    
    GL_BIND_VERTEXARRAY(m_vao);
    
    for (int i = 0; i < 6; i ++)
    {
        float ke[4], ka[4], ks[4], Ns = 10.0;
        ka[0] = 0.2;    ka[1] = 0.2;    ka[2] = 0.2;
        ke[0] = 0.0;    ke[1] = 0.0;    ke[2] = 0.0;
        ks[0] = 0.0;    ks[1] = 0.0;    ks[2] = 0.0;
        Ns = 10.0;
        
        glUniform3f(pShader->getUniformLocation("material.kd"), kd[i][0], kd[i][1], kd[i][2]);
        glUniform3f(pShader->getUniformLocation("material.ka"), ka[0], ka[1], ka[2]);
        glUniform3f(pShader->getUniformLocation("material.ke"), ke[0], ke[1], ke[2]);
        glUniform3f(pShader->getUniformLocation("material.ks"), ks[0], ks[1], ks[2]);
        glUniform1f(pShader->getUniformLocation("material.Ns"), Ns);
        glUniform1i(pShader->getUniformLocation("hasTex"), 0);
        glDrawElements(GL_TRIANGLE_STRIP, 4,  GL_UNSIGNED_BYTE, &indices[i*4]);
    }
    
    GL_BIND_VERTEXARRAY(0);
    CZCheckGLError();
    
    return true;
}