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
	int vertexCount;
	int vertexSize;
	int flags;
	float* data;
};
struct tIndexBuffer {
	int id;
	int indexCount;
	uint16_t* data;
};
struct tVegVertexBuffer {
	int id;
	int vertexCount;
	int vertexSize;
	float* data;
};
struct tMaterial {
	uint32_t identifier; // MATC
	std::string name;
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
	std::string textureNames[3];
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
	int unk;
	std::string name;
	float vCenter[3];
	float vRadius[3];
	float fRadius;
	std::vector<int> aSurfaces;
};
struct tObject {
	uint32_t identifier;
	std::string name1;
	std::string name2;
	uint32_t nFlags;
	float mMatrix[4*4];
};
struct tCompactMesh {
	uint32_t id;
	std::string name1;
	std::string name2;
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
int nSomeMapValue; // always 1 in FO2, doesn't exist in FO1
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
	int numMaterials;
	ReadFromFile(file, &numMaterials, 4);
	for (int i = 0; i < numMaterials; i++) {
		uint32_t identifier;
		ReadFromFile(file, &identifier, 4);
		if (identifier != 0x4354414D) return false; // "MATC"

		auto textureName = ReadStringFromFile(file);
		int alpha;
		int v92;
		int nNumTextures;
		int v73;
		int v75;
		int v74;
		ReadFromFile(file, &alpha, 4);
		ReadFromFile(file, &v92, 4);
		ReadFromFile(file, &nNumTextures, 4);
		ReadFromFile(file, &v73, 4);
		ReadFromFile(file, &v75, 4);
		ReadFromFile(file, &v74, 4);

		int v108[3];
		int v109[3];
		int v98[4];
		int v99[4];
		int v100[4];
		int v101[4];
		int v102;
		ReadFromFile(file, v108, 12);
		ReadFromFile(file, v109, 12);
		ReadFromFile(file, v98, 16);
		ReadFromFile(file, v99, 16);
		ReadFromFile(file, v100, 16);
		ReadFromFile(file, v101, 16);
		ReadFromFile(file, &v102, 4);

		auto textureName2 = ReadStringFromFile(file);
		auto textureName3 = ReadStringFromFile(file);
		auto textureName4 = ReadStringFromFile(file);

		tMaterial mat;
		mat.identifier = 0x4354414D;
		mat.nAlpha = alpha;
		mat.v92 = v92;
		mat.nNumTextures = nNumTextures;
		mat.v73 = v73;
		mat.v75 = v75;
		mat.v74 = v74;
		memcpy(mat.v108, v108, sizeof(v108));
		memcpy(mat.v109, v109, sizeof(v109));
		memcpy(mat.v98, v98, sizeof(v98));
		memcpy(mat.v99, v99, sizeof(v99));
		memcpy(mat.v100, v100, sizeof(v100));
		memcpy(mat.v101, v101, sizeof(v101));
		mat.v102 = v102;
		mat.name = textureName;
		mat.textureNames[0] = textureName2;
		mat.textureNames[1] = textureName3;
		mat.textureNames[2] = textureName4;
		aMaterials.push_back(mat);
	}
	return true;
}

bool ParseW32Streams(std::ifstream& file) {
	int numStreams;
	ReadFromFile(file, &numStreams, 4);
	for (int i = 0; i < numStreams; i++) {
		int dataType;
		ReadFromFile(file, &dataType, 4);
		if (dataType == 1) {
			// 0x202 flag seems to be the terrain

			int field_0;
			int vertexCount;
			int vertexSize;
			int flags;
			ReadFromFile(file, &field_0, 4);
			ReadFromFile(file, &vertexCount, 4);
			ReadFromFile(file, &vertexSize, 4);
			ReadFromFile(file, &flags, 4);

			auto dataSize = vertexCount * (vertexSize / sizeof(float));
			auto vertexData = new float[dataSize];
			ReadFromFile(file, vertexData, dataSize * sizeof(float));

			tVertexBuffer buf;
			buf.vertexCount = vertexCount;
			buf.vertexSize = vertexSize;
			buf.flags = flags;
			buf.id = i;
			buf.data = vertexData;
			aVertexBuffers.push_back(buf);
		}
		else if (dataType == 2) {
			int field_0;
			int valueCount;
			ReadFromFile(file, &field_0, 4);
			ReadFromFile(file, &valueCount, 4);

			auto dataSize = valueCount;
			auto indexData = new uint16_t[dataSize];
			ReadFromFile(file, indexData, dataSize * sizeof(uint16_t));

			tIndexBuffer buf;
			buf.indexCount = valueCount;
			buf.id = i;
			buf.data = indexData;
			aIndexBuffers.push_back(buf);
		}
		else if (dataType == 3) {
			int field_0;
			int valueCount;
			int valueSize;
			ReadFromFile(file, &field_0, 4);
			ReadFromFile(file, &valueCount, 4);
			ReadFromFile(file, &valueSize, 4);

			auto dataSize = valueCount * (valueSize / sizeof(float));
			auto data = new float[dataSize];
			ReadFromFile(file, data, dataSize * sizeof(float));

			tVegVertexBuffer buf;
			buf.vertexCount = valueCount;
			buf.vertexSize = valueSize;
			buf.id = i;
			buf.data = data;
			aVegVertexBuffers.push_back(buf);
		}
		else {
			WriteFile("Unknown type " + std::to_string(dataType));
			return false;
		}
	}
	return true;
}

