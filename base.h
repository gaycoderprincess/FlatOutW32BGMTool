// research:
// vertex buffer data:
// 1 2 3 always coords
// 0x202 flag seems to be the terrain
// 0x152 -> 4 5 6 are normals, 7 unknown, 8 and 9 are UV coords
// 0x212 -> 4 5 6 are normals, 7 8 9 10 unknown

// gulbroz stuff:
// Vertex stream flags
#define VERTEX_POSITION			0x2
#define VERTEX_UV				0x100
#define VERTEX_UV2				0x200
#define VERTEX_NORMAL			0x10
#define VERTEX_COLOR			0x40
#define VERTEX_INT16			0x2000
#define STREAM_VERTEX_DECAL		(VERTEX_POSITION | VERTEX_UV)
#define STREAM_VERTEX_MODEL		(VERTEX_POSITION | VERTEX_UV  | VERTEX_NORMAL)
#define STREAM_VERTEX_STATIC	(VERTEX_POSITION | VERTEX_UV  | VERTEX_COLOR)
#define STREAM_VERTEX_WINDOW	(VERTEX_POSITION | VERTEX_UV  | VERTEX_NORMAL | VERTEX_COLOR)
#define STREAM_VERTEX_TERRAIN	(VERTEX_POSITION | VERTEX_UV2 | VERTEX_NORMAL)
#define STREAM_VERTEX_TERRAIN2	(VERTEX_POSITION | VERTEX_UV2)

std::filesystem::path sFileName;
std::filesystem::path sFileNameNoExt;
std::filesystem::path sFileFolder;
std::filesystem::path sFBXFileName;

bool bIsBGMModel = false;
bool bIsFOUCModel = false;
bool bIsXboxBetaModel = false;
const double fFOUCBGMScaleMultiplier = 0.000977; // the game seems hardcoded to use this multiplier for BGMs
enum {
	LOG_ALL,
	LOG_MINOR_WARNINGS,
	LOG_WARNINGS,
	LOG_ERRORS,
	LOG_ALWAYS = 255,
} nLoggingSeverity = LOG_ALL;

void WriteConsole(const std::string& str, int logType) {
	if (logType < nLoggingSeverity) return;

	auto& out = std::cout;
	out << str;
	out << "\n";
	out.flush();
}

void WriteFile(const std::string& str) {
	static auto out = std::ofstream(sFileNameNoExt.string() + "_log.txt");
	out << str;
	out << "\n";
	out.flush();
}

std::string GetFileVersion(int value) {
	if (value == 0x10003) return "Retro Demo / Tough Trucks Track (Export Only)";
	if (value == 0x43524143) return "Retro Demo / Tough Trucks Car (Unsupported)";
	if (value == 0x10004) return "FlatOut 1 Car";
	if (value == 0x10005) return "FlatOut 1 Track";
	if (value == 0x20000) return "FlatOut 2 / Ultimate Carnage Car";
	if (value == 0x20001) return "FlatOut 2 Track";
	if (value == 0x20002) return "FlatOut Ultimate Carnage Track";
	if (value >= 0x20000) return "Unknown FlatOut 2";
	return "Unknown FlatOut 1";
}

