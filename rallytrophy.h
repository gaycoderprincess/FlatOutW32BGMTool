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

void ParseRallyTrophyModel(std::ifstream& file);

tCompactMesh* meshToAddModelsTo = nullptr;
void ParseRallyTrophyMesh(std::ifstream& file) {
	WriteFile(std::format("Reading mesh at {:X}", (uint32_t)file.tellg()));

	tCompactMesh mesh;
	mesh.sName1 = ReadStringFromFile(file);
	//WriteConsole(std::format("reading mesh {}", mesh.sName1), LOG_ALWAYS);

	int tmp;
	ReadFromFile(file, &tmp, 4); // 0
	//WriteConsole(std::format("tmp {}", tmp), LOG_ALWAYS);
	ReadFromFile(file, mesh.mMatrix, sizeof(mesh.mMatrix));

	//WriteConsole(std::format("{}, {}, {}, {}", mesh.mMatrix[0], mesh.mMatrix[1], mesh.mMatrix[2], mesh.mMatrix[3]), LOG_ALWAYS);
	//WriteConsole(std::format("{}, {}, {}, {}", mesh.mMatrix[4], mesh.mMatrix[5], mesh.mMatrix[6], mesh.mMatrix[7]), LOG_ALWAYS);
	//WriteConsole(std::format("{}, {}, {}, {}", mesh.mMatrix[8], mesh.mMatrix[9], mesh.mMatrix[10], mesh.mMatrix[11]), LOG_ALWAYS);
	//WriteConsole(std::format("{}, {}, {}, {}", mesh.mMatrix[12], mesh.mMatrix[13], mesh.mMatrix[14], mesh.mMatrix[15]), LOG_ALWAYS);

	//WriteConsole(std::format("mesh matrix ends at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);

	// bunch of floats from 2B2ACE til 2B2B12

	ReadFromFile(file, &mesh.mMatrix[12], 4*3);
	aCompactMeshes.push_back(mesh);
	meshToAddModelsTo = &aCompactMeshes[aCompactMeshes.size()-1];

	float tmpf[17-3];
	ReadFromFile(file, tmpf, sizeof(tmpf));
	//WriteConsole(std::format("mesh float array ends at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);

	ReadFromFile(file, &tmp, 4); // 0
	ReadFromFile(file, &tmp, 4); // 127277
	//WriteConsole(std::format("model begins at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);

	if (ReadStringFromFile(file) != "MODEL") {
		WriteConsole("ERROR: Failed to find MODEL segment!", LOG_ERRORS);
		return;
	}
	return ParseRallyTrophyModel(file);
}

