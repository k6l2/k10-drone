#include "ModelAsset.h"
#include "GfxVertexArray.h"
bool ModelAsset::load(string const& fileName, string const& gfxSubDir, 
					  Assimp::Importer& assImp)
{
	free();
	const unsigned assImpFlags = 
		aiProcess_Triangulate | 
		aiProcess_GenSmoothNormals | 
		aiProcess_FlipUVs | 
		aiProcess_JoinIdenticalVertices;
	aiScene const* const assImpScene = assImp.ReadFile(fileName, assImpFlags);
	if (!assImpScene)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Error parsing '%s' : '%s'\n", 
			fileName.c_str(), assImp.GetErrorString());
		return false;
	}
	if (!initFromScene(assImpScene, gfxSubDir))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to initialize model '%s' from aiScene.\n",
			fileName.c_str());
		return false;
	}
	return true;
}
void ModelAsset::free()
{
	for (Mesh& mesh : meshes)
	{
		mesh.vboVertices.destroy();
		mesh.eboVertexIndices.destroy();
	}
	meshes.clear();
	textureNames.clear();
}
bool ModelAsset::initFromScene(aiScene const* const assImpScene, 
							   string const& gfxSubDir)
{
	textureNames.resize(assImpScene->mNumMaterials);
	if (!initMaterials(assImpScene, gfxSubDir))
	{
		return false;
	}
	meshes.resize(assImpScene->mNumMeshes);
	for (size_t m = 0; m < assImpScene->mNumMeshes; m++)
	{
		aiMesh const* const assImpMesh = assImpScene->mMeshes[m];
		initMesh(m, assImpMesh);
	}
	return true;
}
bool ModelAsset::initMaterials(aiScene const* const assImpScene, 
							   string const& gfxSubDir)
{
	for (size_t m = 0; m < assImpScene->mNumMaterials; m++)
	{
		aiMaterial const* const assImpMat = assImpScene->mMaterials[m];
		if (assImpMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString aiStrTexturePath;
			if (assImpMat->GetTexture(aiTextureType_DIFFUSE, 0, 
									  &aiStrTexturePath, nullptr, nullptr, 
									  nullptr, nullptr, nullptr) == AI_SUCCESS)
			{
				textureNames[m] = gfxSubDir + aiStrTexturePath.data;
			}
		}
		if (textureNames[m].empty())
		{
			textureNames[m] = "ERROR";
		}
	}
	return true;
}
bool ModelAsset::initMesh(size_t meshIndex, aiMesh const* const assImpMesh)
{
	// Gather mesh vertex data from the mesh asset //
	static const aiVector3D ZERO_3D(0.f, 0.f, 0.f);
	static const aiColor4D ASSIMP_WHITE(1.f, 1.f, 1.f, 1.f);
	vector<Mesh::Vertex> vertices(assImpMesh->mNumVertices);
	vector<GLuint> indices(assImpMesh->mNumFaces * 3);
	for (size_t v = 0; v < assImpMesh->mNumVertices; v++)
	{
		aiVector3D const* const pos      = &(assImpMesh->mVertices[v]);
		aiVector3D const* const norm     = &(assImpMesh->mNormals[v]);
		aiVector3D const* const texCoord = assImpMesh->HasTextureCoords(0) ?
			&(assImpMesh->mTextureCoords[0][v]) : &ZERO_3D;
		aiColor4D const* const color     = assImpMesh->HasVertexColors(0) ?
			&(assImpMesh->mColors[0][v]) : &ASSIMP_WHITE;
		vertices[v] = {
			v3f(pos->x , pos->y , pos->z ), 
			v3f(norm->x, norm->y, norm->z), 
			v2f(texCoord->x, texCoord->y), 
			Color(color->r, color->g, color->b, color->a) };
	}
	for (size_t f = 0; f < assImpMesh->mNumFaces; f++)
	{
		aiFace const& assImpFace = assImpMesh->mFaces[f];
		SDL_assert(assImpFace.mNumIndices == 3);
		indices[f * 3 + 0] = assImpFace.mIndices[0];
		indices[f * 3 + 1] = assImpFace.mIndices[1];
		indices[f * 3 + 2] = assImpFace.mIndices[2];
	}
	// Upload the mesh data into the GPU //
	meshes[meshIndex].materialIndex = assImpMesh->mMaterialIndex;
	if (!meshes[meshIndex].vboVertices.create(
									GfxBuffer::BufferTarget::VERTEX_ATTRIBUTES,
									GfxBuffer::MemoryUsage::STATIC, 
									vertices.size(), sizeof(Mesh::Vertex)))
	{
		return false;
	}
	if (!meshes[meshIndex].vboVertices.upload(0, vertices.size(), 
											  vertices.data()))
	{
		return false;
	}
	if (!meshes[meshIndex].eboVertexIndices.create(
									GfxBuffer::BufferTarget::VERTEX_INDICES,
									GfxBuffer::MemoryUsage::STATIC,
									indices.size(), sizeof(GLuint)))
	{
		return false;
	}
	if (!meshes[meshIndex].eboVertexIndices.upload(0, indices.size(),
												   indices.data()))
	{
		return false;
	}
	meshes[meshIndex].vertexCount = vertices.size();
	meshes[meshIndex].indexCount  = indices.size();
	return true;
}
bool ModelAsset::render() const
{
	GfxVertexArray::use(&k10::vaoMesh);
	for (Mesh const& mesh : meshes)
	{
		k10::vaoMesh.bindVertexBuffer(mesh.vboVertices, 0, 0);
		k10::vaoMesh.bindElementBuffer(mesh.eboVertexIndices);
		Texture const* tex = nullptr;
		if (mesh.materialIndex >= 0 &&
			mesh.materialIndex < textureNames.size())
		{
			tex = k10::assetDb.getTexture(
				textureNames[mesh.materialIndex].c_str());
		}
		if (tex)
		{
			tex->bind(0);
		}
		glDrawElements(GL_TRIANGLES, 
					   static_cast<GLsizei>(mesh.eboVertexIndices.getNumElements()), 
					   GL_UNSIGNED_INT, 0);
		if (glCheckError() != GL_NO_ERROR)
		{
			SDL_assert(false);
			return false;
		}
	}
	return true;
}