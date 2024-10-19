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

tCompactMesh* meshToAddModelsTo = nullptr;
void ParseRallyTrophyMesh(std::ifstream& file) {
	tCompactMesh mesh;
	mesh.sName1 = ReadStringFromFile(file);

	int tmp;
	ReadFromFile(file, &tmp, 4); // 0
	//WriteConsole(std::format("tmp {}", tmp), LOG_ALWAYS);
	ReadFromFile(file, mesh.mMatrix, sizeof(mesh.mMatrix));

	//WriteConsole(std::format("{}, {}, {}, {}", mesh.mMatrix[0], mesh.mMatrix[1], mesh.mMatrix[2], mesh.mMatrix[3]), LOG_ALWAYS);
	//WriteConsole(std::format("{}, {}, {}, {}", mesh.mMatrix[4], mesh.mMatrix[5], mesh.mMatrix[6], mesh.mMatrix[7]), LOG_ALWAYS);
	//WriteConsole(std::format("{}, {}, {}, {}", mesh.mMatrix[8], mesh.mMatrix[9], mesh.mMatrix[10], mesh.mMatrix[11]), LOG_ALWAYS);
	//WriteConsole(std::format("{}, {}, {}, {}", mesh.mMatrix[12], mesh.mMatrix[13], mesh.mMatrix[14], mesh.mMatrix[15]), LOG_ALWAYS);

	// bunch of floats from 2B2ACE til 2B2B12

	ReadFromFile(file, &mesh.mMatrix[12], 4*3);
	aCompactMeshes.push_back(mesh);
	meshToAddModelsTo = &aCompactMeshes[aCompactMeshes.size()-1];

	float tmpf[17-3];
	ReadFromFile(file, tmpf, sizeof(tmpf));

	ReadFromFile(file, &tmp, 4); // 0
	ReadFromFile(file, &tmp, 4); // 127277
}

void ParseRallyTrophyHierarchy(std::ifstream& file) {
	int tmp;
	ReadFromFile(file, &tmp, 4); // 3694, prolly count
	ReadFromFile(file, &tmp, 4); // 164

	if (ReadStringFromFile(file) != "SCENE") {
		WriteConsole("ERROR: Failed to find SCENE segment!", LOG_ERRORS);
		return;
	}

	auto name = ReadStringFromFile(file);
	//WriteConsole(std::format("reading {}", name), LOG_ALWAYS);

	ReadFromFile(file, &tmp, 4); // 0

	float matrix[4*4];
	ReadFromFile(file, matrix, sizeof(matrix));

	//WriteConsole(std::format("matrix ends at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);

	uint8_t scenetmp[0x50];
	ReadFromFile(file, scenetmp, sizeof(scenetmp));

	ReadFromFile(file, &tmp, 4); // 127443

	//WriteConsole(std::format("scenetmp ends at {:X}", (uint32_t)file.tellg()), LOG_ALWAYS);
}

void FinishUpRallyTrophyModel(tModel& model) {
	if (auto mesh = meshToAddModelsTo) {
		mesh->aModels.push_back(aModels.size());

		for (auto& id : model.aSurfaces) {
			aSurfaces[id].RegisterReference(SURFACE_REFERENCE_MODEL);
		}

		aModels.push_back(model);
	}
}

void ParseRallyTrophyCamera(std::ifstream& file) {
	ReadStringFromFile(file);

	// 900468 - 90050D
	uint8_t tmp[0xA5-9];
	ReadFromFile(file, tmp, sizeof(tmp));
}

void ParseRallyTrophyLight(std::ifstream& file) {
	ReadStringFromFile(file);

	// 900E72 - 900F37
	uint8_t tmp[0xC6-6];
	ReadFromFile(file, tmp, sizeof(tmp));
}

