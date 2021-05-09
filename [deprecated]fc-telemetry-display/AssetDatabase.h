#pragma once
#include "GfxProgram.h"
#include "Texture.h"
#include "TextureCubemap.h"
#include "ModelAsset.h"
class AssetDatabase
{
public:
	bool load();
	Texture const* getTexture(char const* filePath) const;
	TextureCubemap const* getCubemap(char const* name) const;
	GfxProgram const* getGfxProgram(char const* name) const;
	ModelAsset const* getModel(char const* filePath) const;
private:
	static const string PATH_GFX;
	static const string PATH_SHADER_BIN;
private:
	bool loadTexture(char const* filePath);
	bool loadTextureCubemap(char const* name, char const* filePath[6]);
	bool loadGfxProgram(char const* programName,
						char const* filePathVertexShader,
						char const* filePathFragmentShader);
	bool loadModel(char const* filePath);
private:
	map<string, Texture> textures;
	// string key refers to a special name, not fileName, because it is 
	//	possible for a cubemap to be composed of many files.
	map<string, TextureCubemap> cubemaps;
	// string key here refers to a special designator name for the program,
	//	not a file name, since programs are composed of several shader files.
	map<string, GfxProgram> gfxPrograms;
	map<string, ModelAsset> models;
	Assimp::Importer assImp;
};