int nImportFileVersion;
int nExportFileVersion;
std::string GetShaderName(int value) {
	if (nImportFileVersion <= 0x10003) {
		switch (value) {
			case 0: return "default static";
			case 1: return "default dynamic";
			case 2: return "lightmapped";
			case 3: return "car body";
			case 4: return "car window";
			case 5: return "rendertarget shadow";
			case 6: return "sunmap 1";
			case 7: return "sunmap 2";
			case 8: return "sunmap 3";
			case 9: return "sunmap track 1";
			case 10: return "sunmap track 2";
			case 11: return "intensity map";
			case 12: return "sunflare";
			case 13: return "default static";
			default: return "UNKNOWN";
		}
	}
	else if (bIsFOUCModel) {
		switch (value) {
			case 0: return "static prelit";
			case 1: return "terrain";
			case 2: return "terrain specular";
			case 3: return "dynamic diffuse";
			case 4: return "dynamic specular";
			case 5: return "car body";
			case 6: return "car window";
			case 7: return "car diffuse";
			case 8: return "car metal";
			case 9: return "car tire rim";
			case 10: return "car lights";
			case 11: return "car shear";
			case 12: return "car scale";
			case 13: return "shadow project";
			case 14: return "car lights unlit";
			case 15: return "default";
			case 16: return "vertex color";
			case 17: return "shadow sampler";
			case 18: return "grass";
			case 19: return "tree trunk";
			case 20: return "tree branch";
			case 21: return "tree leaf";
			case 22: return "particle";
			case 23: return "sunflare";
			case 24: return "intensitymap";
			case 25: return "water";
			case 26: return "skinning";
			case 27: return "tree lod (default)";
			case 28: return "@deprecated: streak shader on PS2";
			case 29: return "clouds (uvscroll)";
			case 30: return "car bodylod";
			case 31: return "@deprecated: vertex color static (now used as depth buffer visualization shader)";
			case 32: return "car window damaged";
			case 33: return "skin shadow(deprecated)";
			case 34: return "reflecting window shader (static)";
			case 35: return "reflecting window shader (dynamic)";
			case 36: return "@deprecated: old STATIC_SPECULAR, same as #35 - STATIC_WINDOW";
			case 37: return "skybox";
			case 38: return "horizon";
			case 39: return "ghost body";
			case 40: return "static nonlit";
			case 41: return "dynamic nonlit";
			case 42: return "skid marks";
			case 43: return "car interior";
			case 44: return "car tire";
			case 45: return "puddle";
			case 46: return "ambient shadow";
			case 47: return "Local water shader";
			case 48: return "Static specular/hilight shader (SHADER_STATIC_HILIGHT)";
			case 49: return "Lightmapped planar reflection";
			case 50: return "racemap";
			case 51: return "HDR default shader (runtime)";
			case 52: return "Ambient particle shader";
			case 53: return "Videoscreen shader (dynamic)";
			case 54: return "Videoscreen shader (static)";
		}
	}
	else {
		switch (value) {
			case 0: return "static prelit";
			case 1: return "terrain";
			case 2: return "terrain specular";
			case 3: return "dynamic diffuse";
			case 4: return "dynamic specular";
			case 5: return "car body";
			case 6: return "car window";
			case 7: return "car diffuse";
			case 8: return "car metal";
			case 9: return "car tire";
			case 10: return "car lights";
			case 11: return "car shear";
			case 12: return "car scale";
			case 13: return "shadow project";
			case 14: return "car lights unlit";
			case 15: return "default";
			case 16: return "vertex color";
			case 17: return "shadow sampler";
			case 18: return "grass";
			case 19: return "tree trunk";
			case 20: return "tree branch";
			case 21: return "tree leaf";
			case 22: return "particle";
			case 23: return "sunflare";
			case 24: return "intensitymap";
			case 25: return "water";
			case 26: return "skinning";
			case 27: return "tree lod (default)";
			case 28: return "DUMMY (streak shader on PS2)";
			case 29: return "clouds (uvscroll)";
			case 30: return "car bodylod";
			case 31: return "vertex color static (dummy? same as vertexcolor)";
			case 32: return "car window damaged";
			case 33: return "skin shadow";
			case 34: return "reflecting window shader (static)";
			case 35: return "reflecting window shader (dynamic)";
			case 36: return "@deprecated: old STATIC_SPECULAR, same as #35 - STATIC_WINDOW";
			case 37: return "skybox";
			case 38: return "ghost body";
			case 39: return "static nonlit";
			case 40: return "dynamic nonlit";
			case 41: return "racemap";
			default: return "UNKNOWN";
		}
	}
	return "UNKNOWN";
}

std::string GetFBXTextureInFO2Style(aiMaterial* material, int id) {
	aiString tmp;
	material->GetTexture(aiTextureType_DIFFUSE, id, &tmp);
	std::string str = tmp.C_Str();
	while (str.find('\\') != std::string::npos) {
		str.erase(str.begin());
	}
	while (str.find('/') != std::string::npos) {
		str.erase(str.begin());
	}
	if (str.find('.') == std::string::npos) {
		str += ".tga";
	}
	else if (str.length() > 3) {
		for (int j = 0; j < 3; j++) {
			str.pop_back();
		}
		str += "tga";
	}
	return str;
}

