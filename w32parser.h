bool ParseW32Materials(std::ifstream& file, bool print = true) {
	if (print) WriteConsole("Parsing materials...", LOG_ALWAYS);

	uint32_t numMaterials;
	ReadFromFile(file, &numMaterials, 4);
	aMaterials.reserve(numMaterials);
	for (int i = 0; i < numMaterials; i++) {
		tMaterial material;
		ReadFromFile(file, &material.identifier, 4);
		if (material.identifier != 0x4354414D) return false; // "MATC"

		// tough trucks:
		// 8 is mat start, 0x19 is name end
		// textures start at 8D, +0x74

		material.sName = ReadStringFromFile(file);
		ReadFromFile(file, &material.nAlpha, 4); // +0
		if (nImportFileVersion >= 0x10004 || nImportFileVersion == 0x10002) {
			ReadFromFile(file, &material.v92, 4);
			ReadFromFile(file, &material.nNumTextures, 4);
			ReadFromFile(file, &material.nShaderId, 4);
			ReadFromFile(file, &material.nUseColormap, 4);
			ReadFromFile(file, &material.v74, 4);
			ReadFromFile(file, material.v108, 12);
			ReadFromFile(file, material.v109, 12);
		}
		else {
			int tmp = 0;
			if (bIsToughTrucksModel) {
				// all this needs to be size 0x30
				// +0x21 is shader id, that'd be +8 off name
				ReadFromFile(file, &tmp, 4); // +4
				ReadFromFile(file, &material.nShaderId, 4); // +8
				ReadFromFile(file, &material.nUseColormap, 4); // +C
				ReadFromFile(file, material.v98, 16); // +10
				ReadFromFile(file, material.v99, 16); // +20
			}
			else {
				ReadFromFile(file, &tmp, 4); // 1
				ReadFromFile(file, &tmp, 4); // 0
				ReadFromFile(file, &tmp, 4); // 2
				ReadFromFile(file, &material.nShaderId, 4); // 2
				ReadFromFile(file, &material.nUseColormap, 4); // 1
				ReadFromFile(file, &material.v108, 4);
			}
		}
		// size 0x44 here
		ReadFromFile(file, material.v98, 16);
		ReadFromFile(file, material.v99, 16);
		ReadFromFile(file, material.v100, 16);
		ReadFromFile(file, material.v101, 16);
		ReadFromFile(file, &material.v102, 4);
		material.sTextureNames[0] = ReadStringFromFile(file);
		material.sTextureNames[1] = ReadStringFromFile(file);
		material.sTextureNames[2] = ReadStringFromFile(file);
		if (bReplaceCommonWithGrille) {
			if (MaterialStringCompare(material.sTextureNames[0], "common.tga")) {
				material.sTextureNames[0] = "grille.tga";
			}
		}
		aMaterials.push_back(material);
	}
	return true;
}

bool ParseW32Streams(std::ifstream& file) {
	WriteConsole("Parsing streams...", LOG_ALWAYS);

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
			tVertexBuffer buf;
			buf.isVegetation = true;
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &buf.vertexCount, 4);
			ReadFromFile(file, &buf.vertexSize, 4);
			buf.flags = 0;

			auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));
			auto data = new float[dataSize];
			ReadFromFile(file, data, dataSize * sizeof(float));

			buf.id = i;
			buf.data = data;
			aVertexBuffers.push_back(buf);
		}
		// unknown, maybe some custom type of buffer in the xbox beta bgm format
		else if (dataType == 4) {
			bIsXboxBetaModel = true;

			tXboxCPUBuffer buf;
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &buf.count, 4);
			ReadFromFile(file, &buf.size, 4);

			auto dataSize = buf.count * buf.size;
			auto data = new uint8_t[dataSize];
			ReadFromFile(file, data, dataSize);

			buf.id = i;
			buf.data = data;
			aXboxCPUBuffers.push_back(buf);
		}
		else if (dataType == 5) {
			bIsXboxBetaModel = true;

			tXboxGPUBuffer buf;
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &buf.count, 4);
			ReadFromFile(file, &buf.size, 4);

			auto dataSize = buf.count * buf.size;
			auto data = new uint8_t[dataSize];
			ReadFromFile(file, data, dataSize);

			buf.id = i;
			buf.data = data;
			aXboxGPUBuffers.push_back(buf);
		}
		else {
			WriteConsole("Unknown stream type " + std::to_string(dataType), LOG_ERRORS);
			return false;
		}
	}
	return true;
}

