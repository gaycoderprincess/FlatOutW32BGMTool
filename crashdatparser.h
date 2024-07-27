bool ParseCrashDatStreams(std::ifstream& file) {
	WriteConsole("Parsing streams...");

	uint32_t numStreams;
	ReadFromFile(file, &numStreams, 4);
	for (int i = 0; i < numStreams; i++) {
		int dataType;
		ReadFromFile(file, &dataType, 4);
		if (dataType == 1) {
			tVertexBuffer buf;
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			if (buf.foucExtraFormat > 0) bIsFOUCModel = true;
			ReadFromFile(file, &buf.vertexCount, 4);
			ReadFromFile(file, &buf.vertexSize, 4);
			ReadFromFile(file, &buf.flags, 4);

			if (bIsFOUCModel) { // no clue what or why or when or how, this is a bugbear specialty
				int formatType = buf.foucExtraFormat - 22;
				switch (formatType) {
					case 0:
					case 1: {
						std::vector<uint32_t> aValues;

						for (int j = 0; j < buf.vertexCount * 8; j++) { // game reads it in packs of 8 here
							uint32_t value;
							ReadFromFile(file, &value, 4);
							aValues.push_back(value);
						}

						auto vertexData = new uint32_t[aValues.size()];
						memcpy(vertexData, &aValues[0], aValues.size() * sizeof(uint32_t));
						buf.id = i;
						buf.data = (float*)vertexData;
						aVertexBuffers.push_back(buf);
					} break;
					case 2: {
						std::vector<uint32_t> aValues;

						for (int j = 0; j < buf.vertexCount * 6; j++) { // game reads it in packs of 6 here
							uint32_t value;
							ReadFromFile(file, &value, 4);
							aValues.push_back(value);
						}

						auto vertexData = new uint32_t[aValues.size()];
						memcpy(vertexData, &aValues[0], aValues.size() * sizeof(uint32_t));
						buf.id = i;
						buf.data = (float*)vertexData;
						aVertexBuffers.push_back(buf);
					} break;
					case 3: {
						std::vector<uint32_t> aValues;

						int someCount = (2 * buf.vertexCount) >> 5;
						if (someCount) {
							for (int j = 0; j < someCount * 32; j++) { // read in groups of 32 by the game
								uint32_t value;
								ReadFromFile(file, &value, 4);
								aValues.push_back(value);
							}
						}

						int someOtherCount = (16 * someCount);
						int readCount = 2 * (buf.vertexCount - someOtherCount);
						for (int j = 0; j < readCount; j++) {
							uint32_t value;
							ReadFromFile(file, &value, 4);
							aValues.push_back(value);
						}

						auto vertexData = new uint32_t[aValues.size()];
						memcpy(vertexData, &aValues[0], aValues.size() * sizeof(uint32_t));
						buf.id = i;
						buf.data = (float*)vertexData;
						aVertexBuffers.push_back(buf);
					} break;
					case 4: {
						std::vector<uint32_t> aValues;
						std::vector<uint32_t> aOrigValues;

						for (int j = 0; j < buf.vertexCount; j++) { // game reads it in packs of 5 here
							uint32_t values[5];
							ReadFromFile(file, values, 20);
							for (int k = 0; k < 5; k++) {
								aOrigValues.push_back(values[k]);
							}

							// then does.... huh?
							for (int k = 0; k < 4; k++) {
								uint16_t tmp[2];
								tmp[0] = k;
								tmp[1] = k;
								aValues.push_back(*(uint32_t*)tmp);
								aValues.push_back(values[0]);
								aValues.push_back(values[1]);
								aValues.push_back(values[2]);
								aValues.push_back(values[3]);
								aValues.push_back(values[4]);
							}
						}

						if (!aOrigValues.empty()) {
							auto vertexData2 = new uint32_t[aOrigValues.size()];
							memcpy(vertexData2, &aOrigValues[0], aOrigValues.size() * sizeof(uint32_t));
							buf.origDataForFOUCExport = (float*)vertexData2;
						}

						auto vertexData = new uint32_t[aValues.size()];
						memcpy(vertexData, &aValues[0], aValues.size() * sizeof(uint32_t));
						buf._vertexCountForFOUC = aValues.size() / 6;
						buf._vertexSizeForFOUC = 6 * 4;
						buf.id = i;
						buf.data = (float*)vertexData;
						aVertexBuffers.push_back(buf);
					} break;
					default: {
						auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));
						auto vertexData = new float[dataSize];
						ReadFromFile(file, vertexData, dataSize * sizeof(float));

						buf.id = i;
						buf.data = vertexData;
						aVertexBuffers.push_back(buf);
					} break;
				}
			}
			else {
				auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));
				auto vertexData = new float[dataSize];
				ReadFromFile(file, vertexData, dataSize * sizeof(float));

				buf.id = i;
				buf.data = vertexData;
				aVertexBuffers.push_back(buf);
			}
		}
		else if (dataType == 2) {
			tIndexBuffer buf;
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			if (buf.foucExtraFormat > 0) bIsFOUCModel = true;
			ReadFromFile(file, &buf.indexCount, 4);

			std::vector<uint16_t> aValues;

			int remainingIndexCount = buf.indexCount;
			if (bIsFOUCModel) {
				if (auto extraValue = buf.indexCount >> 6) {
					aValues.reserve(extraValue * 32 * 2); // size 128 each
					// not sure why this is done at all here, these are all still int16 to my knowledge
					for (int j = 0; j < extraValue * 32 * 2; j++) {
						uint16_t tmp;
						ReadFromFile(file, &tmp, 2);
						aValues.push_back(tmp);
					}
				}

				remainingIndexCount = -64 * (buf.indexCount >> 6) + buf.indexCount;
			}

			for (int j = 0; j < remainingIndexCount; j++) {
				uint16_t tmp;
				ReadFromFile(file, &tmp, 2);
				aValues.push_back(tmp);
			}

			auto dataSize = aValues.size();
			auto indexData = new uint16_t[dataSize];
			memcpy(indexData, &aValues[0], dataSize * sizeof(uint16_t));

			buf.id = i;
			buf.data = indexData;
			aIndexBuffers.push_back(buf);
		}
		else if (dataType == 3) {
			tVegVertexBuffer buf;
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &buf.vertexCount, 4);
			ReadFromFile(file, &buf.vertexSize, 4);

			auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));
			auto data = new float[dataSize];
			ReadFromFile(file, data, dataSize * sizeof(float));

			buf.id = i;
			buf.data = data;
			aVegVertexBuffers.push_back(buf);
		}
		// unknown, maybe some custom type of buffer in the xbox beta bgm format
		else if (dataType == 4 || dataType == 5) {
			bIsXboxBetaModel = true;

			tVertexBuffer buf;
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &buf.vertexCount, 4);
			ReadFromFile(file, &buf.vertexSize, 4);
			for (int j = 0; j < buf.vertexCount * buf.vertexSize; j++) {
				uint8_t tmp = 0;
				ReadFromFile(file, &tmp, 1);
			}
		}
		else {
			WriteConsole("Unknown stream type " + std::to_string(dataType));
			return false;
		}
	}
	return true;
}

bool ParseCrashDat(const std::string& fileName) {
	if (!fileName.ends_with(".dat")) {
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
					WriteConsole("ERROR: crash.dat is in the incorrect format! If you're importing an FOUC model, make sure to use -fouc_crash_dat");
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

	WriteConsole("Parsing finished");
	return true;
}