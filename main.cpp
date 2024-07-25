#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <format>
#include <vector>
#include "assimp/Exporter.hpp"
#include "assimp/Logger.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

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

// import options
bool bDumpIntoTextFile = false;
bool bDumpIntoFBX = false;
bool bDumpIntoW32 = false;
bool bDumpMaterialData = false;
bool bDumpStreams = false;
bool bDumpFOUCOffsetedStreams = false;

// export options
bool bDisableObjects = false;
bool bDisableProps = false;
bool bConvertToFO1 = false;

// special options
bool bEmptyOutTrackBVH = false;

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

			if (nImportMapVersion >= 0x20002) { // no clue what or why or when or how, this is a bugbear specialty
				int formatType = buf.foucExtraFormat - 22;
				switch (formatType) {
					case 0:
					case 1: {
						std::vector<uint32_t> aValues;

						int size = buf.vertexCount;
						for (int j = 0; j < buf.vertexCount * 8; j++) { // game reads it in packs of 8 here
							uint32_t value;
							ReadFromFile(file, &value, 4);
							aValues.push_back(value);
						}

						auto vertexData = new uint32_t[aValues.size()];
						memcpy(vertexData, &aValues[0], aValues.size() * sizeof(uint32_t));
						buf.id = i;
						buf.data = (float*)vertexData;
						aVertexBuffers.push_back(buf);
					} break;
					case 2: {
						std::vector<uint32_t> aValues;

						int size = buf.vertexCount;
						for (int j = 0; j < buf.vertexCount * 6; j++) { // game reads it in packs of 6 here
							uint32_t value;
							ReadFromFile(file, &value, 4);
							aValues.push_back(value);
						}

						auto vertexData = new uint32_t[aValues.size()];
						memcpy(vertexData, &aValues[0], aValues.size() * sizeof(uint32_t));
						buf.id = i;
						buf.data = (float*)vertexData;
						aVertexBuffers.push_back(buf);
					} break;
					case 3: {
						std::vector<uint32_t> aValues;

						int someCount = (2 * buf.vertexCount) >> 5;
						if (someCount) {
							for (int j = 0; j < someCount * 32; j++) { // read in groups of 32 by the game
								uint32_t value;
								ReadFromFile(file, &value, 4);
								aValues.push_back(value);
							}
						}

						int someOtherCount = (16 * someCount);
						int readCount = 2 * (buf.vertexCount - someOtherCount);
						for (int j = 0; j < readCount; j++) {
							uint32_t value;
							ReadFromFile(file, &value, 4);
							aValues.push_back(value);
						}

						auto vertexData = new uint32_t[aValues.size()];
						memcpy(vertexData, &aValues[0], aValues.size() * sizeof(uint32_t));
						buf.id = i;
						buf.data = (float*)vertexData;
						aVertexBuffers.push_back(buf);
					} break;
					case 4: {
						std::vector<uint32_t> aValues;
						std::vector<uint32_t> aOrigValues;

						for (int j = 0; j < buf.vertexCount; j++) { // game reads it in packs of 5 here
							uint32_t values[5];
							ReadFromFile(file, values, 20);
							for (int k = 0; k < 5; k++) {
								aOrigValues.push_back(values[k]);
							}

							// then does.... huh?
							for (int k = 0; k < 4; k++) {
								uint16_t tmp[2];
								tmp[0] = k;
								tmp[1] = k;
								aValues.push_back(*(uint32_t*)tmp);
								aValues.push_back(values[0]);
								aValues.push_back(values[1]);
								aValues.push_back(values[2]);
								aValues.push_back(values[3]);
								aValues.push_back(values[4]);
							}
						}

						if (!aOrigValues.empty()) {
							auto vertexData2 = new uint32_t[aOrigValues.size()];
							memcpy(vertexData2, &aOrigValues[0], aOrigValues.size() * sizeof(uint32_t));
							buf.origDataForFOUCExport = (float*)vertexData2;
						}

						auto vertexData = new uint32_t[aValues.size()];
						memcpy(vertexData, &aValues[0], aValues.size() * sizeof(uint32_t));
						buf._vertexCountForFOUC = aValues.size() / 6;
						buf._vertexSizeForFOUC = 6 * 4;
						buf.id = i;
						buf.data = (float*)vertexData;
						aVertexBuffers.push_back(buf);
					} break;
					default: {
						auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));
						auto vertexData = new float[dataSize];
						ReadFromFile(file, vertexData, dataSize * sizeof(float));

						buf.id = i;
						buf.data = vertexData;
						aVertexBuffers.push_back(buf);
					} break;
				}
			}
			else if (nImportMapVersion < 0x20002) {
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

			std::vector<uint16_t> aValues;

			int remainingIndexCount = buf.indexCount;
			if (nImportMapVersion >= 0x20002) {
				if (auto extraValue = buf.indexCount >> 6) {
					aValues.reserve(extraValue * 32 * 2); // size 128 each
					// not sure why this is done at all here, these are all still int16 to my knowledge
					for (int j = 0; j < extraValue * 32 * 2; j++) {
						uint16_t tmp;
						ReadFromFile(file, &tmp, 2);
						aValues.push_back(tmp);
					}
				}

				remainingIndexCount = -64 * (buf.indexCount >> 6) + buf.indexCount;
			}

			for (int j = 0; j < remainingIndexCount; j++) {
				uint16_t tmp;
				ReadFromFile(file, &tmp, 2);
				aValues.push_back(tmp);
			}

			auto dataSize = aValues.size();
			auto indexData = new uint16_t[dataSize];
			memcpy(indexData, &aValues[0], dataSize * sizeof(uint16_t));

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
		ReadFromFile(file, &surface.nVertexCount, 4);
		ReadFromFile(file, &surface.nFlags, 4);
		ReadFromFile(file, &surface.nPolyCount, 4);
		ReadFromFile(file, &surface.nPolyMode, 4);
		ReadFromFile(file, &surface.nNumIndicesUsed, 4);

		if (mapVersion < 0x20000) {
			ReadFromFile(file, surface.vAbsoluteCenter, 12);
			ReadFromFile(file, surface.vRelativeCenter, 12);
		}

		if (mapVersion >= 0x20002) {
			ReadFromFile(file, surface.foucVertexMultiplier, sizeof(surface.foucVertexMultiplier));
		}

		ReadFromFile(file, &surface.nNumStreamsUsed, 4);
		if (surface.nNumStreamsUsed <= 0 || surface.nNumStreamsUsed > 2) return false;

		for (int j = 0; j < surface.nNumStreamsUsed; j++) {
			ReadFromFile(file, &surface.nStreamId[j], 4);
			ReadFromFile(file, &surface.nStreamOffset[j], 4);
		}

		auto id = surface.nStreamId[0];
		auto vBuf = FindVertexBuffer(id);
		auto vegvBuf = FindVegVertexBuffer(id);
		if (!vBuf && !vegvBuf) return false;
		if (nImportMapVersion >= 0x20002 && vBuf) {
			auto ptr = (uintptr_t)vBuf->data;
			ptr += surface.nStreamOffset[0];
			for (int j = 0; j < surface.nVertexCount; j++) {
				float value[3];
				for (int k = 0; k < 3; k++) {
					value[k] = *(int16_t*)(ptr + (2 * k));
					value[k] += surface.foucVertexMultiplier[k];
					value[k] *= surface.foucVertexMultiplier[3];
					vBuf->_coordsAfterFOUCMult.push_back(value[k]);
				}
				ptr += vBuf->vertexSize;
			}
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

			ReadFromFile(file, treeMesh.foucExtraData1, sizeof(treeMesh.foucExtraData1));
			ReadFromFile(file, treeMesh.foucExtraData2, sizeof(treeMesh.foucExtraData2));
			ReadFromFile(file, treeMesh.foucExtraData3, sizeof(treeMesh.foucExtraData3));
			ReadFromFile(file, &treeMesh.nSurfaceId3, 4);
			ReadFromFile(file, &treeMesh.nSurfaceId4, 4);
			ReadFromFile(file, &treeMesh.nSurfaceId5, 4);

			if (treeMesh.nSurfaceId3 >= 0 && treeMesh.nSurfaceId3 < aSurfaces.size()) aSurfaces[treeMesh.nSurfaceId3].RegisterReference(SURFACE_REFERENCE_TREEMESH_3);
			if (treeMesh.nSurfaceId4 >= 0 && treeMesh.nSurfaceId4 < aSurfaces.size()) aSurfaces[treeMesh.nSurfaceId4].RegisterReference(SURFACE_REFERENCE_TREEMESH_4);
			if (treeMesh.nSurfaceId5 >= 0 && treeMesh.nSurfaceId5 < aSurfaces.size()) aSurfaces[treeMesh.nSurfaceId5].RegisterReference(SURFACE_REFERENCE_TREEMESH_5);
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
		if (buf.origDataForFOUCExport) {
			file.write((char*)buf.origDataForFOUCExport, buf.vertexCount * buf.vertexSize);
		}
		else {
			file.write((char*)buf.data, buf.vertexCount * buf.vertexSize);
		}
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
	file.write((char*)&surface.nVertexCount, 4);
	file.write((char*)&surface.nFlags, 4);
	file.write((char*)&surface.nPolyCount, 4);
	file.write((char*)&surface.nPolyMode, 4);
	file.write((char*)&surface.nNumIndicesUsed, 4);
	if (nExportMapVersion < 0x20000) {
		file.write((char*)surface.vAbsoluteCenter, 12);
		file.write((char*)surface.vRelativeCenter, 12);
	}
	if (nExportMapVersion >= 0x20002) {
		file.write((char*)surface.foucVertexMultiplier, sizeof(surface.foucVertexMultiplier));
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
	if (nExportMapVersion >= 0x20002) {
		file.write((char*)treeMesh.foucExtraData1, sizeof(treeMesh.foucExtraData1));
		file.write((char*)treeMesh.foucExtraData2, sizeof(treeMesh.foucExtraData2));
		file.write((char*)treeMesh.foucExtraData3, sizeof(treeMesh.foucExtraData3));
		file.write((char*)&treeMesh.nSurfaceId3, 4);
		file.write((char*)&treeMesh.nSurfaceId4, 4);
		file.write((char*)&treeMesh.nSurfaceId5, 4);
	}
	else {
		file.write((char*)&treeMesh.nSurfaceId3, 4);
		file.write((char*)&treeMesh.nSurfaceId4, 4);
		file.write((char*)&treeMesh.nSurfaceId5, 4);
		file.write((char*)&treeMesh.nIdInUnkArray1, 4);
		file.write((char*)&treeMesh.nIdInUnkArray2, 4);
		file.write((char*)&treeMesh.nMaterialId, 4);
	}
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

void WriteW32(uint32_t exportMapVersion) {
	WriteConsole("Writing output w32 file...");

	nExportMapVersion = exportMapVersion;
	if ((nExportMapVersion >= 0x20002 || nImportMapVersion >= 0x20002) && nImportMapVersion != nExportMapVersion) {
		WriteConsole("ERROR: FOUC conversions are currently not supported!");
		return;
	}

	std::ofstream file(sFileNameNoExt + "_out.w32", std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	file.write((char*)&nExportMapVersion, 4);
	if (nExportMapVersion >= 0x20000) file.write((char*)&nSomeMapValue, 4);

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

	if (nExportMapVersion < 0x20002) {
		uint32_t unk1Count = aUnknownArray1.size();
		file.write((char*)&unk1Count, 4);
		for (auto &data: aUnknownArray1) {
			file.write((char*)&data, 4);
		}
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

	if (bDisableObjects) {
		uint32_t tmpCount = 0;
		file.write((char*)&tmpCount, 4); // objects
	}
	else {
		uint32_t objectCount = aObjects.size();
		file.write((char*)&objectCount, 4);
		for (auto& object : aObjects) {
			WriteObjectToFile(file, object);
		}
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

	WriteConsole("W32 export finished");
}

bool ParseW32(const std::string fileName) {
	if (!sFileName.ends_with(".w32")) {
		return false;
	}

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
				std::string uvFlagsReadable = "";
				if ((buf.flags & VERTEX_POSITION) != 0) uvFlagsReadable += "Position ";
				if ((buf.flags & VERTEX_NORMAL) != 0) uvFlagsReadable += "Normals ";
				if ((buf.flags & VERTEX_COLOR) != 0) uvFlagsReadable += "VertexColor ";
				if ((buf.flags & VERTEX_UV) != 0) uvFlagsReadable += "UVMap ";
				if ((buf.flags & VERTEX_UV2) != 0) uvFlagsReadable += "DoubleUVMap ";
				if ((buf.flags & VERTEX_INT16) != 0) uvFlagsReadable += "Int16 ";
				WriteFile(std::format("nFlags: 0x{:X} {}", buf.flags, uvFlagsReadable));

				if (nImportMapVersion >= 0x20002) {
					if (bDumpFOUCOffsetedStreams && !buf._coordsAfterFOUCMult.empty()) {
						int counter = 0;
						std::string out;
						for (auto& pos : buf._coordsAfterFOUCMult) {
							out += std::to_string(pos);
							out += " ";
							counter++;
							if (counter == 3) {
								WriteFile(out);
								counter = 0;
								out = "";
							}
						}
					}
					else if (bDumpStreams) {
						auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(uint16_t));

						auto data = (uint16_t*)buf.data;
						if (buf.origDataForFOUCExport) data = (uint16_t*)buf.origDataForFOUCExport;

						size_t j = 0;
						while (j < dataSize) {
							std::string out;
							for (int k = 0; k < buf.vertexSize / sizeof(uint16_t); k++) {
								out += std::format("0x{:04X}", *(uint16_t*)&data[j]);
								out += " ";
								j++;
							}
							WriteFile(out);
						}
					}
				}
				else if (bDumpStreams) {
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
								out += std::format("0x{:X}", *(uint32_t *) &buf.data[j]);
							} else {
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
				if (nImportMapVersion >= 0x20002) WriteFile(std::format("foucExtraFormat: {}", buf.foucExtraFormat));
				WriteFile(std::format("Index Count: {}", buf.indexCount));
				if (bDumpStreams) {
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
		WriteFile("nVertexCount: " + std::to_string(surface.nVertexCount));
		WriteFile(std::format("nFormat: 0x{:X}", surface.nFlags));
		WriteFile("nPolyCount: " + std::to_string(surface.nPolyCount));
		WriteFile("nPolyMode: " + std::to_string(surface.nPolyMode)); // 4-triindx or 5-tristrip
		WriteFile("nNumIndicesUsed: " + std::to_string(surface.nNumIndicesUsed));
		WriteFile("vAbsoluteCenter.x: " + std::to_string(surface.vAbsoluteCenter[0]));
		WriteFile("vAbsoluteCenter.y: " + std::to_string(surface.vAbsoluteCenter[1]));
		WriteFile("vAbsoluteCenter.z: " + std::to_string(surface.vAbsoluteCenter[2]));
		WriteFile("vRelativeCenter.x: " + std::to_string(surface.vRelativeCenter[0]));
		WriteFile("vRelativeCenter.y: " + std::to_string(surface.vRelativeCenter[1]));
		WriteFile("vRelativeCenter.z: " + std::to_string(surface.vRelativeCenter[2]));
		if (nImportMapVersion >= 0x20002) {
			WriteFile("foucVertexMultiplier.x: " + std::to_string(surface.foucVertexMultiplier[0]));
			WriteFile("foucVertexMultiplier.y: " + std::to_string(surface.foucVertexMultiplier[1]));
			WriteFile("foucVertexMultiplier.z: " + std::to_string(surface.foucVertexMultiplier[2]));
			WriteFile("foucVertexMultiplier.w: " + std::to_string(surface.foucVertexMultiplier[3]));
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
			WriteFile("nMaterialId: " + std::to_string(treeMesh.foucExtraData1[0]));
			WriteFile("foucData1[1]: " + std::to_string(treeMesh.foucExtraData1[1]));
			WriteFile("foucData1[2]: " + std::to_string(treeMesh.foucExtraData1[2]));
			WriteFile("foucData1[3]: " + std::to_string(treeMesh.foucExtraData1[3]));
			WriteFile("foucData1[4]: " + std::to_string(treeMesh.foucExtraData1[4]));
			WriteFile("foucData1[5]: " + std::to_string(treeMesh.foucExtraData1[5]));
			WriteFile("foucData1[6]: " + std::to_string(treeMesh.foucExtraData1[6]));
			WriteFile("foucData1[7]: " + std::to_string(treeMesh.foucExtraData1[7]));
			WriteFile("foucData1[8]: " + std::to_string(treeMesh.foucExtraData1[8]));
			WriteFile("foucData2[0]: " + std::to_string(treeMesh.foucExtraData2[0]));
			WriteFile("foucData2[1]: " + std::to_string(treeMesh.foucExtraData2[1]));
			WriteFile("foucData2[2]: " + std::to_string(treeMesh.foucExtraData2[2]));
			WriteFile("nSomeId1: " + std::to_string(treeMesh.foucExtraData2[3]));
			WriteFile(std::format("nSomeOffset1: {:X}", treeMesh.foucExtraData2[4]));
			WriteFile("nSomeId2: " + std::to_string(treeMesh.foucExtraData2[5]));
			WriteFile(std::format("nSomeOffset2: {:X}", treeMesh.foucExtraData2[6]));
			WriteFile("nSomeId3: " + std::to_string(treeMesh.foucExtraData2[7]));
			WriteFile(std::format("nSomeOffset3: {:X}", treeMesh.foucExtraData2[8]));
			WriteFile("nMaterialId2: " + std::to_string(treeMesh.foucExtraData3[0]));
			WriteFile("foucData3[1]: " + std::to_string(treeMesh.foucExtraData3[1]));
			WriteFile("nSomeId4: " + std::to_string(treeMesh.foucExtraData3[2]));
			WriteFile(std::format("nSomeOffset4: {:X}", (uint32_t)treeMesh.foucExtraData3[3]));
			WriteFile("nSurfaceId3: " + std::to_string(treeMesh.nSurfaceId3));
			WriteFile("nSurfaceId4: " + std::to_string(treeMesh.nSurfaceId4));
			WriteFile("nSurfaceId5: " + std::to_string(treeMesh.nSurfaceId5));
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

	WriteConsole("Text file export finished");
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

aiNode* CreateNodeForTreeMesh(aiScene* scene, const tTreeMesh& treeMesh) {
	int numTreeMeshes = 0;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId2)) numTreeMeshes++;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId3)) numTreeMeshes++;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId4)) numTreeMeshes++;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId5)) numTreeMeshes++;
	if (numTreeMeshes <= 0) return nullptr;

	auto node = new aiNode();
	node->mName = "TreeMesh" + std::to_string(&treeMesh - &aTreeMeshes[0]);
	node->mMeshes = new uint32_t[numTreeMeshes];
	node->mNumMeshes = numTreeMeshes;

	int i = 0;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId2)) node->mMeshes[i++] = aSurfaces[treeMesh.nSurfaceId2]._nFBXModelId;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId3)) node->mMeshes[i++] = aSurfaces[treeMesh.nSurfaceId3]._nFBXModelId;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId4)) node->mMeshes[i++] = aSurfaces[treeMesh.nSurfaceId4]._nFBXModelId;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId5)) node->mMeshes[i++] = aSurfaces[treeMesh.nSurfaceId5]._nFBXModelId;
	return node;
}