bool ParseW32Surfaces(std::ifstream& file, int mapVersion) {
	WriteConsole("Parsing surfaces...", LOG_ALWAYS);

	if (bIsXboxBetaModel) {
		uint32_t nCPUPushBufferCount;
		ReadFromFile(file, &nCPUPushBufferCount, 4);
		aCPUPushBuffers.reserve(nCPUPushBufferCount);
		for (int i = 0; i < nCPUPushBufferCount; i++) {
			tCPUPushBuffer buf;
			ReadFromFile(file, &buf, sizeof(buf));
			aCPUPushBuffers.push_back(buf);
		}

		uint32_t nGPUPushBufferCount;
		ReadFromFile(file, &nGPUPushBufferCount, 4);
		aGPUPushBuffers.reserve(nCPUPushBufferCount);
		for (int i = 0; i < nGPUPushBufferCount; i++) {
			tGPUPushBuffer buf;
			ReadFromFile(file, &buf, sizeof(buf));
			aGPUPushBuffers.push_back(buf);
		}
	}

	uint32_t nSurfaceCount;
	ReadFromFile(file, &nSurfaceCount, 4);
	aSurfaces.reserve(nSurfaceCount);
	for (int i = 0; i < nSurfaceCount; i++) {
		tSurface surface;
		ReadFromFile(file, &surface.nIsVegetation, 4);
		ReadFromFile(file, &surface.nMaterialId, 4);
		ReadFromFile(file, &surface.nVertexCount, 4);
		ReadFromFile(file, &surface.nFlags, 4);
		ReadFromFile(file, &surface.nPolyCount, 4);
		ReadFromFile(file, &surface.nPolyMode, 4);
		ReadFromFile(file, &surface.nNumIndicesUsed, 4);
		aMaterials[surface.nMaterialId]._nNumReferences++;

		if (bIsXboxBetaModel) {
			uint32_t tmp;
			ReadFromFile(file, &tmp, 4);
		}

		if (mapVersion < 0x20000) {
			ReadFromFile(file, surface.vCenter, 12);
			ReadFromFile(file, surface.vRadius, 12);
		}

		if (bIsFOUCModel) {
			ReadFromFile(file, surface.foucVertexMultiplier, sizeof(surface.foucVertexMultiplier));
		}

		ReadFromFile(file, &surface.nNumStreamsUsed, 4);
		if (surface.nNumStreamsUsed <= 0 || surface.nNumStreamsUsed > 2) return false;

		for (int j = 0; j < surface.nNumStreamsUsed; j++) {
			ReadFromFile(file, &surface.nStreamId[j], 4);
			ReadFromFile(file, &surface.nStreamOffset[j], 4);
		}

		if (bIsXboxBetaModel && surface.nNumStreamsUsed == 2) {
			if (surface.nStreamId[1]) {
				auto buf = &aGPUPushBuffers[surface.nStreamOffset[1]];
				surface.nStreamId[1] = aXboxGPUBuffers[0].id;
				surface.nStreamOffset[1] = buf->offset;
			}
			else {
				auto buf = &aCPUPushBuffers[surface.nStreamOffset[1]];
				surface.nStreamId[1] = aXboxCPUBuffers[0].id;
				surface.nStreamOffset[1] = buf->offset;
			}
		}

		if (bIsFOUCModel) {
			auto id = surface.nStreamId[0];
			auto vBuf = FindVertexBuffer(id);
			if (!vBuf) return false;

			auto ptr = (uintptr_t)vBuf->data;
			ptr += surface.nStreamOffset[0];
			for (int j = 0; j < surface.nVertexCount; j++) {
				float value[3];
				for (int k = 0; k < 3; k++) {
					value[k] = *(int16_t*)(ptr + (2 * k));
					value[k] += surface.foucVertexMultiplier[k];
					value[k] *= surface.foucVertexMultiplier[3];
					vBuf->_coordsAfterFOUCMult.push_back(value[k]);
				}
				ptr += vBuf->vertexSize;
			}
		}

		aSurfaces.push_back(surface);
	}
	return true;
}

bool ParseW32StaticBatches(std::ifstream& file, int mapVersion) {
	WriteConsole("Parsing static batches...", LOG_ALWAYS);

	uint32_t numStaticBatches;
	ReadFromFile(file, &numStaticBatches, 4);
	aStaticBatches.reserve(numStaticBatches);
	for (int i = 0; i < numStaticBatches; i++) {
		tStaticBatch staticBatch;
		ReadFromFile(file, &staticBatch.nId1, 4);
		ReadFromFile(file, &staticBatch.nBVHId1, 4);
		ReadFromFile(file, &staticBatch.nBVHId2, 4);

		bool bIsSurfaceValid = staticBatch.nBVHId1 < aSurfaces.size();
		if (nImportFileVersion != 0x20002 && !bIsSurfaceValid) return false;

		if (bIsSurfaceValid) {
			aSurfaces[staticBatch.nBVHId1].RegisterReference(SURFACE_REFERENCE_STATICBATCH);
		}

		if (mapVersion >= 0x20000) {
			ReadFromFile(file, staticBatch.vCenter, 12);
			ReadFromFile(file, staticBatch.vRadius, 12);

			if (bIsSurfaceValid) {
				// backwards compatibility
				memcpy(aSurfaces[staticBatch.nBVHId1].vCenter, staticBatch.vCenter, 12);
				memcpy(aSurfaces[staticBatch.nBVHId1].vRadius, staticBatch.vRadius, 12);
			}
		}
		else {
			ReadFromFile(file, &staticBatch.nUnk, 4); // always 0?

			if (bIsSurfaceValid) {
				// forwards compatibility
				memcpy(staticBatch.vCenter, aSurfaces[staticBatch.nBVHId1].vCenter, 12);
				memcpy(staticBatch.vRadius, aSurfaces[staticBatch.nBVHId1].vRadius, 12);
			}
		}

		aStaticBatches.push_back(staticBatch);
	}
	return true;
}

