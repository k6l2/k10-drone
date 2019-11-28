#include "AssetDatabase.h"
#include "Image.h"
const string AssetDatabase::PATH_GFX = "gfx";
const string AssetDatabase::PATH_SHADER_BIN = "shader-bins";
bool AssetDatabase::load()
{
//	if (!loadTexture("marble.jpg")) return false;
//	if (!loadTexture("cockatrice.jpg")) return false;
	if (!loadTexture("droid-fighter/DiffuseTexture.png")) return false;
	if (!loadTexture("droid-fighter/IlluminationTexture.png")) return false;
	if (!loadTexture("droid-fighter/SpecularTexture.png")) return false;
	if (!loadModel("droid-fighter/droid-fighter.fbx")) return false;
//	if (!loadTexture("Meteor/Meteor_d.tga")) return false;
//	if (!loadTexture("Meteor/Meteor_g.tga")) return false;
//	if (!loadTexture("Meteor/Meteor_n.tga")) return false;
//	if (!loadTexture("Meteor/Meteor_s.tga")) return false;
//	if (!loadModel("Meteor/Meteor.fbx")) return false;
//	if (!loadModel("sphere-1m-radius/sphere-1m-radius.fbx")) return false;
//	if (!loadModel("cube-1m-side/cube-1m-side.fbx")) return false;
//	{
//		char const* filePaths[6] = {
//			"spacescape-exports/green-nebula-starsRT.png",
//			"spacescape-exports/green-nebula-starsLF.png",
//			"spacescape-exports/green-nebula-starsBK.png",
//			"spacescape-exports/green-nebula-starsFT.png",
//			"spacescape-exports/green-nebula-starsUP.png",
//			"spacescape-exports/green-nebula-starsDN.png",
//		};
//		if (!loadTextureCubemap("green-nebula-stars", filePaths))
//		{
//			return false;
//		}
//	}
	if (!loadGfxProgram("simple-draw-tri",
						"simple-draw-tri-vert.spv",
						"simple-draw-tri-frag.spv"))
	{
		return false;
	}
	if (!loadGfxProgram("simple-draw",
						"simple-draw-vert.spv",
						"simple-draw-frag.spv"))
	{
		return false;
	}
	if (!loadGfxProgram("simple-draw-textured",
						"simple-draw-textured-vert.spv",
						"simple-draw-textured-frag.spv"))
	{
		return false;
	}
	if (!loadGfxProgram("skybox",
						"skybox-vert.spv",
						"skybox-frag.spv"))
	{
		return false;
	}
	if (!loadGfxProgram("bullet-debug",
						"bullet-debug-vert.spv",
						"simple-draw-frag.spv"))
	{
		return false;
	}
	return true;
}
Texture const* AssetDatabase::getTexture(char const* filePath) const
{
	auto it = textures.find(filePath);
	if (it == textures.end())
	{
		return nullptr;
	}
	return &it->second;
}
TextureCubemap const* AssetDatabase::getCubemap(char const* name) const
{
	auto it = cubemaps.find(name);
	if (it == cubemaps.end())
	{
		return nullptr;
	}
	return &it->second;
}
GfxProgram const* AssetDatabase::getGfxProgram(char const* name) const
{
	auto it = gfxPrograms.find(name);
	if (it == gfxPrograms.end())
	{
		return nullptr;
	}
	return &it->second;
}
ModelAsset const* AssetDatabase::getModel(char const* filePath) const
{
	auto it = models.find(filePath);
	if (it == models.end())
	{
		return nullptr;
	}
	return &it->second;
}
bool AssetDatabase::loadTexture(char const* filePath)
{
	const string applicationFilePath = std::filesystem::path(
		k10::applicationBasePath + PATH_GFX + "/" + filePath)
			.make_preferred().string();
	Image img;
	if (!img.load(applicationFilePath))
	{
		return false;
	}
	Texture newTex;
	if (!newTex.createFromImage(img))
	{
		newTex.free();
		return false;
	}
	textures.insert({ filePath, newTex });
	return true;
}
bool AssetDatabase::loadTextureCubemap(char const* name, char const* filePath[6])
{
	// these image postprocessing operations are needed because the format of
	//	the default cubemap exporter settings in starscape have weird transforms
	static const u8 ROT_CCW_90_DEG_SIDES = 0b000011;
	static const u8 H_FLIP_SIDES         = 0b100110;
	static const u8 V_FLIP_SIDES         = 0b100110;
	Image imgs[6];
	for (size_t i = 0; i < 6; i++)
	{

		const string applicationFilePath = std::filesystem::path(
			k10::applicationBasePath + PATH_GFX + "/" + filePath[i])
				.make_preferred().string();
		if (V_FLIP_SIDES & (1 << i))
		{
			stbi_set_flip_vertically_on_load(true);
		}
		if(!imgs[i].load(applicationFilePath))
		{
			return false;
		}
		if (H_FLIP_SIDES & (1 << i))
		{
			imgs[i].flipHorizontally();
		}
		if (ROT_CCW_90_DEG_SIDES & (1 << i))
		{
			imgs[i].rotate90DegreesCW();
		}
		stbi_set_flip_vertically_on_load(false);
	}
	TextureCubemap newCubemap;
	if (!newCubemap.createFromImages(imgs))
	{
		return false;
	}
	cubemaps.insert({ name, newCubemap });
	return true;
}
bool AssetDatabase::loadGfxProgram(char const* programName,
								   char const* filePathVertexShader,
								   char const* filePathFragmentShader)
{
	const string appFilePathVertShader = std::filesystem::path(
		k10::applicationBasePath + PATH_SHADER_BIN + "/" + filePathVertexShader)
		.make_preferred().string();
	const string appFilePathFragShader = std::filesystem::path(
		k10::applicationBasePath + PATH_SHADER_BIN + "/" + filePathFragmentShader)
		.make_preferred().string();
	GfxProgram newProg;
	if (!newProg.load(appFilePathVertShader, appFilePathFragShader))
	{
		return false;
	}
	gfxPrograms.insert({ programName, newProg });
	return true;
}
bool AssetDatabase::loadModel(char const* filePath)
{
	const string applicationFilePath = std::filesystem::path(
		k10::applicationBasePath + PATH_GFX + "/" + filePath)
			.make_preferred().string();
	const size_t dirSeparatorIndex = string(filePath).find_last_of('/');
	const string gfxSubDir = dirSeparatorIndex == string::npos ?
		"" : string(filePath).substr(0, dirSeparatorIndex) + "/";
	ModelAsset newModel;
	if (!newModel.load(applicationFilePath, gfxSubDir, assImp))
	{
		return false;
	}
	models.insert({ filePath, newModel });
	return true;
}