aiScene GenerateScene() {
	aiScene scene;
	scene.mRootNode = new aiNode();

	// materials
	scene.mMaterials = new aiMaterial*[aMaterials.size()];
	for (int i = 0; i < aMaterials.size(); i++) {
		scene.mMaterials[i] = new aiMaterial();

		auto& src = aMaterials[i];
		scene.mMaterials[i] = new aiMaterial();
		auto dest = scene.mMaterials[i];
		aiString matName(src.sName);
		dest->AddProperty( &matName, AI_MATKEY_NAME );
		for (int j = 0; j < 3; j++) {
			if (src.sTextureNames[j].empty()) continue;
			auto texName = src.sTextureNames[j];
			if (texName.ends_with(".tga") || texName.ends_with(".TGA")) {
				texName.pop_back();
				texName.pop_back();
				texName.pop_back();
				texName += "dds";
			}
			aiString fileName(texName);
			dest->AddProperty(&fileName, AI_MATKEY_TEXTURE_DIFFUSE(j));
		}
		if (src.sTextureNames[0].empty()) {
			std::string str = "null";
			aiString fileName(str);
			dest->AddProperty(&fileName, AI_MATKEY_TEXTURE_DIFFUSE(0));
		}
		if (src.sName.empty()) {
			std::string str = "null";
			aiString fileName(str);
			dest->AddProperty( &matName, AI_MATKEY_NAME );
		}
	}
	scene.mNumMaterials = aMaterials.size();

	int numSurfaces = 0;
	for (auto& surface : aSurfaces) {
		if (CanSurfaceBeExported(&surface)) numSurfaces++;
	}
	WriteConsole(std::to_string(numSurfaces) + " surfaces of " + std::to_string(aSurfaces.size()) + " can be exported");

	scene.mMeshes = new aiMesh*[numSurfaces];
	scene.mNumMeshes = numSurfaces;

	int counter = 0;
	for (auto& src : aSurfaces) {
		if (!CanSurfaceBeExported(&src)) continue;

		int i = counter++;
		src._nFBXModelId = i;

		scene.mMeshes[i] = new aiMesh();

		auto dest = scene.mMeshes[i];
		dest->mName = "Surface" + std::to_string(&src - &aSurfaces[0]);
		dest->mMaterialIndex = src.nMaterialId;

		auto vBuf = FindVertexBuffer(src.nStreamId[0]);
		auto iBuf = FindIndexBuffer(src.nStreamId[1]);

		auto stride = vBuf->vertexSize;
		uintptr_t vertexData = ((uintptr_t)vBuf->data) + src.nStreamOffset[0];
		uintptr_t indexData = ((uintptr_t)iBuf->data) + src.nStreamOffset[1];

		uint32_t baseVertexOffset = src.nStreamOffset[0] / vBuf->vertexSize;

		dest->mVertices = new aiVector3D[src.nVertexCount];
		dest->mNumVertices = src.nVertexCount;
		dest->mFaces = new aiFace[src.nPolyCount];
		dest->mNumFaces = src.nPolyCount;
		dest->mTextureCoords[0] = new aiVector3D[src.nVertexCount];
		dest->mNumUVComponents[0] = 2;
		if ((vBuf->flags & VERTEX_UV2) != 0) {
			dest->mTextureCoords[1] = new aiVector3D[src.nVertexCount];
			dest->mNumUVComponents[1] = 2;
		}
		if ((vBuf->flags & VERTEX_NORMAL) != 0) {
			dest->mNormals = new aiVector3D[src.nVertexCount];
		}

		if (nImportMapVersion >= 0x20002) {
			for (int j = 0; j < src.nVertexCount; j++) {
				auto vertices = (int16_t*)vertexData;

				// uvs always seem to be the last 2 or 4 values in the vertex buffer
				auto uvOffset = stride - 4;
				if ((vBuf->flags & VERTEX_UV2) != 0) uvOffset -= 4;
				auto uvs = (int16_t*)(vertexData + uvOffset);

				dest->mVertices[j].x = vertices[0];
				dest->mVertices[j].y = vertices[1];
				dest->mVertices[j].z = vertices[2];
				dest->mVertices[j].x += src.foucVertexMultiplier[0];
				dest->mVertices[j].y += src.foucVertexMultiplier[1];
				dest->mVertices[j].z += src.foucVertexMultiplier[2];
				dest->mVertices[j].x *= src.foucVertexMultiplier[3];
				dest->mVertices[j].y *= src.foucVertexMultiplier[3];
				dest->mVertices[j].z *= -src.foucVertexMultiplier[3];
				vertices += 3;

				if ((vBuf->flags & VERTEX_NORMAL) != 0) {
					dest->mNormals[j].x = vertices[0] / 32767.0;
					dest->mNormals[j].y = vertices[1] / 32767.0;
					dest->mNormals[j].z = -vertices[2] / 32767.0;
					vertices += 3; // 3 floats
				}
				if ((vBuf->flags & VERTEX_COLOR) != 0) vertices += 1; // 1 int32
				if ((vBuf->flags & VERTEX_UV) != 0 || (vBuf->flags & VERTEX_UV2) != 0) {
					dest->mTextureCoords[0][j].x = uvs[0] / 2048.0;
					dest->mTextureCoords[0][j].y = 1 - (uvs[1] / 2048.0);
					dest->mTextureCoords[0][j].z = 0;
				}
				if ((vBuf->flags & VERTEX_UV2) != 0) {
					dest->mTextureCoords[1][j].x = uvs[2] / 2048.0;
					dest->mTextureCoords[1][j].y = 1 - (uvs[3] / 2048.0);
					dest->mTextureCoords[1][j].z = 0;
				}
				vertexData += stride;
			}
		}
		else {
			for (int j = 0; j < src.nVertexCount; j++) {
				auto vertices = (float*)vertexData;
				dest->mVertices[j].x = vertices[0];
				dest->mVertices[j].y = vertices[1];
				dest->mVertices[j].z = -vertices[2];
				vertices += 3;

				if ((vBuf->flags & VERTEX_NORMAL) != 0) {
					dest->mNormals[j].x = vertices[0];
					dest->mNormals[j].y = vertices[1];
					dest->mNormals[j].z = -vertices[2];
					vertices += 3; // 3 floats
				}
				if ((vBuf->flags & VERTEX_COLOR) != 0) vertices += 1; // 1 int32
				if ((vBuf->flags & VERTEX_UV) != 0 || (vBuf->flags & VERTEX_UV2) != 0) {
					dest->mTextureCoords[0][j].x = vertices[0];
					dest->mTextureCoords[0][j].y = 1 - vertices[1];
					dest->mTextureCoords[0][j].z = 0;
					vertices += 2;
				}
				if ((vBuf->flags & VERTEX_UV2) != 0) {
					dest->mTextureCoords[1][j].x = vertices[0];
					dest->mTextureCoords[1][j].y = 1 - vertices[1];
					dest->mTextureCoords[1][j].z = 0;
					vertices += 2;
				}
				vertexData += stride;
			}
		}

		if (src.nPolyMode == 5) {
			bool bFlip = false;
			for (int j = 0; j < src.nPolyCount; j++) {
				auto tmp = (uint16_t*)indexData;
				int indices[3] = {tmp[0], tmp[1], tmp[2]};
				indices[0] -= baseVertexOffset;
				indices[1] -= baseVertexOffset;
				indices[2] -= baseVertexOffset;
				if (indices[0] < 0 || indices[0] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[0])); exit(0); }
				if (indices[1] < 0 || indices[1] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[1])); exit(0); }
				if (indices[2] < 0 || indices[2] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[2])); exit(0); }
				dest->mFaces[j].mIndices = new uint32_t[3];
				if (bFlip) {
					dest->mFaces[j].mIndices[0] = indices[2];
					dest->mFaces[j].mIndices[1] = indices[1];
					dest->mFaces[j].mIndices[2] = indices[0];
				}
				else {
					dest->mFaces[j].mIndices[0] = indices[0];
					dest->mFaces[j].mIndices[1] = indices[1];
					dest->mFaces[j].mIndices[2] = indices[2];
				}
				dest->mFaces[j].mNumIndices = 3;
				indexData += 2;
				bFlip = !bFlip;
			}
		}
		else {
			for (int j = 0; j < src.nPolyCount; j++) {
				auto tmp = (uint16_t*)indexData;
				int indices[3] = {tmp[0], tmp[1], tmp[2]};
				indices[0] -= baseVertexOffset;
				indices[1] -= baseVertexOffset;
				indices[2] -= baseVertexOffset;
				if (indices[0] < 0 || indices[0] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[0])); exit(0); }
				if (indices[1] < 0 || indices[1] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[1])); exit(0); }
				if (indices[2] < 0 || indices[2] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[2])); exit(0); }
				dest->mFaces[j].mIndices = new uint32_t[3];
				dest->mFaces[j].mIndices[0] = indices[0];
				dest->mFaces[j].mIndices[1] = indices[1];
				dest->mFaces[j].mIndices[2] = indices[2];
				dest->mFaces[j].mNumIndices = 3;
				indexData += 2 * 3;
			}
		}
	}

	if (auto node = new aiNode()) {
		int numStaticMeshes = 0;
		for (auto& batch : aStaticBatches) {
			if (IsSurfaceValidAndExportable(batch.nSurfaceId)) numStaticMeshes++;
		}

		node->mName = "StaticBatch";
		scene.mRootNode->addChildren(1, &node);
		node->mMeshes = new uint32_t[numStaticMeshes];
		node->mNumMeshes = numStaticMeshes;
		int i = 0;
		for (auto &batch: aStaticBatches) {
			if (!IsSurfaceValidAndExportable(batch.nSurfaceId)) continue;
			node->mMeshes[i++] = aSurfaces[batch.nSurfaceId]._nFBXModelId;
		}
	}

	if (auto node = new aiNode()) {
		node->mName = "TreeMesh";
		scene.mRootNode->addChildren(1, &node);
		for (auto& treeMesh : aTreeMeshes) {
			if (auto treeNode = CreateNodeForTreeMesh(&scene, treeMesh)) {
				node->addChildren(1, &treeNode);
			}
		}
	}

	if (auto node = new aiNode()) {
		node->mName = "Objects";
		scene.mRootNode->addChildren(1, &node);
		for (auto& object : aObjects) {
			auto objectNode = new aiNode();
			node->addChildren(1, &objectNode);

			objectNode->mName = object.sName1;
			objectNode->mTransformation.a1 = object.mMatrix[0];
			objectNode->mTransformation.b1 = object.mMatrix[1];
			objectNode->mTransformation.c1 = -object.mMatrix[2];
			objectNode->mTransformation.d1 = object.mMatrix[3];
			objectNode->mTransformation.a2 = object.mMatrix[4];
			objectNode->mTransformation.b2 = object.mMatrix[5];
			objectNode->mTransformation.c2 = -object.mMatrix[6];
			objectNode->mTransformation.d2 = object.mMatrix[7];
			objectNode->mTransformation.a3 = object.mMatrix[8];
			objectNode->mTransformation.b3 = object.mMatrix[9];
			objectNode->mTransformation.c3 = -object.mMatrix[10];
			objectNode->mTransformation.d3 = object.mMatrix[11];
			objectNode->mTransformation.a4 = object.mMatrix[12];
			objectNode->mTransformation.b4 = object.mMatrix[13];
			objectNode->mTransformation.c4 = -object.mMatrix[14];
			objectNode->mTransformation.d4 = object.mMatrix[15];
		}
	}

	if (auto node = new aiNode()) {
		node->mName = "CompactMesh";
		scene.mRootNode->addChildren(1, &node);
		for (auto& compactMesh : aCompactMeshes) {
			auto meshNode = new aiNode();
			meshNode->mName = compactMesh.sName1;
			meshNode->mTransformation.a1 = compactMesh.mMatrix[0];
			meshNode->mTransformation.b1 = compactMesh.mMatrix[1];
			meshNode->mTransformation.c1 = -compactMesh.mMatrix[2];
			meshNode->mTransformation.d1 = compactMesh.mMatrix[3];
			meshNode->mTransformation.a2 = compactMesh.mMatrix[4];
			meshNode->mTransformation.b2 = compactMesh.mMatrix[5];
			meshNode->mTransformation.c2 = -compactMesh.mMatrix[6];
			meshNode->mTransformation.d2 = compactMesh.mMatrix[7];
			meshNode->mTransformation.a3 = compactMesh.mMatrix[8];
			meshNode->mTransformation.b3 = compactMesh.mMatrix[9];
			meshNode->mTransformation.c3 = -compactMesh.mMatrix[10];
			meshNode->mTransformation.d3 = compactMesh.mMatrix[11];
			meshNode->mTransformation.a4 = compactMesh.mMatrix[12];
			meshNode->mTransformation.b4 = compactMesh.mMatrix[13];
			meshNode->mTransformation.c4 = -compactMesh.mMatrix[14];
			meshNode->mTransformation.d4 = compactMesh.mMatrix[15];
			node->addChildren(1, &meshNode);

			// not loading the damaged or lod parts here
			if (!compactMesh.aLODMeshIds.empty()) {
				auto model = aModels[compactMesh.aLODMeshIds[0]];
				std::vector<int> aMeshes;
				for (auto& surfaceId : model.aSurfaces) {
					if (!IsSurfaceValidAndExportable(surfaceId)) continue;
					aMeshes.push_back(aSurfaces[surfaceId]._nFBXModelId);
				}
				meshNode->mMeshes = new uint32_t[aMeshes.size()];
				memcpy(meshNode->mMeshes, &aMeshes[0], aMeshes.size() * sizeof(uint32_t));
				meshNode->mNumMeshes = aMeshes.size();
			}
		}
	}

	return scene;
}

