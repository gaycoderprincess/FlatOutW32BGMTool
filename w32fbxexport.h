aiNode* CreateNodeForTreeMesh(aiScene* scene, const tTreeMesh& treeMesh) {
	std::vector<int> surfaceIds;
	if (IsSurfaceValidAndExportable(treeMesh.nTrunkSurfaceId)) surfaceIds.push_back(treeMesh.nTrunkSurfaceId);
	if (IsSurfaceValidAndExportable(treeMesh.nBranchSurfaceId)) surfaceIds.push_back(treeMesh.nBranchSurfaceId);
	if (IsSurfaceValidAndExportable(treeMesh.nLeafSurfaceId)) surfaceIds.push_back(treeMesh.nLeafSurfaceId);
	if (surfaceIds.empty()) return nullptr;

	auto node = new aiNode();
	node->mName = "TreeMesh" + std::to_string(&treeMesh - &aTreeMeshes[0]);
	node->mMeshes = new uint32_t[surfaceIds.size()];
	node->mNumMeshes = surfaceIds.size();
	//FO2MatrixToFBXMatrix(treeMesh.mMatrix, &node->mTransformation);
	int i = 0;
	for (auto& surfaceId : surfaceIds) {
		node->mMeshes[i++] = aSurfaces[surfaceId]._nFBXModelId;
	}
	return node;
}

aiNode* CreateNodeForBVHNode(aiScene* scene, const tTrackBVHNode& bvhNode) {
	auto node = new aiNode();
	node->mName = "BVHNode" + std::to_string(&bvhNode - &aBVHNodes[0]) + "_" + std::to_string(bvhNode.nUnk1) + "_" + std::to_string(bvhNode.nUnk2);
	node->mMeshes = new uint32_t[1];
	node->mMeshes[0] = scene->mNumMeshes - 1;
	node->mNumMeshes = 1;
	node->mTransformation.a1 = bvhNode.vRadius[0];
	node->mTransformation.a2 = 0;
	node->mTransformation.a3 = 0;
	node->mTransformation.a4 = 0;
	node->mTransformation.b1 = 0;
	node->mTransformation.b2 = bvhNode.vRadius[1];
	node->mTransformation.b3 = 0;
	node->mTransformation.b4 = 0;
	node->mTransformation.c1 = 0;
	node->mTransformation.c2 = 0;
	node->mTransformation.c3 = bvhNode.vRadius[2];
	node->mTransformation.c4 = 0;
	node->mTransformation.a4 = bvhNode.vPos[0];
	node->mTransformation.b4 = bvhNode.vPos[1];
	node->mTransformation.c4 = -bvhNode.vPos[2];
	node->mTransformation.d4 = 1;
	return node;
}

