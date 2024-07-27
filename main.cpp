#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <format>
#include <vector>
#include "assimp/Importer.hpp"
#include "assimp/Exporter.hpp"
#include "assimp/Logger.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "base.h"
#include "config.h"
#include "crashdatparser.h"
#include "w32parser.h"
#include "w32writer.h"
#include "w32textexport.h"
#include "w32fbxexport.h"
#include "trackbvh.h"

void ProcessCommandlineArguments(int argc, char* argv[]) {
	const char* aSupportedFormats[] = {
			".w32",
			".bgm",
			".car", // retro demo
			".gen", // track_bvh.gen
			".fbx",
			".dat", // crash.dat
	};
	sFileName = argv[1];
	sFileNameNoExt = sFileName;
	for (auto& format : aSupportedFormats) {
		if (sFileName.ends_with(format)) {
			for (int i = 0; i < 4; i++) {
				sFileNameNoExt.pop_back();
			}
		}
	}
	for (int i = 2; i < argc; i++) {
		auto arg = argv[i];
		if (!strcmp(arg, "-export_fbx")) bDumpIntoFBX = true;
		if (!strcmp(arg, "-export_w32")) bDumpIntoW32 = true;
		if (!strcmp(arg, "-export_bgm")) bDumpIntoBGM = true;
		if (!strcmp(arg, "-export_text")) bDumpIntoTextFile = true;
		if (!strcmp(arg, "-fouc_crash_dat")) {
			bIsFOUCModel = true;
		}
		if (!strcmp(arg, "-create_fo1_bgm")) {
			bCreateBGMFromFBX = true;
			nExportFileVersion = 0x10004;
		}
		if (!strcmp(arg, "-create_fo2_bgm")) {
			bCreateBGMFromFBX = true;
			nExportFileVersion = 0x20000;
		}
		if (!strcmp(arg, "-create_fouc_bgm")) {
			bCreateBGMFromFBX = true;
			nExportFileVersion = 0x20000;
			bIsFOUCModel = true;
		}
		if (!strcmp(arg, "-text_streams")) {
			bDumpStreams = true;
			bDumpIntoTextFile = true;
		}
		if (!strcmp(arg, "-text_materials")) {
			bDumpMaterialData = true;
			bDumpIntoTextFile = true;
		}
		if (!strcmp(arg, "-text_streams_fouc_offseted")) {
			bDumpFOUCOffsetedStreams = true;
			bDumpIntoTextFile = true;
		}
		if (!strcmp(arg, "-text_streams_fouc_normalized")) {
			bDumpStreams = true;
			bDumpFOUCNormalizedStreams = true;
			bDumpIntoTextFile = true;
		}
		if (!strcmp(arg, "-text_streams_fouc_int8")) {
			bDumpStreams = true;
			bDumpFOUCInt8Streams = true;
			bDumpIntoTextFile = true;
		}
		if (!strcmp(arg, "-remove_object_dummies")) {
			bDisableObjects = true;
			bDumpIntoW32 = true;
		}
		if (!strcmp(arg, "-remove_props")) {
			bDisableProps = true;
			bDumpIntoW32 = true;
		}
		if (!strcmp(arg, "-enable_all_props")) {
			bEnableAllProps = true;
			bDumpIntoW32 = true;
		}
		if (!strcmp(arg, "-convert_to_fo1")) {
			bConvertToFO1 = true;
			bDumpIntoW32 = true;
			bDumpIntoBGM = true;
		}
		if (!strcmp(arg, "-convert_to_fo2")) {
			bConvertToFO2 = true;
			bDumpIntoW32 = true;
			bDumpIntoBGM = true;
		}
		if (!strcmp(arg, "-empty_bvh_gen")) {
			bEmptyOutTrackBVH = true;
		}
		if (!strcmp(arg, "-import_moved_props")) {
			bImportPropsFromFBX = true;
			bLoadFBX = true;
			bDumpIntoW32 = true;
			if (!strcmp(arg, "-ungroup_moved_props")) {
				bUngroupMovedPropsFromFBX = true;
			}
		}
		if (!strcmp(arg, "-import_surfaces")) {
			bImportSurfacesFromFBX = true;
			bLoadFBX = true;
			bDumpIntoW32 = true;
			if (!strcmp(arg, "-import_materials")) {
				bImportSurfaceMaterialsFromFBX = true;
			}
		}
		if (!strcmp(arg, "-import_deletions")) {
			bImportDeletionFromFBX = true;
			bLoadFBX = true;
			bDumpIntoW32 = true;
		}
	}
}

bool ParseFBX(const std::string& fileName) {
	if (!fileName.ends_with(".fbx")) return false;

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
	pParsedFBXScene = importer.ReadFile(fileName.c_str(), flags);
	if (bCreateBGMFromFBX) return pParsedFBXScene != nullptr && GetFBXNodeForCarMeshArray() && GetFBXNodeForObjectsArray();
	return pParsedFBXScene != nullptr && GetFBXNodeForStaticBatchArray() && GetFBXNodeForTreeMeshArray() && GetFBXNodeForCompactMeshArray();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		WriteConsole("Usage: FlatOut2W32Tool_gcp.exe <filename>");
		return 0;
	}
	ProcessCommandlineArguments(argc, argv);
	if (bCreateBGMFromFBX) {
		if (!ParseFBX(argv[1])) {
			WriteConsole("Failed to load " + (std::string) argv[1] + "!");
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
			if (!ParseFBX(argv[2])) {
				WriteConsole("Failed to load " + (std::string) argv[2] + "!");
				exit(0);
			} else {
				WriteConsole("Parsing finished");
			}
		}
		if (bEmptyOutTrackBVH) {
			if (!ReadAndEmptyTrackBVH()) {
				WriteConsole("Failed to load " + sFileName + "!");
			}
			return 0;
		} else {
			if (sFileName.ends_with(".dat")) {
				if (!ParseCrashDat(sFileName)) {
					WriteConsole("Failed to load " + sFileName + "!");
				} else {
					if (bDumpIntoTextFile) WriteCrashDatToText();
				}
			} else if (sFileName.ends_with(".bgm") || sFileName.ends_with(".car")) {
				if (!ParseBGM(sFileName)) {
					WriteConsole("Failed to load " + sFileName + "!");
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
			} else {
				if (!ParseW32(sFileName)) {
					WriteConsole("Failed to load " + sFileName + "!");
				} else {
					if (bDumpIntoTextFile) WriteW32ToText();
					if (bDumpIntoFBX) WriteToFBX();
					if (bDumpIntoW32) WriteW32(bConvertToFO1 ? 0x10005 : nImportFileVersion);
				}
			}
		}
	}
	return 0;
}
