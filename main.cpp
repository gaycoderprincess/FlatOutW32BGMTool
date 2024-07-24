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

bool bDumpMeshData = true;
bool bDumpMaterialData = true;
bool bDumpSurfaceData = true;
bool bDumpSurfaceCenterData = true;
bool bDumpUnknownIntArrayData = true;
bool bDumpUnknownIntArray2Data = true;
bool bDumpTreeMeshData = true;
bool bDumpModelData = true;
bool bDumpObjectData = true;
bool bDumpMeshObjectData = true;

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
std::vector<tVertexBuffer> aVertexBuffers;
std::vector<tIndexBuffer> aIndexBuffers;
std::vector<tVegVertexBuffer> aVegVertexBuffers;
std::vector<tMaterial> aMaterials;

bool ParseW32Materials(std::ifstream& file) {
	int numMaterials;
	ReadFromFile(file, &numMaterials, 4);
	WriteFile("Material Count: " + std::to_string(numMaterials));

	WriteFile(""); // newline

	for (int i = 0; i < numMaterials; i++) {
		WriteFile("Material " + std::to_string(i));

		uint32_t identifier;
		ReadFromFile(file, &identifier, 4);
		if (identifier != 0x4354414D) return false; // "MATC"

		auto textureName = ReadStringFromFile(file);
		WriteFile("sName: " + textureName);

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

		if (bDumpMaterialData) {
			WriteFile("nAlpha: " + std::to_string(alpha));
			WriteFile("nUnknown1: " + std::to_string(v92));
			WriteFile("nNumTextures: " + std::to_string(nNumTextures));
			WriteFile("nUnknown2: " + std::to_string(v73));
			WriteFile("nUnknown3: " + std::to_string(v75));
			WriteFile("nUnknown4: " + std::to_string(v74));
		}

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

		if (bDumpMaterialData) {
			WriteFile("nUnknown5: " + std::to_string(v108[0]) + ", " + std::to_string(v108[1]) + ", " + std::to_string(v108[2]));
			WriteFile("nUnknown6: " + std::to_string(v109[0]) + ", " + std::to_string(v109[1]) + ", " + std::to_string(v109[2]));
			WriteFile("nUnknown7: " + std::to_string(v98[0]) + ", " + std::to_string(v98[1]) + ", " + std::to_string(v98[2]) + ", " + std::to_string(v98[3]));
			WriteFile("nUnknown8: " + std::to_string(v99[0]) + ", " + std::to_string(v99[1]) + ", " + std::to_string(v99[2]) + ", " + std::to_string(v99[3]));
			WriteFile("nUnknown9: " + std::to_string(v100[0]) + ", " + std::to_string(v100[1]) + ", " + std::to_string(v100[2]) + ", " + std::to_string(v100[3]));
			WriteFile("nUnknown10: " + std::to_string(v101[0]) + ", " + std::to_string(v101[1]) + ", " + std::to_string(v101[2]) + ", " + std::to_string(v101[3]));
			WriteFile("nUnknown11: " + std::to_string(v102));
		}

		auto textureName2 = ReadStringFromFile(file);
		WriteFile("sTextureFile1: " + textureName2);
		auto textureName3 = ReadStringFromFile(file);
		WriteFile("sTextureFile2: " + textureName3);
		auto textureName4 = ReadStringFromFile(file);
		WriteFile("sTextureFile3: " + textureName4);

		WriteFile(""); // newline

		tMaterial mat;
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
	WriteFile("Stream Count: " + std::to_string(numStreams));

	WriteFile(""); // newline

	for (int i = 0; i < numStreams; i++) {
		WriteFile("Stream " + std::to_string(i));

		int dataType;
		ReadFromFile(file, &dataType, 4);
		if (dataType == 1) {
			WriteFile("Vertex buffer");

			// 0x202 flag seems to be the terrain

			int field_0;
			int vertexCount;
			int vertexSize;
			int flags;
			ReadFromFile(file, &field_0, 4);
			ReadFromFile(file, &vertexCount, 4);
			ReadFromFile(file, &vertexSize, 4);
			ReadFromFile(file, &flags, 4);
			WriteFile("field_0: " + std::to_string(field_0));
			WriteFile("Vertex count: " + std::to_string(vertexCount));
			WriteFile("Struct size: " + std::to_string(vertexSize));
			WriteFile(std::format("Flags: 0x{:X}", flags));

			auto dataSize = vertexCount * (vertexSize / sizeof(float));
			auto vertexData = new float[dataSize];
			ReadFromFile(file, vertexData, dataSize * sizeof(float));

			if (bDumpMeshData) {
				bool bIs4thValueInt = flags == 0x142;
				size_t j = 0;
				while (j < dataSize) {
					std::string out;
					for (int k = 0; k < vertexSize / sizeof(float); k++) {
						if (bIs4thValueInt && k == 3) {
							out += std::format("0x{:X}", *(uint32_t*)&vertexData[j]);
						}
						else {
							out += std::to_string(vertexData[j]);
						}
						out += " ";
						j++;
					}
					WriteFile(out);
				}
			}

			tVertexBuffer buf;
			buf.vertexCount = vertexCount;
			buf.vertexSize = vertexSize;
			buf.flags = flags;
			buf.id = i;
			buf.data = vertexData;
			aVertexBuffers.push_back(buf);
		}
		else if (dataType == 2) {
			WriteFile("Index buffer");

			int field_0;
			int valueCount;
			ReadFromFile(file, &field_0, 4);
			ReadFromFile(file, &valueCount, 4);
			WriteFile("field_0: " + std::to_string(field_0));
			WriteFile("Value count: " + std::to_string(valueCount));
			WriteFile(std::format("Start: 0x{:X}", (uint64_t)file.tellg()));

			auto dataSize = valueCount;
			auto indexData = new uint16_t[dataSize];
			ReadFromFile(file, indexData, dataSize * sizeof(uint16_t));

			if (bDumpMeshData) {
				for (int j = 0; j < dataSize; j++) {
					WriteFile(std::to_string(indexData[j]));
				}
			}

			tIndexBuffer buf;
			buf.indexCount = valueCount;
			buf.id = i;
			buf.data = indexData;
			aIndexBuffers.push_back(buf);
		}
		else if (dataType == 3) {
			WriteFile("Vegetation vertex buffer");

			int field_0;
			int valueCount;
			int valueSize;
			ReadFromFile(file, &field_0, 4);
			ReadFromFile(file, &valueCount, 4);
			ReadFromFile(file, &valueSize, 4);
			WriteFile("field_0: " + std::to_string(field_0));
			WriteFile("Value count: " + std::to_string(valueCount));
			WriteFile("Struct size: " + std::to_string(valueSize));
			WriteFile(std::format("Start: 0x{:X}", (uint64_t)file.tellg()));

			auto dataSize = valueCount * (valueSize / sizeof(float));
			auto data = new float[dataSize];
			ReadFromFile(file, data, dataSize * sizeof(float));

			if (bDumpMeshData) {
				size_t j = 0;
				while (j < dataSize) {
					std::string out;
					for (int k = 0; k < valueSize / sizeof(float); k++) {
						if (k == 6) {
							out += std::format("0x{:X}", *(uint32_t *) &data[j]);
						} else {
							out += std::to_string(data[j]);
						}
						out += " ";
						j++;
					}
					WriteFile(out);
				}
			}

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

		WriteFile(""); // newline
	}
	return true;
}

bool ParseW32Surfaces(std::ifstream& file, int mapVersion) {
	uint32_t nSurfaceCount;
	ReadFromFile(file, &nSurfaceCount, 4);
	WriteFile("Surface count: " + std::to_string(nSurfaceCount));

	for (int i = 0; i < nSurfaceCount; i++) {
		int v37[7];
		ReadFromFile(file, v37, 28);

		if (bDumpSurfaceData) {
			WriteFile("nIsVegetation: " + std::to_string(v37[0]));
			WriteFile("nMatIdx: " + std::to_string(v37[1]));
			WriteFile("nVertNum: " + std::to_string(v37[2]));
			WriteFile(std::format("nFormat: 0x{:X}", v37[3]));
			WriteFile("nPolyNum: " + std::to_string(v37[4]));
			WriteFile("nPolyMode: " + std::to_string(v37[5])); // 4-triindx or 5-tristrip
			WriteFile("nPolyNumIndex: " + std::to_string(v37[6]));
		}

		if (mapVersion < 0x20000) {
			float vAbsoluteCenter[3];
			float vRelativeCenter[3];
			ReadFromFile(file, vAbsoluteCenter, 12);
			ReadFromFile(file, vRelativeCenter, 12);
			if (bDumpSurfaceData) {
				WriteFile("vAbsoluteCenter.x: " + std::to_string(vAbsoluteCenter[0]));
				WriteFile("vAbsoluteCenter.y: " + std::to_string(vAbsoluteCenter[1]));
				WriteFile("vAbsoluteCenter.z: " + std::to_string(vAbsoluteCenter[2]));
				WriteFile("vRelativeCenter.x: " + std::to_string(vRelativeCenter[0]));
				WriteFile("vRelativeCenter.y: " + std::to_string(vRelativeCenter[1]));
				WriteFile("vRelativeCenter.z: " + std::to_string(vRelativeCenter[2]));
			}
		}

		uint32_t nStreamIdx[2];
		uint32_t nStreamOffset[2];

		int nNumStreamsUsed;
		ReadFromFile(file, &nNumStreamsUsed, 4);
		if (bDumpSurfaceData) {
			WriteFile("nNumStreamsUsed: " + std::to_string(nNumStreamsUsed));
		}
		if (nNumStreamsUsed > 2) return false;

		for (int j = 0; j < nNumStreamsUsed; j++) {
			ReadFromFile(file, &nStreamIdx[j], 4);
			ReadFromFile(file, &nStreamOffset[j], 4);
			if (bDumpSurfaceData) {
				WriteFile("nStreamIdx: " + std::to_string(nStreamIdx[j]));
				WriteFile(std::format("nStreamOffset: 0x{:X}", nStreamOffset[j]));
			}
		}

		if (bDumpSurfaceData) {
			WriteFile(""); // newline
		}
	}
	return true;
}

bool ParseW32SurfaceCenters(std::ifstream& file, int mapVersion) {
	uint32_t numStaticBatches;
	ReadFromFile(file, &numStaticBatches, 4);
	WriteFile("Surface center count: " + std::to_string(numStaticBatches));

	for (int i = 0; i < numStaticBatches; i++) {
		uint32_t nCenterId1, nCenterId2, nSurfaceId;
		ReadFromFile(file, &nCenterId1, 4);
		ReadFromFile(file, &nCenterId2, 4);
		ReadFromFile(file, &nSurfaceId, 4);

		if (bDumpSurfaceCenterData) {
			WriteFile("nCenterId1: " + std::to_string(nCenterId1));
			WriteFile("nCenterId2: " + std::to_string(nCenterId2));
			WriteFile("nSurfaceId: " + std::to_string(nSurfaceId));
		}

		if (mapVersion >= 0x20000) {
			float vAbsoluteCenter[3];
			float vRelativeCenter[3];
			ReadFromFile(file, vAbsoluteCenter, 12);
			ReadFromFile(file, vRelativeCenter, 12);
			if (bDumpSurfaceCenterData) {
				WriteFile("vAbsoluteCenter.x: " + std::to_string(vAbsoluteCenter[0]));
				WriteFile("vAbsoluteCenter.y: " + std::to_string(vAbsoluteCenter[1]));
				WriteFile("vAbsoluteCenter.z: " + std::to_string(vAbsoluteCenter[2]));
				WriteFile("vRelativeCenter.x: " + std::to_string(vRelativeCenter[0]));
				WriteFile("vRelativeCenter.y: " + std::to_string(vRelativeCenter[1]));
				WriteFile("vRelativeCenter.z: " + std::to_string(vRelativeCenter[2]));
			}
		}
		else {
			uint32_t nUnk;
			ReadFromFile(file, &nUnk, 4);
			if (bDumpSurfaceCenterData) {
				WriteFile("nUnknown1: " + std::to_string(nUnk)); // always 0?
			}
		}

		if (bDumpSurfaceCenterData) {
			WriteFile(""); // newline
		}
	}
	WriteFile("");
	return true;
}

bool ParseW32TreeMeshes(std::ifstream& file) {
	uint32_t treeMeshCount;
	ReadFromFile(file, &treeMeshCount, 4);
	WriteFile("Tree mesh count: " + std::to_string(treeMeshCount));
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
		if (bDumpTreeMeshData) {
			WriteFile("nUnknown1: " + std::to_string(nUnk1));
			WriteFile("nUnknown2: " + std::to_string(nUnk2));
			WriteFile("nSurfaceId: " + std::to_string(nSurfaceId));
			WriteFile("nUnknown3: " + std::to_string(nUnk3));
			for (int j = 0; j < 19; j++) {
				WriteFile("fUnk[" + std::to_string(j) + "]: " + std::to_string(fUnk[j]));
			}
			WriteFile("nSurfaceId2: " + std::to_string(nSurfaceId2));
			WriteFile("nSurfaceId3: " + std::to_string(nSurfaceId3));
			WriteFile("nSurfaceId4: " + std::to_string(nSurfaceId4));
			WriteFile("nUnknown4: " + std::to_string(nUnk4));
			WriteFile("nUnknown5: " + std::to_string(nUnk5));
			WriteFile("nMaterialId: " + std::to_string(nMaterialId));
			WriteFile(""); // newline
		}
	}
	WriteFile(""); // newline
	return true;
}

bool ParseW32Models(std::ifstream& file) {
	uint32_t modelCount;
	ReadFromFile(file, &modelCount, 4);
	WriteFile("Model count: " + std::to_string(modelCount));
	for (int i = 0; i < modelCount; i++) {
		int id;
		ReadFromFile(file, &id, sizeof(id)); // BMOD
		int unk;
		ReadFromFile(file, &unk, sizeof(unk));
		auto name = ReadStringFromFile(file);
		if (bDumpModelData) {
			WriteFile("nUnknown1: " + std::to_string(unk));
			WriteFile("sName: " + name);
		}
		float vCenter[3];
		float vRadius[3];
		float fRadius;
		ReadFromFile(file, vCenter, sizeof(vCenter));
		ReadFromFile(file, vRadius, sizeof(vRadius));
		ReadFromFile(file, &fRadius, sizeof(fRadius));
		if (bDumpModelData) {
			WriteFile("vCenter.x: " + std::to_string(vCenter[0]));
			WriteFile("vCenter.y: " + std::to_string(vCenter[1]));
			WriteFile("vCenter.z: " + std::to_string(vCenter[2]));
			WriteFile("vRadius.x: " + std::to_string(vRadius[0]));
			WriteFile("vRadius.y: " + std::to_string(vRadius[1]));
			WriteFile("vRadius.z: " + std::to_string(vRadius[2]));
			WriteFile("fRadius: " + std::to_string(fRadius)); // this is entirely skipped in the reader and instead calculated
		}
		int numSurfaces;
		ReadFromFile(file, &numSurfaces, sizeof(numSurfaces));
		if (bDumpModelData) {
			WriteFile("nNumSurfaces: " + std::to_string(numSurfaces));
		}
		for (int j = 0; j < numSurfaces; j++) {
			int surface;
			ReadFromFile(file, &surface, sizeof(surface));
			if (bDumpModelData) {
				WriteFile(std::to_string(surface));
			}
		}
		if (bDumpModelData) {
			WriteFile(""); // newline
		}
	}
	return true;
}

bool ParseW32Objects(std::ifstream& file) {
	uint32_t objectCount;
	ReadFromFile(file, &objectCount, 4);
	WriteFile("Object count: " + std::to_string(objectCount));
	for (int i = 0; i < objectCount; i++) {
		uint32_t id;
		ReadFromFile(file, &id, sizeof(id));
		auto name1 = ReadStringFromFile(file);
		auto name2 = ReadStringFromFile(file);
		int nFlags;
		ReadFromFile(file, &nFlags, sizeof(nFlags));
		float mMatrix[4*4];
		ReadFromFile(file, &mMatrix, sizeof(mMatrix));
		if (bDumpObjectData) {
			WriteFile("sName: " + name1);
			WriteFile("sUnknown: " + name2);
			WriteFile(std::format("nFlags: 0x{:X}", nFlags));
			WriteFile("mMatrix: ");
			WriteFile(std::format("{}, {}, {}, {}", mMatrix[0], mMatrix[1], mMatrix[2], mMatrix[3]));
			WriteFile(std::format("{}, {}, {}, {}", mMatrix[4], mMatrix[5], mMatrix[6], mMatrix[7]));
			WriteFile(std::format("{}, {}, {}, {}", mMatrix[8], mMatrix[9], mMatrix[10], mMatrix[11]));
			WriteFile(std::format("{}, {}, {}, {}", mMatrix[12], mMatrix[12], mMatrix[13], mMatrix[14]));
			WriteFile(""); // newline
		}
	}
	WriteFile(""); // newline
	return true;
}

bool ParseW32MeshObjects(std::ifstream& file, uint32_t mapVersion) {
	uint32_t meshCount;
	uint32_t meshGroupCount;
	ReadFromFile(file, &meshCount, 4);
	ReadFromFile(file, &meshGroupCount, 4);
	WriteFile("CompactMesh count: " + std::to_string(meshCount));
	WriteFile("CompactMesh group count: " + std::to_string(meshGroupCount));
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
		if (bDumpMeshObjectData) {
			WriteFile("sModelName: " + name);
			WriteFile("sObjectName: " + name2);
			WriteFile(std::format("nFlags: 0x{:X}", nFlags));
			WriteFile("nGroup: " + std::to_string(nGroup));
			WriteFile("mMatrix: ");
			WriteFile(std::format("{}, {}, {}, {}", mMatrix[0], mMatrix[1], mMatrix[2], mMatrix[3]));
			WriteFile(std::format("{}, {}, {}, {}", mMatrix[4], mMatrix[5], mMatrix[6], mMatrix[7]));
			WriteFile(std::format("{}, {}, {}, {}", mMatrix[8], mMatrix[9], mMatrix[10], mMatrix[11]));
			WriteFile(std::format("{}, {}, {}, {}", mMatrix[12], mMatrix[12], mMatrix[13], mMatrix[14]));
		}
		if (mapVersion >= 0x20000) {
			uint32_t nUnk;
			int nBBoxIndex;
			ReadFromFile(file, &nUnk, 4);
			ReadFromFile(file, &nBBoxIndex, 4);
			if (bDumpMeshObjectData) {
				WriteFile("nUnknown1: " + std::to_string(nUnk));
				WriteFile("nBBoxIndex: " + std::to_string(nBBoxIndex));
			}
		}
		else {
			uint32_t nUnkCount;
			ReadFromFile(file, &nUnkCount, 4);
			if (bDumpMeshObjectData) {
				WriteFile("nUnknownCount: " + std::to_string(nUnkCount));
				for (int j = 0; j < nUnkCount; j++) {
					uint32_t nUnkValue;
					ReadFromFile(file, &nUnkValue, 4);
					if (bDumpMeshObjectData) {
						WriteFile(std::to_string(nUnkValue));
					}
				}
			}
		}
		if (bDumpMeshObjectData) {
			WriteFile(""); // newline
		}
	}
	return true;
}