bool bSurfaceImportNoSubtract = false;
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
				if (vertexColorOffset >= 0xFF000000 || bIsBGMModel || bIsRallyTrophyModel) {
					auto rgb = (uint8_t*)&vertexColorOffset;
					dest->mColors[0][j].r = rgb[0] / 255.0;
					dest->mColors[0][j].g = rgb[1] / 255.0;
					dest->mColors[0][j].b = rgb[2] / 255.0;
					dest->mColors[0][j].a = bIsRallyTrophyModel ? 1.0 : (rgb[3] / 255.0);
				}
				else if (!aVertexColors.empty()) {
					int id = vertexColorOffset & 0xFFFFFF;
					if (id >= aVertexColors.size()) {
						WriteConsole("ERROR: Vertex colors for surface " + std::to_string(&src - &aSurfaces[0]) + " out of bounds!", LOG_ERRORS);
						WriteConsole(std::to_string(id) + "/" + std::to_string(aVertexColors.size()), LOG_ERRORS);
						WaitAndExitOnFail();
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

	// quads, not sure how these are read
	if (src.nPolyMode == 0) {
		for (int j = 0; j < src.nPolyCount; j++) {
			int indices[4] = {0, 0, 0, 0};
			indices[0] = j * 4;
			indices[1] = (j * 4) + 1;
			indices[2] = (j * 4) + 2;
			indices[3] = (j * 4) + 3;
			if (indices[0] < 0 || indices[0] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[0]), LOG_ERRORS); WaitAndExitOnFail(); }
			if (indices[1] < 0 || indices[1] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[1]), LOG_ERRORS); WaitAndExitOnFail(); }
			if (indices[2] < 0 || indices[2] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[2]), LOG_ERRORS); WaitAndExitOnFail(); }
			if (indices[3] < 0 || indices[3] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[3]), LOG_ERRORS); WaitAndExitOnFail(); }
			dest->mFaces[j].mIndices = new uint32_t[4];
			dest->mFaces[j].mIndices[0] = indices[3];
			dest->mFaces[j].mIndices[1] = indices[2];
			dest->mFaces[j].mIndices[2] = indices[1];
			dest->mFaces[j].mIndices[3] = indices[0];
			dest->mFaces[j].mNumIndices = 4;
		}
	}
	// tristrip
	else if (src.nPolyMode == 5) {
		bool bFlip = false;
		for (int j = 0; j < src.nPolyCount; j++) {
			auto tmp = (uint16_t*)indexData;
			int indices[3] = {tmp[0], tmp[1], tmp[2]};
			if (!bSurfaceImportNoSubtract) {
				indices[0] -= baseVertexOffset;
				indices[1] -= baseVertexOffset;
				indices[2] -= baseVertexOffset;
			}
			if (indices[0] < 0 || indices[0] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[0]), LOG_ERRORS); WaitAndExitOnFail(); }
			if (indices[1] < 0 || indices[1] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[1]), LOG_ERRORS); WaitAndExitOnFail(); }
			if (indices[2] < 0 || indices[2] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[2]), LOG_ERRORS); WaitAndExitOnFail(); }
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
	// tris
	else {
		for (int j = 0; j < src.nPolyCount; j++) {
			auto tmp = (uint16_t*)indexData;
			int indices[3] = {tmp[0], tmp[1], tmp[2]};
			if (!bSurfaceImportNoSubtract) {
				indices[0] -= baseVertexOffset;
				indices[1] -= baseVertexOffset;
				indices[2] -= baseVertexOffset;
			}
			if (indices[0] < 0 || indices[0] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[0]), LOG_ERRORS); WaitAndExitOnFail(); }
			if (indices[1] < 0 || indices[1] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[1]), LOG_ERRORS); WaitAndExitOnFail(); }
			if (indices[2] < 0 || indices[2] >= src.nVertexCount) { WriteConsole("ERROR: Index out of bounds: " + std::to_string(indices[2]), LOG_ERRORS); WaitAndExitOnFail(); }
			dest->mFaces[j].mIndices = new uint32_t[3];
			dest->mFaces[j].mIndices[0] = indices[2];
			dest->mFaces[j].mIndices[1] = indices[1];
			dest->mFaces[j].mIndices[2] = indices[0];
			dest->mFaces[j].mNumIndices = 3;
			indexData += 2 * 3;
		}
	}
}

