#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <format>
#include <vector>
#include "assimp/Exporter.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

void WriteConsole(const std::string& str) {
	auto& out = std::cout;
	out << str;
	out << "\n";
	out.flush();
}

void WriteFile(const std::string& str) {
	static auto out = std::ofstream("nya.txt");
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
	int vertexCount;
	int vertexSize;
	int flags;
	float* data;
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
	int v37[7];
	float vAbsoluteCenter[3];
	float vRelativeCenter[3];
	int nNumStreamsUsed;
	uint32_t nStreamId[2];
	uint32_t nStreamOffset[2];
};
struct tStaticBatch {
	uint32_t nCenterId1;
	uint32_t nCenterId2;
	uint32_t nSurfaceId;
	uint32_t nUnk;
	float vAbsoluteCenter[3];
	float vRelativeCenter[3];
};
struct tUnknownStructure {
	float vPos[3];
	float fValues[2];
	int nValues[2];
};
struct tTreeMesh {
	int nUnk1;
	int nUnk2;
	int nSurfaceId;
	int nUnk3;
	float fUnk[19];
	int nSurfaceId2;
	int nSurfaceId3;
	int nSurfaceId4;
	int nUnk4;
	int nUnk5;
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
	uint32_t identifier;
	std::string sName1;
	std::string sName2;
	uint32_t nFlags;
	float mMatrix[4*4];
};
struct tCompactMesh {
	uint32_t identifier;
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
int nMapVersion;
int nSomeMapValue = 1; // always 1 in FO2, doesn't exist in FO1
std::vector<tVertexBuffer> aVertexBuffers;
std::vector<tIndexBuffer> aIndexBuffers;
std::vector<tVegVertexBuffer> aVegVertexBuffers;
std::vector<tMaterial> aMaterials;
std::vector<tSurface> aSurfaces;
std::vector<tStaticBatch> aStaticBatches;
std::vector<int> aUnknownArray1;
std::vector<tUnknownStructure> aUnknownArray2;
std::vector<tTreeMesh> aTreeMeshes;
std::vector<float> aUnknownArray3;
std::vector<tModel> aModels;
std::vector<tObject> aObjects;
std::vector<tCompactMesh> aCompactMeshes;
std::vector<tBoundingBox> aBoundingBoxes;
std::vector<tBoundingBoxMeshAssoc> aBoundingBoxMeshAssoc;
uint32_t nMeshGroupCount;

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
		ReadFromFile(file, surface.v37, 28);

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

		if (mapVersion >= 0x20000) {
			ReadFromFile(file, staticBatch.vAbsoluteCenter, 12);
			ReadFromFile(file, staticBatch.vRelativeCenter, 12);
		}
		else {
			ReadFromFile(file, &staticBatch.nUnk, 4);
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
		ReadFromFile(file, &treeMesh.nUnk3, 4);
		ReadFromFile(file, treeMesh.fUnk, sizeof(treeMesh.fUnk));
		ReadFromFile(file, &treeMesh.nSurfaceId2, 4);
		ReadFromFile(file, &treeMesh.nSurfaceId3, 4);
		ReadFromFile(file, &treeMesh.nSurfaceId4, 4);
		ReadFromFile(file, &treeMesh.nUnk4, 4);
		ReadFromFile(file, &treeMesh.nUnk5, 4);
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
		ReadFromFile(file, &model.identifier, 4); // BMOD
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
		object.sName1 = ReadStringFromFile(file);
		object.sName2 = ReadStringFromFile(file);
		ReadFromFile(file, &object.nFlags, 4);
		ReadFromFile(file, object.mMatrix, sizeof(object.mMatrix));
		aObjects.push_back(object);
	}
	return true;
}