bool ParseW32TreeMeshes(std::ifstream& file) {
	WriteConsole("Parsing tree meshes...", LOG_ALWAYS);

	uint32_t treeMeshCount;
	ReadFromFile(file, &treeMeshCount, 4);
	aTreeMeshes.reserve(treeMeshCount);
	for (int i = 0; i < treeMeshCount; i++) {
		tTreeMesh treeMesh;
		ReadFromFile(file, &treeMesh.nIsBush, 4);
		ReadFromFile(file, &treeMesh.nUnk2Unused, 4);
		ReadFromFile(file, &treeMesh.nBVHId1, 4);
		ReadFromFile(file, &treeMesh.nBVHId2, 4);
		ReadFromFile(file, treeMesh.mMatrix, sizeof(treeMesh.mMatrix));
		ReadFromFile(file, treeMesh.fScale, sizeof(treeMesh.fScale));

		if (bIsFOUCModel) {
			ReadFromFile(file, &treeMesh.foucTrunk, sizeof(treeMesh.foucTrunk));
			ReadFromFile(file, &treeMesh.foucBranch, sizeof(treeMesh.foucBranch));
			ReadFromFile(file, &treeMesh.foucLeaf, sizeof(treeMesh.foucLeaf));
			ReadFromFile(file, treeMesh.foucExtraData4, sizeof(treeMesh.foucExtraData4));
		}
		else {
			ReadFromFile(file, &treeMesh.nTrunkSurfaceId, 4);
			ReadFromFile(file, &treeMesh.nBranchSurfaceId, 4);
			ReadFromFile(file, &treeMesh.nLeafSurfaceId, 4);
			ReadFromFile(file, &treeMesh.nColorId, 4);
			ReadFromFile(file, &treeMesh.nLodId, 4);
			ReadFromFile(file, &treeMesh.nMaterialId, 4);

			//if (treeMesh.nSurfaceId1 >= 0 && treeMesh.nSurfaceId1 >= aSurfaces.size()) return false;
			//if (treeMesh.nSurfaceId1 >= 0) aSurfaces[treeMesh.nSurfaceId1]._bUsedByAnything = true;

			//if (treeMesh.nUnkId2 >= 0 && treeMesh.nUnkId2 >= aSurfaces.size()) return false;
			if (treeMesh.nTrunkSurfaceId >= 0 && treeMesh.nTrunkSurfaceId >= aSurfaces.size()) return false;
			if (treeMesh.nBranchSurfaceId >= 0 && treeMesh.nBranchSurfaceId >= aSurfaces.size()) return false;
			if (treeMesh.nLeafSurfaceId >= 0 && treeMesh.nLeafSurfaceId >= aSurfaces.size()) return false;
			//if (treeMesh.nUnkId2 >= 0) aSurfaces[treeMesh.nUnkId2].RegisterReference(SURFACE_REFERENCE_TREEMESH_2);
			if (treeMesh.nTrunkSurfaceId >= 0) aSurfaces[treeMesh.nTrunkSurfaceId].RegisterReference(SURFACE_REFERENCE_TREEMESH_3);
			if (treeMesh.nBranchSurfaceId >= 0) aSurfaces[treeMesh.nBranchSurfaceId].RegisterReference(SURFACE_REFERENCE_TREEMESH_4);
			if (treeMesh.nLeafSurfaceId >= 0) aSurfaces[treeMesh.nLeafSurfaceId].RegisterReference(SURFACE_REFERENCE_TREEMESH_5);
		}

		aTreeMeshes.push_back(treeMesh);
	}
	return true;
}

tCrashData* GetCrashDataForModel(const std::string& name) {
	for (auto& crashData : aCrashData) {
		if (crashData.sName == name + "_crash") return &crashData;
	}
	return nullptr;
}

