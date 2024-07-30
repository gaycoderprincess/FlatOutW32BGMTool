aiNode* CreateNodeForTreeMesh(aiScene* scene, const tTreeMesh& treeMesh) {
	std::vector<int> surfaceIds;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId2, true)) surfaceIds.push_back(treeMesh.nSurfaceId2);
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId3, true)) surfaceIds.push_back(treeMesh.nSurfaceId3);
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId4, true)) surfaceIds.push_back(treeMesh.nSurfaceId4);
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId5, true)) surfaceIds.push_back(treeMesh.nSurfaceId5);
	if (surfaceIds.empty()) return nullptr;

	auto node = new aiNode();
	node->mName = "TreeMesh" + std::to_string(&treeMesh - &aTreeMeshes[0]);
	node->mMeshes = new uint32_t[surfaceIds.size()];
	node->mNumMeshes = surfaceIds.size();
	int i = 0;
	for (auto& surfaceId : surfaceIds) {
		node->mMeshes[i++] = aSurfaces[surfaceId]._nFBXModelId;
	}
	return node;
}

void FillFBXMeshFromSurface(aiMesh* dest, tVertexBuffer* vBuf, tIndexBuffer* iBuf, tSurface& src, uint32_t vertexOffset, tCrashDataWeights* aCrashWeightsFO2 = nullptr, tCrashDataWeightsFOUC* aCrashWeightsFOUC = nullptr) {
	auto stride = vBuf->vertexSize;
	uintptr_t vertexData = ((uintptr_t)vBuf->data) + vertexOffset;
	uintptr_t indexData = ((uintptr_t)iBuf->data) + src.nStreamOffset[1];

	uint32_t baseVertexOffset = src.nStreamOffset[0] / vBuf->vertexSize;

	bool bHasNormals = (vBuf->flags & VERTEX_NORMAL) != 0;
	if (bIsFOUCModel && (vBuf->flags & VERTEX_COLOR) != 0) bHasNormals = true;

	dest->mVertices = new aiVector3D[src.nVertexCount];
	dest->mNumVertices = src.nVertexCount;
	dest->mFaces = new aiFace[src.nPolyCount];
	dest->mNumFaces = src.nPolyCount;
	dest->mTextureCoords[0] = new aiVector3D[src.nVertexCount];
	dest->mNumUVComponents[0] = 2;
	if ((vBuf->flags & VERTEX_UV2) != 0) {
		dest->mTextureCoords[1] = new aiVector3D[src.nVertexCount];
		dest->mNumUVComponents[1] = 2;
	}
	if (bHasNormals) {
		dest->mNormals = new aiVector3D[src.nVertexCount];
	}
	if ((vBuf->flags & VERTEX_COLOR) != 0) {
		dest->mColors[0] = new aiColor4D[src.nVertexCount];
	}
	if (bIsFOUCModel) {
		for (int j = 0; j < src.nVertexCount; j++) {
			auto vertices = (int16_t*)vertexData;

			if (aCrashWeightsFOUC) {
				dest->mVertices[j].x = aCrashWeightsFOUC->vCrashPos[0];
				dest->mVertices[j].y = aCrashWeightsFOUC->vCrashPos[1];
				dest->mVertices[j].z = aCrashWeightsFOUC->vCrashPos[2];
			}
			else {
				dest->mVertices[j].x = vertices[0];
				dest->mVertices[j].y = vertices[1];
				dest->mVertices[j].z = vertices[2];
			}
			dest->mVertices[j].x += src.foucVertexMultiplier[0];
			dest->mVertices[j].y += src.foucVertexMultiplier[1];
			dest->mVertices[j].z += src.foucVertexMultiplier[2];
			dest->mVertices[j].x *= src.foucVertexMultiplier[3];
			dest->mVertices[j].y *= src.foucVertexMultiplier[3];
			dest->mVertices[j].z *= -src.foucVertexMultiplier[3];
			vertices += 3;

			if (bHasNormals) {
				vertices += 4;
				auto int8Vertices = (uint8_t*)vertices;
				int8Vertices += 2;
				if (aCrashWeightsFOUC) {
					int8Vertices = aCrashWeightsFOUC->vCrashNormals;
				}
				dest->mNormals[j].z = ((int8Vertices[0] / 127.0) - 1) * -1;
				dest->mNormals[j].y = (int8Vertices[1] / 127.0) - 1;
				dest->mNormals[j].x = (int8Vertices[2] / 127.0) - 1;
				vertices += 3; // 3 floats
			}
			if ((vBuf->flags & VERTEX_COLOR) != 0) {
				auto vertexColorOffset = *(uint32_t*)&vertices[0];
				auto rgb = (uint8_t*)&vertexColorOffset;
				dest->mColors[0][j].r = rgb[0] / 255.0;
				dest->mColors[0][j].g = rgb[1] / 255.0;
				dest->mColors[0][j].b = rgb[2] / 255.0;
				dest->mColors[0][j].a = rgb[3] / 255.0;
				vertices += 2; // 1 int32
			}
			if ((vBuf->flags & VERTEX_UV) != 0 || (vBuf->flags & VERTEX_UV2) != 0) {
				dest->mTextureCoords[0][j].x = vertices[0] / 2048.0;
				dest->mTextureCoords[0][j].y = 1 - (vertices[1] / 2048.0);
				dest->mTextureCoords[0][j].z = 0;
				vertices += 2; // 2 floats
			}
			if ((vBuf->flags & VERTEX_UV2) != 0) {
				dest->mTextureCoords[1][j].x = vertices[0] / 2048.0;
				dest->mTextureCoords[1][j].y = 1 - (vertices[1] / 2048.0);
				dest->mTextureCoords[1][j].z = 0;
				vertices += 2; // 2 floats
			}
			vertexData += stride;
			if (aCrashWeightsFOUC) aCrashWeightsFOUC++;
		}
	}
	else {
		for (int j = 0; j < src.nVertexCount; j++) {
			auto vertices = (float*)vertexData;
			if (aCrashWeightsFO2) {
				dest->mVertices[j].x = aCrashWeightsFO2->vCrashPos[0];
				dest->mVertices[j].y = aCrashWeightsFO2->vCrashPos[1];
				dest->mVertices[j].z = -aCrashWeightsFO2->vCrashPos[2];
			}
			else {
				dest->mVertices[j].x = vertices[0];
				dest->mVertices[j].y = vertices[1];
				dest->mVertices[j].z = -vertices[2];
			}
			vertices += 3;

			if ((vBuf->flags & VERTEX_NORMAL) != 0) {
				if (aCrashWeightsFO2) {
					dest->mNormals[j].x = aCrashWeightsFO2->vCrashNormal[0];
					dest->mNormals[j].y = aCrashWeightsFO2->vCrashNormal[1];
					dest->mNormals[j].z = -aCrashWeightsFO2->vCrashNormal[2];
				}
				else {
					dest->mNormals[j].x = vertices[0];
					dest->mNormals[j].y = vertices[1];
					dest->mNormals[j].z = -vertices[2];
				}
				vertices += 3; // 3 floats
			}
			if ((vBuf->flags & VERTEX_COLOR) != 0) {
				auto vertexColorOffset = *(uint32_t*)&vertices[0];
				if (vertexColorOffset >= 0xFF000000 || bIsBGMModel) {
					auto rgb = (uint8_t*)&vertexColorOffset;
					dest->mColors[0][j].r = rgb[0] / 255.0;
					dest->mColors[0][j].g = rgb[1] / 255.0;
					dest->mColors[0][j].b = rgb[2] / 255.0;
					dest->mColors[0][j].a = rgb[3] / 255.0;
				}
				else if (!aVertexColors.empty()) {
					int id = vertexColorOffset & 0xFFFFFF;
					if (id >= aVertexColors.size()) {
						WriteConsole("ERROR: Vertex colors for surface " + std::to_string(&src - &aSurfaces[0]) + " out of bounds!", LOG_ERRORS);
						WriteConsole(std::to_string(id) + "/" + std::to_string(aVertexColors.size()), LOG_ERRORS);
						exit(0);
					}
					auto rgb = (uint8_t*)&aVertexColors[id];
					dest->mColors[0][j].r = rgb[0] / 255.0;
					dest->mColors[0][j].g = rgb[1] / 255.0;
					dest->mColors[0][j].b = rgb[2] / 255.0;
					dest->mColors[0][j].a = 1;
				}
				else {
					dest->mColors[0][j].r = 1;
					dest->mColors[0][j].g = 1;
					dest->mColors[0][j].b = 1;
					dest->mColors[0][j].a = 1;
				}
				vertices += 1; // 1 int32
			}
			if ((vBuf->flags & VERTEX_UV) != 0 || (vBuf->flags & VERTEX_UV2) != 0) {
				dest->mTextureCoords[0][j].x = vertices[0];
				dest->mTextureCoords[0][j].y = 1 - vertices[1];
				dest->mTextureCoords[0][j].z = 0;
				vertices += 2; // 2 floats
			}
			if ((vBuf->flags & VERTEX_UV2) != 0) {
				dest->mTextureCoords[1][j].x = vertices[0];
				dest->mTextureCoords[1][j].y = 1 - vertices[1];
				dest->mTextureCoords[1][j].z = 0;
				vertices += 2; // 2 floats
			}
			vertexData += stride;
			if (aCrashWeightsFO2) aCrashWeightsFO2++;
		}
	}

	if (src.nPolyMode == 5) {
		bool bFlip = false;
		for (int j = 0; j < src.nPolyCount; j++) {
			auto tmp = (uint16_t*)indexData;
			int indices[3] = {tmp[0], tmp[1], tmp[2]};
			indices[0] -= baseVertexOffset;
			indices[1] -= baseVertexOffset;
			indices[2] -= baseVertexOffset;
			if (indices[0] < 0 || indices[0] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[0]), LOG_ERRORS); exit(0); }
			if (indices[1] < 0 || indices[1] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[1]), LOG_ERRORS); exit(0); }
			if (indices[2] < 0 || indices[2] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[2]), LOG_ERRORS); exit(0); }
			dest->mFaces[j].mIndices = new uint32_t[3];
			if (bFlip) {
				dest->mFaces[j].mIndices[0] = indices[0];
				dest->mFaces[j].mIndices[1] = indices[1];
				dest->mFaces[j].mIndices[2] = indices[2];
			}
			else {
				dest->mFaces[j].mIndices[0] = indices[2];
				dest->mFaces[j].mIndices[1] = indices[1];
				dest->mFaces[j].mIndices[2] = indices[0];
			}
			dest->mFaces[j].mNumIndices = 3;
			indexData += 2;
			bFlip = !bFlip;
		}
	}
	else {
		for (int j = 0; j < src.nPolyCount; j++) {
			auto tmp = (uint16_t*)indexData;
			int indices[3] = {tmp[0], tmp[1], tmp[2]};
			indices[0] -= baseVertexOffset;
			indices[1] -= baseVertexOffset;
			indices[2] -= baseVertexOffset;
			if (indices[0] < 0 || indices[0] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[0]), LOG_ERRORS); exit(0); }
			if (indices[1] < 0 || indices[1] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[1]), LOG_ERRORS); exit(0); }
			if (indices[2] < 0 || indices[2] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[2]), LOG_ERRORS); exit(0); }
			dest->mFaces[j].mIndices = new uint32_t[3];
			dest->mFaces[j].mIndices[0] = indices[2];
			dest->mFaces[j].mIndices[1] = indices[1];
			dest->mFaces[j].mIndices[2] = indices[0];
			dest->mFaces[j].mNumIndices = 3;
			indexData += 2 * 3;
		}
	}
}