void WriteW32ToFBX() {
	WriteConsole("Writing model file...");

	auto scene = GenerateScene();

	Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
	Assimp::DefaultLogger::create("export_log.txt",severity, aiDefaultLogStream_FILE);

	Assimp::Exporter exporter;
	if (exporter.Export(&scene, "fbx", sFileNameNoExt + "_out.fbx") != aiReturn_SUCCESS) {
		WriteConsole("Model export failed!");
	}
	else {
		WriteConsole("Model export finished");
	}
}

void ProcessCommandlineArguments(int argc, char* argv[]) {
	sFileName = argv[1];
	sFileNameNoExt = sFileName;
	if (sFileNameNoExt.ends_with(".w32") || sFileNameNoExt.ends_with(".gen")) {
		for (int i = 0; i < 4; i++) {
			sFileNameNoExt.pop_back();
		}
	}
	for (int i = 2; i < argc; i++) {
		auto arg = argv[i];
		if (!strcmp(arg, "-export_fbx")) bDumpIntoFBX = true;
		if (!strcmp(arg, "-export_w32")) bDumpIntoW32 = true;
		if (!strcmp(arg, "-export_text")) bDumpIntoTextFile = true;
		if (!strcmp(arg, "-export_streams_into_text")) bDumpStreams = true;
		if (!strcmp(arg, "-streams_fouc_offseted")) bDumpFOUCOffsetedStreams = true;
		if (!strcmp(arg, "-remove_object_dummies")) {
			bDisableObjects = true;
			bDumpIntoW32 = true;
		}
		if (!strcmp(arg, "-remove_props")) {
			bDisableProps = true;
			bDumpIntoW32 = true;
		}
		if (!strcmp(arg, "-convert_to_fo1")) {
			bConvertToFO1 = true;
			bDumpIntoW32 = true;
		}
		if (!strcmp(arg, "-empty_bvh_gen")) {
			bEmptyOutTrackBVH = true;
		}
	}
}

