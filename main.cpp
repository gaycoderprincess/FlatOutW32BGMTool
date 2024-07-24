#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <format>
#include <vector>

// research:
// vertex buffer data:
// 1 2 3 always coords
// 0x202 flag seems to be the terrain
// 0x152 -> 4 5 6 are normals, 7 unknown, 8 and 9 are UV coords
// 0x212 -> 4 5 6 are normals, 7 8 9 10 unknown

// gulbroz stuff:
// Vertex stream flags
#define VERTEX_POSITION			(1<<1)
#define VERTEX_UV				(1<<8)
#define VERTEX_UV2				(1<<9)
#define VERTEX_NORMAL			(1<<4)
#define VERTEX_BLEND			(1<<6)
#define STREAM_VERTEX_DECAL		(VERTEX_POSITION | VERTEX_UV)
#define STREAM_VERTEX_MODEL		(VERTEX_POSITION | VERTEX_UV  | VERTEX_NORMAL)
#define STREAM_VERTEX_STATIC	(VERTEX_POSITION | VERTEX_UV  | VERTEX_BLEND)
#define STREAM_VERTEX_WINDOW	(VERTEX_POSITION | VERTEX_UV  | VERTEX_NORMAL | VERTEX_BLEND)
#define STREAM_VERTEX_TERRAIN	(VERTEX_POSITION | VERTEX_UV2 | VERTEX_NORMAL)
#define STREAM_VERTEX_TERRAIN2	(VERTEX_POSITION | VERTEX_UV2)

// import options
bool bDumpMaterialData = true;
bool bDumpStreams = true;

// export options
bool bDisableProps = false;
bool bConvertToFO1 = true;

std::string sFileName;

void WriteConsole(const std::string& str) {
	auto& out = std::cout;
	out << str;
	out << "\n";
	out.flush();
}

void WriteFile(const std::string& str) {
	static auto out = std::ofstream(sFileName + "_log.txt");
	out << str;
	out << "\n";
	out.flush();
}

std::string GetMapVersion(int value) {
	if (value == 0x10003) return "BugBear Retro Demo (Unsupported)";
	if (value == 0x10005) return "FlatOut 1";
	if (value == 0x20001) return "FlatOut 2";
	if (value == 0x20002) return "FlatOut Ultimate Carnage (Unsupported)";
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

	uint32_t _vertexSizeBeforeFO1 = 0;
};
struct tIndexBuffer {
	int id;
	int foucExtraFormat;
	uint32_t indexCount;
	uint16_t* data;

	std::vector<uint32_t> foucExtraData;
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
	int nVertNum;
	int nFlags;
	int nPolyNum;
	int nPolyMode;
	int nPolyNumIndex;
	float vAbsoluteCenter[3] = { 0, 0, 0 };
	float vRelativeCenter[3] = { 0, 0, 0 };
	int nNumStreamsUsed;
	uint32_t nStreamId[2];
	uint32_t nStreamOffset[2];

	float foucExtraData1[4];

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

	int foucData1[9];
	int foucData2[9];
	int foucData3[4];
	int nSurfaceIdsFouc[3];
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

bool ParseW32Materials(std::ifstream& file) {
	WriteConsole("Parsing materials...");

	uint32_t numMaterials;
	ReadFromFile(file, &numMaterials, 4);
	aMaterials.reserve(numMaterials);
	for (int i = 0; i < numMaterials; i++) {
		tMaterial material;
		ReadFromFile(file, &material.identifier, 4);
		if (material.identifier != 0x4354414D) return false; // "MATC"

		material.sName = ReadStringFromFile(file);
		ReadFromFile(file, &material.nAlpha, 4);
		ReadFromFile(file, &material.v92, 4);
		ReadFromFile(file, &material.nNumTextures, 4);
		ReadFromFile(file, &material.v73, 4);
		ReadFromFile(file, &material.v75, 4);
		ReadFromFile(file, &material.v74, 4);
		ReadFromFile(file, material.v108, 12);
		ReadFromFile(file, material.v109, 12);
		ReadFromFile(file, material.v98, 16);
		ReadFromFile(file, material.v99, 16);
		ReadFromFile(file, material.v100, 16);
		ReadFromFile(file, material.v101, 16);
		ReadFromFile(file, &material.v102, 4);
		material.sTextureNames[0] = ReadStringFromFile(file);
		material.sTextureNames[1] = ReadStringFromFile(file);
		material.sTextureNames[2] = ReadStringFromFile(file);
		aMaterials.push_back(material);
	}
	return true;
}

bool ParseW32Streams(std::ifstream& file) {
	WriteConsole("Parsing streams...");

	uint32_t numStreams;
	ReadFromFile(file, &numStreams, 4);
	for (int i = 0; i < numStreams; i++) {
		int dataType;
		ReadFromFile(file, &dataType, 4);
		if (dataType == 1) {
			tVertexBuffer buf;
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &buf.vertexCount, 4);
			ReadFromFile(file, &buf.vertexSize, 4);
			ReadFromFile(file, &buf.flags, 4);

			if (nImportMapVersion >= 0x20002 && buf.foucExtraFormat - 22 <= 4) { // no clue what or why or when or how, this is a bugbear specialty
				auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));
				auto vertexData = new float[dataSize];
				ReadFromFile(file, vertexData, dataSize * sizeof(float));

				buf.id = i;
				buf.data = vertexData;
				aVertexBuffers.push_back(buf);
			}
		}
		else if (dataType == 2) {
			tIndexBuffer buf;

			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &buf.indexCount, 4);

			if (nImportMapVersion >= 0x20002) {
				if (auto extraValue = buf.indexCount >> 6) {
					buf.foucExtraData.reserve(extraValue * 32); // size 128 each
					for (int j = 0; j < extraValue * 32; j++) {
						int tmp;
						ReadFromFile(file, &tmp, 4);
						buf.foucExtraData.push_back(tmp);
					}
				}

				buf.indexCount = -64 * (buf.indexCount >> 6) + buf.indexCount;
			}

			auto dataSize = buf.indexCount;
			auto indexData = new uint16_t[dataSize];
			ReadFromFile(file, indexData, dataSize * sizeof(uint16_t));

			buf.id = i;
			buf.data = indexData;
			aIndexBuffers.push_back(buf);
		}
		else if (dataType == 3) {
			tVegVertexBuffer buf;

			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &buf.vertexCount, 4);
			ReadFromFile(file, &buf.vertexSize, 4);

			auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));
			auto data = new float[dataSize];
			ReadFromFile(file, data, dataSize * sizeof(float));

			buf.id = i;
			buf.data = data;
			aVegVertexBuffers.push_back(buf);
		}
		else {
			WriteConsole("Unknown stream type " + std::to_string(dataType));
			return false;
		}
	}
	return true;
}