std::string ReadStringFromFile(std::ifstream& file) {
	std::string string;
	char value = 0;
	do {
		file.read(&value, 1);
		if (value) string.push_back(value);
	} while (value);
	return string;
}

void ReadFromFile(std::ifstream& file, void* out, size_t numBytes) {
	file.read((char*)out, numBytes);
}

enum eSurfaceReference {
	SURFACE_REFERENCE_STATICBATCH,
	SURFACE_REFERENCE_MODEL,
	//SURFACE_REFERENCE_TREEMESH_2,
	SURFACE_REFERENCE_TREEMESH_3,
	SURFACE_REFERENCE_TREEMESH_4,
	SURFACE_REFERENCE_TREEMESH_5,
	NUM_SURFACE_REFERENCE_TYPES
};

struct tVertexBuffer {
	bool isVegetation = false;
	int id;
	int foucExtraFormat = 0;
	uint32_t vertexCount;
	uint32_t vertexSize;
	uint32_t flags;
	float* data;
	float* origDataForFOUCExport = nullptr;

	uint32_t _vertexSizeBeforeFO1 = 0;
	uint32_t _vertexCountForFOUC = 0;
	uint32_t _vertexSizeForFOUC = 0;
	std::vector<float> _coordsAfterFOUCMult;
};
struct tIndexBuffer {
	int id;
	int foucExtraFormat = 0;
	uint32_t indexCount;
	uint16_t* data;
};
struct tVertexDataFOUC {
	int16_t vPos[3];
	uint16_t nUnk32;
	uint8_t vUnknownProllyBumpmaps[4]; // ends in FF
	uint8_t vUnknownProllyBumpmaps2[4]; // ends in FF
	uint8_t vNormals[4]; // ends in FF
	uint8_t vVertexColors[4];
	uint16_t vUV1[2];
	uint16_t vUV2[2];
};
struct tVertexDataFOUC24 {
	int16_t vPos[3];
	uint16_t nUnk32;
	uint8_t vUnknownProllyBumpmaps[4]; // ends in FF
	uint8_t vUnknownProllyBumpmaps2[4]; // ends in FF
	uint8_t vNormals[4]; // ends in FF
	uint16_t vUV1[2];
};
struct tCrashDataWeights {
	float vBasePos[3];
	float vCrashPos[3];
	float vBaseNormal[3];
	float vCrashNormal[3];
};
struct tCrashDataWeightsFOUC {
	int16_t vBasePos[3];
	int16_t vCrashPos[3];
	uint8_t vBaseUnknownProllyBumpmaps[4]; // ends in FF
	uint8_t vCrashUnknownProllyBumpmaps[4]; // ends in FF
	uint8_t vBaseUnknownProllyBumpmaps2[4]; // ends in FF
	uint8_t vCrashUnknownProllyBumpmaps2[4]; // ends in FF
	uint8_t vBaseNormals[4]; // ends in FF
	uint8_t vCrashNormals[4]; // ends in FF
	uint16_t vBaseUVs[2]; // doesn't have a crash equivalent
};
struct tCrashDataSurface {
	tVertexBuffer vBuffer;
	std::vector<tCrashDataWeights> aCrashWeights;
	std::vector<tCrashDataWeightsFOUC> aCrashWeightsFOUC;
};
struct tCrashData {
	std::string sName;
	std::vector<tCrashDataSurface> aSurfaces;
};
struct tMaterial {
	uint32_t identifier = 0x4354414D; // MATC
	std::string sName;
	int nAlpha = 0;
	int v92 = 0;
	int nNumTextures = 0;
	int nShaderId = 0;
	int nUseColormap = 0;
	int v74 = 0;
	int v108[3];
	int v109[3];
	int v98[4];
	int v99[4];
	int v100[4];
	int v101[4];
	int v102 = 0;
	std::string sTextureNames[3];

