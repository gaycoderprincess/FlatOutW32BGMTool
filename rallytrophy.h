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

void ParseRallyTrophyModel(std::ifstream& file) {
	tModel model;

	int tmp;
	//int surfaceCount;
	//ReadFromFile(file, &surfaceCount, 4);
	ReadFromFile(file, &tmp, 4);
	WriteConsole(std::format("tmp1 {}", tmp), LOG_ALWAYS);

	ReadFromFile(file, model.vCenter, sizeof(model.vCenter));
	ReadFromFile(file, model.vRadius, sizeof(model.vRadius));

	//WriteConsole(std::format("{} {} {} {} {} {}", model.vCenter[0],model.vCenter[1],model.vCenter[2],model.vRadius[0],model.vRadius[1],model.vRadius[2]), LOG_ALWAYS);

	float tmpf[4];
	ReadFromFile(file, tmpf, sizeof(tmpf));

	//WriteConsole(std::format("{} {} {} {}", tmpf[0],tmpf[1],tmpf[2], tmpf[3]), LOG_ALWAYS);

	ReadFromFile(file, &tmp, 4);
	WriteConsole(std::format("tmp2 {}", tmp), LOG_ALWAYS);

	while (true) {
		auto str = ReadStringFromFile(file);
		if (str == "MODEL") {
			return ParseRallyTrophyModel(file);
		}
		if (str != "BATCH2") {
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
	while ((str = ReadStringFromFile(fin)) == "MODEL") {
		ParseRallyTrophyModel(fin);
	}

	for (auto& surface : aSurfaces) {
		if (surface._nNumReferences <= 0) {
			tStaticBatch batch;
			batch.nId1 = aStaticBatches.size();
			batch.nBVHId1 = &surface - &aSurfaces[0];
			memcpy(batch.vCenter, surface.vCenter, sizeof(batch.vCenter));
			memcpy(batch.vRadius, surface.vRadius, sizeof(batch.vRadius));
			aStaticBatches.push_back(batch);
		}
	}

	return true;
}