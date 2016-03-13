/*
 *Copyright (c) 2012 Freescale Semiconductor, Inc.
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without modification,
 *are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this 
 *list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this 
 *list of conditions and the following disclaimer in the documentation and/or other
 *materials provided with the distribution.
 *
 * Neither the name of the Freescale Semiconductor, Inc. nor the names of its 
 *contributors may be used to endorse or promote products derived from this software
 *without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 *ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 *IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 *INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *POSSIBILITY OF SUCH DAMAGE.
 */
// assimp include files. These three are usually needed.
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <IL/il.h>

#include <vector>
#include <map>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include <stdio.h>
#include "gUtil.h"

// Information to render each assimp node

struct MyMaterial{

	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float shininess;
	float opacity;
	int texCount;
};

struct MyMesh{
	
	GLuint VBO[4]; //to store faces, vertices, normals and texcoords
	GLuint texIndex;
	MyMaterial matInfo;
	int numFaces;
};

enum vboNames { VBO_faces,
				VBO_vertices,
				VBO_normals,
				VBO_texCoords
				};
class Obj3d{

	std::vector<struct MyMesh> myMeshes;
	std::map<std::string, GLuint> textureIdMap;	
	const struct aiScene *scene;
	GLuint modelMatrixLoc;
	GLuint vertexLoc;
	GLuint normalLoc;
	GLuint texCoordLoc;

	bool hasTextures; //the model
	float matRot[16];
	bool hasNonOpaque;
	
	GLuint diffuseLoc;
	GLuint ambientLoc;
	GLuint specularLoc;
	GLuint emissiveLoc;
	GLuint shininessLoc;
	GLuint opacityLoc;
	GLuint texCountLoc;
	GLuint hasTexLoc;

	GLuint texUnitLoc;
	GLuint cubeHandle;

	GLuint shaderProg;

	void loadAsset(const char * path, Obj3d &asset);

	public:

	Obj3d(bool HasTx);
	void start(GLuint shaderPrg, const char * path, Obj3d &obj);

	const struct aiScene * getScene(void){return scene;};
	float * getRotMat(void){return matRot;};
	GLuint getCubeHandle(void){return cubeHandle;};
	GLuint getShaderProgram(void){return shaderProg;};
	void setCubeHandle(GLuint handle){cubeHandle = handle;};
	void set_float4(float f[4], float a, float b, float c, float d);

	void color4_to_float4(const aiColor4D *c, float f[4]);

	void recursive_render (float * currentTransform, const struct aiNode* nd, Obj3d &asset, bool opaque);
	int LoadGLTextures(const aiScene* scene, Obj3d &asset);
	void draw(float * matMV, float * matP, GLuint matMVLoc, GLuint matPLoc);

};