bool ParseW32BoundingBoxes(std::ifstream& file) {
	uint32_t boundingBoxCount;
	ReadFromFile(file, &boundingBoxCount, 4);
	WriteFile("Bounding box count: " + std::to_string(boundingBoxCount));
	for (int i = 0; i < boundingBoxCount; i++) {
		uint32_t modelCount;
		ReadFromFile(file, &modelCount, 4);
		WriteFile("Model count: " + std::to_string(modelCount));
		for (int j = 0; j < modelCount; j++) {
			uint32_t modelId;
			ReadFromFile(file, &modelId, 4);
			WriteFile(std::to_string(modelId));
		}

		float vCenter[3];
		float vRadius[3];
		ReadFromFile(file, vCenter, sizeof(vCenter));
		ReadFromFile(file, vRadius, sizeof(vRadius));
		WriteFile("vCenter.x: " + std::to_string(vCenter[0]));
		WriteFile("vCenter.y: " + std::to_string(vCenter[1]));
		WriteFile("vCenter.z: " + std::to_string(vCenter[2]));
		WriteFile("vRadius.x: " + std::to_string(vRadius[0]));
		WriteFile("vRadius.y: " + std::to_string(vRadius[1]));
		WriteFile("vRadius.z: " + std::to_string(vRadius[2]));
		WriteFile(""); // newline
	}
	WriteFile(""); // newline
	return true;
}

