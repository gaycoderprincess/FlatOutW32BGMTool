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
	uint32_t identifier; // MATC
	std::string sName;
	int nAlpha;
	int v92;
	int nNumTextures;
	int v73;
	int v75;
	int v74;
	int v108[3];
	int v109[3];
	int v98[4];
	int v99[4];
	int v100[4];
	int v101[4];
	int v102;
	std::string sTextureNames[3];
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

int IsSurfaceValidAndExportable(int id) {
	if (id < 0 || id >= aSurfaces.size()) return false;
	if (!CanSurfaceBeExported(&aSurfaces[id])) return false;
	return true;
}

const aiScene* pParsedFBXScene = nullptr;
aiNode* GetFBXNodeForCompactMeshArray() {
	auto root = pParsedFBXScene->mRootNode;
	if (!root) return nullptr;

	for (int i = 0; i < root->mNumChildren; i++) {
		auto node = root->mChildren[i];
		if (!strcmp(node->mName.C_Str(), "CompactMesh")) return node;
	}
	return nullptr;
}

aiNode* FindFBXNodeForCompactMesh(const std::string& name) {
	auto root = GetFBXNodeForCompactMeshArray();
	if (!root) return nullptr;

	for (int i = 0; i < root->mNumChildren; i++) {
		auto node = root->mChildren[i];
		if (!strcmp(node->mName.C_Str(), name.c_str())) return node;
	}
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