void CreateFBXCube(aiScene* scene) {
	std::vector<aiVector3D> aVertices;
	aVertices.push_back({ 1, 1, 1 });
	aVertices.push_back({ -1, 1, 1 });
	aVertices.push_back({ -1, -1, 1 });
	aVertices.push_back({ 1, -1, 1 });
	aVertices.push_back({ 1, -1, -1 });
	aVertices.push_back({ 1, -1, 1 });
	aVertices.push_back({ -1, -1, 1 });
	aVertices.push_back({ -1, -1, -1 });
	aVertices.push_back({ -1, -1, -1 });
	aVertices.push_back({ -1, -1, 1 });
	aVertices.push_back({ -1, 1, 1 });
	aVertices.push_back({ -1, 1, -1 });
	aVertices.push_back({ -1, 1, -1 });
	aVertices.push_back({ 1, 1, -1 });
	aVertices.push_back({ 1, -1, -1 });
	aVertices.push_back({ -1, -1, -1 });
	aVertices.push_back({ 1, 1, -1 });
	aVertices.push_back({ 1, 1, 1 });
	aVertices.push_back({ 1, -1, 1 });
	aVertices.push_back({ 1, -1, -1 });
	aVertices.push_back({ -1, 1, -1 });
	aVertices.push_back({ -1, 1, 1 });
	aVertices.push_back({ 1, 1, 1 });
	aVertices.push_back({ 1, 1, -1 });
	std::vector<aiVector3D> aIndices;
	aIndices.push_back({ 0, 1, 2 });
	aIndices.push_back({ 0, 2, 3 });
	aIndices.push_back({ 4, 5, 6 });
	aIndices.push_back({ 4, 6, 7 });
	aIndices.push_back({ 8, 9, 10 });
	aIndices.push_back({ 8, 10, 11 });
	aIndices.push_back({ 12, 13, 14 });
	aIndices.push_back({ 12, 14, 15 });
	aIndices.push_back({ 16, 17, 18 });
	aIndices.push_back({ 16, 18, 19 });
	aIndices.push_back({ 20, 21, 22 });
	aIndices.push_back({ 20, 22, 23 });

	if (auto mesh = new aiMesh) {
		mesh->mVertices = new aiVector3D[aVertices.size()];
		mesh->mNumVertices = aVertices.size();
		mesh->mFaces = new aiFace[aIndices.size()];
		mesh->mNumFaces = aIndices.size();

		for (int i = 0; i < mesh->mNumVertices; i++) {
			mesh->mVertices[i] = aVertices[i];
		}
		for (int i = 0; i < mesh->mNumFaces; i++) {
			mesh->mFaces[i].mIndices = new uint32_t[3];
			mesh->mFaces[i].mIndices[0] = aIndices[i].x;
			mesh->mFaces[i].mIndices[1] = aIndices[i].y;
			mesh->mFaces[i].mIndices[2] = aIndices[i].z;
			mesh->mFaces[i].mNumIndices = 3;
		}

		scene->mMeshes[scene->mNumMeshes - 1] = mesh;
	}
}

aiNode* CreateFBXNodeAtPosition(aiVector3D pos) {
	auto node = new aiNode();
	node->mTransformation.a4 = pos.x;
	node->mTransformation.b4 = pos.y;
	node->mTransformation.c4 = -pos.z;
	node->mTransformation.d4 = 1;
	return node;
}

void CreateSplineArrayForVector(aiNode* rootNode, std::vector<aiVector3D>& vec, const std::string& name) {
	if (vec.empty()) return;

	if (auto node = new aiNode()) {
		node->mName = name;
		rootNode->addChildren(1, &node);
		for (auto& point: vec) {
			auto pNode = new aiNode();
			pNode->mTransformation.a4 = point.x;
			pNode->mTransformation.b4 = point.y;
			pNode->mTransformation.c4 = -point.z;
			pNode->mTransformation.d4 = 1;
			pNode->mName = name + "_Node" + std::to_string((&point - &vec[0]) + 1);
			node->addChildren(1, &pNode);
		}
	}
}

void PerformMaterialAutodetectTest(tMaterial& material) {
	if (material.sTextureNames[1].empty()) return;

	auto testMaterial = material;
	testMaterial.sTextureNames[1] = "";
	FixupFBXMapMaterial(testMaterial, true, false);
	if (!MaterialStringCompare(testMaterial.sTextureNames[1], material.sTextureNames[1])) {
		WriteConsole("WARNING: Texture autodetect failed! Material " + material.sName + " with texture " + material.sTextureNames[1] + " after re-importing will use " + testMaterial.sTextureNames[1], LOG_WARNINGS);
	}
	if (testMaterial.nShaderId != material.nShaderId) {
		auto shader1 = GetShaderName(material.nShaderId, nImportFileVersion);
		auto shader2 = GetShaderName(testMaterial.nShaderId, nImportFileVersion <= 0x10003 ? 0x10004 : nImportFileVersion);
		WriteConsole("WARNING: Shader autodetect failed! Material " + material.sName + " using shader " + shader1 + " after re-importing will use " + shader2, LOG_WARNINGS);
	}
}