bool ParseW32BoundingBoxMeshAssoc(std::ifstream& file) {
	uint32_t assocCount;
	ReadFromFile(file, &assocCount, 4);
	WriteFile("Bounding box association count: " + std::to_string(assocCount));
	for (int i = 0; i < assocCount; i++) {
		auto name = ReadStringFromFile(file);
		WriteFile("sName: " + name);

		int nIds[2];
		ReadFromFile(file, nIds, sizeof(nIds));
		WriteFile("nIds[0]: " + std::to_string(nIds[0]));
		WriteFile("nIds[1]: " + std::to_string(nIds[1]));
		WriteFile(""); // newline
	}
	WriteFile(""); // newline
	return true;
}

bool ParseW32(const std::string& fileName) {
	std::ifstream fin(fileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	fin.seekg(0, std::ios::end);
	size_t fileSize = fin.tellg();
	fin.seekg(0, std::ios::beg);

	uint32_t mapVersion;
	ReadFromFile(fin, &mapVersion, 4);

	WriteFile(std::format("Map Version: 0x{:X} {}", mapVersion, GetMapVersion(mapVersion)));

	if (mapVersion > 0x20000) {
		int someMapValue;
		ReadFromFile(fin, &someMapValue, 4);
		WriteFile("field 4: " + std::to_string(someMapValue)); // always seems to be 1?
	}

	if (!ParseW32Materials(fin)) return false;
	if (!ParseW32Streams(fin)) return false;
	if (!ParseW32Surfaces(fin, mapVersion)) return false;
	if (!ParseW32SurfaceCenters(fin, mapVersion)) return false;

	{
		uint32_t someCount;
		ReadFromFile(fin, &someCount, 4);
		WriteFile("unknown array count: " + std::to_string(someCount));
		for (int i = 0; i < someCount; i++) {
			int someValue;
			ReadFromFile(fin, &someValue, 4);
			if (bDumpUnknownIntArrayData) WriteFile(std::format("0x{:X}", someValue));
		}
		WriteFile(""); // newline
	}

	{
		uint32_t someCount;
		ReadFromFile(fin, &someCount, 4);
		WriteFile("unknown array 2 count: " + std::to_string(someCount));
		for (int i = 0; i < someCount; i++) {
			float vPos[3];
			float fValues[2];
			int nValues[2];
			ReadFromFile(fin, vPos, sizeof(vPos));
			ReadFromFile(fin, fValues, sizeof(fValues));
			ReadFromFile(fin, nValues, sizeof(nValues));
			if (bDumpUnknownIntArray2Data) {
				WriteFile("vPos.x: " + std::to_string(vPos[0]));
				WriteFile("vPos.y: " + std::to_string(vPos[1]));
				WriteFile("vPos.z: " + std::to_string(vPos[2]));
				WriteFile("fUnknown[0]: " + std::to_string(fValues[0]));
				WriteFile("fUnknown[1]: " + std::to_string(fValues[1]));
				WriteFile(std::format("nUnknown[0]: 0x{:X}", nValues[0]));
				WriteFile(std::format("nUnknown[1]: 0x{:X}", nValues[1]));
				WriteFile(""); // newline
			}
		}
		WriteFile(""); // newline
	}

	if (!ParseW32TreeMeshes(fin)) return false;

	if (mapVersion >= 0x10004) {
		WriteFile("Unknown array:");
		for (int i = 0; i < 16; i++) {
			float value;
			ReadFromFile(fin, &value, sizeof(value));
			WriteFile(std::to_string(value));
		}
		WriteFile(""); // newline
	}

	if (!ParseW32Models(fin)) return false;
	if (!ParseW32Objects(fin)) return false;

	if (mapVersion >= 0x20000) {
		if (!ParseW32BoundingBoxes(fin)) return false;
		if (!ParseW32BoundingBoxMeshAssoc(fin)) return false;
		if (!ParseW32MeshObjects(fin, mapVersion)) return false;
	}
	else {
		if (!ParseW32MeshObjects(fin, mapVersion)) return false;
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