bool ParseW32Surfaces(std::ifstream& file, int mapVersion) {
	WriteConsole("Parsing surfaces...");

	uint32_t nSurfaceCount;
	ReadFromFile(file, &nSurfaceCount, 4);
	aSurfaces.reserve(nSurfaceCount);
	for (int i = 0; i < nSurfaceCount; i++) {
		tSurface surface;
		ReadFromFile(file, &surface.nIsVegetation, 4);
		ReadFromFile(file, &surface.nMaterialId, 4);
		ReadFromFile(file, &surface.nVertNum, 4);
		ReadFromFile(file, &surface.nFlags, 4);
		ReadFromFile(file, &surface.nPolyNum, 4);
		ReadFromFile(file, &surface.nPolyMode, 4);
		ReadFromFile(file, &surface.nPolyNumIndex, 4);

		if (mapVersion < 0x20000) {
			ReadFromFile(file, surface.vAbsoluteCenter, 12);
			ReadFromFile(file, surface.vRelativeCenter, 12);
		}

		if (mapVersion >= 0x20002) {
			ReadFromFile(file, surface.foucExtraData1, sizeof(surface.foucExtraData1));
		}

		ReadFromFile(file, &surface.nNumStreamsUsed, 4);
		if (surface.nNumStreamsUsed > 2) return false;

		for (int j = 0; j < surface.nNumStreamsUsed; j++) {
			ReadFromFile(file, &surface.nStreamId[j], 4);
			ReadFromFile(file, &surface.nStreamOffset[j], 4);
		}

		aSurfaces.push_back(surface);
	}
	return true;
}

bool ParseW32StaticBatches(std::ifstream& file, int mapVersion) {
	WriteConsole("Parsing static batches...");

	uint32_t numStaticBatches;
	ReadFromFile(file, &numStaticBatches, 4);
	aStaticBatches.reserve(numStaticBatches);
	for (int i = 0; i < numStaticBatches; i++) {
		tStaticBatch staticBatch;
		ReadFromFile(file, &staticBatch.nCenterId1, 4);
		ReadFromFile(file, &staticBatch.nCenterId2, 4);
		ReadFromFile(file, &staticBatch.nSurfaceId, 4);

		bool bIsSurfaceValid = staticBatch.nSurfaceId < aSurfaces.size();
		if (nImportMapVersion < 0x20002 && !bIsSurfaceValid) return false;

		if (bIsSurfaceValid) {
			aSurfaces[staticBatch.nSurfaceId].RegisterReference(SURFACE_REFERENCE_STATICBATCH);
		}

		if (mapVersion >= 0x20000) {
			ReadFromFile(file, staticBatch.vAbsoluteCenter, 12);
			ReadFromFile(file, staticBatch.vRelativeCenter, 12);

			if (bIsSurfaceValid) {
				// backwards compatibility
				memcpy(aSurfaces[staticBatch.nSurfaceId].vAbsoluteCenter, staticBatch.vAbsoluteCenter, 12);
				memcpy(aSurfaces[staticBatch.nSurfaceId].vRelativeCenter, staticBatch.vRelativeCenter, 12);
			}
		}
		else {
			ReadFromFile(file, &staticBatch.nUnk, 4); // always 0?

			if (bIsSurfaceValid) {
				// forwards compatibility
				memcpy(staticBatch.vAbsoluteCenter, aSurfaces[staticBatch.nSurfaceId].vAbsoluteCenter, 12);
				memcpy(staticBatch.vRelativeCenter, aSurfaces[staticBatch.nSurfaceId].vRelativeCenter, 12);
			}
		}

		aStaticBatches.push_back(staticBatch);
	}
	return true;
}

bool ParseW32TreeMeshes(std::ifstream& file) {
	WriteConsole("Parsing tree meshes...");

	uint32_t treeMeshCount;
	ReadFromFile(file, &treeMeshCount, 4);
	aTreeMeshes.reserve(treeMeshCount);
	for (int i = 0; i < treeMeshCount; i++) {
		tTreeMesh treeMesh;
		ReadFromFile(file, &treeMesh.nUnk1, 4);
		ReadFromFile(file, &treeMesh.nUnk2Unused, 4);
		ReadFromFile(file, &treeMesh.nSurfaceId1Unused, 4);
		ReadFromFile(file, &treeMesh.nSurfaceId2, 4);
		ReadFromFile(file, treeMesh.fUnk, sizeof(treeMesh.fUnk));

		if (nImportMapVersion >= 0x20002) {
			if (treeMesh.nSurfaceId2 >= 0 && treeMesh.nSurfaceId2 < aSurfaces.size()) aSurfaces[treeMesh.nSurfaceId2].RegisterReference(SURFACE_REFERENCE_TREEMESH_2);

			ReadFromFile(file, treeMesh.foucData1, sizeof(treeMesh.foucData1));
			ReadFromFile(file, treeMesh.foucData2, sizeof(treeMesh.foucData2));
			ReadFromFile(file, treeMesh.foucData3, sizeof(treeMesh.foucData3));
			ReadFromFile(file, treeMesh.nSurfaceIdsFouc, sizeof(treeMesh.nSurfaceIdsFouc));
			for (int j = 0; j < 3; j++) {
				if (treeMesh.nSurfaceIdsFouc[j] < 0) continue;
				if (treeMesh.nSurfaceIdsFouc[j] >= aSurfaces.size()) continue;
				aSurfaces[treeMesh.nSurfaceIdsFouc[j]].RegisterReference(SURFACE_REFERENCE_TREEMESH_2 + j);
			}
		}
		else {
			ReadFromFile(file, &treeMesh.nSurfaceId3, 4);
			ReadFromFile(file, &treeMesh.nSurfaceId4, 4);
			ReadFromFile(file, &treeMesh.nSurfaceId5, 4);
			ReadFromFile(file, &treeMesh.nIdInUnkArray1, 4);
			ReadFromFile(file, &treeMesh.nIdInUnkArray2, 4);
			ReadFromFile(file, &treeMesh.nMaterialId, 4);

			//if (treeMesh.nSurfaceId1 >= 0 && treeMesh.nSurfaceId1 >= aSurfaces.size()) return false;
			//if (treeMesh.nSurfaceId1 >= 0) aSurfaces[treeMesh.nSurfaceId1]._bUsedByAnything = true;

			if (treeMesh.nSurfaceId2 >= 0 && treeMesh.nSurfaceId2 >= aSurfaces.size()) return false;
			if (treeMesh.nSurfaceId2 >= 0 && treeMesh.nSurfaceId2 >= aSurfaces.size()) return false;
			if (treeMesh.nSurfaceId3 >= 0 && treeMesh.nSurfaceId3 >= aSurfaces.size()) return false;
			if (treeMesh.nSurfaceId4 >= 0 && treeMesh.nSurfaceId4 >= aSurfaces.size()) return false;
			if (treeMesh.nSurfaceId5 >= 0 && treeMesh.nSurfaceId5 >= aSurfaces.size()) return false;
			if (treeMesh.nSurfaceId2 >= 0) aSurfaces[treeMesh.nSurfaceId2].RegisterReference(SURFACE_REFERENCE_TREEMESH_2);
			if (treeMesh.nSurfaceId3 >= 0) aSurfaces[treeMesh.nSurfaceId3].RegisterReference(SURFACE_REFERENCE_TREEMESH_3);
			if (treeMesh.nSurfaceId4 >= 0) aSurfaces[treeMesh.nSurfaceId4].RegisterReference(SURFACE_REFERENCE_TREEMESH_4);
			if (treeMesh.nSurfaceId5 >= 0) aSurfaces[treeMesh.nSurfaceId5].RegisterReference(SURFACE_REFERENCE_TREEMESH_5);
		}

		aTreeMeshes.push_back(treeMesh);
	}
	return true;
}

