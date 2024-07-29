#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <format>
#include <vector>
#include <filesystem>
#include "assimp/Importer.hpp"
#include "assimp/Exporter.hpp"
#include "assimp/Logger.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "nya_commonmath.h"

#include "base.h"
#include "config.h"
#include "crashdatparser.h"
#include "w32parser.h"
#include "w32writer.h"
#include "w32textexport.h"
#include "w32fbxexport.h"
#include "trackbvh.h"
#include "plants.h"
#include "commandline.h"

bool ParseFBX() {
	if (sFBXFileName.extension() != ".fbx") return false;

	WriteConsole("Parsing FBX...");

	Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
	Assimp::DefaultLogger::create("fbx_import_log.txt", severity, aiDefaultLogStream_FILE);

	//uint32_t flags = aiProcessPreset_TargetRealtime_Quality;
	uint32_t flags = 0;
	flags |= aiProcess_CalcTangentSpace;
	flags |= aiProcess_GenSmoothNormals;
	//flags |= aiProcess_JoinIdenticalVertices;
	//flags |= aiProcess_ImproveCacheLocality;
	//flags |= aiProcess_LimitBoneWeights;
	//flags |= aiProcess_RemoveRedundantMaterials;
	//flags |= aiProcess_SplitLargeMeshes;
	flags |= aiProcess_Triangulate;
	flags |= aiProcess_GenUVCoords;
	//flags |= aiProcess_SortByPType;
	//flags |= aiProcess_FindDegenerates;
	//flags |= aiProcess_FindInvalidData;

	flags |= aiProcess_GenBoundingBoxes;

	static Assimp::Importer importer;
	pParsedFBXScene = importer.ReadFile(sFBXFileName.string().c_str(), flags);
	if (bImportAndAutoMatchAllMeshesFromFBX) return pParsedFBXScene != nullptr;
	if (bCreateBGMFromFBX) return pParsedFBXScene != nullptr && GetFBXNodeForBGMMeshArray() && GetFBXNodeForObjectsArray();
	return pParsedFBXScene != nullptr && GetFBXNodeForStaticBatchArray() && GetFBXNodeForTreeMeshArray() && GetFBXNodeForCompactMeshArray();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		WriteConsole("Usage: FlatOut2W32BGMTool_gcp.exe <filename>");
		WriteConsole("Run FlatOut2W32BGMTool_gcp.exe -help for a list of arguments");
		return 0;
	}
	ProcessCommandlineArguments(argc, argv);
	if (!std::filesystem::exists(sFileName)) {
		WriteConsole("Failed to load " + std::filesystem::absolute(sFileName).string() + "! (File doesn't exist)");
		exit(0);
	}
	if (bCreateEmptyPlantVDB) {
		WriteEmptyPlantVDB();
	}
	if (bCreateBGMFromFBX) {
		if (!ParseFBX()) {
			WriteConsole("Failed to load " + sFBXFileName.string() + "!");
			exit(0);
		}
		else {
			WriteConsole("Parsing finished");

			FillBGMFromFBX();
			WriteBGM(nExportFileVersion);
			WriteCrashDat(nExportFileVersion);
		}
	}
	else {
		if (bLoadFBX) {
			if (!ParseFBX()) {
				WriteConsole("Failed to load " + sFBXFileName.string() + "!");
				exit(0);
			} else {
				WriteConsole("Parsing finished");
			}
		}
		if (bEmptyOutTrackBVH) {
			if (!ReadAndEmptyTrackBVH()) {
				WriteConsole("Failed to load " + sFileName.string() + "!");
			}
			return 0;
		} else {
			if (sFileName.extension() == ".bgm" || sFileName.extension() == ".car") {
				if (!ParseBGM()) {
					WriteConsole("Failed to load " + sFileName.string() + "!");
				} else {
					if (bDumpIntoTextFile) WriteBGMToText();
					if (bDumpIntoFBX) WriteToFBX();
					if (bDumpIntoBGM) {
						uint32_t version = nImportFileVersion;
						if (bConvertToFO1) version = 0x10004;
						if (bConvertToFO2) version = 0x20000;
						WriteBGM(version);
					}
				}
			} else if (sFileName.extension() == ".w32") {
				if (!ParseW32()) {
					WriteConsole("Failed to load " + sFileName.string() + "!");
				} else {
					if (bDumpIntoTextFile) WriteW32ToText();
					if (bDumpIntoFBX) WriteToFBX();
					if (bDumpIntoW32) WriteW32(bConvertToFO1 ? 0x10005 : nImportFileVersion);
				}
			} else {
				WriteConsole("Unrecognized file format for " + sFileName.string());
			}
		}
	}
	return 0;
}
