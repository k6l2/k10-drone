#pragma once
#include "GfxBuffer.h"
#include "Color.h"
class ModelAsset
{
public:
	struct Mesh
	{
		struct Vertex
		{
			v3f position;
			v3f normal;
			v2f texCoord;
			Color color;
		};
		GfxBuffer vboVertices;
		GfxBuffer eboVertexIndices;
		size_t vertexCount;
		size_t indexCount;
		unsigned materialIndex;
	};
public:
	bool load(string const& fileName, string const& gfxSubDir, 
			  Assimp::Importer& assImp);
	void free();
	bool render() const;
private:
	bool initFromScene(aiScene const* const assImpScene, string const& gfxSubDir);
	bool initMaterials(aiScene const* const assImpScene, string const& gfxSubDir);
	bool initMesh(size_t meshIndex, aiMesh const* const assImpMesh);
private:
	vector<Mesh> meshes;
	vector<string> textureNames;
};