bool CanFOUCTreeBeExported(tFOUCTreeSurface& tree) {
	if (tree.nMaterialId < 0) return false;
	return true;
}

void FillSurfaceFromFOUCTree(tSurface& out, tFOUCTreeSurface& tree) {
	out.nIsVegetation = 0;
	out.nMaterialId = tree.nMaterialId;
	out.nVertexCount = tree.nVertexCount;
	//out.nPolyCount = tree.nPolyCount;
	out.nPolyCount = tree.nVertexCount / 3;
	out.nNumIndicesUsed = out.nPolyCount * 3;
	out.nPolyMode = 4;
	out.foucVertexMultiplier[0] = 0;
	out.foucVertexMultiplier[1] = 0;
	out.foucVertexMultiplier[2] = 0;
	out.foucVertexMultiplier[3] = fFOUCBGMScaleMultiplier;
	out.nNumStreamsUsed = 2;
	out.nStreamId[0] = tree.nVertexBuffer;
	out.nStreamId[1] = tree.nIndexBuffer;
	out.nStreamOffset[0] = tree.nVertexOffset;
	out.nStreamOffset[1] = tree.nIndexOffset;

	/*
		foucBranch.nMaterialId: 188
		foucBranch.nVertexCount: 240
		foucBranch.nPolyCount: 128
		foucBranch.nVertexBuffer: 7
		foucBranch.nVertexOffset: 93264
		foucBranch.nIndexBuffer: 15
		foucBranch.nIndexOffset: 25602
		foucBranch.nUnknown[0]: 8
		foucBranch.nUnknown[1]: 1432656
	*/
}

void AddFBXNodeFromFOUCTree(aiScene* scene, aiNode* parentNode, tFOUCTreeSurface& tree, float* matrix, int nameId, int meshId) {
	tSurface tmp;
	FillSurfaceFromFOUCTree(tmp, tree);

	auto dest = scene->mMeshes[meshId] = new aiMesh();
	dest->mName = "TreeSurface" + std::to_string(nameId);
	dest->mMaterialIndex = tmp.nMaterialId;
	auto vBuf = FindVertexBuffer(tmp.nStreamId[0]);
	auto iBuf = FindIndexBuffer(tmp.nStreamId[1]);
	tmp.nFlags = vBuf->flags;

	bSurfaceImportNoSubtract = true;
	FillFBXMeshFromSurface(dest, vBuf, iBuf, tmp, tmp.nStreamOffset[0]);
	bSurfaceImportNoSubtract = false;

	if (auto node = new aiNode()) {
		node->mName = dest->mName;
		node->mMeshes = new unsigned int[1];
		node->mMeshes[0] = meshId;
		node->mNumMeshes = 1;
		FO2MatrixToFBXMatrix(matrix, &node->mTransformation);
		parentNode->addChildren(1, &node);
	}
}