bool ParseW32CompactMeshes(std::ifstream& file, uint32_t mapVersion) {
	uint32_t meshCount;
	ReadFromFile(file, &meshCount, 4);
	ReadFromFile(file, &nMeshGroupCount, 4);
	aCompactMeshes.reserve(meshCount);
	for (int i = 0; i < meshCount; i++) {
		tCompactMesh compactMesh;
		ReadFromFile(file, &compactMesh.identifier, 4);
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

void WriteMaterialToFile(std::ofstream& file, tMaterial& material) {
	file.write((char*)&material.identifier, 4);
	file.write(material.sName.c_str(), material.sName.length() + 1);
	file.write((char*)&material.nAlpha, 4);
	file.write((char*)&material.v92, 4);
	file.write((char*)&material.nNumTextures, 4);
	file.write((char*)&material.v73, 4);
	file.write((char*)&material.v75, 4);
	file.write((char*)&material.v74, 4);
	file.write((char*)&material.v108, sizeof(material.v108));
	file.write((char*)&material.v109, sizeof(material.v109));
	file.write((char*)&material.v98, sizeof(material.v98));
	file.write((char*)&material.v99, sizeof(material.v99));
	file.write((char*)&material.v100, sizeof(material.v100));
	file.write((char*)&material.v101, sizeof(material.v101));
	file.write((char*)&material.v102, 4);
	for (int i = 0; i < 3; i++) {
		file.write(material.sTextureNames[i].c_str(), material.sTextureNames[i].length() + 1);
	}
}

void WriteVertexBufferToFile(std::ofstream& file, tVertexBuffer& buf) {
	int type = 1;
	file.write((char*)&type, 4);
	file.write((char*)&buf.field_0, 4);
	file.write((char*)&buf.vertexCount, 4);
	file.write((char*)&buf.vertexSize, 4);
	file.write((char*)&buf.flags, 4);
	file.write((char*)&buf.data, buf.vertexCount * buf.vertexSize);
}

void WriteIndexBufferToFile(std::ofstream& file, tIndexBuffer& buf) {
	int type = 2;
	file.write((char*)&type, 4);
	file.write((char*)&buf.field_0, 4);
	file.write((char*)&buf.indexCount, 4);
	file.write((char*)&buf.data, buf.indexCount * 2);
}

void WriteVegVertexBufferToFile(std::ofstream& file, tVegVertexBuffer& buf) {
	int type = 3;
	file.write((char*)&type, 4);
	file.write((char*)&buf.field_0, 4);
	file.write((char*)&buf.vertexCount, 4);
	file.write((char*)&buf.vertexSize, 4);
	file.write((char*)&buf.data, buf.vertexCount * buf.vertexSize);
}

void WriteW32(const std::string& fileName, bool isFO2) {
	std::ofstream file(fileName + "_out.w32", std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	uint32_t mapVersion = isFO2 ? 0x20001 : 0x10005;
	file.write((char*)&mapVersion, 4);
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
				continue;
			}
		}
		for (auto& buf : aVegVertexBuffers) {
			if (buf.id == i) {
				WriteVegVertexBufferToFile(file, buf);
				continue;
			}
		}
		for (auto& buf : aIndexBuffers) {
			if (buf.id == i) {
				WriteIndexBufferToFile(file, buf);
				continue;
			}
		}
	}
}

bool ParseW32(const std::string& fileName) {
	std::ifstream fin(fileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	ReadFromFile(fin, &nMapVersion, 4);
	if (nMapVersion > 0x20000) ReadFromFile(fin, &nSomeMapValue, 4);

	if (!ParseW32Materials(fin)) return false;
	if (!ParseW32Streams(fin)) return false;
	if (!ParseW32Surfaces(fin, nMapVersion)) return false;
	if (!ParseW32StaticBatches(fin, nMapVersion)) return false;

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

	if (nMapVersion >= 0x10004) {
		for (int i = 0; i < 16; i++) {
			float value;
			ReadFromFile(fin, &value, sizeof(value));
			aUnknownArray3.push_back(value);
		}
	}

	if (!ParseW32Models(fin)) return false;
	if (!ParseW32Objects(fin)) return false;

	if (nMapVersion >= 0x20000) {
		if (!ParseW32BoundingBoxes(fin)) return false;
		if (!ParseW32BoundingBoxMeshAssoc(fin)) return false;
		if (!ParseW32CompactMeshes(fin, nMapVersion)) return false;
	}
	else {
		if (!ParseW32CompactMeshes(fin, nMapVersion)) return false;
	}
	return true;
}

void WriteW32ToText() {
	WriteFile(std::format("nMapVersion: 0x{:X} {}", nMapVersion, GetMapVersion(nMapVersion)));
	if (nMapVersion > 0x20000) {
		WriteFile("nSomeMapValue: " + std::to_string(nSomeMapValue));
	}

	WriteFile("Materials begin");
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
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		WriteConsole("Usage: FlatOut2W32Extractor_gcp.exe <filename>");
		return 0;
	}
	if (!ParseW32(argv[1])) {
		WriteConsole("Failed to load " + (std::string)argv[1] + "!");
	}
	else {
		WriteW32(argv[1], false);
	}
	WriteW32ToText();
	return 0;
}