bool ParseW32Models(std::ifstream& file) {
	WriteConsole("Parsing models...");

	uint32_t modelCount;
	ReadFromFile(file, &modelCount, 4);
	aModels.reserve(modelCount);
	for (int i = 0; i < modelCount; i++) {
		tModel model;
		ReadFromFile(file, &model.identifier, 4);
		if (model.identifier != 0x444F4D42) return false; // "BMOD"

		ReadFromFile(file, &model.nUnk, 4);
		model.sName = ReadStringFromFile(file);
		ReadFromFile(file, model.vCenter, sizeof(model.vCenter));
		ReadFromFile(file, model.vRadius, sizeof(model.vRadius));
		ReadFromFile(file, &model.fRadius, 4);
		uint32_t numSurfaces;
		ReadFromFile(file, &numSurfaces, sizeof(numSurfaces));
		for (int j = 0; j < numSurfaces; j++) {
			int surface;
			ReadFromFile(file, &surface, sizeof(surface));
			model.aSurfaces.push_back(surface);

			if (surface >= aSurfaces.size()) return false;
			aSurfaces[surface].RegisterReference(SURFACE_REFERENCE_MODEL);
		}
		aModels.push_back(model);
	}
	return true;
}

bool ParseW32Objects(std::ifstream& file) {
	WriteConsole("Parsing objects...");

	uint32_t objectCount;
	ReadFromFile(file, &objectCount, 4);
	aObjects.reserve(objectCount);
	for (int i = 0; i < objectCount; i++) {
		tObject object;
		ReadFromFile(file, &object.identifier, 4);
		if (object.identifier != 0x434A424F) return false; // "OBJC"

		object.sName1 = ReadStringFromFile(file);
		object.sName2 = ReadStringFromFile(file);
		ReadFromFile(file, &object.nFlags, 4);
		ReadFromFile(file, object.mMatrix, sizeof(object.mMatrix));
		aObjects.push_back(object);
	}
	return true;
}

bool ParseW32CompactMeshes(std::ifstream& file, uint32_t mapVersion) {
	WriteConsole("Parsing compact meshes...");

	uint32_t compactMeshCount;
	ReadFromFile(file, &nCompactMeshGroupCount, 4);
	ReadFromFile(file, &compactMeshCount, 4);
	aCompactMeshes.reserve(compactMeshCount);
	for (int i = 0; i < compactMeshCount; i++) {
		tCompactMesh compactMesh;
		ReadFromFile(file, &compactMesh.identifier, 4);
		if (compactMesh.identifier != 0x4853454D) return false; // "MESH"

		compactMesh.sName1 = ReadStringFromFile(file);
		compactMesh.sName2 = ReadStringFromFile(file);
		ReadFromFile(file, &compactMesh.nFlags, 4);
		ReadFromFile(file, &compactMesh.nGroup, 4);
		ReadFromFile(file, &compactMesh.mMatrix, sizeof(compactMesh.mMatrix));

		if (mapVersion >= 0x20000) {
			ReadFromFile(file, &compactMesh.nUnk1, 4);
			ReadFromFile(file, &compactMesh.nBBoxAssocId, 4);

			auto bboxAssoc = aBoundingBoxMeshAssoc[compactMesh.nBBoxAssocId];
			auto bbox = aBoundingBoxes[bboxAssoc.nIds[0]];
			compactMesh.aLODMeshIds = bbox.aModels;
		}
		else {
			uint32_t nLODCount;
			ReadFromFile(file, &nLODCount, 4);
			compactMesh.aLODMeshIds.reserve(nLODCount);
			for (int j = 0; j < nLODCount; j++) {
				uint32_t nMeshIndex;
				ReadFromFile(file, &nMeshIndex, 4);
				compactMesh.aLODMeshIds.push_back(nMeshIndex);
			}
		}
		aCompactMeshes.push_back(compactMesh);
	}
	return true;
}

bool ParseW32BoundingBoxes(std::ifstream& file) {
	WriteConsole("Parsing bounding boxes...");

	uint32_t boundingBoxCount;
	ReadFromFile(file, &boundingBoxCount, 4);
	aBoundingBoxes.reserve(boundingBoxCount);
	for (int i = 0; i < boundingBoxCount; i++) {
		tBoundingBox boundingBox;

		uint32_t modelCount;
		ReadFromFile(file, &modelCount, 4);
		for (int j = 0; j < modelCount; j++) {
			uint32_t modelId;
			ReadFromFile(file, &modelId, 4);
			boundingBox.aModels.push_back(modelId);
		}

		ReadFromFile(file, boundingBox.vCenter, sizeof(boundingBox.vCenter));
		ReadFromFile(file, boundingBox.vRadius, sizeof(boundingBox.vRadius));
		aBoundingBoxes.push_back(boundingBox);
	}
	return true;
}

bool ParseW32BoundingBoxMeshAssoc(std::ifstream& file) {
	WriteConsole("Parsing bounding box mesh associations...");

	uint32_t assocCount;
	ReadFromFile(file, &assocCount, 4);
	aBoundingBoxMeshAssoc.reserve(assocCount);
	for (int i = 0; i < assocCount; i++) {
		tBoundingBoxMeshAssoc assoc;
		assoc.sName = ReadStringFromFile(file);
		ReadFromFile(file, assoc.nIds, sizeof(assoc.nIds));
		aBoundingBoxMeshAssoc.push_back(assoc);
	}
	return true;
}

void WriteMaterialToFile(std::ofstream& file, const tMaterial& material) {
	file.write((char*)&material.identifier, 4);
	file.write(material.sName.c_str(), material.sName.length() + 1);
	file.write((char*)&material.nAlpha, 4);
	file.write((char*)&material.v92, 4);
	file.write((char*)&material.nNumTextures, 4);
	file.write((char*)&material.v73, 4);
	file.write((char*)&material.v75, 4);
	file.write((char*)&material.v74, 4);
	file.write((char*)material.v108, sizeof(material.v108));
	file.write((char*)material.v109, sizeof(material.v109));
	file.write((char*)material.v98, sizeof(material.v98));
	file.write((char*)material.v99, sizeof(material.v99));
	file.write((char*)material.v100, sizeof(material.v100));
	file.write((char*)material.v101, sizeof(material.v101));
	file.write((char*)&material.v102, 4);
	for (int i = 0; i < 3; i++) {
		file.write(material.sTextureNames[i].c_str(), material.sTextureNames[i].length() + 1);
	}
}

// FO1 doesn't support the vertex color + vertex normal combo and so crashes
// for exporting to FO1, i adjust the vertex buffers to not have normals if they also have vertex colors
// this could prolly be fixed with a plugin too to actually get rid of the issue
bool IsBufferReductionRequiredForFO1(uint32_t flags) {
	if ((flags & 0x10) == 0) return false;
	if ((flags & 0x40) == 0) return false;
	return true;
}

