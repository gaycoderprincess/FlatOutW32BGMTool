bool ParseRallyTrophyMaterial(std::ifstream& file) {
	tMaterial mat;

	int tmp = 0;
	ReadFromFile(file, &tmp, 4); // 2
	ReadFromFile(file, &tmp, 4); // 0
	ReadFromFile(file, &mat.nShaderId, 4); // 2
	ReadFromFile(file, &tmp, 4); // 1
	ReadFromFile(file, &tmp, 4); // 0

	float tmpf = 0;
	for (int i = 0; i < 16; i++) {
		ReadFromFile(file, &tmpf, 4);
	}

	ReadFromFile(file, &tmp, 4);
	ReadFromFile(file, &tmp, 4);
	mat.sName = ReadStringFromFile(file);
	mat.sTextureNames[0] = ReadStringFromFile(file);
	mat.sTextureNames[1] = ReadStringFromFile(file);
	mat.sTextureNames[2] = ReadStringFromFile(file);
	ReadFromFile(file, &tmp, 4);

	aMaterials.push_back(mat);

	return true;
}

bool ParseRallyTrophyBMF() {
	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	nImportFileVersion = 0x10003;

	int tmp = 0;
	ReadFromFile(fin, &tmp, 4);
	if (ReadStringFromFile(fin) != "MAIN") {
		WriteConsole("ERROR: Failed to find MAIN segment!", LOG_ERRORS);
		return false;
	}
	ReadFromFile(fin, &tmp, 4);
	if (ReadStringFromFile(fin) != "INFO") {
		WriteConsole("ERROR: Failed to find INFO segment!", LOG_ERRORS);
		return false;
	}
	ReadFromFile(fin, &tmp, 4);
	if (ReadStringFromFile(fin) != "BMF") {
		WriteConsole("ERROR: Failed to find BMF segment!", LOG_ERRORS);
		return false;
	}
	auto exporter = ReadStringFromFile(fin);
	WriteConsole(std::format("BMF exported from {}", exporter), LOG_ALWAYS);
	ReadFromFile(fin, &tmp, 4);
	if (ReadStringFromFile(fin) != "MATERIALLIST") {
		WriteConsole("ERROR: Failed to find MATERIALLIST segment!", LOG_ERRORS);
		return false;
	}
	ReadFromFile(fin, &tmp, 4); // material count
	ReadFromFile(fin, &tmp, 4);
	std::string str;
	while ((str = ReadStringFromFile(fin)) == "MATERIAL") {
		ParseRallyTrophyMaterial(fin);
	}
	if (str != "TRACK") {
		WriteConsole("ERROR: Failed to find TRACK segment!", LOG_ERRORS);
		return false;
	}
	uint8_t tmptrack[45];
	ReadFromFile(fin, tmptrack, sizeof(tmptrack));
	return true;
}