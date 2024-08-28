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

#include "toml++/toml.hpp"
#include "nya_commonmath.h"

#include "base.h"
#include "config.h"
#include "crashdatparser.h"
#include "trackbvh.h"
#include "track4b.h"
#include "splitpoints.h"
#include "w32parser.h"
#include "w32writer.h"
#include "w32textexport.h"
#include "w32fbxexport.h"
#include "plants.h"
#include "commandline.h"

bool ParseFBX() {
	if (sFBXFileName.extension() != ".fbx") return false;

	WriteConsole("Parsing FBX...", LOG_ALWAYS);

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
		WriteConsole("Usage: FlatOutW32BGMTool_gcp.exe <filename>", LOG_ALWAYS);
		WriteConsole("Run FlatOutW32BGMTool_gcp.exe -help for a list of arguments", LOG_ALWAYS);
		return 0;
	}
	ProcessCommandlineArguments(argc, argv);

	if (bCreateEmptyPlantVDB) {
		WriteEmptyPlantVDB();
		WaitAndExitOnSuccess();
	}
	if (!std::filesystem::exists(sFileName)) {
		WriteConsole("ERROR: Failed to load " + std::filesystem::absolute(sFileName).string() + "! (File doesn't exist)", LOG_ERRORS);
		WaitAndExitOnFail();
	}
	if (bCreate4BFromBMP) {
		Write4BFromBMP();
	}
	else if (bCreateW32FromFBX) {
		if (!ParseFBX()) {
			WriteConsole("ERROR: Failed to load " + sFBXFileName.string() + "!", LOG_ERRORS);
			WaitAndExitOnFail();
		}
		else {
			WriteConsole("Parsing finished", LOG_ALWAYS);

			FillW32FromFBX();
			WriteW32(nExportFileVersion);
		}
	}
	else if (bCreateBGMFromFBX) {
		if (!ParseFBX()) {
			WriteConsole("ERROR: Failed to load " + sFBXFileName.string() + "!", LOG_ERRORS);
			WaitAndExitOnFail();
		}
		else {
			WriteConsole("Parsing finished", LOG_ALWAYS);

			FillBGMFromFBX();
			WriteBGM(nExportFileVersion);
			WriteCrashDat(nExportFileVersion);
		}
	}
	else {
		if (bLoadFBX) {
			if (!ParseFBX()) {
				WriteConsole("ERROR: Failed to load " + sFBXFileName.string() + "!", LOG_ERRORS);
				WaitAndExitOnFail();
			} else {
				WriteConsole("Parsing finished", LOG_ALWAYS);
			}
		}
		if (bEmptyOutTrackBVH) {
			if (!ReadAndEmptyTrackBVH()) {
				WriteConsole("ERROR: Failed to load " + sFileName.string() + "!", LOG_ERRORS);
			}
			return 0;
		} else {
			if (sFileName.extension() == ".4b") {
				if (!Parse4B()) {
					WriteConsole("ERROR: Failed to load " + sFileName.string() + "!", LOG_ERRORS);
				}
				else {
					if (bDumpIntoBMP) Write4BToBMP();
				}
			}
			else if (sFileName.extension() == ".bgm" || sFileName.extension() == ".car") {
				if (!ParseBGM()) {
					WriteConsole("ERROR: Failed to load " + sFileName.string() + "!", LOG_ERRORS);
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
			} else if (sFileName.extension() == ".w32" || sFileName.extension() == ".trk") {
				if (!ParseW32()) {
					WriteConsole("ERROR: Failed to load " + sFileName.string() + "!", LOG_ERRORS);
				} else {
					if (bDumpIntoFBX) WriteToFBX();
					if (bDumpIntoW32) {
						WriteW32(bConvertToFO1 ? 0x10005 : nImportFileVersion);
					}
				}

				if (bDumpIntoTextFile) {
					WriteW32ToText();
					WriteTrackBVHToText();
				}
			} else {
				WriteConsole("ERROR: Unrecognized file format for " + sFileName.string(), LOG_ERRORS);
			}
		}
	}
	WaitAndExitOnSuccess();
	return 0;
}
