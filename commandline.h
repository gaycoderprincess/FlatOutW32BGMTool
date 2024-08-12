void CMD_ExportFBX() { bDumpIntoFBX = true; }
void CMD_ExportW32() { bDumpIntoW32 = true; }
void CMD_ExportBGM() { bDumpIntoBGM = true; }
void CMD_ExportText() { bDumpIntoTextFile = true; }
void CMD_ExportBMP() { bDumpIntoBMP = true; }
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
void CMD_ExportW32_FO1() {
	bCreateW32FromFBX = true;
	nImportFileVersion = 0x10005;
	nExportFileVersion = 0x10005;
}
void CMD_ExportW32_FO2() {
	bCreateW32FromFBX = true;
	nImportFileVersion = 0x20001;
	nExportFileVersion = 0x20001;
}
void CMD_ExportW32_FOUC() {
	bCreateW32FromFBX = true;
	bIsFOUCModel = true;
	nImportFileVersion = 0x20002;
	nExportFileVersion = 0x20002;
}
void CMD_Export4B() {
	bCreate4BFromBMP = true;
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
void CMD_W32_RemoveObjectDummies() {
	bDisableObjects = true;
	bDumpIntoW32 = true;
}
void CMD_W32_RemoveProps() {
	bDisableProps = true;
	bDumpIntoW32 = true;
}
void CMD_W32_FBXSkipHiddenPropsA() {
	bFBXSkipHiddenProps = true;
	nFBXSkipHiddenPropsFlag = 0x2000;
}
void CMD_W32_FBXSkipHiddenPropsB() {
	bFBXSkipHiddenProps = true;
	nFBXSkipHiddenPropsFlag = 0x4000;
}
void CMD_W32_FBXSkipHiddenPropsC() {
	bFBXSkipHiddenProps = true;
	nFBXSkipHiddenPropsFlag = 0x8000;
}
void CMD_W32_FBXExportBVHNodes() {
	bFBXExportBVHNodes = true;
}
void CMD_W32_EnableAllProps() {
	bEnableAllProps = true;
	bDumpIntoW32 = true;
}
void CMD_W32_DisableCarCollisions() {
	bDisableCarCollisions = true;
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
void CMD_EmptyPlantVDB() {
	bCreateEmptyPlantVDB = true;
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
void CMD_W32_ImportAllProps() {
	bImportAllPropsFromFBX = true;
	bLoadFBX = true;
	bDumpIntoW32 = true;
}
void CMD_W32_ImportAllObjects() {
	bImportAllObjectsFromFBX = true;
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
void CMD_W32_ImportAndAutoMatchAllMeshesFromFBX() {
	bImportSurfacesFromFBX = true;
	bImportAllSurfacesFromFBX = true;
	bImportAndAutoMatchAllSurfacesFromFBX = true;
	bImportAndAutoMatchAllMeshesFromFBX = true;
	bLoadFBX = true;
	bDumpIntoW32 = true;
}
void CMD_W32_ClearOriginalMaterials() {
	bClearOriginalMaterials = true;
}
void CMD_W32_NoMaterialReuse() {
	bNoMaterialReuse = true;
}
void CMD_W32_NoTreeHack() {
	bNoTreeHack = true;
}
void CMD_W32_ImportDeletions() {
	bImportDeletionFromFBX = true;
	bLoadFBX = true;
	bDumpIntoW32 = true;
}
void CMD_LogWarningsOnly() {
	nLoggingSeverity = LOG_WARNINGS;
}
void CMD_LogErrorsOnly() {
	nLoggingSeverity = LOG_ERRORS;
}
void CMD_UseVanillaNames() {
	bUseVanillaNames = true;
}
void CMD_CombineMaterialsForMenucar() {
	bMenuCarCombineMaterials = true;
}
void CMD_NoRimAlphaForCar() {
	bCarNoRimAlpha = true;
}
void CMD_ForceRimAlphaForCar() {
	bCarForceRimAlpha = true;
}
void CMD_ForceTireAlphaForCar() {
	bCarForceTireAlpha = true;
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
		{ "-export_bmp", CMD_ExportBMP, "Exports the input .4b file into a BMP image" },
		{ "-use_vanilla_names", CMD_UseVanillaNames, "Exports the files as their original names, e.g. track_geom.w32, track_bvh.gen" },

		// export bgm versions
		{ "-create_fo1_bgm", CMD_ExportBGM_FO1, "Creates a FlatOut 1 .bgm from an input .fbx file", "FBX to BGM" },
		{ "-create_fo2_bgm", CMD_ExportBGM_FO2, "Creates a FlatOut 2 .bgm from an input .fbx file" },
		{ "-create_fouc_bgm", CMD_ExportBGM_FOUC, "Creates a FlatOut: Ultimate Carnage .bgm from an input .fbx file" },

		// export w32 versions
		{ "-create_fo1_w32", CMD_ExportW32_FO1, "Creates a FlatOut 1 .w32 from an input .fbx file", "FBX to W32" },
		{ "-create_fo2_w32", CMD_ExportW32_FO2, "Creates a FlatOut 2 .w32 from an input .fbx file" },
		{ "-create_fouc_w32", CMD_ExportW32_FOUC, "Creates a FlatOut: Ultimate Carnage .w32 from an input .fbx file" },

		// export 4B
		{ "-create_4b", CMD_Export4B, "Creates a 4B map from an input .bmp file", "BMP to 4B" },

		// static common options
		{ "-convert_to_fo1", CMD_ConvertToFO1, "Converts an input car model to the FlatOut 1 format", "In-place BGM conversions" },
		{ "-convert_to_fo2", CMD_ConvertToFO2, "Converts an input car model to the FlatOut 2 format" },

		// static common options
		{ "-menucar_combine_materials", CMD_CombineMaterialsForMenucar, "Combines materials to fit into the 16 material limit for menucars", "BGM parameters" },
		{ "-no_rim_alpha", CMD_NoRimAlphaForCar, "Disables alpha for the rim texture on cars, useful for UC -> FO2 ports" },
		{ "-force_rim_alpha", CMD_ForceRimAlphaForCar, "Forces alpha for the rim texture on cars, useful for FO2 -> UC ports" },
		{ "-force_tire_alpha", CMD_ForceTireAlphaForCar, "Forces alpha for the tire texture on cars, useful for FO2 -> UC ports" },

		// static map options
		{ "-remove_object_dummies", CMD_W32_RemoveObjectDummies, "Removes all object dummies from an input map file", "W32 editing" },
		{ "-remove_props", CMD_W32_RemoveProps, "Removes all dynamic props from an input map file" },
		{ "-skip_hidden_props_a", CMD_W32_FBXSkipHiddenPropsA, "Only exports the props from track variant A into the .fbx file" },
		{ "-skip_hidden_props_b", CMD_W32_FBXSkipHiddenPropsB, "Only exports the props from track variant B into the .fbx file" },
		{ "-skip_hidden_props_c", CMD_W32_FBXSkipHiddenPropsC, "Only exports the props from track variant C into the .fbx file" },
		{ "-export_bvh_nodes", CMD_W32_FBXExportBVHNodes, "Exports BVH culling zones into the .fbx file" },
		{ "-enable_all_props", CMD_W32_EnableAllProps, "Enables all hidden dynamic props in an input map file" },
		{ "-disable_car_collisions", CMD_W32_DisableCarCollisions, "Disables car-to-car collisions in an input map file" },
		{ "-empty_bvh_gen", CMD_EmptyTrackBVH, "Takes a track_bvh.gen file as the first argument and generates a new empty one, disables all culling" },
		{ "-empty_plant_vdb", CMD_EmptyPlantVDB, "Generates an empty plant_vdb.gen, removes all grass from the map" },

		// map fbx import options
		{ "-import_moved_props", CMD_W32_ImportMovedProps, "Imports moved prop positions from an .fbx, takes an .fbx file as the second argument", "W32 + FBX editing" },
		{ "-ungroup_moved_props", CMD_W32_UngroupMovedProps, "Ungroups props that have been moved via -import_moved_props, fixes some physics behavior" },
		{ "-import_cloned_props", CMD_W32_ImportClonedProps, "Imports new cloned props from an .fbx, takes an .fbx file as the second argument" },
		{ "-import_all_props", CMD_W32_ImportAllProps, "Imports all props from an .fbx and deletes the original w32 ones, takes an .fbx file as the second argument" },
		{ "-import_all_object_dummies", CMD_W32_ImportAllObjects, "Imports all object dummies from an .fbx and deletes the original w32 ones, takes an .fbx file as the second argument" },
		{ "-import_surfaces", CMD_W32_ImportSurfaces, "Imports modified surfaces from an .fbx if they have the '_export' suffix, takes an .fbx file as the second argument" },
		{ "-import_all_surfaces", CMD_W32_ImportAllSurfaces, "Imports all modified surfaces from an .fbx, takes an .fbx file as the second argument" },
		{ "-import_and_match_all_surfaces", CMD_W32_ImportAndAutoMatchAllSurfaces, "Imports all surfaces from an .fbx and matches them up to any valid w32 surface, takes an .fbx file as the second argument" },
		{ "-import_and_match_all_meshes", CMD_W32_ImportAndAutoMatchAllMeshesFromFBX, "Imports all meshes from an .fbx and matches them up to any valid w32 surface, takes an .fbx file as the second argument" },
		{ "-import_deletions", CMD_W32_ImportDeletions, "Deletes surfaces that have been deleted in an .fbx, takes an .fbx file as the second argument" },
		{ "-clear_old_materials", CMD_W32_ClearOriginalMaterials, "Removes all original materials from the w32 before importing the ones from the .fbx" },
		{ "-no_material_reuse", CMD_W32_NoMaterialReuse, "Ignores all original w32 materials, instead always importing new ones from the .fbx" },
		{ "-no_tree_hack", CMD_W32_NoTreeHack, "Disables the adjusting of normal vectors for tree shaders in Ultimate Carnage" },

		// text options
		{ "-text_materials", CMD_DumpText_Materials, "Dumps all material data into a text file", "Text dumps" },
		{ "-text_streams", CMD_DumpText_Streams, "Dumps all mesh data into a text file" },
		{ "-text_streams_fouc_offseted", CMD_DumpText_Streams_FOUCOffset, "Dumps FOUC mesh data into text with offsets applied" },
		{ "-text_streams_fouc_normalized", CMD_DumpText_Streams_FOUCNormal, "Dumps FOUC mesh data into text as normalized floating points" },

		{ "-log_warnings_only", CMD_LogWarningsOnly, "Only prints errors and warnings into the console", "Console logging" },
		{ "-log_errors_only", CMD_LogErrorsOnly, "Only prints errors into the console" },

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
	WriteConsole("Commandline arguments", LOG_ALWAYS);
	for (auto& cmdArg : aArguments) {
		if (!cmdArg.categoryName.empty()) WriteConsole("\n- " + cmdArg.categoryName, LOG_ALWAYS);
		WriteConsole(cmdArg.name + " - " + cmdArg.description, LOG_ALWAYS);
	}
}

void ProcessCommandlineArguments(int argc, char* argv[]) {
	WriteConsole("FlatOut W32 & BGM Tool by Chloe @ gaycoderprincess\n", LOG_ALWAYS);

	// display argument list and exit
	if (!strcmp(argv[1], "-help")) {
		PrintCommandlineHelp();
		exit(0);
	}

	// process all arguments
	for (int i = 1; i < argc; i++) {
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
			WriteConsole("WARNING: Unrecognized commandline argument " + (std::string)arg, LOG_ALWAYS);
		}
	}

	// do dummy checks and then process the input filename
	if (!bDumpIntoW32 &&
		!bDumpIntoBGM &&
		!bDumpIntoBMP &&
		!bDumpIntoFBX &&
		!bDumpIntoTextFile &&
		!bCreateBGMFromFBX &&
		!bCreateW32FromFBX &&
		!bCreate4BFromBMP &&
		!bCreateEmptyPlantVDB &&
		!bEmptyOutTrackBVH) {
		WriteConsole("WARNING: No export output specified, the tool will not generate any files!", LOG_ALWAYS);
	}
	if (bUngroupMovedPropsFromFBX && !bImportPropsFromFBX) {
		WriteConsole("WARNING: -ungroup_moved_props used without -import_moved_props, the argument will be ignored", LOG_ALWAYS);
	}
	if (bConvertToFO2) {
		WriteConsole("WARNING: Direct conversions from Ultimate Carnage to FlatOut 2 will skip crash.dat, export the model to .fbx first to keep car damage!", LOG_ALWAYS);
	}

	if (bCreateW32FromFBX) {
		bCreateBGMFromFBX = false;
		//bDisableObjects = false;
		//bDisableProps = false;
		bEnableAllProps = false;
		bImportPropsFromFBX = false;
		bImportClonedPropsFromFBX = false;
		bImportAllPropsFromFBX = false;
		bImportAllObjectsFromFBX = false;
		bImportSurfacesFromFBX = false;
		bImportAllSurfacesFromFBX = false;
		bImportAndAutoMatchAllSurfacesFromFBX = false;
		bImportAndAutoMatchAllMeshesFromFBX = false;
		bImportDeletionFromFBX = false;
		bClearOriginalMaterials = false;
		bNoMaterialReuse = false;
		bUngroupMovedPropsFromFBX = false;
	}

	sFileName = argv[1];
	if (bCreateBGMFromFBX || bCreateW32FromFBX) {
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