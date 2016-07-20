
#ifndef _CZDEFINE_H_
#define _CZDEFINE_H_

//
//#include "CZFbo.h"
//#include "CZShader.h"
//#include "CZTexture.h"

#define DEFAULT_GLSL_DIR "../../src/glsl/"
#ifndef _DEBUG
	#define _DEBUG
#endif

#define NAMESPACE CZ3D

#include "CZLog.h"

namespace CZ3D {
 
extern void CZCheckGLError_(const char *file, int line);

extern void modelLoadingDone();

}

#ifdef _DEBUG
#define CZCheckGLError()	CZ3D::CZCheckGLError_(__FILE__, __LINE__)
#else
#define CZCheckGLError()
#endif

#endif