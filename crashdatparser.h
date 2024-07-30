bool ParseCrashDat(const std::filesystem::path& fileName) {
	if (fileName.extension() != ".dat") {
		return false;
	}

	std::ifstream fin(fileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	uint32_t nodeCount;
	ReadFromFile(fin, &nodeCount, 4);
	for (int i = 0; i < nodeCount; i++) {
		tCrashData data;
		data.sName = ReadStringFromFile(fin);
		uint32_t numSurfaces;
		ReadFromFile(fin, &numSurfaces, 4);
		for (int j = 0; j < numSurfaces; j++) {
			tCrashDataSurface surface;

			if (bIsFOUCModel) {
				uint32_t numVertices;
				ReadFromFile(fin, &numVertices, 4);
				surface.aCrashWeights.reserve(numVertices);
				for (int k = 0; k < numVertices; k++) {
					tCrashDataWeightsFOUC weights;
					ReadFromFile(fin, &weights, sizeof(weights));
					surface.aCrashWeightsFOUC.push_back(weights);
				}
			}
			else {
				uint32_t numVertices;
				ReadFromFile(fin, &numVertices, 4);
				uint32_t numVerticesInBytes;
				ReadFromFile(fin, &numVerticesInBytes, 4);
				surface.vBuffer.vertexCount = numVertices;
				surface.vBuffer.vertexSize = numVerticesInBytes / numVertices;
				if (surface.vBuffer.vertexSize > 45 || surface.vBuffer.vertexSize < 8) {
					WriteConsole("ERROR: crash.dat is in the incorrect format, it might be mismatched with the bgm version!", LOG_ERRORS);
					return false;
				}
				surface.vBuffer.data = new float[numVerticesInBytes / 4];
				ReadFromFile(fin, surface.vBuffer.data, numVerticesInBytes);
				surface.aCrashWeights.reserve(numVertices);
				for (int k = 0; k < numVertices; k++) {
					tCrashDataWeights weights;
					ReadFromFile(fin, &weights, sizeof(weights));
					surface.aCrashWeights.push_back(weights);
				}
			}
			data.aSurfaces.push_back(surface);
		}
		aCrashData.push_back(data);
	}

	WriteConsole("Parsing finished", LOG_ALWAYS);
	return true;
}