bool ParseW32Models(std::ifstream& file) {
	WriteConsole("Parsing models...", LOG_ALWAYS);

	uint32_t modelCount;
	ReadFromFile(file, &modelCount, 4);
	aModels.reserve(modelCount);
	for (int i = 0; i < modelCount; i++) {
		tModel model;
		ReadFromFile(file, &model.identifier, 4);
		if (model.identifier != 0x444F4D42) return false; // "BMOD"

		ReadFromFile(file, &model.nUnk, 4);
		model.sName = ReadStringFromFile(file);
		auto crashData = GetCrashDataForModel(model.sName);
		ReadFromFile(file, model.vCenter, sizeof(model.vCenter));
		ReadFromFile(file, model.vRadius, sizeof(model.vRadius));
		ReadFromFile(file, &model.fRadius, 4);
		uint32_t numSurfaces;
		ReadFromFile(file, &numSurfaces, sizeof(numSurfaces));
		if (crashData) {
			if (crashData->aSurfaces.size() != numSurfaces) {
				WriteConsole("ERROR: crash.dat mismatch, damage data will not be exported for " + model.sName + "!", LOG_ERRORS);
				WriteConsole(model.sName + " has " + std::to_string(numSurfaces) + " surfaces, but " + crashData->sName + " has " + std::to_string(crashData->aSurfaces.size()), LOG_ERRORS);
				crashData = nullptr;
			}
		}
		model._pCrashData = crashData;
		for (int j = 0; j < numSurfaces; j++) {
			int surface;
			ReadFromFile(file, &surface, sizeof(surface));
			model.aSurfaces.push_back(surface);

			if (surface >= aSurfaces.size()) {
				WriteConsole(std::format("ERROR: Invalid surface for model {}! ({})", model.sName, surface), LOG_ERRORS);
				return false;
			}
			if (crashData) {
				crashData->aSurfaces[j].vBuffer.flags = aSurfaces[surface].nFlags;
				aSurfaces[surface]._pCrashDataSurface = &crashData->aSurfaces[j];
			}
			aSurfaces[surface].RegisterReference(SURFACE_REFERENCE_MODEL);
		}
		aModels.push_back(model);
	}
	return true;
}

bool ParseW32Objects(std::ifstream& file) {
	WriteConsole("Parsing objects...", LOG_ALWAYS);

	uint32_t objectCount;
	ReadFromFile(file, &objectCount, 4);
	aObjects.reserve(objectCount);
	for (int i = 0; i < objectCount; i++) {
		tObject object;
		ReadFromFile(file, &object.identifier, 4);
		if (object.identifier != 0x434A424F) return false; // "OBJC"

		object.sName1 = ReadStringFromFile(file);
		object.sName2 = ReadStringFromFile(file);
		ReadFromFile(file, &object.nFlags, 4);
		ReadFromFile(file, object.mMatrix, sizeof(object.mMatrix));
		aObjects.push_back(object);
	}
	return true;
}

bool ParseW32CompactMeshes(std::ifstream& file, uint32_t mapVersion) {
	WriteConsole("Parsing compact meshes...", LOG_ALWAYS);

	uint32_t compactMeshCount;
	ReadFromFile(file, &nCompactMeshGroupCount, 4);
	ReadFromFile(file, &compactMeshCount, 4);
	aCompactMeshes.reserve(compactMeshCount);
	for (int i = 0; i < compactMeshCount; i++) {
		tCompactMesh compactMesh;
		ReadFromFile(file, &compactMesh.identifier, 4);
		if (compactMesh.identifier != 0x4853454D) return false; // "MESH"

		compactMesh.sName1 = ReadStringFromFile(file);
		compactMesh.sName2 = ReadStringFromFile(file);
		ReadFromFile(file, &compactMesh.nFlags, 4);
		ReadFromFile(file, &compactMesh.nGroup, 4);
		ReadFromFile(file, &compactMesh.mMatrix, sizeof(compactMesh.mMatrix));

		if (mapVersion >= 0x20000) {
			ReadFromFile(file, &compactMesh.nUnk1, 4);
			ReadFromFile(file, &compactMesh.nDamageAssocId, 4);

			auto bboxAssoc = aMeshDamageAssoc[compactMesh.nDamageAssocId];
			auto bbox = aCollidableModels[bboxAssoc.nIds[0]];
			compactMesh.aModels = bbox.aModels;
		}
		else {
			uint32_t nLODCount;
			ReadFromFile(file, &nLODCount, 4);
			compactMesh.aModels.reserve(nLODCount);
			for (int j = 0; j < nLODCount; j++) {
				uint32_t nMeshIndex;
				ReadFromFile(file, &nMeshIndex, 4);
				compactMesh.aModels.push_back(nMeshIndex);
			}
		}
		for (auto& id : compactMesh.aModels) {
			auto mdl = &aModels[id];
			for (auto& surfId : mdl->aSurfaces) {
				auto surf = &aSurfaces[surfId];
				surf->_nLODLevel = &id - &compactMesh.aModels[0];
			}
		}
		aCompactMeshes.push_back(compactMesh);
	}
	return true;
}

