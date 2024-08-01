// import options
bool bDumpIntoTextFile = false;
bool bDumpIntoFBX = false;
bool bDumpIntoW32 = false;
bool bDumpIntoBGM = false;
bool bDumpMaterialData = false;
bool bDumpStreams = false;
bool bDumpFOUCNormalizedStreams = false;
bool bDumpFOUCOffsetedStreams = false;
bool bFBXSkipHiddenProps = false;
int nFBXSkipHiddenPropsFlag;
bool bFBXExportBVHNodes = false;

// export options
bool bDisableObjects = false;
bool bDisableProps = false;
bool bEnableAllProps = false;
bool bDisableCarCollisions = false;
bool bConvertToFO1 = false;
bool bConvertToFO2 = false;
bool bImportPropsFromFBX = false;
bool bImportClonedPropsFromFBX = false;
bool bImportAllPropsFromFBX = false;
bool bImportAllObjectsFromFBX = false;
bool bImportSurfacesFromFBX = false;
bool bImportAllSurfacesFromFBX = false;
bool bImportAndAutoMatchAllSurfacesFromFBX = false;
bool bImportAndAutoMatchAllMeshesFromFBX = false;
bool bImportDeletionFromFBX = false;
bool bClearOriginalMaterials = false;
bool bNoMaterialReuse = false;
bool bNoTreeHack = false;
bool bUngroupMovedPropsFromFBX = false;
bool bCreateBGMFromFBX = false;
bool bCreateW32FromFBX = false;
bool bW32UseVanillaNames = false;
bool bLoadFBX = false;

// special options
bool bEmptyOutTrackBVH = false;
bool bCreateEmptyPlantVDB = false;