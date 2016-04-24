/* Author: Francisco Sandoval */

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

/*
 * MyMaterial - structure to keep material information. self explanatory
 */

struct MyMaterial{

	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float shininess;
	float opacity;
	int texCount;
};

/*
 * MyMesh - structure to keep mesh information
 * VBO - vertex buffer object array to store faces, vertices, normals, and texture coordinates.
 * texIndex - texture index for this mesh
 * matInfo - previous structure, keeps material info for this mesh
 * numFaces - number of faces in this mesh
 */
struct MyMesh{
	
	GLuint VBO[4];
	GLuint texIndex;
	MyMaterial matInfo;
	int numFaces;
};

/*
 * vboNames - enum for the 4 VBOs. Self descriptive
 */
enum vboNames { VBO_faces,
				VBO_vertices,
				VBO_normals,
				VBO_texCoords
				};

/*
 * Obj3d - class for parsing and loading a 3d mesh using assetImporter (assImp)
 */
class Obj3d{

	std::vector<struct MyMesh> myMeshes;
	std::map<std::string, GLuint> textureIdMap;	
	const struct aiScene *scene;
	GLuint modelMatrixLoc;
	GLuint vertexLoc;
	GLuint normalLoc;
	GLuint texCoordLoc;

	bool hasTextures;
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