	int _nNumReferences = 0;
	bool _bIsCustom = false;
	bool _bIsCustomFOUCTree = false;
	tMaterial() {
		memset(v108, 0, sizeof(v108));
		memset(v109, 0, sizeof(v109));
		memset(v98, 0, sizeof(v98));
		memset(v99, 0, sizeof(v99));
		memset(v100, 0, sizeof(v100));
		memset(v101, 0, sizeof(v101));
	}
};
struct tSurface {
	int nIsVegetation = 0;
	int nMaterialId;
	int nVertexCount;
	int nFlags;
	int nPolyCount;
	int nPolyMode;
	int nNumIndicesUsed;
	float vCenter[3] = { 0, 0, 0 };
	float vRadius[3] = { 0, 0, 0 };
	float foucVertexMultiplier[4];
	int nNumStreamsUsed;
	uint32_t nStreamId[2];
	uint32_t nStreamOffset[2];

	tCrashDataSurface* _pCrashDataSurface = nullptr;
	bool _bIsReplacedMapSurface = false;
	aiMesh* _pReplacedMapSurfaceMesh = nullptr;
	int _nFBXModelId = -1;
	int _nFBXCrashModelId = -1;
	int _nNumReferencesByType[NUM_SURFACE_REFERENCE_TYPES] = {};
	int _nNumReferences = 0;
	void RegisterReference(int type) {
		_nNumReferences++;
		_nNumReferencesByType[type]++;
	}
};
struct tStaticBatch {
	uint32_t nId1;
	uint32_t nBVHId1; // surface id
	uint32_t nBVHId2; // second id in bvh array 1
	uint32_t nUnk = 0; // seems to always be 0
	float vCenter[3] = { 0, 0, 0 };
	float vRadius[3] = { 0, 0, 0 };
};
struct tTreeLOD {
	float vPos[3];
	float fScale[2];
	uint32_t nValues[2];
};
struct tTreeMesh {
	int nIsBush;

	// these 2 values are unused and completely unread
	int nUnk2Unused;
	int nBVHId1; // first id in bvh array 1, leaf surface id
	int nBVHId2; // second id in bvh array 1
	float mMatrix[4*4];
	float fScale[3];
	int nTrunkSurfaceId = -1;
	int nBranchSurfaceId = -1;
	int nLeafSurfaceId = -1;
	int nColorId;
	int nLodId;
	int nMaterialId;

	int foucExtraData1[9];
	int foucExtraData2[9];
	int foucExtraData3[4];
	int foucExtraData4[3];
};
struct tModel {
	uint32_t identifier = 0x444F4D42; // BMOD
	int nUnk;
	std::string sName;
	float vCenter[3];
	float vRadius[3];
	float fRadius;
	std::vector<int> aSurfaces;
	std::vector<int> aCrashSurfaces;
	tCrashData* _pCrashData;
};
struct tObject {
	uint32_t identifier = 0x434A424F; // OBJC
	std::string sName1;
	std::string sName2;
	uint32_t nFlags;
	float mMatrix[4*4];
};
struct tCompactMesh {
	uint32_t identifier = 0x4853454D; // MESH
	std::string sName1;
	std::string sName2;
	uint32_t nFlags;
	int nGroup = -1;
	float mMatrix[4*4];
	uint32_t nUnk1;
	uint32_t nDamageAssocId;
	std::vector<int> aLODMeshIds;
};
struct tCollidableModel {
	std::vector<int> aModels;
	float vCenter[3];
	float vRadius[3];
};
struct tMeshDamageAssoc {
	std::string sName;
	int nIds[2];
};
struct tBGMMesh {
	uint32_t identifier = 0x4853454D;
	std::string sName1;
	std::string sName2;
	uint32_t nFlags;
	int nGroup;
	float mMatrix[4*4];
	std::vector<int> aModels;
};
struct tRetroDemoTMOD {
	uint32_t identifier; // TMOD
	uint32_t someId;
	std::vector<aiVector3D> aVertices;
	std::vector<int> aIndices;
};
int nSomeMapValue = 1; // always 1 in FO2, doesn't exist in FO1
std::vector<tVertexBuffer> aVertexBuffers;
std::vector<tIndexBuffer> aIndexBuffers;
std::vector<tMaterial> aMaterials;
std::vector<tSurface> aSurfaces;
std::vector<tSurface> aCrashSurfaces;
std::vector<tStaticBatch> aStaticBatches;
std::vector<uint32_t> aTreeColors;
std::vector<tTreeLOD> aTreeLODs;
std::vector<tTreeMesh> aTreeMeshes;
float aTrackCollisionOffsetMatrix[4*4];
std::vector<tModel> aModels;
std::vector<tObject> aObjects;
std::vector<tCompactMesh> aCompactMeshes;
std::vector<tCollidableModel> aCollidableModels;
std::vector<tMeshDamageAssoc> aMeshDamageAssoc;
std::vector<tBGMMesh> aBGMMeshes;
std::vector<uint32_t> aVertexColors;
//std::vector<tVertexBuffer> aCrashVertexBuffers;
std::vector<tCrashData> aCrashData;
std::vector<tRetroDemoTMOD> aRetroDemoTMOD;
uint32_t nCompactMeshGroupCount;

