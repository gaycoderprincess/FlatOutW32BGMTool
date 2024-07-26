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
#include "w32parser.h"
#include "w32writer.h"
#include "w32textexport.h"
#include "w32fbxexport.h"
#include "trackbvh.h"

void ProcessCommandlineArguments(int argc, char* argv[]) {
	sFileName = argv[1];
	sFileNameNoExt = sFileName;
	if (sFileName.ends_with(".w32") || sFileName.ends_with(".bgm") || sFileName.ends_with(".gen")) {
		for (int i = 0; i < 4; i++) {
			sFileNameNoExt.pop_back();
		}
	}
	for (int i = 2; i < argc; i++) {
		auto arg = argv[i];
		if (!strcmp(arg, "-export_fbx")) bDumpIntoFBX = true;
		if (!strcmp(arg, "-export_w32")) bDumpIntoW32 = true;
		if (!strcmp(arg, "-export_bgm")) bDumpIntoBGM = true;
		if (!strcmp(arg, "-export_text")) bDumpIntoTextFile = true;
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
		}
		if (!strcmp(arg, "-convert_to_fo2")) {
			bConvertToFO2 = true;
			bDumpIntoW32 = true;
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
	WriteConsole("Parsing FBX...");

	static Assimp::Importer importer;
	pParsedFBXScene = importer.ReadFile(fileName.c_str(), aiProcessPreset_TargetRealtime_Quality);
	return pParsedFBXScene != nullptr && GetFBXNodeForStaticBatchArray() && GetFBXNodeForTreeMeshArray() && GetFBXNodeForCompactMeshArray();
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		WriteConsole("Usage: FlatOut2W32Tool_gcp.exe <filename>");
		return 0;
	}
	ProcessCommandlineArguments(argc, argv);
	if (bLoadFBX) {
		if (!ParseFBX(argv[2])) {
			WriteConsole("Failed to load " + (std::string)argv[2] + "!");
			exit(0);
		}
		else {
			WriteConsole("Parsing finished");
		}
	}
	if (bEmptyOutTrackBVH) {
		if (!ReadAndEmptyTrackBVH()) {
			WriteConsole("Failed to load " + sFileName + "!");
		}
		return 0;
	}
	else {
		if (sFileName.ends_with(".bgm")) {
			if (!ParseBGM(sFileName)) {
				WriteConsole("Failed to load " + sFileName + "!");
			}
			else {
				if (bDumpIntoTextFile) WriteBGMToText();
				if (bDumpIntoFBX) WriteToFBX();
				if (bDumpIntoBGM) {
					uint32_t version = nImportFileVersion;
					if (bConvertToFO1) version = 0x10004;
					if (bConvertToFO2) version = 0x20000;
					WriteBGM(version);
				}
			}
		}
		else {
			if (!ParseW32(sFileName)) {
				WriteConsole("Failed to load " + sFileName + "!");
			}
			else {
				if (bDumpIntoTextFile) WriteW32ToText();
				if (bDumpIntoFBX) WriteToFBX();
				if (bDumpIntoW32) WriteW32(bConvertToFO1 ? 0x10005 : nImportFileVersion);
			}
		}
	}
	return 0;
}