bool ParseW32CollidableModels(std::ifstream& file) {
	WriteConsole("Parsing collidable models...", LOG_ALWAYS);

	uint32_t colCount;
	ReadFromFile(file, &colCount, 4);
	aCollidableModels.reserve(colCount);
	for (int i = 0; i < colCount; i++) {
		tCollidableModel collidableModel;

		uint32_t modelCount;
		ReadFromFile(file, &modelCount, 4);
		for (int j = 0; j < modelCount; j++) {
			uint32_t modelId;
			ReadFromFile(file, &modelId, 4);
			collidableModel.aModels.push_back(modelId);
		}

		ReadFromFile(file, collidableModel.vCenter, sizeof(collidableModel.vCenter));
		ReadFromFile(file, collidableModel.vRadius, sizeof(collidableModel.vRadius));
		aCollidableModels.push_back(collidableModel);
	}
	return true;
}

bool ParseW32MeshDamageAssoc(std::ifstream& file) {
	WriteConsole("Parsing mesh damage associations...", LOG_ALWAYS);

	uint32_t assocCount;
	ReadFromFile(file, &assocCount, 4);
	aMeshDamageAssoc.reserve(assocCount);
	for (int i = 0; i < assocCount; i++) {
		tMeshDamageAssoc assoc;
		assoc.sName = ReadStringFromFile(file);
		ReadFromFile(file, assoc.nIds, sizeof(assoc.nIds));
		aMeshDamageAssoc.push_back(assoc);
	}
	return true;
}

bool ParseVertexColors(const std::string& fileName) {
	WriteConsole("Parsing vertex colors...", LOG_ALWAYS);

	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	if (!file.is_open()) return false;

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	if (nImportFileVersion <= 0x10003) {
		aVertexColors.reserve(fileSize);
		for (int i = 0; i < fileSize; i++) {
			uint8_t color;
			file.read((char*)&color, sizeof(color));
			uint8_t tmp[] = {color,color,color,color};
			aVertexColors.push_back(*(uint32_t*)tmp);
		}
	}
	else {
		aVertexColors.reserve(fileSize / 4);
		for (int i = 0; i < fileSize / 4; i++) {
			uint32_t color;
			file.read((char*)&color, sizeof(color));
			aVertexColors.push_back(color);
		}
	}
	return true;
}

bool ParseBGMMeshes(std::ifstream& file) {
	WriteConsole("Parsing BGM meshes...", LOG_ALWAYS);

	uint32_t meshCount;
	ReadFromFile(file, &meshCount, 4);
	aCompactMeshes.reserve(meshCount);
	for (int i = 0; i < meshCount; i++) {
		tCompactMesh mesh;
		ReadFromFile(file, &mesh.identifier, 4);
		if (mesh.identifier != 0x4853454D) return false; // "MESH"

		mesh.sName1 = ReadStringFromFile(file);
		mesh.sName2 = ReadStringFromFile(file);
		ReadFromFile(file, &mesh.nFlags, 4);
		ReadFromFile(file, &mesh.nGroup, 4);
		ReadFromFile(file, mesh.mMatrix, sizeof(mesh.mMatrix));
		int numModels;
		ReadFromFile(file, &numModels, 4);
		for (int j = 0; j < numModels; j++) {
			int id;
			ReadFromFile(file, &id, 4);
			mesh.aModels.push_back(id);
		}
		aCompactMeshes.push_back(mesh);
	}

	return true;
}

bool ParseW32RetroDemoSurfaces(std::ifstream& file) {
	WriteConsole("Parsing surfaces...", LOG_ALWAYS);

	uint32_t count;
	ReadFromFile(file, &count, 4);
	for (int i = 0; i < count; i++) {
		tModel model;
		ReadFromFile(file, &model.identifier, 4);
		int tmp;
		ReadFromFile(file, &tmp, 4);

		if (bIsToughTrucksModel) {
			model.sName = ReadStringFromFile(file);
			ReadFromFile(file, &tmp, 4);
		}
		ReadFromFile(file, model.vCenter, sizeof(model.vCenter));
		ReadFromFile(file, model.vRadius, sizeof(model.vRadius));
		uint32_t surfaceCount;
		ReadFromFile(file, &surfaceCount, 4);
		model.aSurfaces.reserve(surfaceCount);
		for (int j = 0; j < surfaceCount; j++) {
			int id = aVertexBuffers.size() + aIndexBuffers.size();

			tSurface surface;

			tVertexBuffer buf;
			buf.id = id;
			ReadFromFile(file, &tmp, 4); // identifier, BATC
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &surface.nMaterialId, 4);
			ReadFromFile(file, &buf.vertexCount, 4);
			ReadFromFile(file, &buf.flags, 4);
			uint32_t dataSize;
			ReadFromFile(file, &dataSize, 4);
			buf.vertexSize = dataSize / buf.vertexCount;
			buf.data = new float[dataSize / 4];
			ReadFromFile(file, buf.data, dataSize);

			tIndexBuffer iBuf;
			iBuf.id = id + 1;
			ReadFromFile(file, &tmp, 4);
			ReadFromFile(file, &surface.nPolyMode, 4);
			ReadFromFile(file, &dataSize, 4);
			iBuf.indexCount = dataSize / 2;
			iBuf.data = new uint16_t[dataSize / 2];
			ReadFromFile(file, iBuf.data, dataSize);

			surface.nVertexCount = buf.vertexCount;
			surface.nFlags = buf.flags;
			surface.nNumIndicesUsed = iBuf.indexCount;
			surface.nPolyCount = surface.nPolyMode == 4 ? iBuf.indexCount / 3 : iBuf.indexCount - 2;
			surface.nNumStreamsUsed = 2;
			if (surface.nPolyMode == 0) {
				surface.nPolyCount = 0;
				buf.isVegetation = true;
				surface.nNumStreamsUsed = 1;
			}
			surface.nStreamId[0] = id;
			surface.nStreamId[1] = id + 1;
			surface.nStreamOffset[0] = 0;
			surface.nStreamOffset[1] = 0;
			if (bIsToughTrucksModel) {
				ReadFromFile(file, &tmp, 4);
			}
			ReadFromFile(file, surface.vCenter, sizeof(surface.vCenter));
			ReadFromFile(file, surface.vRadius, sizeof(surface.vRadius));

			model.aSurfaces.push_back(aSurfaces.size());
			aSurfaces.push_back(surface);

			aVertexBuffers.push_back(buf);
			aIndexBuffers.push_back(iBuf);
		}
		aModels.push_back(model);
	}
	return true;
}