void WriteVertexBufferToFile(std::ofstream& file, tVertexBuffer& buf) {
	bool bRemoveNormals = false;
	if (nExportMapVersion < 0x20000 && nExportMapVersion != nImportMapVersion && IsBufferReductionRequiredForFO1(buf.flags)) {
		buf._vertexSizeBeforeFO1 = buf.vertexSize;
		buf.flags -= 0x10;
		buf.vertexSize -= 0xC;
		bRemoveNormals = true;
	}

	int type = 1;
	file.write((char*)&type, 4);
	file.write((char*)&buf.foucExtraFormat, 4);
	file.write((char*)&buf.vertexCount, 4);
	file.write((char*)&buf.vertexSize, 4);
	file.write((char*)&buf.flags, 4);
	if (bRemoveNormals) {
		int numWritten = 0;

		auto dataSize = buf.vertexCount * (buf._vertexSizeBeforeFO1 / sizeof(float));
		size_t j = 0;
		while (j < dataSize) {
			for (int k = 0; k < buf._vertexSizeBeforeFO1 / sizeof(float); k++) {
				if (k == 3 || k == 4 || k == 5) {
					j++;
					continue;
				}
				file.write((char*)&buf.data[j], 4);
				j++;
				numWritten++;
			}
		}

		if (numWritten != buf.vertexCount * (buf.vertexSize / 4)) {
			WriteConsole("Write mismatch!");
			WriteConsole(std::to_string(buf.vertexCount * (buf.vertexSize / 4)));
			WriteConsole(std::to_string(numWritten));
		}
	}
	else {
		file.write((char*)buf.data, buf.vertexCount * buf.vertexSize);
	}
}

void WriteIndexBufferToFile(std::ofstream& file, const tIndexBuffer& buf) {
	int type = 2;
	file.write((char*)&type, 4);
	file.write((char*)&buf.foucExtraFormat, 4);
	file.write((char*)&buf.indexCount, 4);
	file.write((char*)buf.data, buf.indexCount * 2);
}

void WriteVegVertexBufferToFile(std::ofstream& file, const tVegVertexBuffer& buf) {
	int type = 3;
	file.write((char*)&type, 4);
	file.write((char*)&buf.foucExtraFormat, 4);
	file.write((char*)&buf.vertexCount, 4);
	file.write((char*)&buf.vertexSize, 4);
	file.write((char*)buf.data, buf.vertexCount * buf.vertexSize);
}

void WriteSurfaceToFile(std::ofstream& file, tSurface& surface) {
	if (nExportMapVersion < 0x20000 && nExportMapVersion != nImportMapVersion && IsBufferReductionRequiredForFO1(surface.nFlags)) {
		surface.nFlags -= 0x10;
		auto stream = FindVertexBuffer(surface.nStreamId[0]);
		auto vertexSizeBefore = stream->_vertexSizeBeforeFO1;
		auto vertexSizeAfter = stream->vertexSize;
		surface.nStreamOffset[0] /= vertexSizeBefore;
		surface.nStreamOffset[0] *= vertexSizeAfter;
	}

	file.write((char*)&surface.nIsVegetation, 4);
	file.write((char*)&surface.nMaterialId, 4);
	file.write((char*)&surface.nVertNum, 4);
	file.write((char*)&surface.nFlags, 4);
	file.write((char*)&surface.nPolyNum, 4);
	file.write((char*)&surface.nPolyMode, 4);
	file.write((char*)&surface.nPolyNumIndex, 4);
	if (nExportMapVersion < 0x20000) {
		file.write((char*)surface.vAbsoluteCenter, 12);
		file.write((char*)surface.vRelativeCenter, 12);
	}
	file.write((char*)&surface.nNumStreamsUsed, 4);
	for (int j = 0; j < surface.nNumStreamsUsed; j++) {
		file.write((char*)&surface.nStreamId[j], 4);
		file.write((char*)&surface.nStreamOffset[j], 4);
	}
}

void WriteStaticBatchToFile(std::ofstream& file, const tStaticBatch& staticBatch) {
	file.write((char*)&staticBatch.nCenterId1, 4);
	file.write((char*)&staticBatch.nCenterId2, 4);
	file.write((char*)&staticBatch.nSurfaceId, 4);
	if (nExportMapVersion >= 0x20000) {
		file.write((char*)staticBatch.vAbsoluteCenter, 12);
		file.write((char*)staticBatch.vRelativeCenter, 12);
	}
	else {
		file.write((char*)&staticBatch.nUnk, 4);
	}
}

void WriteTreeMeshToFile(std::ofstream& file, const tTreeMesh& treeMesh) {
	file.write((char*)&treeMesh.nUnk1, 4);
	file.write((char*)&treeMesh.nUnk2Unused, 4);
	file.write((char*)&treeMesh.nSurfaceId1Unused, 4);
	file.write((char*)&treeMesh.nSurfaceId2, 4);
	file.write((char*)treeMesh.fUnk, sizeof(treeMesh.fUnk));
	file.write((char*)&treeMesh.nSurfaceId3, 4);
	file.write((char*)&treeMesh.nSurfaceId4, 4);
	file.write((char*)&treeMesh.nSurfaceId5, 4);
	file.write((char*)&treeMesh.nIdInUnkArray1, 4);
	file.write((char*)&treeMesh.nIdInUnkArray2, 4);
	file.write((char*)&treeMesh.nMaterialId, 4);
}

void WriteModelToFile(std::ofstream& file, const tModel& model) {
	file.write((char*)&model.identifier, 4);
	file.write((char*)&model.nUnk, 4);
	file.write(model.sName.c_str(), model.sName.length() + 1);
	file.write((char*)model.vCenter, sizeof(model.vCenter));
	file.write((char*)model.vRadius, sizeof(model.vRadius));
	file.write((char*)&model.fRadius, 4);
	int numSurfaces = model.aSurfaces.size();
	file.write((char*)&numSurfaces, 4);
	for (auto& surface : model.aSurfaces) {
		file.write((char*)&surface, 4);
	}
}

void WriteObjectToFile(std::ofstream& file, const tObject& object) {
	file.write((char*)&object.identifier, 4);
	file.write(object.sName1.c_str(), object.sName1.length() + 1);
	file.write(object.sName2.c_str(), object.sName2.length() + 1);
	file.write((char*)&object.nFlags, 4);
	file.write((char*)object.mMatrix, sizeof(object.mMatrix));
}

void WriteBoundingBoxToFile(std::ofstream& file, const tBoundingBox& bbox) {
	int numModels = bbox.aModels.size();
	file.write((char*)&numModels, 4);
	for (auto& model : bbox.aModels) {
		file.write((char*)&model, 4);
	}
	file.write((char*)bbox.vCenter, sizeof(bbox.vCenter));
	file.write((char*)bbox.vRadius, sizeof(bbox.vRadius));
}

void WriteBoundingBoxMeshAssocToFile(std::ofstream& file, const tBoundingBoxMeshAssoc& assoc) {
	file.write(assoc.sName.c_str(), assoc.sName.length() + 1);
	file.write((char*)assoc.nIds, sizeof(assoc.nIds));
}

void WriteCompactMeshToFile(std::ofstream& file, const tCompactMesh& mesh) {
	file.write((char*)&mesh.identifier, 4);
	file.write(mesh.sName1.c_str(), mesh.sName1.length() + 1);
	file.write(mesh.sName2.c_str(), mesh.sName2.length() + 1);
	file.write((char*)&mesh.nFlags, 4);
	file.write((char*)&mesh.nGroup, 4);
	file.write((char*)mesh.mMatrix, sizeof(mesh.mMatrix));
	if (nExportMapVersion >= 0x20000) {
		file.write((char*)&mesh.nUnk1, 4);
		file.write((char*)&mesh.nBBoxAssocId, 4);
	}
	else {
		int numLODs = mesh.aLODMeshIds.size();
		file.write((char*)&numLODs, 4);
		for (auto model : mesh.aLODMeshIds) {
			file.write((char*)&model, 4);
		}
	}
}