bool ParseW32Surfaces(std::ifstream& file, int mapVersion) {
	uint32_t nSurfaceCount;
	ReadFromFile(file, &nSurfaceCount, 4);
	for (int i = 0; i < nSurfaceCount; i++) {
		int v37[7];
		ReadFromFile(file, v37, 28);

		float vAbsoluteCenter[3];
		float vRelativeCenter[3];
		if (mapVersion < 0x20000) {
			ReadFromFile(file, vAbsoluteCenter, 12);
			ReadFromFile(file, vRelativeCenter, 12);
		}

		uint32_t nStreamId[2];
		uint32_t nStreamOffset[2];

		int nNumStreamsUsed;
		ReadFromFile(file, &nNumStreamsUsed, 4);
		if (nNumStreamsUsed > 2) return false;

		for (int j = 0; j < nNumStreamsUsed; j++) {
			ReadFromFile(file, &nStreamId[j], 4);
			ReadFromFile(file, &nStreamOffset[j], 4);
		}

		tSurface surface;
		surface.nNumStreamsUsed = nNumStreamsUsed;
		memcpy(surface.v37, v37, sizeof(v37));
		memcpy(surface.nStreamId, nStreamId, sizeof(nStreamId));
		memcpy(surface.nStreamOffset, nStreamOffset, sizeof(nStreamOffset));
		memcpy(surface.vAbsoluteCenter, vAbsoluteCenter, sizeof(vAbsoluteCenter));
		memcpy(surface.vRelativeCenter, vRelativeCenter, sizeof(vRelativeCenter));
		aSurfaces.push_back(surface);
	}
	return true;
}

bool ParseW32StaticBatches(std::ifstream& file, int mapVersion) {
	uint32_t numStaticBatches;
	ReadFromFile(file, &numStaticBatches, 4);
	for (int i = 0; i < numStaticBatches; i++) {
		uint32_t nCenterId1, nCenterId2, nSurfaceId;
		ReadFromFile(file, &nCenterId1, 4);
		ReadFromFile(file, &nCenterId2, 4);
		ReadFromFile(file, &nSurfaceId, 4);

		float vAbsoluteCenter[3];
		float vRelativeCenter[3];
		uint32_t nUnk;

		if (mapVersion >= 0x20000) {
			ReadFromFile(file, vAbsoluteCenter, 12);
			ReadFromFile(file, vRelativeCenter, 12);
		}
		else {
			ReadFromFile(file, &nUnk, 4);
		}

		tStaticBatch staticBatch;
		staticBatch.nCenterId1 = nCenterId1;
		staticBatch.nCenterId2 = nCenterId2;
		staticBatch.nSurfaceId = nSurfaceId;
		staticBatch.nUnk = nUnk;
		memcpy(staticBatch.vAbsoluteCenter, vAbsoluteCenter, sizeof(vAbsoluteCenter));
		memcpy(staticBatch.vRelativeCenter, vRelativeCenter, sizeof(vRelativeCenter));
		aStaticBatches.push_back(staticBatch);
	}
	return true;
}