tVertexBuffer* FindVertexBuffer(int id) {
	for (auto& buf : aVertexBuffers) {
		if (buf.id == id) return &buf;
	}
	return nullptr;
}

tIndexBuffer* FindIndexBuffer(int id) {
	for (auto& buf : aIndexBuffers) {
		if (buf.id == id) return &buf;
	}
	return nullptr;
}

bool CanSurfaceBeExported(tSurface* surface) {
	auto vBuf = FindVertexBuffer(surface->nStreamId[0]);
	if (!vBuf) return false;
	if (surface->nNumStreamsUsed != 2) return false;
	auto iBuf = FindIndexBuffer(surface->nStreamId[1]);
	if (!iBuf) return false;
	//if (surface->nNumStreamsUsed >= 2) {
	//	auto iBuf = FindIndexBuffer(surface->nStreamId[1]);
	//	if (!iBuf) return false;
	//}
	return true;
}

int IsSurfaceValidAndExportable(int id) {
	if (id < 0 || id >= aSurfaces.size()) return false;
	if (!CanSurfaceBeExported(&aSurfaces[id])) return false;
	return true;
}

const aiScene* pParsedFBXScene = nullptr;
aiNode* FindFBXNodeFromRoot(const char* name) {
	if (!pParsedFBXScene) return nullptr;
	auto root = pParsedFBXScene->mRootNode;
	if (!root) return nullptr;

	for (int i = 0; i < root->mNumChildren; i++) {
		auto node = root->mChildren[i];
		if (!strcmp(node->mName.C_Str(), name)) return node;
	}
	return nullptr;
}

aiNode* GetFBXNodeForBGMMeshArray() {
	auto node = FindFBXNodeFromRoot("BGMMesh");
	if (!node) node = FindFBXNodeFromRoot("CarMesh");
	return node;
}
aiNode* GetFBXNodeForObjectsArray() { return FindFBXNodeFromRoot("Objects"); }
aiNode* GetFBXNodeForCompactMeshArray() { return FindFBXNodeFromRoot("CompactMesh"); }
aiNode* GetFBXNodeForStaticBatchArray() { return FindFBXNodeFromRoot("StaticBatch"); }
aiNode* GetFBXNodeForTreeMeshArray() { return FindFBXNodeFromRoot("TreeMesh"); }

aiNode* FindFBXNodeForCompactMesh(const std::string& name) {
	auto root = GetFBXNodeForCompactMeshArray();
	if (!root) return nullptr;

	for (int i = 0; i < root->mNumChildren; i++) {
		auto node = root->mChildren[i];
		if (!strcmp(node->mName.C_Str(), name.c_str())) return node;
	}
	return nullptr;
}

aiNode* FindFBXNodeForObject(const std::string& name) {
	auto root = GetFBXNodeForObjectsArray();
	if (!root) return nullptr;

	for (int i = 0; i < root->mNumChildren; i++) {
		auto node = root->mChildren[i];
		if (!strcmp(node->mName.C_Str(), name.c_str())) return node;
	}
	return nullptr;
}

aiNode* FindFBXNodeForStaticBatchSurface(const std::string& name) {
	auto root = GetFBXNodeForStaticBatchArray();
	if (!root) return nullptr;

	for (int i = 0; i < root->mNumChildren; i++) {
		auto node = root->mChildren[i];
		if (node->mNumChildren == 0 && node->mNumMeshes == 0) continue;
		if (!strcmp(node->mName.C_Str(), name.c_str())) return node;
	}
	return nullptr;
}