std::string ParseRallyTrophyModel(std::ifstream& file) {
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
		if (str != "BATCH2") {
			FinishUpRallyTrophyModel(model);
			return str;
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

bool ParseRallyTrophyToken(std::ifstream& file, const std::string& str) {
	if (str == "MAIN") {
		int tmp;
		ReadFromFile(file, &tmp, 4);
		return true;
	}
	if (str == "INFO") {
		int tmp;
		ReadFromFile(file, &tmp, 4);
		return true;
	}
	if (str == "BMF") {
		auto exporter = ReadStringFromFile(file);
		WriteConsole(std::format("BMF exported from {}", exporter), LOG_ALWAYS);
		int tmp;
		ReadFromFile(file, &tmp, 4);
		return true;
	}
	if (str == "MATERIALLIST") {
		int tmp;
		ReadFromFile(file, &tmp, 4); // material count
		ReadFromFile(file, &tmp, 4);
		return true;
	}
	if (str == "MATERIAL") {
		return ParseRallyTrophyMaterial(file);
	}
	if (str == "MODEL") {
		return ParseRallyTrophyToken(file, ParseRallyTrophyModel(file));
	}
	if (str == "TRACK") {
		bIsRallyTrophyTrack = true;
		uint8_t tmptrack[44];
		ReadFromFile(file, tmptrack, sizeof(tmptrack));
		return true;
	}
	if (str == "MESH") {
		ParseRallyTrophyMesh(file);
		return true;
	}
	if (str == "LIGHT") {
		ParseRallyTrophyLight(file);
		return true;
	}
	if (str == "CAMERA2") {
		ParseRallyTrophyCamera(file);
		return true;
	}
	if (str == "HIERARCHY") {
		ParseRallyTrophyHierarchy(file);
		return true;
	}
	if (!str.empty()) WriteConsole(std::format("Parsing ended, unrecognized token {}", str), LOG_ALWAYS);
	return false;
}

bool ParseRallyTrophyBMF() {
	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	nImportFileVersion = 0x10003;

	int tmp;
	ReadFromFile(fin, &tmp, 4);
	while (ParseRallyTrophyToken(fin, ReadStringFromFile(fin))) {}

	for (auto& surface : aSurfaces) {
		if (surface._nNumReferences <= 0) {
			tStaticBatch batch;
			batch.nId1 = aStaticBatches.size();
			batch.nBVHId1 = &surface - &aSurfaces[0];
			batch.nBVHId2 = &surface - &aSurfaces[0];
			memcpy(batch.vCenter, surface.vCenter, sizeof(batch.vCenter));
			memcpy(batch.vRadius, surface.vRadius, sizeof(batch.vRadius));
			aStaticBatches.push_back(batch);
			surface.RegisterReference(SURFACE_REFERENCE_STATICBATCH);
		}
	}

	if (IsRallyTrophyCar()) {
		// create objects for meshes with 1 model that has 24 vertices (the dummy cubes)
		for (auto &compactMesh: aCompactMeshes) {
			if (compactMesh.aModels.size() != 1) continue;

			auto model = &aModels[compactMesh.aModels[0]];
			if (model->aSurfaces.size() != 1) continue;

			auto surface = &aSurfaces[model->aSurfaces[0]];
			if (surface->nVertexCount != 24) continue;
			if (surface->nPolyCount != 12) continue;

			tObject object;
			object.sName1 = compactMesh.sName1;
			object.sName2 = compactMesh.sName2;
			object.nFlags = compactMesh.nFlags;
			memcpy(object.mMatrix, compactMesh.mMatrix, sizeof(object.mMatrix));
			aObjects.push_back(object);
		}

		// create tire dummies
		for (auto &compactMesh: aCompactMeshes) {
			if (compactMesh.sName1.starts_with("tire_") && !compactMesh.sName1.ends_with("_lod")) {
				tObject object;
				object.sName1 = "placeholder_" + compactMesh.sName1;
				object.sName2 = compactMesh.sName2;
				object.nFlags = compactMesh.nFlags;
				memcpy(object.mMatrix, compactMesh.mMatrix, sizeof(object.mMatrix));
				aObjects.push_back(object);
			}

			if (compactMesh.sName1 == "hood" || compactMesh.sName1 == "trunk") {
				tObject object;
				object.sName1 = compactMesh.sName1 + "_joint";
				object.sName2 = compactMesh.sName2;
				object.nFlags = compactMesh.nFlags;
				memcpy(object.mMatrix, compactMesh.mMatrix, sizeof(object.mMatrix));
				aObjects.push_back(object);
			}
		}
	}

	return true;
}