bool ParseW32TreeMeshes(std::ifstream& file) {
	uint32_t treeMeshCount;
	ReadFromFile(file, &treeMeshCount, 4);
	for (int i = 0; i < treeMeshCount; i++) {
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
		ReadFromFile(file, &nUnk1, sizeof(nUnk1));
		ReadFromFile(file, &nUnk2, sizeof(nUnk2));
		ReadFromFile(file, &nSurfaceId, sizeof(nSurfaceId));
		ReadFromFile(file, &nUnk3, sizeof(nUnk3));
		ReadFromFile(file, fUnk, sizeof(fUnk));
		ReadFromFile(file, &nSurfaceId2, sizeof(nSurfaceId2));
		ReadFromFile(file, &nSurfaceId3, sizeof(nSurfaceId3));
		ReadFromFile(file, &nSurfaceId4, sizeof(nSurfaceId4));
		ReadFromFile(file, &nUnk4, sizeof(nUnk4));
		ReadFromFile(file, &nUnk5, sizeof(nUnk5));
		ReadFromFile(file, &nMaterialId, sizeof(nMaterialId));

		tTreeMesh treeMesh;
		treeMesh.nUnk1 = nUnk1;
		treeMesh.nUnk2 = nUnk2;
		treeMesh.nSurfaceId = nSurfaceId;
		treeMesh.nUnk3 = nUnk3;
		memcpy(treeMesh.fUnk, fUnk, sizeof(fUnk));
		treeMesh.nSurfaceId2 = nSurfaceId2;
		treeMesh.nSurfaceId3 = nSurfaceId3;
		treeMesh.nSurfaceId4 = nSurfaceId4;
		treeMesh.nUnk4 = nUnk4;
		treeMesh.nUnk5 = nUnk5;
		treeMesh.nMaterialId = nMaterialId;
		aTreeMeshes.push_back(treeMesh);
	}
	return true;
}

