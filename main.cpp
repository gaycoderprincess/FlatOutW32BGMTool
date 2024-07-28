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

void ProcessCommandlineArguments(int argc, char* argv[]) {
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
		if (!strcmp(arg, "-import_cloned_props")) {
			bImportClonedPropsFromFBX = true;
			bLoadFBX = true;
			bDumpIntoW32 = true;
		}
		if (!strcmp(arg, "-import_surfaces")) {
			bImportSurfacesFromFBX = true;
			bLoadFBX = true;
			bDumpIntoW32 = true;
		}
		if (!strcmp(arg, "-import_deletions")) {
			bImportDeletionFromFBX = true;
			bLoadFBX = true;
			bDumpIntoW32 = true;
		}
	}
	sFileName = argv[1];
	if (bCreateBGMFromFBX) {
		sFBXFileName = argv[1];
	}
	else if (bLoadFBX) {
		sFBXFileName = argv[2];
	}
	sFileNameNoExt = sFileName;
	sFileNameNoExt.replace_extension("");
	sFileFolder = sFileName;
	sFileFolder.remove_filename();
}

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
	if (bCreateBGMFromFBX) return pParsedFBXScene != nullptr && GetFBXNodeForCarMeshArray() && GetFBXNodeForObjectsArray();
	return pParsedFBXScene != nullptr && GetFBXNodeForStaticBatchArray() && GetFBXNodeForTreeMeshArray() && GetFBXNodeForCompactMeshArray();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		WriteConsole("Usage: FlatOut2W32BGMTool_gcp.exe <filename>");
		return 0;
	}
	ProcessCommandlineArguments(argc, argv);
	if (!std::filesystem::exists(sFileName)) {
		WriteConsole("Failed to load " + std::filesystem::absolute(sFileName).string() + "! (File doesn't exist)");
		exit(0);
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
			if (sFileName.string().ends_with("crash.dat")) {
				if (!ParseCrashDat(sFileName)) {
					WriteConsole("Failed to load " + sFileName.string() + "!");
				} else {
					if (bDumpIntoTextFile) WriteCrashDatToText();
				}
			} else if (sFileName.extension() == ".bgm" || sFileName.extension() == ".car") {
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