bool ReadAndEmptyTrackBVH() {
	if (!sFileName.ends_with(".gen")) {
		return false;
	}

	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	std::ofstream fout(sFileNameNoExt + "_out.gen", std::ios::out | std::ios::binary);
	if (fin.is_open() && fout.is_open()) {
		int header[3]; // 0 and 1 are unused, 2 is bvh count
		ReadFromFile(fin, header, sizeof(header));
		fout.write((char*)header, sizeof(header));
		WriteFile(std::format("0x{:08X}", (uint32_t)header[0]));
		WriteFile(std::to_string(header[1]));

		WriteFile("First chunk begin");
		WriteFile("Count: " + std::to_string(header[2]));
		WriteFile(""); // newline

		for (int i = 0; i < header[2]; i++) {
			float data[3];
			ReadFromFile(fin, data, 4 * 3);
			WriteFile(std::to_string(data[0]));
			WriteFile(std::to_string(data[1]));
			WriteFile(std::to_string(data[2]));
			fout.write((char*)data, sizeof(data));

			ReadFromFile(fin, data, 4 * 3);
			WriteFile(std::to_string(data[0]));
			WriteFile(std::to_string(data[1]));
			WriteFile(std::to_string(data[2]));
			data[0] = 10000;
			data[1] = 10000;
			data[2] = 10000;
			fout.write((char*)data, sizeof(data));

			int unkValues[2];
			ReadFromFile(fin, unkValues, sizeof(unkValues));
			WriteFile(std::to_string(unkValues[0]));
			WriteFile(std::to_string(unkValues[1]));
			fout.write((char*)unkValues, sizeof(unkValues));

			WriteFile(""); // newline
		}

		WriteFile("First chunk end");

		WriteFile(""); // newline

		WriteFile("Second chunk begin");

		int chunk2Count;
		ReadFromFile(fin, &chunk2Count, 4);
		WriteFile("Count: " + std::to_string(chunk2Count)); // count
		fout.write((char*)&chunk2Count, 4);

		WriteFile(""); // newline

		for (int i = 0; i < chunk2Count; i++) {
			float data[3];
			ReadFromFile(fin, data, 4 * 3);
			WriteFile(std::to_string(data[0]));
			WriteFile(std::to_string(data[1]));
			WriteFile(std::to_string(data[2]));
			fout.write((char*)data, sizeof(data));

			ReadFromFile(fin, data, 4 * 3);
			WriteFile(std::to_string(data[0]));
			WriteFile(std::to_string(data[1]));
			WriteFile(std::to_string(data[2]));
			data[0] = 100000;
			data[1] = 100000;
			data[2] = 100000;
			fout.write((char*)data, sizeof(data));

			int unkValues[2];
			ReadFromFile(fin, unkValues, sizeof(unkValues));
			WriteFile(std::to_string(unkValues[0]));
			WriteFile(std::to_string(unkValues[1]));
			fout.write((char*)unkValues, sizeof(unkValues));

			WriteFile(""); // newline
		}

		WriteFile("Second chunk end");

		return true;
	}
	return false;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		WriteConsole("Usage: FlatOut2W32Tool_gcp.exe <filename>");
		return 0;
	}
	ProcessCommandlineArguments(argc, argv);
	if (bEmptyOutTrackBVH) {
		if (!ReadAndEmptyTrackBVH()) {
			WriteConsole("Failed to load " + sFileName + "!");
		}
		return 0;
	}
	else {
		if (!ParseW32(sFileName)) {
			WriteConsole("Failed to load " + sFileName + "!");
		}
		else {
			if (bDumpIntoTextFile) WriteW32ToText();
			if (bDumpIntoFBX) WriteW32ToFBX();
			if (bDumpIntoW32) WriteW32(bConvertToFO1 ? 0x10005 : nImportMapVersion);
		}
	}
	return 0;
}
