bool ParseW32Materials(std::ifstream& file) {
	WriteConsole("Parsing materials...");

	uint32_t numMaterials;
	ReadFromFile(file, &numMaterials, 4);
	aMaterials.reserve(numMaterials);
	for (int i = 0; i < numMaterials; i++) {
		tMaterial material;
		ReadFromFile(file, &material.identifier, 4);
		if (material.identifier != 0x4354414D) return false; // "MATC"

		material.sName = ReadStringFromFile(file);
		ReadFromFile(file, &material.nAlpha, 4);
		ReadFromFile(file, &material.v92, 4);
		ReadFromFile(file, &material.nNumTextures, 4);
		ReadFromFile(file, &material.nShaderId, 4);
		ReadFromFile(file, &material.nUseColormap, 4);
		ReadFromFile(file, &material.v74, 4);
		ReadFromFile(file, material.v108, 12);
		ReadFromFile(file, material.v109, 12);
		ReadFromFile(file, material.v98, 16);
		ReadFromFile(file, material.v99, 16);
		ReadFromFile(file, material.v100, 16);
		ReadFromFile(file, material.v101, 16);
		ReadFromFile(file, &material.v102, 4);
		material.sTextureNames[0] = ReadStringFromFile(file);
		material.sTextureNames[1] = ReadStringFromFile(file);
		material.sTextureNames[2] = ReadStringFromFile(file);
		aMaterials.push_back(material);
	}
	return true;
}