aiScene GenerateScene() {
	aiScene scene;
	scene.mRootNode = new aiNode();

	// materials
	scene.mMaterials = new aiMaterial*[aMaterials.size()];
	for (int i = 0; i < aMaterials.size(); i++) {
		auto& src = aMaterials[i];
		scene.mMaterials[i] = new aiMaterial();
		auto dest = scene.mMaterials[i];
		aiString matName(src.sName);
		dest->AddProperty(&matName, AI_MATKEY_NAME);
		for (int j = 0; j < 3; j++) {
			if (src.sTextureNames[j].empty()) continue;
			auto texName = src.sTextureNames[j];
			if (texName.ends_with(".tga") || texName.ends_with(".TGA")) {
				texName.pop_back();
				texName.pop_back();
				texName.pop_back();
				texName += "dds";
			}
			aiString fileName(texName);
			dest->AddProperty(&fileName, AI_MATKEY_TEXTURE_DIFFUSE(j));
		}
		if (src.sTextureNames[0].empty()) {
			std::string str = "null";
			aiString fileName(str);
			dest->AddProperty(&fileName, AI_MATKEY_TEXTURE_DIFFUSE(0));
		}
		if (src.sName.empty()) {
			std::string str = "null";
			aiString fileName(str);
			dest->AddProperty( &matName, AI_MATKEY_NAME );
		}
	}
	scene.mNumMaterials = aMaterials.size();

	int numSurfaces = 0;
	int numBaseSurfaces = 0;
	for (auto& surface : aSurfaces) {
		if (!CanSurfaceBeExported(&surface)) continue;
		numSurfaces++;
		numBaseSurfaces++;
		if (surface._pCrashDataSurface) numSurfaces++;
	}
	WriteConsole(std::to_string(numBaseSurfaces) + " surfaces of " + std::to_string(aSurfaces.size()) + " can be exported", LOG_ALWAYS);

	scene.mMeshes = new aiMesh*[numSurfaces];
	scene.mNumMeshes = numSurfaces;

	int counter = 0;
	for (auto& src : aSurfaces) {
		if (!CanSurfaceBeExported(&src)) continue;

		int i = counter++;
		src._nFBXModelId = i;

		auto dest = scene.mMeshes[i] = new aiMesh();
		dest->mName = "Surface" + std::to_string(&src - &aSurfaces[0]);
		dest->mMaterialIndex = src.nMaterialId;
		auto vBuf = FindVertexBuffer(src.nStreamId[0]);
		auto iBuf = FindIndexBuffer(src.nStreamId[1]);

		FillFBXMeshFromSurface(dest, vBuf, iBuf, src, src.nStreamOffset[0]);
		if (src._pCrashDataSurface) {
			i = counter++;
			src._nFBXCrashModelId = i;

			auto destCrash = scene.mMeshes[i] = new aiMesh();
			destCrash->mName = "Surface" + std::to_string(&src - &aSurfaces[0]) + "_crash";
			destCrash->mMaterialIndex = src.nMaterialId;
			auto weightsFO2 = bIsFOUCModel ? nullptr : &src._pCrashDataSurface->aCrashWeights[0];
			auto weightsFOUC = bIsFOUCModel ? &src._pCrashDataSurface->aCrashWeightsFOUC[0] : nullptr;
			auto vBuffer = bIsFOUCModel ? vBuf : &src._pCrashDataSurface->vBuffer;
			FillFBXMeshFromSurface(destCrash, vBuffer, iBuf, src, 0, weightsFO2, weightsFOUC);
		}
	}

	if (!bIsBGMModel) {
		if (auto node = new aiNode()) {
			std::vector<int> surfaceIds;
			for (auto &batch: aStaticBatches) {
				if (IsSurfaceValidAndExportable(batch.nSurfaceId, true)) surfaceIds.push_back(batch.nSurfaceId);
			}

			node->mName = "StaticBatch";
			scene.mRootNode->addChildren(1, &node);
			node->mMeshes = new uint32_t[surfaceIds.size()];
			node->mNumMeshes = surfaceIds.size();
			int i = 0;
			for (auto &surfaceId: surfaceIds) {
				node->mMeshes[i++] = aSurfaces[surfaceId]._nFBXModelId;
			}
		}

		if (auto node = new aiNode()) {
			node->mName = "TreeMesh";
			scene.mRootNode->addChildren(1, &node);
			for (auto &treeMesh: aTreeMeshes) {
				if (auto treeNode = CreateNodeForTreeMesh(&scene, treeMesh)) {
					node->addChildren(1, &treeNode);
				}
			}
		}
	}

	if (auto node = new aiNode()) {
		node->mName = "Objects";
		scene.mRootNode->addChildren(1, &node);
		for (auto& object : aObjects) {
			auto objectNode = new aiNode();
			objectNode->mName = object.sName1;
			FO2MatrixToFBXMatrix(object.mMatrix, &objectNode->mTransformation);
			node->addChildren(1, &objectNode);
		}
	}

	if (bIsBGMModel) {
		if (auto node = new aiNode()) {
			node->mName = "BGMMesh";
			scene.mRootNode->addChildren(1, &node);
			for (auto &compactMesh: aBGMMeshes) {
				auto meshNode = new aiNode();
				meshNode->mName = compactMesh.sName1;
				FO2MatrixToFBXMatrix(compactMesh.mMatrix, &meshNode->mTransformation);
				node->addChildren(1, &meshNode);

				for (auto& modelId : compactMesh.aModels) {
					auto model = aModels[modelId];

					auto modelNode = new aiNode();
					modelNode->mName = model.sName;
					meshNode->addChildren(1, &modelNode);

					std::vector<int> aMeshes;
					for (auto &surfaceId: model.aSurfaces) {
						if (!IsSurfaceValidAndExportable(surfaceId)) continue;
						auto& surface = aSurfaces[surfaceId];
						aMeshes.push_back(surface._nFBXModelId);
						if (surface._nFBXCrashModelId >= 0) aMeshes.push_back(surface._nFBXCrashModelId);
					}
					modelNode->mMeshes = new uint32_t[aMeshes.size()];
					memcpy(modelNode->mMeshes, &aMeshes[0], aMeshes.size() * sizeof(uint32_t));
					modelNode->mNumMeshes = aMeshes.size();
				}
			}
		}
	}
	else {
		if (auto node = new aiNode()) {
			node->mName = "CompactMesh";
			scene.mRootNode->addChildren(1, &node);
			for (auto &compactMesh: aCompactMeshes) {
				if (compactMesh.nFlags == 0x8000 && bFBXSkipHiddenProps) continue;

				auto meshNode = new aiNode();
				meshNode->mName = compactMesh.sName1;
				FO2MatrixToFBXMatrix(compactMesh.mMatrix, &meshNode->mTransformation);
				node->addChildren(1, &meshNode);

				// not loading the damaged or lod parts here
				if (!compactMesh.aLODMeshIds.empty()) {
					auto model = aModels[compactMesh.aLODMeshIds[0]];
					std::vector<int> aMeshes;
					for (auto &surfaceId: model.aSurfaces) {
						if (!IsSurfaceValidAndExportable(surfaceId)) continue;
						aMeshes.push_back(aSurfaces[surfaceId]._nFBXModelId);
					}
					meshNode->mMeshes = new uint32_t[aMeshes.size()];
					memcpy(meshNode->mMeshes, &aMeshes[0], aMeshes.size() * sizeof(uint32_t));
					meshNode->mNumMeshes = aMeshes.size();
				}
			}
		}
	}

	return scene;
}

void WriteToFBX() {
	WriteConsole("Writing model file...", LOG_ALWAYS);

	auto scene = GenerateScene();

	Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
	Assimp::DefaultLogger::create("fbx_export_log.txt", severity, aiDefaultLogStream_FILE);

	Assimp::Exporter exporter;
	if (exporter.Export(&scene, "fbx", sFileNameNoExt.string() + "_out.fbx") != aiReturn_SUCCESS) {
		WriteConsole("ERROR: Model export failed!", LOG_ERRORS);
	}
	else {
		WriteConsole("Model export finished", LOG_ALWAYS);
	}
}