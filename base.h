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

std::string sFileName;
std::string sFileNameNoExt;

void WriteConsole(const std::string& str) {
	auto& out = std::cout;
	out << str;
	out << "\n";
	out.flush();
}

void WriteFile(const std::string& str) {
	static auto out = std::ofstream(sFileNameNoExt + "_log.txt");
	out << str;
	out << "\n";
	out.flush();
}

std::string GetMapVersion(int value) {
	if (value == 0x10003) return "BugBear Retro Demo (Unsupported)";
	if (value == 0x10005) return "FlatOut 1";
	if (value == 0x20001) return "FlatOut 2";
	if (value == 0x20002) return "FlatOut Ultimate Carnage";
	if (value >= 0x20000) return "Unknown FlatOut 2";
	return "Unknown FlatOut 1";
}

std::string GetShaderName(int value, int mapVersion) {
	if (mapVersion == 0x20002) {
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
			default: return "UNKNOWN";
		}
	}
	return "Unknown";
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
	SURFACE_REFERENCE_TREEMESH_2,
	SURFACE_REFERENCE_TREEMESH_3,
	SURFACE_REFERENCE_TREEMESH_4,
	SURFACE_REFERENCE_TREEMESH_5,
	NUM_SURFACE_REFERENCE_TYPES
};

// fouc format stuff:
// 26 means vertex count is reduced to 24, vertex size gets multiplied by 4

struct tVertexBuffer {
	int id;
	int foucExtraFormat;
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
	int foucExtraFormat;
	uint32_t indexCount;
	uint16_t* data;
};
struct tVegVertexBuffer {
	int id;
	int foucExtraFormat;
	uint32_t vertexCount;
	uint32_t vertexSize;
	float* data;
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
	int nIsVegetation;
	int nMaterialId;
	int nVertexCount;
	int nFlags;
	int nPolyCount;
	int nPolyMode;
	int nNumIndicesUsed;
	float vAbsoluteCenter[3] = { 0, 0, 0 };
	float vRelativeCenter[3] = { 0, 0, 0 };
	float foucVertexMultiplier[4];
	int nNumStreamsUsed;
	uint32_t nStreamId[2];
	uint32_t nStreamOffset[2];

	int _nFBXModelId;
	int _nNumReferencesByType[NUM_SURFACE_REFERENCE_TYPES] = {};
	int _nNumReferences = 0;
	void RegisterReference(int type) {
		_nNumReferences++;
		_nNumReferencesByType[type]++;
	}
};
struct tStaticBatch {
	uint32_t nCenterId1;
	uint32_t nCenterId2;
	uint32_t nSurfaceId;
	uint32_t nUnk = 0; // seems to always be 0
	float vAbsoluteCenter[3] = { 0, 0, 0 };
	float vRelativeCenter[3] = { 0, 0, 0 };
};
struct tUnknownStructure {
	float vPos[3];
	float fValues[2];
	uint32_t nValues[2];
};
struct tTreeMesh {
	int nUnk1;

	// these 2 values are unused and completely unread
	int nUnk2Unused;
	int nSurfaceId1Unused;

	int nSurfaceId2;
	float fUnk[19];
	int nSurfaceId3;
	int nSurfaceId4;
	int nSurfaceId5;
	int nIdInUnkArray1;
	int nIdInUnkArray2;
	int nMaterialId;

	int foucExtraData1[9];
	int foucExtraData2[9];
	int foucExtraData3[4];
};
struct tModel {
	uint32_t identifier; // BMOD
	int nUnk;
	std::string sName;
	float vCenter[3];
	float vRadius[3];
	float fRadius;
	std::vector<int> aSurfaces;
};
struct tObject {
	uint32_t identifier; // OBJC
	std::string sName1;
	std::string sName2;
	uint32_t nFlags;
	float mMatrix[4*4];
};
struct tCompactMesh {
	uint32_t identifier; // MESH
	std::string sName1;
	std::string sName2;
	uint32_t nFlags;
	int nGroup;
	float mMatrix[4*4];
	uint32_t nUnk1;
	uint32_t nBBoxAssocId;
	std::vector<int> aLODMeshIds;
};
struct tBoundingBox {
	std::vector<int> aModels;
	float vCenter[3];
	float vRadius[3];
};
struct tBoundingBoxMeshAssoc {
	std::string sName;
	int nIds[2];
};
int nImportMapVersion;
int nExportMapVersion;
int nSomeMapValue = 1; // always 1 in FO2, doesn't exist in FO1
std::vector<tVertexBuffer> aVertexBuffers;
std::vector<tIndexBuffer> aIndexBuffers;
std::vector<tVegVertexBuffer> aVegVertexBuffers;
std::vector<tMaterial> aMaterials;
std::vector<tSurface> aSurfaces;
std::vector<tStaticBatch> aStaticBatches;
std::vector<uint32_t> aUnknownArray1;
std::vector<tUnknownStructure> aUnknownArray2;
std::vector<tTreeMesh> aTreeMeshes;
std::vector<float> aUnknownArray3;
std::vector<tModel> aModels;
std::vector<tObject> aObjects;
std::vector<tCompactMesh> aCompactMeshes;
std::vector<tBoundingBox> aBoundingBoxes;
std::vector<tBoundingBoxMeshAssoc> aBoundingBoxMeshAssoc;
uint32_t nCompactMeshGroupCount;

tVertexBuffer* FindVertexBuffer(int id) {
	for (auto& buf : aVertexBuffers) {
		if (buf.id == id) return &buf;
	}
	return nullptr;
}

tVegVertexBuffer* FindVegVertexBuffer(int id) {
	for (auto& buf : aVegVertexBuffers) {
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
	if (surface->nNumStreamsUsed < 2) return false;
	auto iBuf = FindIndexBuffer(surface->nStreamId[1]);
	if (!iBuf) return false;
	return true;
}

int IsSurfaceValidAndExportable(int id, bool checkNotModel = false) {
	if (id < 0 || id >= aSurfaces.size()) return false;
	if (!CanSurfaceBeExported(&aSurfaces[id])) return false;
	if (checkNotModel) {
		for (auto& model : aModels) {
			for (auto& surface : model.aSurfaces) {
				if (surface == id) return false;
			}
		}
	}
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

aiNode* GetFBXNodeForCompactMeshArray() { return FindFBXNodeFromRoot("CompactMesh");}
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

aiNode* FindFBXNodeForStaticBatchSurface(const std::string& name) {
	auto root = GetFBXNodeForStaticBatchArray();
	if (!root) return nullptr;

	for (int i = 0; i < root->mNumChildren; i++) {
		auto node = root->mChildren[i];
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

aiMatrix4x4 GetFullMatrixFromCompactMeshObject(aiNode* node) {
	auto root = pParsedFBXScene->mRootNode;
	auto compactMeshRoot = GetFBXNodeForCompactMeshArray();

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

bool ShouldSurfaceMeshBeImported(aiNode* node) {
	if (!node) return false;
	auto name = (std::string)node->mName.C_Str();
	if (!name.ends_with("_export")) return false;
	if (node->mNumMeshes != 1) {
		WriteConsole("ERROR: Surface " + name + " has more than one mesh or material!");
		return false;
	}
	return true;
}

int FindMaterialIDByName(const std::string& name) {
	for (auto& material : aMaterials) {
		if (material.sName == name) return &material - &aMaterials[0];
	}
	return -1;
}