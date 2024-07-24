#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <format>
#include <vector>
#include "assimp/Exporter.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

std::string sFileName;

bool bDisableProps = true;

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

// vertex buffer data:
// 1 2 3 always coords
// 0x202 flag seems to be the terrain
// 0x152 -> 4 5 6 are normals, 7 unknown, 8 and 9 are UV coords
// 0x212 -> 4 5 6 are normals, 7 8 9 10 unknown

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

struct tVertexBuffer {
	int id;
	int field_0;
	uint32_t vertexCount;
	uint32_t vertexSize;
	uint32_t flags;
	float* data;

	uint32_t _vertexSizeBeforeFO1 = 0;
};
struct tIndexBuffer {
	int id;
	int field_0;
	int indexCount;
	uint16_t* data;
};
struct tVegVertexBuffer {
	int id;
	int field_0;
	int vertexCount;
	int vertexSize;
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
	int nUnk2;
	int nSurfaceId;
	int nIdInSomeMemoryArray;
	float fUnk[19];
	int nSurfaceId2;
	int nSurfaceId3;
	int nSurfaceId4;
	int nIdInUnkArray1;
	int nIdInUnkArray2;
	int nMaterialId;
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
	std::vector<int> aUnkArray;
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

bool ParseW32Materials(std::ifstream& file) {
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
	uint32_t numStreams;
	ReadFromFile(file, &numStreams, 4);
	for (int i = 0; i < numStreams; i++) {
		int dataType;
		ReadFromFile(file, &dataType, 4);
		if (dataType == 1) {
			tVertexBuffer buf;
			ReadFromFile(file, &buf.field_0, 4);
			ReadFromFile(file, &buf.vertexCount, 4);
			ReadFromFile(file, &buf.vertexSize, 4);
			ReadFromFile(file, &buf.flags, 4);

			auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));
			auto vertexData = new float[dataSize];
			ReadFromFile(file, vertexData, dataSize * sizeof(float));

			buf.id = i;
			buf.data = vertexData;
			aVertexBuffers.push_back(buf);
		}
		else if (dataType == 2) {
			tIndexBuffer buf;

			ReadFromFile(file, &buf.field_0, 4);
			ReadFromFile(file, &buf.indexCount, 4);

			auto dataSize = buf.indexCount;
			auto indexData = new uint16_t[dataSize];
			ReadFromFile(file, indexData, dataSize * sizeof(uint16_t));

			buf.id = i;
			buf.data = indexData;
			aIndexBuffers.push_back(buf);
		}
		else if (dataType == 3) {
			tVegVertexBuffer buf;

			ReadFromFile(file, &buf.field_0, 4);
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
	uint32_t numStaticBatches;
	ReadFromFile(file, &numStaticBatches, 4);
	aStaticBatches.reserve(numStaticBatches);
	for (int i = 0; i < numStaticBatches; i++) {
		tStaticBatch staticBatch;
		ReadFromFile(file, &staticBatch.nCenterId1, 4);
		ReadFromFile(file, &staticBatch.nCenterId2, 4);
		ReadFromFile(file, &staticBatch.nSurfaceId, 4);
		if (staticBatch.nSurfaceId >= aSurfaces.size()) return false;

		if (mapVersion >= 0x20000) {
			ReadFromFile(file, staticBatch.vAbsoluteCenter, 12);
			ReadFromFile(file, staticBatch.vRelativeCenter, 12);

			// backwards compatibility
			memcpy(aSurfaces[staticBatch.nSurfaceId].vAbsoluteCenter, staticBatch.vAbsoluteCenter, 12);
			memcpy(aSurfaces[staticBatch.nSurfaceId].vRelativeCenter, staticBatch.vRelativeCenter, 12);
		}
		else {
			ReadFromFile(file, &staticBatch.nUnk, 4); // always 0?

			// forwards compatibility
			memcpy(staticBatch.vAbsoluteCenter, aSurfaces[staticBatch.nSurfaceId].vAbsoluteCenter, 12);
			memcpy(staticBatch.vRelativeCenter, aSurfaces[staticBatch.nSurfaceId].vRelativeCenter, 12);
		}

		aStaticBatches.push_back(staticBatch);
	}
	return true;
}

bool ParseW32TreeMeshes(std::ifstream& file) {
	uint32_t treeMeshCount;
	ReadFromFile(file, &treeMeshCount, 4);
	aTreeMeshes.reserve(treeMeshCount);
	for (int i = 0; i < treeMeshCount; i++) {
		tTreeMesh treeMesh;
		ReadFromFile(file, &treeMesh.nUnk1, 4);
		ReadFromFile(file, &treeMesh.nUnk2, 4);
		ReadFromFile(file, &treeMesh.nSurfaceId, 4);
		ReadFromFile(file, &treeMesh.nIdInSomeMemoryArray, 4);
		ReadFromFile(file, treeMesh.fUnk, sizeof(treeMesh.fUnk));
		ReadFromFile(file, &treeMesh.nSurfaceId2, 4);
		ReadFromFile(file, &treeMesh.nSurfaceId3, 4);
		ReadFromFile(file, &treeMesh.nSurfaceId4, 4);
		ReadFromFile(file, &treeMesh.nIdInUnkArray1, 4);
		ReadFromFile(file, &treeMesh.nIdInUnkArray2, 4);
		ReadFromFile(file, &treeMesh.nMaterialId, 4);
		aTreeMeshes.push_back(treeMesh);
	}
	return true;
}

bool ParseW32Models(std::ifstream& file) {
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
		}
		aModels.push_back(model);
	}
	return true;
}

