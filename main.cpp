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
		if (!strcmp(arg, "-enable_all_props")) {
			bEnableAllProps = true;
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