void ParseRallyTrophyHierarchy(std::ifstream& file) {
	WriteConsole(std::format("hierarchy starts at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);

	int tmp;
	ReadFromFile(file, &tmp, 4); // 3694, prolly count
	ReadFromFile(file, &tmp, 4); // 164

	if (ReadStringFromFile(file) != "SCENE") {
		WriteConsole("ERROR: Failed to find SCENE segment!", LOG_ERRORS);
		return;
	}

	auto name = ReadStringFromFile(file);
	WriteConsole(std::format("reading {}", name), LOG_ALWAYS);

	ReadFromFile(file, &tmp, 4); // 0

	float matrix[4*4];
	ReadFromFile(file, matrix, sizeof(matrix));

	WriteConsole(std::format("matrix ends at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);

	uint8_t scenetmp[0x50];
	ReadFromFile(file, scenetmp, sizeof(scenetmp));

	ReadFromFile(file, &tmp, 4); // 127443

	WriteConsole(std::format("scenetmp ends at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);

	std::string str;
	while ((str = ReadStringFromFile(file)) == "MESH") {
		ParseRallyTrophyMesh(file);
	}
}

void FinishUpRallyTrophyModel(tModel& model) {
	if (auto mesh = meshToAddModelsTo) {
		mesh->aLODMeshIds.push_back(aModels.size());

		for (auto& id : model.aSurfaces) {
			aSurfaces[id].RegisterReference(SURFACE_REFERENCE_MODEL);
		}

		aModels.push_back(model);
	}
}

void ParseRallyTrophyLight(std::ifstream& file);
void ParseRallyTrophyCamera(std::ifstream& file) {
	WriteConsole(std::format("Reading camera at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);

	ReadStringFromFile(file);

	// 900468 - 90050D
	uint8_t tmp[0xA5-9];
	ReadFromFile(file, tmp, sizeof(tmp));

	std::string str = ReadStringFromFile(file);
	if (str == "CAMERA2") {
		return ParseRallyTrophyCamera(file);
	}
	else if (str == "LIGHT") {
		return ParseRallyTrophyLight(file);
	}
	else if (str == "MESH") {
		return ParseRallyTrophyMesh(file);
	}

	WriteConsole(std::format("ended at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);
}

void ParseRallyTrophyLight(std::ifstream& file) {
	WriteConsole(std::format("Reading light at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);

	ReadStringFromFile(file);

	// 900E72 - 900F37
	uint8_t tmp[0xC6-6];
	ReadFromFile(file, tmp, sizeof(tmp));

	std::string str = ReadStringFromFile(file);
	if (str == "CAMERA2") {
		return ParseRallyTrophyCamera(file);
	}
	else if (str == "LIGHT") {
		return ParseRallyTrophyLight(file);
	}
	else if (str == "MESH") {
		return ParseRallyTrophyMesh(file);
	}
	WriteConsole(std::format("ended at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);
}

void ParseRallyTrophyModel(std::ifstream& file) {
	tModel model;

	int tmp;
	//int surfaceCount;
	//ReadFromFile(file, &surfaceCount, 4);
	ReadFromFile(file, &tmp, 4);
	//WriteConsole(std::format("tmp1 {}", tmp), LOG_ALWAYS);

	ReadFromFile(file, model.vCenter, sizeof(model.vCenter));
	ReadFromFile(file, model.vRadius, sizeof(model.vRadius));

	//WriteConsole(std::format("{} {} {} {} {} {}", model.vCenter[0],model.vCenter[1],model.vCenter[2],model.vRadius[0],model.vRadius[1],model.vRadius[2]), LOG_ALWAYS);

	float tmpf[4];
	ReadFromFile(file, tmpf, sizeof(tmpf));

	//WriteConsole(std::format("{} {} {} {}", tmpf[0],tmpf[1],tmpf[2], tmpf[3]), LOG_ALWAYS);

	ReadFromFile(file, &tmp, 4);
	//WriteConsole(std::format("tmp2 {}", tmp), LOG_ALWAYS);

	while (true) {
		auto str = ReadStringFromFile(file);
		if (str == "CAMERA2") {
			FinishUpRallyTrophyModel(model);
			return ParseRallyTrophyCamera(file);
		}
		if (str == "LIGHT") {
			FinishUpRallyTrophyModel(model);
			return ParseRallyTrophyLight(file);
		}
		if (str == "MODEL") {
			FinishUpRallyTrophyModel(model);
			return ParseRallyTrophyModel(file);
		}
		if (str == "HIERARCHY") {
			FinishUpRallyTrophyModel(model);
			return ParseRallyTrophyHierarchy(file);
		}
		if (str == "MESH") {
			FinishUpRallyTrophyModel(model);
			return ParseRallyTrophyMesh(file);
		}
		if (str != "BATCH2") {
			FinishUpRallyTrophyModel(model);
			WriteConsole(std::format("ended at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);
			WriteConsole("ERROR: Failed to find BATCH2 segment!", LOG_ERRORS);
			return;
		}

		tSurface surface;

		ReadFromFile(file, &surface.nMaterialId, sizeof(surface.nMaterialId));
		ReadFromFile(file, &surface.nVertexCount, sizeof(surface.nVertexCount)); // 181
		ReadFromFile(file, &surface.nPolyCount, sizeof(surface.nPolyCount)); // 321, index count / 3
		surface.nNumIndicesUsed = surface.nPolyCount * 3;
		surface.nPolyMode = 4;
		surface.nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_UV;

		// this section begins at 0x2DE4
		// indices begin at 0x4758
		// size 0x1974 (6516)
		// that'd be tmp2 * 36
		// indices end around 0x4EE0, somewhere around size 0x788 (1928) (964 indices)

		int id = aVertexBuffers.size() + aIndexBuffers.size();

		tVertexBuffer buf;
		buf.id = id;
		buf.vertexCount = surface.nVertexCount;
		buf.vertexSize = 36;
		buf.flags = surface.nFlags;
		uint32_t dataSize = buf.vertexCount * buf.vertexSize;
		buf.data = new float[dataSize / 4];
		ReadFromFile(file, buf.data, dataSize);
		aVertexBuffers.push_back(buf);

		tIndexBuffer buf2;
		buf2.id = id + 1;
		buf2.indexCount = surface.nNumIndicesUsed;
		buf2.data = new uint16_t[buf2.indexCount];
		ReadFromFile(file, buf2.data, buf2.indexCount * 2);
		aIndexBuffers.push_back(buf2);

		ReadFromFile(file, &tmp, sizeof(tmp)); // 5291

		surface.nNumStreamsUsed = 2;
		surface.nStreamId[0] = id;
		surface.nStreamId[1] = id + 1;
		surface.nStreamOffset[0] = 0;
		surface.nStreamOffset[1] = 0;
		memcpy(surface.vCenter, model.vCenter, sizeof(model.vCenter));
		memcpy(surface.vRadius, model.vRadius, sizeof(model.vRadius));
		model.aSurfaces.push_back(aSurfaces.size());

		aSurfaces.push_back(surface);
	}
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
	uint8_t tmptrack[44];
	ReadFromFile(fin, tmptrack, sizeof(tmptrack));
	WriteConsole(std::format("models start at {:X}", (uint32_t)fin.tellg()), LOG_ALWAYS);
	if (ReadStringFromFile(fin) == "MODEL") {
		ParseRallyTrophyModel(fin);
	}

	for (auto& surface : aSurfaces) {
		if (surface._nNumReferences <= 0) {
			tStaticBatch batch;
			batch.nId1 = aStaticBatches.size();
			batch.nBVHId1 = &surface - &aSurfaces[0];
			batch.nBVHId2 = &surface - &aSurfaces[0];
			memcpy(batch.vCenter, surface.vCenter, sizeof(batch.vCenter));
			memcpy(batch.vRadius, surface.vRadius, sizeof(batch.vRadius));
			aStaticBatches.push_back(batch);
		}
	}

	return true;
}