void WriteW32(const std::string& fileName, bool isFO2) {
	WriteConsole("Writing output w32 file...");

	std::ofstream file(fileName + "_out.w32", std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	nExportMapVersion = isFO2 ? 0x20001 : 0x10005;
	file.write((char*)&nExportMapVersion, 4);
	if (isFO2) file.write((char*)&nSomeMapValue, 4);

	uint32_t materialCount = aMaterials.size();
	file.write((char*)&materialCount, 4);
	for (auto& material : aMaterials) {
		WriteMaterialToFile(file, material);
	}

	uint32_t streamCount = aVertexBuffers.size() + aVegVertexBuffers.size() + aIndexBuffers.size();
	file.write((char*)&streamCount, 4);
	for (int i = 0; i < streamCount; i++) {
		for (auto& buf : aVertexBuffers) {
			if (buf.id == i) {
				WriteVertexBufferToFile(file, buf);
			}
		}
		for (auto& buf : aVegVertexBuffers) {
			if (buf.id == i) {
				WriteVegVertexBufferToFile(file, buf);
			}
		}
		for (auto& buf : aIndexBuffers) {
			if (buf.id == i) {
				WriteIndexBufferToFile(file, buf);
			}
		}
	}

	uint32_t surfaceCount = aSurfaces.size();
	file.write((char*)&surfaceCount, 4);
	for (auto& surface : aSurfaces) {
		WriteSurfaceToFile(file, surface);
	}

	uint32_t staticBatchCount = aStaticBatches.size();
	file.write((char*)&staticBatchCount, 4);
	for (auto& staticBatch : aStaticBatches) {
		WriteStaticBatchToFile(file, staticBatch);
	}

	uint32_t unk1Count = aUnknownArray1.size();
	file.write((char*)&unk1Count, 4);
	for (auto& data : aUnknownArray1) {
		file.write((char*)&data, 4);
	}

	uint32_t unk2Count = aUnknownArray2.size();
	file.write((char*)&unk2Count, 4);
	for (auto& data : aUnknownArray2) {
		file.write((char*)data.vPos, sizeof(data.vPos));
		file.write((char*)data.fValues, sizeof(data.fValues));
		file.write((char*)data.nValues, sizeof(data.nValues));
	}

	uint32_t treeMeshCount = aTreeMeshes.size();
	file.write((char*)&treeMeshCount, 4);
	for (auto& mesh : aTreeMeshes) {
		WriteTreeMeshToFile(file, mesh);
	}

	if (nExportMapVersion >= 0x10004) {
		for (int i = 0; i < 16; i++) {
			file.write((char*)&aUnknownArray3[i], 4);
		}
	}

	uint32_t modelCount = aModels.size();
	file.write((char*)&modelCount, 4);
	for (auto& model : aModels) {
		WriteModelToFile(file, model);
	}

	uint32_t objectCount = aObjects.size();
	file.write((char*)&objectCount, 4);
	for (auto& object : aObjects) {
		WriteObjectToFile(file, object);
	}

	if (bDisableProps) {
		uint32_t tmpCount = 0;
		if (nExportMapVersion >= 0x20000) {
			file.write((char*)&tmpCount, 4); // bbox
			file.write((char*)&tmpCount, 4); // bbox assoc
		}
		file.write((char*)&tmpCount, 4); // compactmesh groups
		file.write((char*)&tmpCount, 4); // compactmesh
	}
	else {
		if (nExportMapVersion >= 0x20000) {
			uint32_t boundingBoxCount = aBoundingBoxes.size();
			file.write((char*)&boundingBoxCount, 4);
			for (auto& bbox : aBoundingBoxes) {
				WriteBoundingBoxToFile(file, bbox);
			}

			uint32_t boundingBoxAssocCount = aBoundingBoxMeshAssoc.size();
			file.write((char*)&boundingBoxAssocCount, 4);
			for (auto& bboxAssoc : aBoundingBoxMeshAssoc) {
				WriteBoundingBoxMeshAssocToFile(file, bboxAssoc);
			}
		}

		uint32_t compactMeshCount = aCompactMeshes.size();
		file.write((char*)&nCompactMeshGroupCount, 4);
		file.write((char*)&compactMeshCount, 4);
		for (auto& mesh : aCompactMeshes) {
			WriteCompactMeshToFile(file, mesh);
		}
	}

	file.flush();
}

bool ParseW32(const std::string& fileName) {
	std::ifstream fin(fileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	ReadFromFile(fin, &nImportMapVersion, 4);
	if (nImportMapVersion > 0x20000) ReadFromFile(fin, &nSomeMapValue, 4);

	if (!ParseW32Materials(fin)) return false;
	if (!ParseW32Streams(fin)) return false;
	if (!ParseW32Surfaces(fin, nImportMapVersion)) return false;
	if (!ParseW32StaticBatches(fin, nImportMapVersion)) return false;

	WriteConsole("Parsing tree-related data...");

	if (nImportMapVersion < 0x20002) {
		uint32_t someCount;
		ReadFromFile(fin, &someCount, 4);
		for (int i = 0; i < someCount; i++) {
			int someValue;
			ReadFromFile(fin, &someValue, 4);
			aUnknownArray1.push_back(someValue);
		}
	}

	uint32_t someStructCount;
	ReadFromFile(fin, &someStructCount, 4);
	for (int i = 0; i < someStructCount; i++) {
		tUnknownStructure unkStruct;
		ReadFromFile(fin, unkStruct.vPos, sizeof(unkStruct.vPos));
		ReadFromFile(fin, unkStruct.fValues, sizeof(unkStruct.fValues));
		ReadFromFile(fin, unkStruct.nValues, sizeof(unkStruct.nValues));
		aUnknownArray2.push_back(unkStruct);
	}

	if (!ParseW32TreeMeshes(fin)) return false;

	WriteConsole("Parsing unknown data...");
	if (nImportMapVersion >= 0x10004) {
		for (int i = 0; i < 16; i++) {
			float value;
			ReadFromFile(fin, &value, sizeof(value));
			aUnknownArray3.push_back(value);
		}
	}

	if (!ParseW32Models(fin)) return false;
	if (!ParseW32Objects(fin)) return false;

	if (nImportMapVersion >= 0x20000) {
		if (!ParseW32BoundingBoxes(fin)) return false;
		if (!ParseW32BoundingBoxMeshAssoc(fin)) return false;
	}

	if (!ParseW32CompactMeshes(fin, nImportMapVersion)) return false;

	WriteConsole("Parsing finished");
	return true;
}

void WriteW32ToText() {
	WriteConsole("Writing text file...");

	WriteFile(std::format("nMapVersion: 0x{:X} {}", nImportMapVersion, GetMapVersion(nImportMapVersion)));
	if (nImportMapVersion > 0x20000) {
		WriteFile("nSomeMapValue: " + std::to_string(nSomeMapValue));
	}

	WriteFile("");
	WriteFile("Materials begin");
	WriteFile("Count: " + std::to_string(aMaterials.size()));
	WriteFile("");
	for (auto& material : aMaterials) {
		WriteFile("Material " + std::to_string(&material - &aMaterials[0]));
		WriteFile("Name: " + material.sName);
		if (bDumpMaterialData) {
			WriteFile("nAlpha: " + std::to_string(material.nAlpha));
			WriteFile("nUnknown1: " + std::to_string(material.v92));
			WriteFile("nNumTextures: " + std::to_string(material.nNumTextures));
			WriteFile("nUnknown2: " + std::to_string(material.v73));
			WriteFile("nUnknown3: " + std::to_string(material.v75));
			WriteFile("nUnknown4: " + std::to_string(material.v74));
			WriteFile("nUnknown5: " + std::to_string(material.v108[0]) + ", " + std::to_string(material.v108[1]) + ", " + std::to_string(material.v108[2]));
			WriteFile("nUnknown6: " + std::to_string(material.v109[0]) + ", " + std::to_string(material.v109[1]) + ", " + std::to_string(material.v109[2]));
			WriteFile("nUnknown7: " + std::to_string(material.v98[0]) + ", " + std::to_string(material.v98[1]) + ", " + std::to_string(material.v98[2]) + ", " + std::to_string(material.v98[3]));
			WriteFile("nUnknown8: " + std::to_string(material.v99[0]) + ", " + std::to_string(material.v99[1]) + ", " + std::to_string(material.v99[2]) + ", " + std::to_string(material.v99[3]));
			WriteFile("nUnknown9: " + std::to_string(material.v100[0]) + ", " + std::to_string(material.v100[1]) + ", " + std::to_string(material.v100[2]) + ", " + std::to_string(material.v100[3]));
			WriteFile("nUnknown10: " + std::to_string(material.v101[0]) + ", " + std::to_string(material.v101[1]) + ", " + std::to_string(material.v101[2]) + ", " + std::to_string(material.v101[3]));
			WriteFile("nUnknown11: " + std::to_string(material.v102));
		}
		WriteFile("Texture 1: " + material.sTextureNames[0]);
		WriteFile("Texture 2: " + material.sTextureNames[1]);
		WriteFile("Texture 3: " + material.sTextureNames[2]);
		WriteFile("");
	}
	WriteFile("Materials end");
	WriteFile("");

	WriteFile("Streams begin");
	uint32_t numStreams = aVertexBuffers.size() + aVegVertexBuffers.size() + aIndexBuffers.size();
	WriteFile("Count: " + std::to_string(numStreams));
	WriteFile("");
	for (int i = 0; i < numStreams; i++) {
		WriteFile("Stream " + std::to_string(i));
		for (auto& buf : aVertexBuffers) {
			if (buf.id == i) {
				WriteFile("Vertex buffer");
				if (nImportMapVersion >= 0x20002) WriteFile(std::format("foucExtraFormat: {}", buf.foucExtraFormat));
				WriteFile(std::format("Vertex Size: {}", buf.vertexSize));
				WriteFile(std::format("Vertex Count: {}", buf.vertexCount));
				WriteFile(std::format("nFlags: 0x{:X}", buf.flags));
				if (bDumpStreams) {
					auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));

					int nVertexColorOffset = -1;
					if ((buf.flags & 0x40) != 0) {
						nVertexColorOffset = 3;
						if ((buf.flags & 0x10) != 0) {
							nVertexColorOffset = 6;
						}
					}

					size_t j = 0;
					while (j < dataSize) {
						std::string out;
						for (int k = 0; k < buf.vertexSize / sizeof(float); k++) {
							if (k == nVertexColorOffset) {
								out += std::format("0x{:X}", *(uint32_t*)&buf.data[j]);
							}
							else {
								out += std::to_string(buf.data[j]);
							}
							out += " ";
							j++;
						}
						WriteFile(out);
					}
				}
			}
		}
		for (auto& buf : aVegVertexBuffers) {
			if (buf.id == i) {
				WriteFile("Vegetation vertex buffer");
				if (nImportMapVersion >= 0x20002) WriteFile(std::format("foucExtraFormat: {}", buf.foucExtraFormat));
				WriteFile(std::format("Vertex Size: {}", buf.vertexSize));
				WriteFile(std::format("Vertex Count: {}", buf.vertexCount));
				if (bDumpStreams) {
					auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));

					size_t j = 0;
					while (j < dataSize) {
						std::string out;
						for (int k = 0; k < buf.vertexSize / sizeof(float); k++) {
							out += std::to_string(buf.data[j]);
							out += " ";
							j++;
						}
						WriteFile(out);
					}
				}
			}
		}
		for (auto& buf : aIndexBuffers) {
			if (buf.id == i) {
				WriteFile("Index buffer");
				if (nImportMapVersion >= 0x20002) {
					WriteFile(std::format("foucExtraFormat: {}", buf.foucExtraFormat));
					WriteFile(std::format("16bit Index Count: {}", buf.indexCount));
					WriteFile(std::format("32bit Index Count: {}", buf.foucExtraData.size()));
				}
				else {
					WriteFile(std::format("Index Count: {}", buf.indexCount));
				}
				if (bDumpStreams) {
					if (nImportMapVersion >= 0x20002) {
						for (auto &data: buf.foucExtraData) {
							WriteFile(std::to_string(data));
						}
					}
					for (int j = 0; j < buf.indexCount; j++) {
						WriteFile(std::to_string(buf.data[j]));
					}
				}
			}
		}
		WriteFile("");
	}
	WriteFile("Streams end");
	WriteFile("");

	WriteFile("Surfaces begin");
	WriteFile("Count: " + std::to_string(aSurfaces.size()));
	WriteFile("");
	for (auto& surface : aSurfaces) {
		WriteFile("Surface " + std::to_string(&surface - &aSurfaces[0]));
		WriteFile("nIsVegetation: " + std::to_string(surface.nIsVegetation));
		WriteFile("nMaterialId: " + std::to_string(surface.nMaterialId));
		WriteFile("nVertNum: " + std::to_string(surface.nVertNum));
		WriteFile(std::format("nFormat: 0x{:X}", surface.nFlags));
		WriteFile("nPolyNum: " + std::to_string(surface.nPolyNum));
		WriteFile("nPolyMode: " + std::to_string(surface.nPolyMode)); // 4-triindx or 5-tristrip
		WriteFile("nPolyNumIndex: " + std::to_string(surface.nPolyNumIndex));
		WriteFile("vAbsoluteCenter.x: " + std::to_string(surface.vAbsoluteCenter[0]));
		WriteFile("vAbsoluteCenter.y: " + std::to_string(surface.vAbsoluteCenter[1]));
		WriteFile("vAbsoluteCenter.z: " + std::to_string(surface.vAbsoluteCenter[2]));
		WriteFile("vRelativeCenter.x: " + std::to_string(surface.vRelativeCenter[0]));
		WriteFile("vRelativeCenter.y: " + std::to_string(surface.vRelativeCenter[1]));
		WriteFile("vRelativeCenter.z: " + std::to_string(surface.vRelativeCenter[2]));
		if (nImportMapVersion >= 0x20002) {
			WriteFile("foucExtraData[0]: " + std::to_string(surface.foucExtraData1[0]));
			WriteFile("foucExtraData[1]: " + std::to_string(surface.foucExtraData1[1]));
			WriteFile("foucExtraData[2]: " + std::to_string(surface.foucExtraData1[2]));
			WriteFile("foucExtraData[3]: " + std::to_string(surface.foucExtraData1[3]));
		}
		WriteFile("nNumStreamsUsed: " + std::to_string(surface.nNumStreamsUsed));
		for (int j = 0; j < surface.nNumStreamsUsed; j++) {
			WriteFile("nStreamId: " + std::to_string(surface.nStreamId[j]));
			WriteFile(std::format("nStreamOffset: 0x{:X}", surface.nStreamOffset[j]));
		}
		WriteFile("Total references: " + std::to_string(surface._nNumReferences));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_STATICBATCH]) WriteFile("Level geometry references: " + std::to_string(num));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_MODEL]) WriteFile("Prop model references: " + std::to_string(num));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_2]) WriteFile("Tree mesh surface 2 references: " + std::to_string(num));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_3]) WriteFile("Tree mesh surface 3 references: " + std::to_string(num));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_4]) WriteFile("Tree mesh surface 4 references: " + std::to_string(num));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_5]) WriteFile("Tree mesh surface 5 references: " + std::to_string(num));
		WriteFile("");
	}
	WriteFile("Surfaces end");
	WriteFile("");

	WriteFile("Static Batches begin");
	WriteFile("Count: " + std::to_string(aStaticBatches.size()));
	WriteFile("");
	for (auto& staticBatch : aStaticBatches) {
		WriteFile("nCenterId1: " + std::to_string(staticBatch.nCenterId1));
		WriteFile("nCenterId2: " + std::to_string(staticBatch.nCenterId2));
		WriteFile("nSurfaceId: " + std::to_string(staticBatch.nSurfaceId));
		WriteFile("nUnk: " + std::to_string(staticBatch.nUnk));
		WriteFile("vAbsoluteCenter.x: " + std::to_string(staticBatch.vAbsoluteCenter[0]));
		WriteFile("vAbsoluteCenter.y: " + std::to_string(staticBatch.vAbsoluteCenter[1]));
		WriteFile("vAbsoluteCenter.z: " + std::to_string(staticBatch.vAbsoluteCenter[2]));
		WriteFile("vRelativeCenter.x: " + std::to_string(staticBatch.vRelativeCenter[0]));
		WriteFile("vRelativeCenter.y: " + std::to_string(staticBatch.vRelativeCenter[1]));
		WriteFile("vRelativeCenter.z: " + std::to_string(staticBatch.vRelativeCenter[2]));
		WriteFile("");
	}
	WriteFile("Static Batches end");
	WriteFile("");

	WriteFile("Unknown Array 1 begin");
	WriteFile("Count: " + std::to_string(aUnknownArray1.size()));
	WriteFile("");
	for (auto& value : aUnknownArray1) {
		WriteFile(std::format("0x{:X}", value));
	}
	WriteFile("");
	WriteFile("Unknown Array 1 end");
	WriteFile("");

	WriteFile("Unknown Array 2 begin");
	WriteFile("Count: " + std::to_string(aUnknownArray2.size()));
	WriteFile("");
	for (auto& value : aUnknownArray2) {
		WriteFile("vPos.x: " + std::to_string(value.vPos[0]));
		WriteFile("vPos.y: " + std::to_string(value.vPos[1]));
		WriteFile("vPos.z: " + std::to_string(value.vPos[2]));
		WriteFile("fUnknown[0]: " + std::to_string(value.fValues[0]));
		WriteFile("fUnknown[1]: " + std::to_string(value.fValues[1]));
		WriteFile(std::format("nUnknown[0]: 0x{:X}", value.nValues[0]));
		WriteFile(std::format("nUnknown[1]: 0x{:X}", value.nValues[1]));
		WriteFile("");
	}
	WriteFile("Unknown Array 2 end");
	WriteFile("");

	WriteFile("Tree Meshes begin");
	WriteFile("Count: " + std::to_string(aTreeMeshes.size()));
	WriteFile("");
	for (auto& treeMesh : aTreeMeshes) {
		WriteFile("nUnknown1: " + std::to_string(treeMesh.nUnk1));
		WriteFile("nUnknown2: " + std::to_string(treeMesh.nUnk2Unused));
		WriteFile("nSurfaceId1: " + std::to_string(treeMesh.nSurfaceId1Unused));
		WriteFile("nSurfaceId2: " + std::to_string(treeMesh.nSurfaceId2));
		for (int j = 0; j < 19; j++) {
			WriteFile("fUnk[" + std::to_string(j) + "]: " + std::to_string(treeMesh.fUnk[j]));
		}
		if (nImportMapVersion >= 0x20002) {
			WriteFile("nMaterialId: " + std::to_string(treeMesh.foucData1[0]));
			WriteFile("foucData1[1]: " + std::to_string(treeMesh.foucData1[1]));
			WriteFile("foucData1[2]: " + std::to_string(treeMesh.foucData1[2]));
			WriteFile("foucData1[3]: " + std::to_string(treeMesh.foucData1[3]));
			WriteFile("foucData1[4]: " + std::to_string(treeMesh.foucData1[4]));
			WriteFile("foucData1[5]: " + std::to_string(treeMesh.foucData1[5]));
			WriteFile("foucData1[6]: " + std::to_string(treeMesh.foucData1[6]));
			WriteFile("foucData1[7]: " + std::to_string(treeMesh.foucData1[7]));
			WriteFile("foucData1[8]: " + std::to_string(treeMesh.foucData1[8]));
			WriteFile("foucData2[0]: " + std::to_string(treeMesh.foucData2[0]));
			WriteFile("foucData2[1]: " + std::to_string(treeMesh.foucData2[1]));
			WriteFile("foucData2[2]: " + std::to_string(treeMesh.foucData2[2]));
			WriteFile("nSomeId1: " + std::to_string(treeMesh.foucData2[3]));
			WriteFile(std::format("nSomeOffset1: 0x{:X}", treeMesh.foucData2[4]));
			WriteFile("nSomeId2: " + std::to_string(treeMesh.foucData2[5]));
			WriteFile(std::format("nSomeOffset2: 0x{:X}", treeMesh.foucData2[6]));
			WriteFile("nSomeId3: " + std::to_string(treeMesh.foucData2[7]));
			WriteFile(std::format("nSomeOffset3: 0x{:X}", treeMesh.foucData2[8]));
			WriteFile("nMaterialId2: " + std::to_string(treeMesh.foucData3[0]));
			WriteFile("foucData3[1]: " + std::to_string(treeMesh.foucData3[1]));
			WriteFile("nSomeId4: " + std::to_string(treeMesh.foucData3[2]));
			WriteFile(std::format("nSomeOffset4: 0x{:X}", (uint32_t)treeMesh.foucData3[3]));
			WriteFile("nSurfaceId3: " + std::to_string(treeMesh.nSurfaceIdsFouc[0]));
			WriteFile("nSurfaceId4: " + std::to_string(treeMesh.nSurfaceIdsFouc[1]));
			WriteFile("nSurfaceId5: " + std::to_string(treeMesh.nSurfaceIdsFouc[2]));
		}
		else {
			WriteFile("nSurfaceId3: " + std::to_string(treeMesh.nSurfaceId3));
			WriteFile("nSurfaceId4: " + std::to_string(treeMesh.nSurfaceId4));
			WriteFile("nSurfaceId5: " + std::to_string(treeMesh.nSurfaceId5));
			WriteFile("nIdInUnknownArray1: " + std::to_string(treeMesh.nIdInUnkArray1));
			WriteFile("nIdInUnknownArray2: " + std::to_string(treeMesh.nIdInUnkArray2));
			WriteFile("nMaterialId: " + std::to_string(treeMesh.nMaterialId));
		}
		WriteFile("");
	}
	WriteFile("Tree Meshes end");
	WriteFile("");

	WriteFile("Unknown Array 3 begin");
	WriteFile("Count: " + std::to_string(aUnknownArray3.size()));
	WriteFile("");
	for (auto& value : aUnknownArray3) {
		WriteFile(std::to_string(value));
	}
	WriteFile("");
	WriteFile("Unknown Array 3 end");
	WriteFile("");

	WriteFile("Models begin");
	WriteFile("Count: " + std::to_string(aModels.size()));
	WriteFile("");
	for (auto& model : aModels) {
		WriteFile("nUnknown1: " + std::to_string(model.nUnk));
		WriteFile("sName: " + model.sName);
		WriteFile("vCenter.x: " + std::to_string(model.vCenter[0]));
		WriteFile("vCenter.y: " + std::to_string(model.vCenter[1]));
		WriteFile("vCenter.z: " + std::to_string(model.vCenter[2]));
		WriteFile("vRadius.x: " + std::to_string(model.vRadius[0]));
		WriteFile("vRadius.y: " + std::to_string(model.vRadius[1]));
		WriteFile("vRadius.z: " + std::to_string(model.vRadius[2]));
		WriteFile("fRadius: " + std::to_string(model.fRadius)); // this is entirely skipped in the reader and instead calculated
		WriteFile("nNumSurfaces: " + std::to_string(model.aSurfaces.size()));
		for (auto& surface : model.aSurfaces) {
			WriteFile(std::to_string(surface));
		}
		WriteFile("");
	}
	WriteFile("Models end");
	WriteFile("");

	WriteFile("Objects begin");
	WriteFile("Count: " + std::to_string(aObjects.size()));
	WriteFile("");
	for (auto& object : aObjects) {
		WriteFile("sName: " + object.sName1);
		WriteFile("sUnknown: " + object.sName2);
		WriteFile(std::format("nFlags: 0x{:X}", object.nFlags));
		WriteFile("mMatrix: ");
		WriteFile(std::format("{}, {}, {}, {}", object.mMatrix[0], object.mMatrix[1], object.mMatrix[2], object.mMatrix[3]));
		WriteFile(std::format("{}, {}, {}, {}", object.mMatrix[4], object.mMatrix[5], object.mMatrix[6], object.mMatrix[7]));
		WriteFile(std::format("{}, {}, {}, {}", object.mMatrix[8], object.mMatrix[9], object.mMatrix[10], object.mMatrix[11]));
		WriteFile(std::format("{}, {}, {}, {}", object.mMatrix[12], object.mMatrix[13], object.mMatrix[14], object.mMatrix[15]));
		WriteFile("");
	}
	WriteFile("Objects end");
	WriteFile("");

	WriteFile("Bounding Boxes begin");
	WriteFile("Count: " + std::to_string(aBoundingBoxes.size()));
	WriteFile("");
	for (auto& bbox : aBoundingBoxes) {
		WriteFile("Model count: " + std::to_string(bbox.aModels.size()));
		for (auto& model : bbox.aModels) {
			WriteFile(std::to_string(model));
		}
		WriteFile("vCenter.x: " + std::to_string(bbox.vCenter[0]));
		WriteFile("vCenter.y: " + std::to_string(bbox.vCenter[1]));
		WriteFile("vCenter.z: " + std::to_string(bbox.vCenter[2]));
		WriteFile("vRadius.x: " + std::to_string(bbox.vRadius[0]));
		WriteFile("vRadius.y: " + std::to_string(bbox.vRadius[1]));
		WriteFile("vRadius.z: " + std::to_string(bbox.vRadius[2]));
		WriteFile("");
	}
	WriteFile("Bounding Boxes end");
	WriteFile("");

	WriteFile("Bounding Box Mesh Associations begin");
	WriteFile("Count: " + std::to_string(aBoundingBoxMeshAssoc.size()));
	WriteFile("");
	for (auto& assoc : aBoundingBoxMeshAssoc) {
		WriteFile("sName: " + assoc.sName);
		WriteFile("nIds[0]: " + std::to_string(assoc.nIds[0]));
		WriteFile("nIds[1]: " + std::to_string(assoc.nIds[1]));
		WriteFile("");
	}
	WriteFile("Bounding Box Mesh Associations end");
	WriteFile("");

	WriteFile("Compact Meshes begin");
	WriteFile("Group Count: " + std::to_string(nCompactMeshGroupCount));
	WriteFile("Count: " + std::to_string(aCompactMeshes.size()));
	WriteFile("");
	for (auto& mesh : aCompactMeshes) {
		WriteFile("sObjectName: " + mesh.sName1);
		WriteFile("sModelName: " + mesh.sName2);
		WriteFile(std::format("nFlags: 0x{:X}", mesh.nFlags));
		WriteFile("nGroup: " + std::to_string(mesh.nGroup));
		WriteFile("mMatrix: ");
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[0], mesh.mMatrix[1], mesh.mMatrix[2], mesh.mMatrix[3]));
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[4], mesh.mMatrix[5], mesh.mMatrix[6], mesh.mMatrix[7]));
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[8], mesh.mMatrix[9], mesh.mMatrix[10], mesh.mMatrix[11]));
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[12], mesh.mMatrix[13], mesh.mMatrix[14], mesh.mMatrix[15]));
		if (nImportMapVersion >= 0x20000) {
			WriteFile("nUnk1: " + std::to_string(mesh.nUnk1));
			WriteFile("nBBoxAssocId: " + std::to_string(mesh.nBBoxAssocId));
		}
		WriteFile("nNumLODs: " + std::to_string(mesh.aLODMeshIds.size()));
		for (auto unkValue : mesh.aLODMeshIds) {
			auto model = aModels[unkValue];
			WriteFile(std::to_string(unkValue) + " - " + model.sName);
		}
		WriteFile("");
	}
	WriteFile("Compact Meshes end");
	WriteFile("");

	for (auto& surface : aSurfaces) {
		if (!surface._nNumReferences) {
			WriteConsole("WARNING: Surface " + std::to_string(&surface - &aSurfaces[0]) + " goes unused! The game will not like this!!");
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		WriteConsole("Usage: FlatOut2W32Extractor_gcp.exe <filename>");
		return 0;
	}
	sFileName = argv[1];
	if (!ParseW32(argv[1])) {
		WriteConsole("Failed to load " + (std::string)argv[1] + "!");
	}
	else {
		WriteW32ToText();
		WriteW32(argv[1], !bConvertToFO1 && nImportMapVersion >= 0x20000);
	}
	return 0;
}
