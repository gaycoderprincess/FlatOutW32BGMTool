void CMD_ExportFBX() { bDumpIntoFBX = true; }
void CMD_ExportW32() { bDumpIntoW32 = true; }
void CMD_ExportBGM() { bDumpIntoBGM = true; }
void CMD_ExportText() { bDumpIntoTextFile = true; }
void CMD_ExportBGM_FO1() {
	bCreateBGMFromFBX = true;
	nExportFileVersion = 0x10004;
}
void CMD_ExportBGM_FO2() {
	bCreateBGMFromFBX = true;
	nExportFileVersion = 0x20000;
}
void CMD_ExportBGM_FOUC() {
	bCreateBGMFromFBX = true;
	nExportFileVersion = 0x20000;
	bIsFOUCModel = true;
}
void CMD_DumpText_Streams() {
	bDumpStreams = true;
	bDumpIntoTextFile = true;
}
void CMD_DumpText_Materials() {
	bDumpMaterialData = true;
	bDumpIntoTextFile = true;
}
void CMD_DumpText_Streams_FOUCOffset() {
	bDumpFOUCOffsetedStreams = true;
	bDumpIntoTextFile = true;
}
void CMD_DumpText_Streams_FOUCNormal() {
	bDumpStreams = true;
	bDumpFOUCNormalizedStreams = true;
	bDumpIntoTextFile = true;
}
void CMD_DumpText_Streams_FOUCInt8() {
	bDumpStreams = true;
	bDumpFOUCInt8Streams = true;
	bDumpIntoTextFile = true;
}
void CMD_W32_RemoveObjectDummies() {
	bDisableObjects = true;
	bDumpIntoW32 = true;
}
void CMD_W32_RemoveProps() {
	bDisableProps = true;
	bDumpIntoW32 = true;
}
void CMD_W32_EnableAllProps() {
	bEnableAllProps = true;
	bDumpIntoW32 = true;
}
void CMD_ConvertToFO1() {
	bConvertToFO1 = true;
	bDumpIntoW32 = true;
	bDumpIntoBGM = true;
}
void CMD_ConvertToFO2() {
	bConvertToFO2 = true;
	bDumpIntoW32 = true;
	bDumpIntoBGM = true;
}
void CMD_EmptyTrackBVH() {
	bEmptyOutTrackBVH = true;
}
void CMD_W32_ImportMovedProps() {
	bImportPropsFromFBX = true;
	bLoadFBX = true;
	bDumpIntoW32 = true;
}
void CMD_W32_UngroupMovedProps() {
	bUngroupMovedPropsFromFBX = true;
}
void CMD_W32_ImportClonedProps() {
	bImportClonedPropsFromFBX = true;
	bLoadFBX = true;
	bDumpIntoW32 = true;
}
void CMD_W32_ImportSurfaces() {
	bImportSurfacesFromFBX = true;
	bLoadFBX = true;
	bDumpIntoW32 = true;
}
void CMD_W32_ImportAllSurfaces() {
	bImportSurfacesFromFBX = true;
	bImportAllSurfacesFromFBX = true;
	bLoadFBX = true;
	bDumpIntoW32 = true;
}
void CMD_W32_ImportAndAutoMatchAllSurfaces() {
	bImportSurfacesFromFBX = true;
	bImportAllSurfacesFromFBX = true;
	bImportAndAutoMatchAllSurfacesFromFBX = true;
	bLoadFBX = true;
	bDumpIntoW32 = true;
}
void CMD_W32_ClearOriginalMaterials() {
	bClearOriginalMaterials = true;
}
void CMD_W32_ImportDeletions() {
	bImportDeletionFromFBX = true;
	bLoadFBX = true;
	bDumpIntoW32 = true;
}