bool ParseW32Objects(std::ifstream& file) {
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
			uint32_t nUnkCount, nBBoxIndex;
			ReadFromFile(file, &nUnkCount, 4);
			ReadFromFile(file, &nBBoxIndex, 4);
			compactMesh.aUnkArray.push_back(nBBoxIndex);
		}
		else {
			uint32_t nUnkCount;
			ReadFromFile(file, &nUnkCount, 4);
			for (int j = 0; j < nUnkCount; j++) {
				uint32_t nUnkValue;
				ReadFromFile(file, &nUnkValue, 4);
				compactMesh.aUnkArray.push_back(nUnkValue);
			}
		}
		aCompactMeshes.push_back(compactMesh);
	}
	return true;
}

bool ParseW32BoundingBoxes(std::ifstream& file) {
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

void WriteVertexBufferToFile(std::ofstream& file, tVertexBuffer& buf) {
	bool bRemoveNormals = false;
	if (nExportMapVersion < 0x20000 && nExportMapVersion != nImportMapVersion && (buf.flags & 0x10) != 0) {
		buf._vertexSizeBeforeFO1 = buf.vertexSize;
		buf.flags -= 0x10;
		buf.vertexSize -= 0xC;
		bRemoveNormals = true;
	}

	int type = 1;
	file.write((char*)&type, 4);
	file.write((char*)&buf.field_0, 4);
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
	file.write((char*)&buf.field_0, 4);
	file.write((char*)&buf.indexCount, 4);
	file.write((char*)buf.data, buf.indexCount * 2);
}

void WriteVegVertexBufferToFile(std::ofstream& file, const tVegVertexBuffer& buf) {
	int type = 3;
	file.write((char*)&type, 4);
	file.write((char*)&buf.field_0, 4);
	file.write((char*)&buf.vertexCount, 4);
	file.write((char*)&buf.vertexSize, 4);
	file.write((char*)buf.data, buf.vertexCount * buf.vertexSize);
}

tVertexBuffer* FindVertexBuffer(int id) {
	for (auto& buf : aVertexBuffers) {
		if (buf.id == id) return &buf;
	}
	return nullptr;
}

void WriteSurfaceToFile(std::ofstream& file, tSurface& surface) {
	if (nExportMapVersion < 0x20000 && nExportMapVersion != nImportMapVersion && (surface.nFlags & 0x10) != 0) {
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
	file.write((char*)&treeMesh.nUnk2, 4);
	file.write((char*)&treeMesh.nSurfaceId, 4);
	file.write((char*)&treeMesh.nIdInSomeMemoryArray, 4);
	file.write((char*)treeMesh.fUnk, sizeof(treeMesh.fUnk));
	file.write((char*)&treeMesh.nSurfaceId2, 4);
	file.write((char*)&treeMesh.nSurfaceId3, 4);
	file.write((char*)&treeMesh.nSurfaceId4, 4);
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
	int numUnk = mesh.aUnkArray.size();
	file.write((char*)&numUnk, 4);
	if (nExportMapVersion < 0x20000 && nImportMapVersion != nExportMapVersion) {
		int tmp = 0;
		file.write((char*)&tmp, 4);
	}
	else {
		for (auto value : mesh.aUnkArray) {
			file.write((char*)&value, 4);
		}
	}
}

void WriteW32(const std::string& fileName, bool isFO2) {
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
		//if (nExportMapVersion < 0x20000 && nImportMapVersion != nExportMapVersion) {
		//	auto tmp = (uint8_t *) &data;
		//	tmp[3] = 0xFD; // FO2 has 0x02 and FO1 has 0xFD i believe?
		//}
		file.write((char*)&data, 4);
	}

	// tree rendering related crash:
	// +1A0 off of smth with vft 00662968
	// written to by 004C1393 and 004C16AB
	// 004C1393 is terrain, 004C16AB is plants

	uint32_t unk2Count = aUnknownArray2.size();
	file.write((char*)&unk2Count, 4);
	for (auto& data : aUnknownArray2) {
		file.write((char*)data.vPos, sizeof(data.vPos));
		file.write((char*)data.fValues, sizeof(data.fValues));
		//if (nExportMapVersion < 0x20000 && nImportMapVersion != nExportMapVersion) {
		//	auto tmp = (uint8_t *) &data.nValues[1];
		//	tmp[3] = 0xFD; // FO2 has 0x02 and FO1 has 0xFD i believe?
		//}
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

	if (bDisableProps) {
		uint32_t tmpCount = 0;
		file.write((char*)&tmpCount, 4); // models
		file.write((char*)&tmpCount, 4); // objects
		if (nImportMapVersion >= 0x20000) {
			file.write((char*)&tmpCount, 4); // bbox
			file.write((char*)&tmpCount, 4); // bbox assoc
		}
		file.write((char*)&tmpCount, 4); // compactmesh groups
		file.write((char*)&tmpCount, 4); // compactmesh
	}
	else {
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

		if (nImportMapVersion >= 0x20000) {
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

	{
		uint32_t someCount;
		ReadFromFile(fin, &someCount, 4);
		for (int i = 0; i < someCount; i++) {
			int someValue;
			ReadFromFile(fin, &someValue, 4);
			aUnknownArray1.push_back(someValue);
		}
	}

	{
		uint32_t someCount;
		ReadFromFile(fin, &someCount, 4);
		for (int i = 0; i < someCount; i++) {
			tUnknownStructure unkStruct;
			ReadFromFile(fin, unkStruct.vPos, sizeof(unkStruct.vPos));
			ReadFromFile(fin, unkStruct.fValues, sizeof(unkStruct.fValues));
			ReadFromFile(fin, unkStruct.nValues, sizeof(unkStruct.nValues));
			aUnknownArray2.push_back(unkStruct);
		}
	}

	if (!ParseW32TreeMeshes(fin)) return false;

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
	return true;
}

void WriteW32ToText() {
	WriteFile(std::format("nMapVersion: 0x{:X} {}", nImportMapVersion, GetMapVersion(nImportMapVersion)));
	if (nImportMapVersion > 0x20000) {
		WriteFile("nSomeMapValue: " + std::to_string(nSomeMapValue));
	}

	WriteFile("");
	WriteFile("Materials begin");
	WriteFile("Count: " + std::to_string(aMaterials.size()));
	WriteFile("");
	for (auto& material : aMaterials) {
		WriteFile("Name: " + material.sName);
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
		for (auto& buf : aVertexBuffers) {
			if (buf.id == i) {
				WriteFile("Vertex buffer");
				WriteFile(std::format("Vertex Size: {}", buf.vertexSize));
				WriteFile(std::format("Vertex Count: {}", buf.vertexCount));
				WriteFile(std::format("nFlags: 0x{:X}", buf.flags));
			}
		}
		for (auto& buf : aVegVertexBuffers) {
			if (buf.id == i) {
				WriteFile("Vegetation vertex buffer");
			}
		}
		for (auto& buf : aIndexBuffers) {
			if (buf.id == i) {
				WriteFile("Index buffer");
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
		WriteFile("nNumStreamsUsed: " + std::to_string(surface.nNumStreamsUsed));
		for (int j = 0; j < surface.nNumStreamsUsed; j++) {
			WriteFile("nStreamId: " + std::to_string(surface.nStreamId[j]));
			WriteFile(std::format("nStreamOffset: 0x{:X}", surface.nStreamOffset[j]));
		}
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
		WriteFile("nUnknown2: " + std::to_string(treeMesh.nUnk2));
		WriteFile("nSurfaceId: " + std::to_string(treeMesh.nSurfaceId));
		WriteFile("nIdInSomeMemoryArray: " + std::to_string(treeMesh.nIdInSomeMemoryArray));
		for (int j = 0; j < 19; j++) {
			WriteFile("fUnk[" + std::to_string(j) + "]: " + std::to_string(treeMesh.fUnk[j]));
		}
		WriteFile("nSurfaceId2: " + std::to_string(treeMesh.nSurfaceId2));
		WriteFile("nSurfaceId3: " + std::to_string(treeMesh.nSurfaceId3));
		WriteFile("nSurfaceId4: " + std::to_string(treeMesh.nSurfaceId4));
		WriteFile("nIdInUnknownArray1: " + std::to_string(treeMesh.nIdInUnkArray1));
		WriteFile("nIdInUnknownArray2: " + std::to_string(treeMesh.nIdInUnkArray2));
		WriteFile("nMaterialId: " + std::to_string(treeMesh.nMaterialId));
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
		WriteFile("nUnknownCount: " + std::to_string(mesh.aUnkArray.size()));
		for (auto unkValue : mesh.aUnkArray) {
			WriteFile(std::to_string(unkValue));
		}
		WriteFile("");
	}
	WriteFile("Compact Meshes end");
	WriteFile("");
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
		WriteW32(argv[1], false);
	}
	WriteW32ToText();
	return 0;
}