aiNode* FindFBXNodeForTreeMeshSurface(const std::string& name) {
	auto root = GetFBXNodeForTreeMeshArray();
	if (!root) return nullptr;

	for (int i = 0; i < root->mNumChildren; i++) {
		auto node = root->mChildren[i];
		for (int j = 0; j < node->mNumChildren; j++) {
			auto node2 = node->mChildren[j];
			if (node2->mNumChildren == 0 && node2->mNumMeshes == 0) continue;
			if (!strcmp(node2->mName.C_Str(), name.c_str())) return node2;
		}
	}
	return nullptr;
}

aiNode* FindFBXNodeForSurface(int id) {
	std::string name = "Surface" + std::to_string(id);
	if (auto node = FindFBXNodeForStaticBatchSurface(name)) return node;
	if (auto node = FindFBXNodeForTreeMeshSurface(name)) return node;
	name += "_export";
	if (auto node = FindFBXNodeForStaticBatchSurface(name)) return node;
	if (auto node = FindFBXNodeForTreeMeshSurface(name)) return node;
	return nullptr;
}

std::vector<aiNode*> GetAllFBXSurfaceNodes() {
	std::vector<aiNode*> nodes;
	auto staticBatches = GetFBXNodeForStaticBatchArray();
	for (int i = 0; i < staticBatches->mNumChildren; i++) {
		nodes.push_back(staticBatches->mChildren[i]);
	}
	auto treeMeshes = GetFBXNodeForTreeMeshArray();
	for (int i = 0; i < treeMeshes->mNumChildren; i++) {
		auto treeMesh = treeMeshes->mChildren[i];
		for (int j = 0; j < treeMesh->mNumChildren; j++) {
			nodes.push_back(treeMesh->mChildren[j]);
		}
	}
	return nodes;
}

aiMatrix4x4 GetFullMatrixFromCompactMeshObject(aiNode* node) {
	auto root = pParsedFBXScene->mRootNode;
	auto compactMeshRoot = GetFBXNodeForCompactMeshArray();

	return root->mTransformation * compactMeshRoot->mTransformation * node->mTransformation;
}

aiMatrix4x4 GetFullMatrixFromDummyObject(aiNode* node) {
	auto root = pParsedFBXScene->mRootNode;
	auto compactMeshRoot = GetFBXNodeForObjectsArray();

	return root->mTransformation * compactMeshRoot->mTransformation * node->mTransformation;
}

void FO2MatrixToFBXMatrix(const float* src, aiMatrix4x4* dest) {
	dest->a1 = src[0];
	dest->b1 = src[1];
	dest->c1 = -src[2];
	dest->d1 = src[3];
	dest->a2 = src[4];
	dest->b2 = src[5];
	dest->c2 = -src[6];
	dest->d2 = src[7];
	dest->a3 = -src[8];
	dest->b3 = -src[9];
	dest->c3 = src[10];
	dest->d3 = src[11];
	dest->a4 = src[12];
	dest->b4 = src[13];
	dest->c4 = -src[14];
	dest->d4 = src[15];
}

// for some reason the FBX already exports with Z mirrored... odd
// blender seems to change this though, so reimporting directly will break this but blender reexport works fine
void FBXMatrixToFO2Matrix(const aiMatrix4x4& src, float* dest) {
	dest[0] = src.a1;
	dest[1] = src.b1;
	dest[2] = -src.c1;
	dest[3] = src.d1;
	dest[4] = src.a2;
	dest[5] = src.b2;
	dest[6] = -src.c2;
	dest[7] = src.d2;
	dest[8] = -src.a3;
	dest[9] = -src.b3;
	dest[10] = src.c3;
	dest[11] = src.d3;
	dest[12] = src.a4;
	dest[13] = src.b4;
	dest[14] = -src.c4;
	dest[15] = src.d4;
}

int FindMaterialIDByName(const std::string& name, bool customOnly) {
	for (auto& material : aMaterials) {
		if (!material._bIsCustom && customOnly) continue;
		if (material.sName == name) return &material - &aMaterials[0];
	}
	return -1;
}