bool ParseW32Streams(std::ifstream& file) {
	WriteConsole("Parsing streams...");

	uint32_t numStreams;
	ReadFromFile(file, &numStreams, 4);
	for (int i = 0; i < numStreams; i++) {
		int dataType;
		ReadFromFile(file, &dataType, 4);
		if (dataType == 1) {
			tVertexBuffer buf;
			ReadFromFile(file, &buf.foucExtraFormat, 4);
			ReadFromFile(file, &buf.vertexCount, 4);
			ReadFromFile(file, &buf.vertexSize, 4);
			ReadFromFile(file, &buf.flags, 4);

			if (nImportMapVersion >= 0x20002) { // no clue what or why or when or how, this is a bugbear specialty
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
			else if (nImportMapVersion < 0x20002) {
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
			ReadFromFile(file, &buf.indexCount, 4);

			std::vector<uint16_t> aValues;

			int remainingIndexCount = buf.indexCount;
			if (nImportMapVersion >= 0x20002) {
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
		else {
			WriteConsole("Unknown stream type " + std::to_string(dataType));
			return false;
		}
	}
	return true;
}

bool ParseW32Surfaces(std::ifstream& file, int mapVersion) {
	WriteConsole("Parsing surfaces...");

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

		if (mapVersion < 0x20000) {
			ReadFromFile(file, surface.vAbsoluteCenter, 12);
			ReadFromFile(file, surface.vRelativeCenter, 12);
		}

		if (mapVersion >= 0x20002) {
			ReadFromFile(file, surface.foucVertexMultiplier, sizeof(surface.foucVertexMultiplier));
		}

		ReadFromFile(file, &surface.nNumStreamsUsed, 4);
		if (surface.nNumStreamsUsed <= 0 || surface.nNumStreamsUsed > 2) return false;

		for (int j = 0; j < surface.nNumStreamsUsed; j++) {
			ReadFromFile(file, &surface.nStreamId[j], 4);
			ReadFromFile(file, &surface.nStreamOffset[j], 4);
		}

		auto id = surface.nStreamId[0];
		auto vBuf = FindVertexBuffer(id);
		auto vegvBuf = FindVegVertexBuffer(id);
		if (!vBuf && !vegvBuf) return false;
		if (nImportMapVersion >= 0x20002 && vBuf) {
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
	WriteConsole("Parsing static batches...");

	uint32_t numStaticBatches;
	ReadFromFile(file, &numStaticBatches, 4);
	aStaticBatches.reserve(numStaticBatches);
	for (int i = 0; i < numStaticBatches; i++) {
		tStaticBatch staticBatch;
		ReadFromFile(file, &staticBatch.nCenterId1, 4);
		ReadFromFile(file, &staticBatch.nCenterId2, 4);
		ReadFromFile(file, &staticBatch.nSurfaceId, 4);

		bool bIsSurfaceValid = staticBatch.nSurfaceId < aSurfaces.size();
		if (nImportMapVersion < 0x20002 && !bIsSurfaceValid) return false;

		if (bIsSurfaceValid) {
			aSurfaces[staticBatch.nSurfaceId].RegisterReference(SURFACE_REFERENCE_STATICBATCH);
		}

		if (mapVersion >= 0x20000) {
			ReadFromFile(file, staticBatch.vAbsoluteCenter, 12);
			ReadFromFile(file, staticBatch.vRelativeCenter, 12);

			if (bIsSurfaceValid) {
				// backwards compatibility
				memcpy(aSurfaces[staticBatch.nSurfaceId].vAbsoluteCenter, staticBatch.vAbsoluteCenter, 12);
				memcpy(aSurfaces[staticBatch.nSurfaceId].vRelativeCenter, staticBatch.vRelativeCenter, 12);
			}
		}
		else {
			ReadFromFile(file, &staticBatch.nUnk, 4); // always 0?

			if (bIsSurfaceValid) {
				// forwards compatibility
				memcpy(staticBatch.vAbsoluteCenter, aSurfaces[staticBatch.nSurfaceId].vAbsoluteCenter, 12);
				memcpy(staticBatch.vRelativeCenter, aSurfaces[staticBatch.nSurfaceId].vRelativeCenter, 12);
			}
		}

		aStaticBatches.push_back(staticBatch);
	}
	return true;
}

bool ParseW32TreeMeshes(std::ifstream& file) {
	WriteConsole("Parsing tree meshes...");

	uint32_t treeMeshCount;
	ReadFromFile(file, &treeMeshCount, 4);
	aTreeMeshes.reserve(treeMeshCount);
	for (int i = 0; i < treeMeshCount; i++) {
		tTreeMesh treeMesh;
		ReadFromFile(file, &treeMesh.nUnk1, 4);
		ReadFromFile(file, &treeMesh.nUnk2Unused, 4);
		ReadFromFile(file, &treeMesh.nSurfaceId1Unused, 4);
		ReadFromFile(file, &treeMesh.nSurfaceId2, 4);
		ReadFromFile(file, treeMesh.fUnk, sizeof(treeMesh.fUnk));

		if (nImportMapVersion >= 0x20002) {
			if (treeMesh.nSurfaceId2 >= 0 && treeMesh.nSurfaceId2 < aSurfaces.size()) aSurfaces[treeMesh.nSurfaceId2].RegisterReference(SURFACE_REFERENCE_TREEMESH_2);

			ReadFromFile(file, treeMesh.foucExtraData1, sizeof(treeMesh.foucExtraData1));
			ReadFromFile(file, treeMesh.foucExtraData2, sizeof(treeMesh.foucExtraData2));
			ReadFromFile(file, treeMesh.foucExtraData3, sizeof(treeMesh.foucExtraData3));
			ReadFromFile(file, &treeMesh.nSurfaceId3, 4);
			ReadFromFile(file, &treeMesh.nSurfaceId4, 4);
			ReadFromFile(file, &treeMesh.nSurfaceId5, 4);

			if (treeMesh.nSurfaceId3 >= 0 && treeMesh.nSurfaceId3 < aSurfaces.size()) aSurfaces[treeMesh.nSurfaceId3].RegisterReference(SURFACE_REFERENCE_TREEMESH_3);
			if (treeMesh.nSurfaceId4 >= 0 && treeMesh.nSurfaceId4 < aSurfaces.size()) aSurfaces[treeMesh.nSurfaceId4].RegisterReference(SURFACE_REFERENCE_TREEMESH_4);
			if (treeMesh.nSurfaceId5 >= 0 && treeMesh.nSurfaceId5 < aSurfaces.size()) aSurfaces[treeMesh.nSurfaceId5].RegisterReference(SURFACE_REFERENCE_TREEMESH_5);
		}
		else {
			ReadFromFile(file, &treeMesh.nSurfaceId3, 4);
			ReadFromFile(file, &treeMesh.nSurfaceId4, 4);
			ReadFromFile(file, &treeMesh.nSurfaceId5, 4);
			ReadFromFile(file, &treeMesh.nIdInUnkArray1, 4);
			ReadFromFile(file, &treeMesh.nIdInUnkArray2, 4);
			ReadFromFile(file, &treeMesh.nMaterialId, 4);

			//if (treeMesh.nSurfaceId1 >= 0 && treeMesh.nSurfaceId1 >= aSurfaces.size()) return false;
			//if (treeMesh.nSurfaceId1 >= 0) aSurfaces[treeMesh.nSurfaceId1]._bUsedByAnything = true;

			if (treeMesh.nSurfaceId2 >= 0 && treeMesh.nSurfaceId2 >= aSurfaces.size()) return false;
			if (treeMesh.nSurfaceId3 >= 0 && treeMesh.nSurfaceId3 >= aSurfaces.size()) return false;
			if (treeMesh.nSurfaceId4 >= 0 && treeMesh.nSurfaceId4 >= aSurfaces.size()) return false;
			if (treeMesh.nSurfaceId5 >= 0 && treeMesh.nSurfaceId5 >= aSurfaces.size()) return false;
			if (treeMesh.nSurfaceId2 >= 0) aSurfaces[treeMesh.nSurfaceId2].RegisterReference(SURFACE_REFERENCE_TREEMESH_2);
			if (treeMesh.nSurfaceId3 >= 0) aSurfaces[treeMesh.nSurfaceId3].RegisterReference(SURFACE_REFERENCE_TREEMESH_3);
			if (treeMesh.nSurfaceId4 >= 0) aSurfaces[treeMesh.nSurfaceId4].RegisterReference(SURFACE_REFERENCE_TREEMESH_4);
			if (treeMesh.nSurfaceId5 >= 0) aSurfaces[treeMesh.nSurfaceId5].RegisterReference(SURFACE_REFERENCE_TREEMESH_5);
		}

		aTreeMeshes.push_back(treeMesh);
	}
	return true;
}

bool ParseW32Models(std::ifstream& file) {
	WriteConsole("Parsing models...");

	uint32_t modelCount;
	ReadFromFile(file, &modelCount, 4);
	aModels.reserve(modelCount);
	for (int i = 0; i < modelCount; i++) {
		tModel model;
		ReadFromFile(file, &model.identifier, 4);
		if (model.identifier != 0x444F4D42) return false; // "BMOD"

		ReadFromFile(file, &model.nUnk, 4);
		model.sName = ReadStringFromFile(file);
		ReadFromFile(file, model.vCenter, sizeof(model.vCenter));
		ReadFromFile(file, model.vRadius, sizeof(model.vRadius));
		ReadFromFile(file, &model.fRadius, 4);
		uint32_t numSurfaces;
		ReadFromFile(file, &numSurfaces, sizeof(numSurfaces));
		for (int j = 0; j < numSurfaces; j++) {
			int surface;
			ReadFromFile(file, &surface, sizeof(surface));
			model.aSurfaces.push_back(surface);

			if (surface >= aSurfaces.size()) return false;
			aSurfaces[surface].RegisterReference(SURFACE_REFERENCE_MODEL);
		}
		aModels.push_back(model);
	}
	return true;
}

bool ParseW32Objects(std::ifstream& file) {
	WriteConsole("Parsing objects...");

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
	WriteConsole("Parsing compact meshes...");

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
			ReadFromFile(file, &compactMesh.nBBoxAssocId, 4);

			auto bboxAssoc = aBoundingBoxMeshAssoc[compactMesh.nBBoxAssocId];
			auto bbox = aBoundingBoxes[bboxAssoc.nIds[0]];
			compactMesh.aLODMeshIds = bbox.aModels;
		}
		else {
			uint32_t nLODCount;
			ReadFromFile(file, &nLODCount, 4);
			compactMesh.aLODMeshIds.reserve(nLODCount);
			for (int j = 0; j < nLODCount; j++) {
				uint32_t nMeshIndex;
				ReadFromFile(file, &nMeshIndex, 4);
				compactMesh.aLODMeshIds.push_back(nMeshIndex);
			}
		}
		aCompactMeshes.push_back(compactMesh);
	}
	return true;
}

bool ParseW32BoundingBoxes(std::ifstream& file) {
	WriteConsole("Parsing bounding boxes...");

	uint32_t boundingBoxCount;
	ReadFromFile(file, &boundingBoxCount, 4);
	aBoundingBoxes.reserve(boundingBoxCount);
	for (int i = 0; i < boundingBoxCount; i++) {
		tBoundingBox boundingBox;

		uint32_t modelCount;
		ReadFromFile(file, &modelCount, 4);
		for (int j = 0; j < modelCount; j++) {
			uint32_t modelId;
			ReadFromFile(file, &modelId, 4);
			boundingBox.aModels.push_back(modelId);
		}

		ReadFromFile(file, boundingBox.vCenter, sizeof(boundingBox.vCenter));
		ReadFromFile(file, boundingBox.vRadius, sizeof(boundingBox.vRadius));
		aBoundingBoxes.push_back(boundingBox);
	}
	return true;
}

bool ParseW32BoundingBoxMeshAssoc(std::ifstream& file) {
	WriteConsole("Parsing bounding box mesh associations...");

	uint32_t assocCount;
	ReadFromFile(file, &assocCount, 4);
	aBoundingBoxMeshAssoc.reserve(assocCount);
	for (int i = 0; i < assocCount; i++) {
		tBoundingBoxMeshAssoc assoc;
		assoc.sName = ReadStringFromFile(file);
		ReadFromFile(file, assoc.nIds, sizeof(assoc.nIds));
		aBoundingBoxMeshAssoc.push_back(assoc);
	}
	return true;
}

bool ParseVertexColors(const std::string& fileName) {
	std::ifstream file(fileName, std::ios::in | std::ios::binary);
	if (!file.is_open()) return false;

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	aVertexColors.reserve(fileSize / 4);
	for (int i = 0; i < fileSize / 4; i++) {
		uint32_t color;
		file.read((char*)&color, sizeof(color));
		aVertexColors.push_back(color);
	}
	return true;
}

bool ParseW32(const std::string& fileName) {
	if (!sFileName.ends_with(".w32")) {
		return false;
	}

	std::ifstream fin(fileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	ReadFromFile(fin, &nImportMapVersion, 4);
	if (nImportMapVersion > 0x20000) ReadFromFile(fin, &nSomeMapValue, 4);
	if (nImportMapVersion < 0x20002) {
		auto vertexColorsPath = "vertexcolors_w2.w32";
		if (!ParseVertexColors(vertexColorsPath)) {
			WriteConsole("Failed to load " + (std::string)vertexColorsPath + ", vertex colors will not be exported");
		}
	}

	if (!ParseW32Materials(fin)) return false;
	if (!ParseW32Streams(fin)) return false;
	if (!ParseW32Surfaces(fin, nImportMapVersion)) return false;
	if (!ParseW32StaticBatches(fin, nImportMapVersion)) return false;

	WriteConsole("Parsing tree-related data...");

	if (nImportMapVersion < 0x20002) {
		uint32_t someCount;
		ReadFromFile(fin, &someCount, 4);
		for (int i = 0; i < someCount; i++) {
			int someValue;
			ReadFromFile(fin, &someValue, 4);
			aUnknownArray1.push_back(someValue);
		}
	}

	uint32_t someStructCount;
	ReadFromFile(fin, &someStructCount, 4);
	for (int i = 0; i < someStructCount; i++) {
		tUnknownStructure unkStruct;
		ReadFromFile(fin, unkStruct.vPos, sizeof(unkStruct.vPos));
		ReadFromFile(fin, unkStruct.fValues, sizeof(unkStruct.fValues));
		ReadFromFile(fin, unkStruct.nValues, sizeof(unkStruct.nValues));
		aUnknownArray2.push_back(unkStruct);
	}

	if (!ParseW32TreeMeshes(fin)) return false;

	WriteConsole("Parsing unknown data...");
	if (nImportMapVersion >= 0x10004) {
		for (int i = 0; i < 16; i++) {
			float value;
			ReadFromFile(fin, &value, sizeof(value));
			aUnknownArray3.push_back(value);
		}
	}

	if (!ParseW32Models(fin)) return false;
	if (!ParseW32Objects(fin)) return false;

	if (nImportMapVersion >= 0x20000) {
		if (!ParseW32BoundingBoxes(fin)) return false;
		if (!ParseW32BoundingBoxMeshAssoc(fin)) return false;
	}

	if (!ParseW32CompactMeshes(fin, nImportMapVersion)) return false;

	WriteConsole("Parsing finished");
	return true;
}