bool ParseW32RetroDemoTMOD(std::ifstream& file) {
	WriteConsole("Parsing TMOD...", LOG_ALWAYS);

	uint32_t count;
	ReadFromFile(file, &count, 4);
	for (int i = 0; i < count; i++) {
		tRetroDemoTMOD tmod;
		ReadFromFile(file, &tmod.identifier, 4); // TMOD
		ReadFromFile(file, &tmod.someId, 4);

		int numVertices;
		ReadFromFile(file, &numVertices, 4);
		tmod.aVertices.reserve(numVertices);
		for (int j = 0; j < numVertices; j++) {
			float aTmp[3];
			ReadFromFile(file, &aTmp, 0xC);
			tmod.aVertices.push_back({aTmp[0],aTmp[1],aTmp[2]});
		}

		int numIndices;
		ReadFromFile(file, &numIndices, 4);
		for (int j = 0; j < numIndices; j++) {
			int aTmp[3];
			ReadFromFile(file, &aTmp, 0xC);
			tmod.aIndices.push_back(aTmp[0]);
			tmod.aIndices.push_back(aTmp[1]);
			tmod.aIndices.push_back(aTmp[2]);
		}
		aRetroDemoTMOD.push_back(tmod);
	}
	return true;
}

bool ParseW32RetroDemoCompactMeshes(std::ifstream& file) {
	WriteConsole("Parsing compact meshes...", LOG_ALWAYS);

	uint32_t count;
	ReadFromFile(file, &count, 4);
	for (int i = 0; i < count; i++) {
		tCompactMesh mesh;
		ReadFromFile(file, &mesh.identifier, 4);
		mesh.sName1 = ReadStringFromFile(file);
		mesh.sName2 = ReadStringFromFile(file);
		ReadFromFile(file, &mesh.nFlags, 4);
		ReadFromFile(file, mesh.mMatrix, sizeof(mesh.mMatrix));
		uint32_t numModels;
		ReadFromFile(file, &numModels, 4);
		mesh.aModels.reserve(numModels);
		for (int j = 0; j < numModels; j++) {
			uint32_t tmp;
			ReadFromFile(file, &tmp, 4);
			mesh.aModels.push_back(tmp);

			for (auto& id : aModels[tmp].aSurfaces) {
				aSurfaces[id].RegisterReference(SURFACE_REFERENCE_MODEL);
			}
		}

		for (auto& id : mesh.aModels) {
			auto mdl = &aModels[id];
			for (auto& surfId : mdl->aSurfaces) {
				auto surf = &aSurfaces[surfId];
				surf->_nLODLevel = &id - &mesh.aModels[0];
			}
		}

		// todo what are these?
		int32_t tmp;
		ReadFromFile(file, &tmp, 4);
		ReadFromFile(file, &tmp, 4);
		if (!bIsBGMModel && mesh.sName2.empty()) {
			for (auto& lod : mesh.aModels) {
				for (auto& id : aModels[lod].aSurfaces) {
					tStaticBatch batch;
					batch.nId1 = aStaticBatches.size();
					batch.nBVHId1 = id;
					auto& surface = aSurfaces[0];
					memcpy(batch.vCenter, surface.vCenter, sizeof(batch.vCenter));
					memcpy(batch.vRadius, surface.vRadius, sizeof(batch.vRadius));
					aStaticBatches.push_back(batch);
				}
			}
		}
		else {
			aCompactMeshes.push_back(mesh);
		}
	}

	if (bIsBGMModel) {
		uint32_t count;
		ReadFromFile(file, &count, 4);
		for (int i = 0; i < count; i++) {
			tObject obj;
			ReadFromFile(file, &obj.identifier, 4);
			if (obj.identifier != 0x434A424F) return false; // OBJC

			obj.sName1 = ReadStringFromFile(file);
			obj.sName2 = ReadStringFromFile(file);
			ReadFromFile(file, &obj.nFlags, 4);
			ReadFromFile(file, obj.mMatrix, sizeof(obj.mMatrix));
			aObjects.push_back(obj);
		}
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

	// create tire dummies
	if (bIsBGMModel) {
		for (auto &compactMesh: aCompactMeshes) {
			if (compactMesh.sName1.starts_with("tire_") && !compactMesh.sName1.ends_with("shadow")) {
				tObject object;
				object.sName1 = "placeholder_" + compactMesh.sName1;
				object.sName2 = compactMesh.sName2;
				object.nFlags = compactMesh.nFlags;
				memcpy(object.mMatrix, compactMesh.mMatrix, sizeof(object.mMatrix));
				aObjects.push_back(object);
			}
		}
	}

	return true;
}

// read the first stream to see if it has FOUC flags, it should usually be the vertex buffer
void PreParseW32Streams(std::ifstream& file) {
	uint32_t numStreams;
	ReadFromFile(file, &numStreams, 4);
	int dataType;
	ReadFromFile(file, &dataType, 4);
	if (dataType == 1) {
		tVertexBuffer buf;
		ReadFromFile(file, &buf.foucExtraFormat, 4);
		if (buf.foucExtraFormat > 0) bIsFOUCModel = true;
	}
	else if (dataType == 2) {
		tIndexBuffer buf;
		ReadFromFile(file, &buf.foucExtraFormat, 4);
		if (buf.foucExtraFormat > 0) bIsFOUCModel = true;
	}
	else if (dataType == 4 || dataType == 5) {
		bIsXboxBetaModel = true;
	}
}

void PreParseBGMForFileVersion() {
	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return;

	ReadFromFile(fin, &nImportFileVersion, 4);
	if (nImportFileVersion == 0x20002) bIsFOUCModel = true;
	bIsBGMModel = true;

	if (!ParseW32Materials(fin, false)) return;
	PreParseW32Streams(fin);
	aMaterials.clear();
}

bool ParseW32() {
	if (sFileName.extension() == ".bmf" || sFileName.extension() == ".BMF") {
		bIsRallyTrophyModel = true;
		return ParseRallyTrophyBMF();
	}

	if (sFileName.extension() == ".bgm") {
		PreParseBGMForFileVersion();
		bIsBGMModel = true;
	}
	else if (sFileName.extension() == ".car") {
		bIsBGMModel = true;
	}

	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	ReadFromFile(fin, &nImportFileVersion, 4);
	// retro demo CARC, tough trucks BGMC
	if (nImportFileVersion == 0x43524143 || nImportFileVersion == 0x434D4742) {
		if (nImportFileVersion == 0x434D4742) bIsToughTrucksModel = true;
		ReadFromFile(fin, &nImportFileVersion, 4);
		bIsBGMModel = true;
	}

	if (nImportFileVersion == 0x62647370) {
		WriteConsole("ERROR: Plant W32 files are not supported!", LOG_ERRORS);
		return false;
	}

	if (nImportFileVersion > 0x20000) {
		ReadFromFile(fin, &nSomeMapValue, 4);
		for (int i = 0; i < nSomeMapValue - 1; i++) {
			int tmp = 0;
			ReadFromFile(fin, &tmp, 4);
		}
	}
	if (nImportFileVersion == 0x20002) bIsFOUCModel = true;
	else if (!bIsBGMModel) {
		if (nImportFileVersion <= 0x10003) {
			auto vertexColorsPath = sFileNameNoExt.string() + "_sky_vtx.rad";
			if (!ParseVertexColors(vertexColorsPath) && bDumpIntoFBX) {
				WriteConsole("ERROR: Failed to load " + (std::string) vertexColorsPath + "!", LOG_ERRORS);
				WaitAndExitOnFail();
			}
		} else {
			auto vertexColorsPath = sFileNameNoExt.string() + "_vertexcolors.w32";
			if (!std::filesystem::exists(vertexColorsPath))
				vertexColorsPath = sFileNameNoExt.string() + "_vertexcolors_w2.w32";
			if (!std::filesystem::exists(vertexColorsPath))
				vertexColorsPath = sFileFolder.string() + "vertexcolors_w2.w32";
			if (!ParseVertexColors(vertexColorsPath) && bDumpIntoFBX) {
				WriteConsole("ERROR: Failed to load " + (std::string) vertexColorsPath + "!", LOG_ERRORS);
				WaitAndExitOnFail();
			}
		}
	}

	if (bIsBGMModel) {
		// first look for modelname_crash.dat, then look for crash.dat in the folder
		auto crashDatPath = sFileNameNoExt.string() + "_crash.dat";
		if (!std::filesystem::exists(crashDatPath)) crashDatPath = sFileFolder.string() + "crash.dat";

		if (nImportFileVersion >= 0x10004 && !ParseCrashDat(crashDatPath)) {
			WriteConsole(
					"WARNING: Failed to load " + (std::string) crashDatPath + ", damage data will not be exported",
					LOG_WARNINGS);
		}
	}
	else {
		if (bDumpIntoW32 || bDumpIntoTextFile) {
			auto bvhPath = sFileNameNoExt.string() + "_bvh.gen";
			if (!std::filesystem::exists(bvhPath)) bvhPath = sFileFolder.string() + "track_bvh.gen";

			if (!ParseTrackBVH(bvhPath)) {
				WriteConsole("WARNING: Failed to load " + (std::string) bvhPath + ", culling data will not be updated",
							 LOG_WARNINGS);
			}
		}

		auto splitPointsPath = sFileNameNoExt.string() + "_splitpoints.bed";
		if (!std::filesystem::exists(splitPointsPath)) splitPointsPath = sFileFolder.string() + "splitpoints.bed";
		if (!ParseSplitpoints(splitPointsPath)) {
			WriteConsole("WARNING: Failed to load " + (std::string) splitPointsPath + "!", LOG_WARNINGS);
		}

		auto startPointsPath = sFileNameNoExt.string() + "_startpoints.bed";
		if (!std::filesystem::exists(startPointsPath)) startPointsPath = sFileFolder.string() + "startpoints.bed";
		if (!ParseStartpoints(startPointsPath)) {
			WriteConsole("WARNING: Failed to load " + (std::string) startPointsPath + "!", LOG_WARNINGS);
		}

		auto splinesPath = sFileNameNoExt.string() + "_splines.ai";
		if (!std::filesystem::exists(splinesPath)) splinesPath = sFileFolder.string() + "splines.ai";
		if (!ParseSplines(splinesPath)) {
			WriteConsole("WARNING: Failed to load " + (std::string) splinesPath + "!", LOG_WARNINGS);
		}
	}

	if (!ParseW32Materials(fin)) return false;

	if (nImportFileVersion <= 0x10003) {
		if (!ParseW32RetroDemoSurfaces(fin)) return false;
		if (!bIsBGMModel && !ParseW32RetroDemoTMOD(fin)) return false;
		if (!ParseW32RetroDemoCompactMeshes(fin)) return false;
		return true;
	}

	if (!ParseW32Streams(fin)) return false;
	if (!ParseW32Surfaces(fin, nImportFileVersion)) return false;
	if (bIsBGMModel) {
		if (!ParseW32Models(fin)) return false;
		if (!ParseBGMMeshes(fin)) return false;
		if (!ParseW32Objects(fin)) return false;
	}
	else {
		if (!ParseW32StaticBatches(fin, nImportFileVersion)) return false;

		WriteConsole("Parsing tree colors...", LOG_ALWAYS);

		if (nImportFileVersion != 0x20002) {
			uint32_t treeColourCount;
			ReadFromFile(fin, &treeColourCount, 4);
			for (int i = 0; i < treeColourCount; i++) {
				uint32_t color;
				ReadFromFile(fin, &color, 4);
				aTreeColors.push_back(color);
			}
		}

		WriteConsole("Parsing tree LODs...", LOG_ALWAYS);

		uint32_t treeLodCount;
		ReadFromFile(fin, &treeLodCount, 4);
		for (int i = 0; i < treeLodCount; i++) {
			tTreeLOD treeLod;
			ReadFromFile(fin, treeLod.vPos, sizeof(treeLod.vPos));
			ReadFromFile(fin, treeLod.fScale, sizeof(treeLod.fScale));
			ReadFromFile(fin, treeLod.nValues, sizeof(treeLod.nValues));
			aTreeLODs.push_back(treeLod);
		}

		if (!ParseW32TreeMeshes(fin)) return false;

		WriteConsole("Parsing collision offset matrix...", LOG_ALWAYS);
		if (nImportFileVersion >= 0x10004) {
			ReadFromFile(fin, aTrackCollisionOffsetMatrix, sizeof(aTrackCollisionOffsetMatrix));
		}

		if (!ParseW32Models(fin)) return false;
		if (!ParseW32Objects(fin)) return false;

		if (nImportFileVersion >= 0x20000) {
			if (!ParseW32CollidableModels(fin)) return false;
			if (!ParseW32MeshDamageAssoc(fin)) return false;
		}

		if (!ParseW32CompactMeshes(fin, nImportFileVersion)) return false;
	}

	WriteConsole("Parsing finished", LOG_ALWAYS);
	return true;
}