aiScene GenerateScene() {
	aiScene scene;
	scene.mRootNode = new aiNode();

	// materials
	scene.mMaterials = new aiMaterial*[aMaterials.size()];
	for (int i = 0; i < aMaterials.size(); i++) {
		auto src = aMaterials[i];
		src.SetFBXCompatibleName();
		PerformMaterialAutodetectTest(src);
		scene.mMaterials[i] = new aiMaterial();
		auto dest = scene.mMaterials[i];
		aiString matName(src.sName);
		dest->AddProperty(&matName, AI_MATKEY_NAME);
		for (int j = 0; j < 3; j++) {
			if (src.sTextureNames[j].empty()) continue;
			auto texName = src.sTextureNames[j];
			if (!bIsRallyTrophyModel && (texName.ends_with(".tga") || texName.ends_with(".TGA"))) {
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

	int numPotentialFOUCTrees = 0;
	int numSurfaces = 0;
	int numBaseSurfaces = 0;
	for (auto& surface : aSurfaces) {
		if (!CanSurfaceBeExported(&surface)) continue;
		numSurfaces++;
		numBaseSurfaces++;
		if (surface._pCrashDataSurface) numSurfaces++;
	}
	if (bIsFOUCModel) {
		for (auto& treeMesh : aTreeMeshes) {
			if (treeMesh.foucTrunk.nMaterialId >= 0) numPotentialFOUCTrees++;
			if (treeMesh.foucBranch.nMaterialId >= 0) numPotentialFOUCTrees++;
			if (treeMesh.foucLeaf.nMaterialId >= 0) numPotentialFOUCTrees++;

			if (CanFOUCTreeBeExported(treeMesh.foucTrunk)) {
				numSurfaces++;
				numBaseSurfaces++;
			}
			if (CanFOUCTreeBeExported(treeMesh.foucBranch)) {
				numSurfaces++;
				numBaseSurfaces++;
			}
		}
	}
	WriteConsole(std::to_string(numBaseSurfaces) + " surfaces of " + std::to_string(aSurfaces.size() + numPotentialFOUCTrees) + " can be exported", LOG_ALWAYS);

	if (bFBXExportBVHNodes) {
		scene.mMeshes = new aiMesh*[numSurfaces + 1];
		scene.mNumMeshes = numSurfaces + 1;
		CreateFBXCube(&scene);
	}
	else {
		scene.mMeshes = new aiMesh*[numSurfaces];
		scene.mNumMeshes = numSurfaces;
	}

	int counter = 0;
	for (auto& src : aSurfaces) {
		if (!CanSurfaceBeExported(&src)) continue;

		int i = counter++;
		src._nFBXModelId = i;

		auto name = "Surface" + std::to_string(&src - &aSurfaces[0]);
		if (src._nLODLevel > 0) name += std::format("_LOD{}", src._nLODLevel);

		auto dest = scene.mMeshes[i] = new aiMesh();
		dest->mName = name;
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
			if (bIsFOUCModel) {
				FillFBXMeshFromSurface(destCrash, vBuf, iBuf, src, src.nStreamOffset[0], nullptr, &src._pCrashDataSurface->aCrashWeightsFOUC[0]);
			}
			else {
				FillFBXMeshFromSurface(destCrash, &src._pCrashDataSurface->vBuffer, iBuf, src, 0, &src._pCrashDataSurface->aCrashWeights[0], nullptr);
			}
		}
	}
	if (bIsFOUCModel && !bIsBGMModel) {
		if (auto node = new aiNode()) {
			node->mName = "TreeMesh";
			scene.mRootNode->addChildren(1, &node);
			for (auto& treeMesh : aTreeMeshes) {
				int i = counter;
				if (CanFOUCTreeBeExported(treeMesh.foucTrunk)) {
					i = counter++;
					AddFBXNodeFromFOUCTree(&scene, node, treeMesh.foucTrunk, treeMesh.mMatrix, i, i);
				}
				if (CanFOUCTreeBeExported(treeMesh.foucBranch)) {
					i = counter++;
					AddFBXNodeFromFOUCTree(&scene, node, treeMesh.foucBranch, treeMesh.mMatrix, i, i);
				}
			}
		}
	}

	if (!bIsBGMModel) {
		if (!IsRallyTrophyCar()) {
			if (auto node = new aiNode()) {
				std::vector<int> surfaceIds;
				for (auto &batch: aStaticBatches) {
					if (IsSurfaceValidAndExportable(batch.nBVHId1)) surfaceIds.push_back(batch.nBVHId1);
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
		}

		if (!bIsFOUCModel && !IsRallyTrophyCar()) {
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

		if (bFBXExportBVHNodes) {
			if (auto node = new aiNode()) {
				node->mName = "BVHNode";
				scene.mRootNode->addChildren(1, &node);
				for (auto &bvh: aBVHNodes) {
					if (auto bvhNode = CreateNodeForBVHNode(&scene, bvh)) {
						node->addChildren(1, &bvhNode);
					}
				}
			}
		}

		if (!aSplitpoints.empty()) {
			if (auto node = new aiNode()) {
				node->mName = "Splitpoints";
				scene.mRootNode->addChildren(1, &node);
				for (auto& point: aSplitpoints) {
					auto baseName = "Splitpoint" + std::to_string((&point - &aSplitpoints[0]) + 1);

					auto pNode = CreateFBXNodeAtPosition(point.pos);
					auto lNode = CreateFBXNodeAtPosition(point.left);
					auto rNode = CreateFBXNodeAtPosition(point.right);
					pNode->mName = baseName + "_Position";
					lNode->mName = baseName + "_Left";
					rNode->mName = baseName + "_Right";
					node->addChildren(1, &pNode);
					node->addChildren(1, &lNode);
					node->addChildren(1, &rNode);
				}
			}
		}

		if (!aStartpoints.empty()) {
			if (auto node = new aiNode()) {
				node->mName = "Startpoints";
				scene.mRootNode->addChildren(1, &node);
				for (auto& point: aStartpoints) {
					auto pNode = new aiNode();
					FO2MatrixToFBXMatrix(point.mMatrix, &pNode->mTransformation);
					pNode->mName = "Startpoint" + std::to_string((&point - &aStartpoints[0]) + 1);
					node->addChildren(1, &pNode);
				}
			}
		}

		if (!aAISplines.empty()) {
			if (auto node = new aiNode()) {
				node->mName = "Splines";
				scene.mRootNode->addChildren(1, &node);
				for (auto& spline : aAISplines) {
					CreateSplineArrayForVector(node, spline.values, spline.name);
				}
			}
		}
	}

	if (auto node = new aiNode()) {
		node->mName = "Objects";
		scene.mRootNode->addChildren(1, &node);
		for (auto& object : aObjects) {
			uint32_t tmpFlags = object.nFlags - (object.nFlags % 0x1000);
			if (bFBXSkipHiddenProps && tmpFlags != 0xE000 && tmpFlags != nFBXSkipHiddenPropsFlag) continue;

			auto objectNode = new aiNode();
			objectNode->mName = object.sName1;
			FO2MatrixToFBXMatrix(object.mMatrix, &objectNode->mTransformation);
			node->addChildren(1, &objectNode);
		}
	}

	if (auto node = new aiNode()) {
		node->mName = bIsBGMModel || IsRallyTrophyCar() ? "BGMMesh" : "CompactMesh";
		scene.mRootNode->addChildren(1, &node);
		for (auto& compactMesh: aCompactMeshes) {
			if (bFBXSkipHiddenProps && compactMesh.nFlags != 0xE000 && compactMesh.nFlags != nFBXSkipHiddenPropsFlag) continue;

			auto meshNode = new aiNode();
			meshNode->mName = compactMesh.sName1;
			FO2MatrixToFBXMatrix(compactMesh.mMatrix, &meshNode->mTransformation);
			node->addChildren(1, &meshNode);

			if (!bIsBGMModel) {
				auto typeNode = new aiNode();
				typeNode->mName = std::format("{}TYPE_{}", &compactMesh - &aCompactMeshes[0], compactMesh.sName2);
				meshNode->addChildren(1, &typeNode);

				if (compactMesh.nGroup != -1) {
					auto groupNode = new aiNode();
					groupNode->mName = std::format("{}GROUP_{}", &compactMesh - &aCompactMeshes[0],
												   compactMesh.nGroup);
					meshNode->addChildren(1, &groupNode);
				}
			}

			if (!compactMesh.aModels.empty()) {
				int numToExport = 1;
				if (bExportAllLODs) numToExport = compactMesh.aModels.size();
				std::vector<int> aMeshes;
				for (int i = 0; i < numToExport; i++) {
					auto model = aModels[compactMesh.aModels[i]];
					for (auto &surfaceId: model.aSurfaces) {
						if (!IsSurfaceValidAndExportable(surfaceId)) continue;
						auto& surface = aSurfaces[surfaceId];
						aMeshes.push_back(surface._nFBXModelId);
					}
				}
				meshNode->mMeshes = new uint32_t[aMeshes.size()];
				memcpy(meshNode->mMeshes, &aMeshes[0], aMeshes.size() * sizeof(uint32_t));
				meshNode->mNumMeshes = aMeshes.size();
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