struct tCommandlineArgument {
	std::string name;
	void(*pFunction)();
	std::string description;
	std::string categoryName;
	bool enabled;
};
tCommandlineArgument aArguments[] = {
		// export formats
		{ "-export_fbx", CMD_ExportFBX, "Exports the input file into an .fbx model", "Export formats" },
		{ "-export_text", CMD_ExportText, "Exports the input file into a text dump" },

		// export bgm versions
		{ "-create_fo1_bgm", CMD_ExportBGM_FO1, "Creates a FlatOut 1 .bgm from an input .fbx file", "FBX to BGM" },
		{ "-create_fo2_bgm", CMD_ExportBGM_FO2, "Creates a FlatOut 2 .bgm from an input .fbx file" },
		{ "-create_fouc_bgm", CMD_ExportBGM_FOUC, "Creates a FlatOut: Ultimate Carnage .bgm from an input .fbx file" },

		// static common options
		{ "-convert_to_fo1", CMD_ConvertToFO1, "Converts an input car model to the FlatOut 1 format", "In-place BGM conversions" },
		{ "-convert_to_fo2", CMD_ConvertToFO2, "Converts an input car model to the FlatOut 2 format" },

		// static map options
		{ "-remove_object_dummies", CMD_W32_RemoveObjectDummies, "Removes all object dummies from an input map file", "W32 editing" },
		{ "-remove_props", CMD_W32_RemoveProps, "Removes all dynamic props from an input map file" },
		{ "-enable_all_props", CMD_W32_EnableAllProps, "Enables all hidden dynamic props in an input map file" },
		{ "-empty_bvh_gen", CMD_EmptyTrackBVH, "Takes a track_bvh.gen file as the first argument and generates a new empty one, required to avoid culling issues!" },

		// map fbx import options
		{ "-import_moved_props", CMD_W32_ImportMovedProps, "Imports moved prop positions from an .fbx, takes an .fbx file as the second argument", "FBX to W32" },
		{ "-ungroup_moved_props", CMD_W32_UngroupMovedProps, "Ungroups props that have been moved via -import_moved_props, fixes some physics behavior" },
		{ "-import_cloned_props", CMD_W32_ImportClonedProps, "Imports new cloned props from an .fbx, takes an .fbx file as the second argument" },
		{ "-import_surfaces", CMD_W32_ImportSurfaces, "Imports modified surfaces from an .fbx if they have the '_export' suffix, takes an .fbx file as the second argument" },
		{ "-import_all_surfaces", CMD_W32_ImportAllSurfaces, "Imports all modified surfaces from an .fbx, takes an .fbx file as the second argument" },
		{ "-import_and_match_all_surfaces", CMD_W32_ImportAndAutoMatchAllSurfaces, "Imports all surfaces from an .fbx and matches them up to any valid w32 surface, takes an .fbx file as the second argument" },
		{ "-clear_old_materials", CMD_W32_ClearOriginalMaterials, "Removes all original materials from the w32 before importing the ones from the .fbx" },
		{ "-import_deletions", CMD_W32_ImportDeletions, "Deletes surfaces that have been deleted in an .fbx, takes an .fbx file as the second argument" },

		// text options
		{ "-text_materials", CMD_DumpText_Materials, "Dumps all material data into a text file", "Text dumps" },
		{ "-text_streams", CMD_DumpText_Streams, "Dumps all mesh data into a text file" },
		{ "-text_streams_fouc_offseted", CMD_DumpText_Streams_FOUCOffset, "Dumps FOUC mesh data into text with offsets applied" },
		{ "-text_streams_fouc_normalized", CMD_DumpText_Streams_FOUCNormal, "Dumps FOUC mesh data into text as normalized floating points" },
		{ "-text_streams_fouc_int8", CMD_DumpText_Streams_FOUCInt8, "Dumps FOUC mesh data into text as a hex dump" },

		// debug options
		{ "-export_w32", CMD_ExportW32, "Exports a map into an identical .w32, for advanced users", "Debug options" },
		{ "-export_bgm", CMD_ExportBGM, "Exports a car into an identical .bgm, for advanced users" },
};

bool IsValidCommandlineArgument(const char* arg) {
	for (auto& cmdArg : aArguments) {
		if (cmdArg.name == arg) return true;
	}
	return false;
}

void PrintCommandlineHelp() {
	WriteConsole("Commandline arguments");
	for (auto& cmdArg : aArguments) {
		if (!cmdArg.categoryName.empty()) WriteConsole("\n- " + cmdArg.categoryName);
		WriteConsole(cmdArg.name + " - " + cmdArg.description);
	}
}

void ProcessCommandlineArguments(int argc, char* argv[]) {
	WriteConsole("FlatOut 2 W32 & BGM Tool by Chloe @ gaycoderprincess\n");

	// display argument list and exit
	if (!strcmp(argv[1], "-help")) {
		PrintCommandlineHelp();
		exit(0);
	}

	// process all arguments
	for (int i = 2; i < argc; i++) {
		auto arg = argv[i];
		for (auto& cmdArg : aArguments) {
			if (cmdArg.name == arg) {
				cmdArg.pFunction();
				cmdArg.enabled = true;
			}
		}
	}
	// afterwards check if any of them are unrecognized (need to detect bLoadFBX beforehand or this will flag the fbx filename)
	for (int i = (bLoadFBX ? 3 : 2); i < argc; i++) {
		auto arg = argv[i];
		if (!IsValidCommandlineArgument(arg)) {
			WriteConsole("WARNING: Unrecognized commandline argument " + (std::string)arg);
		}
	}

	// do dummy checks and then process the input filename
	if (!bDumpIntoW32 && !bDumpIntoBGM && !bDumpIntoFBX && !bDumpIntoTextFile && !bCreateBGMFromFBX && !bEmptyOutTrackBVH) {
		WriteConsole("WARNING: No export output specified, the tool will not generate any files!");
	}
	if (bUngroupMovedPropsFromFBX && !bImportPropsFromFBX) {
		WriteConsole("WARNING: -ungroup_moved_props used without -import_moved_props, the argument will be ignored");
	}
	if (bConvertToFO2) {
		WriteConsole("WARNING: Direct conversions from Ultimate Carnage to FlatOut 2 will skip crash.dat, export the model to .fbx first to keep car damage!");
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