bool ParseW32Models(std::ifstream& file) {
	uint32_t modelCount;
	ReadFromFile(file, &modelCount, 4);
	for (int i = 0; i < modelCount; i++) {
		int id;
		ReadFromFile(file, &id, sizeof(id)); // BMOD
		int unk;
		ReadFromFile(file, &unk, sizeof(unk));
		auto name = ReadStringFromFile(file);
		float vCenter[3];
		float vRadius[3];
		float fRadius;
		ReadFromFile(file, vCenter, sizeof(vCenter));
		ReadFromFile(file, vRadius, sizeof(vRadius));
		ReadFromFile(file, &fRadius, sizeof(fRadius));
		int numSurfaces;
		ReadFromFile(file, &numSurfaces, sizeof(numSurfaces));

		tModel model;
		model.identifier = id;
		model.unk = unk;
		model.name = name;
		memcpy(model.vCenter, vCenter, sizeof(vCenter));
		memcpy(model.vRadius, vRadius, sizeof(vRadius));
		model.fRadius = fRadius;
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
	for (int i = 0; i < objectCount; i++) {
		uint32_t id;
		ReadFromFile(file, &id, sizeof(id));
		auto name1 = ReadStringFromFile(file);
		auto name2 = ReadStringFromFile(file);
		int nFlags;
		ReadFromFile(file, &nFlags, sizeof(nFlags));
		float mMatrix[4*4];
		ReadFromFile(file, &mMatrix, sizeof(mMatrix));

		tObject object;
		object.identifier = id;
		object.name1 = name1;
		object.name2 = name2;
		object.nFlags = nFlags;
		memcpy(object.mMatrix, mMatrix, sizeof(mMatrix));
		aObjects.push_back(object);
	}
	return true;
}

bool ParseW32CompactMeshes(std::ifstream& file, uint32_t mapVersion) {
	uint32_t meshCount;
	ReadFromFile(file, &meshCount, 4);
	ReadFromFile(file, &nMeshGroupCount, 4);
	for (int i = 0; i < meshCount; i++) {
		uint32_t id;
		ReadFromFile(file, &id, sizeof(id));
		auto name = ReadStringFromFile(file);
		auto name2 = ReadStringFromFile(file);
		uint32_t nFlags;
		int nGroup;
		ReadFromFile(file, &nFlags, 4);
		ReadFromFile(file, &nGroup, 4);
		float mMatrix[4*4];
		ReadFromFile(file, &mMatrix, sizeof(mMatrix));

		tCompactMesh compactMesh;
		compactMesh.id = id;
		compactMesh.name1 = name;
		compactMesh.name2 = name2;
		compactMesh.nFlags = nFlags;
		compactMesh.nGroup = nGroup;
		if (mapVersion >= 0x20000) {
			uint32_t nUnkCount, nBBoxIndex;
			ReadFromFile(file, &nUnkCount, 4);
			ReadFromFile(file, &nBBoxIndex, 4);
			compactMesh.aUnkArray.push_back(nBBoxIndex);
		}
		else {
			uint32_t nUnkCount;
			ReadFromFile(file, &nUnkCount, 4);
		}

		memcpy(compactMesh.mMatrix, mMatrix, sizeof(mMatrix));
		aCompactMeshes.push_back(compactMesh);
	}
	return true;
}

bool ParseW32BoundingBoxes(std::ifstream& file) {
	uint32_t boundingBoxCount;
	ReadFromFile(file, &boundingBoxCount, 4);
	for (int i = 0; i < boundingBoxCount; i++) {
		tBoundingBox boundingBox;

		uint32_t modelCount;
		ReadFromFile(file, &modelCount, 4);
		for (int j = 0; j < modelCount; j++) {
			uint32_t modelId;
			ReadFromFile(file, &modelId, 4);
			boundingBox.aModels.push_back(modelId);
		}

		float vCenter[3];
		float vRadius[3];
		ReadFromFile(file, vCenter, sizeof(vCenter));
		ReadFromFile(file, vRadius, sizeof(vRadius));

		memcpy(boundingBox.vCenter, vCenter, sizeof(vCenter));
		memcpy(boundingBox.vRadius, vRadius, sizeof(vRadius));
		aBoundingBoxes.push_back(boundingBox);
	}
	return true;
}

bool ParseW32BoundingBoxMeshAssoc(std::ifstream& file) {
	uint32_t assocCount;
	ReadFromFile(file, &assocCount, 4);
	for (int i = 0; i < assocCount; i++) {
		auto name = ReadStringFromFile(file);

		int nIds[2];
		ReadFromFile(file, nIds, sizeof(nIds));

		tBoundingBoxMeshAssoc assoc;
		assoc.sName = name;
		assoc.nIds[0] = nIds[0];
		assoc.nIds[1] = nIds[1];
		aBoundingBoxMeshAssoc.push_back(assoc);
	}
	return true;
}

bool ParseW32(const std::string& fileName) {
	std::ifstream fin(fileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	uint32_t mapVersion;
	ReadFromFile(fin, &mapVersion, 4);

	WriteFile(std::format("Map Version: 0x{:X} {}", mapVersion, GetMapVersion(mapVersion)));

	if (mapVersion > 0x20000) {
		ReadFromFile(fin, &nSomeMapValue, 4);
		WriteFile("field 4: " + std::to_string(nSomeMapValue)); // always seems to be 1?
	}

	if (!ParseW32Materials(fin)) return false;
	if (!ParseW32Streams(fin)) return false;
	if (!ParseW32Surfaces(fin, mapVersion)) return false;
	if (!ParseW32StaticBatches(fin, mapVersion)) return false;

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
			float vPos[3];
			float fValues[2];
			int nValues[2];
			ReadFromFile(fin, vPos, sizeof(vPos));
			ReadFromFile(fin, fValues, sizeof(fValues));
			ReadFromFile(fin, nValues, sizeof(nValues));

			tUnknownStructure unkStruct;
			memcpy(unkStruct.vPos, vPos, sizeof(vPos));
			memcpy(unkStruct.fValues, fValues, sizeof(fValues));
			memcpy(unkStruct.nValues, nValues, sizeof(nValues));
			aUnknownArray2.push_back(unkStruct);
		}
	}

	if (!ParseW32TreeMeshes(fin)) return false;

	if (mapVersion >= 0x10004) {
		for (int i = 0; i < 16; i++) {
			float value;
			ReadFromFile(fin, &value, sizeof(value));
			aUnknownArray3.push_back(value);
		}
	}

	if (!ParseW32Models(fin)) return false;
	if (!ParseW32Objects(fin)) return false;

	if (mapVersion >= 0x20000) {
		if (!ParseW32BoundingBoxes(fin)) return false;
		if (!ParseW32BoundingBoxMeshAssoc(fin)) return false;
		if (!ParseW32CompactMeshes(fin, mapVersion)) return false;
	}
	else {
		if (!ParseW32CompactMeshes(fin, mapVersion)) return false;
	}
	return true;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		WriteConsole("Usage: FlatOut2W32Extractor_gcp.exe <filename>");
		return 0;
	}
	if (!ParseW32(argv[1])) {
		WriteConsole("Failed to load " + (std::string)argv[1] + "!");
	}
	return 0;
}
