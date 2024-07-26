aiNode* CreateNodeForTreeMesh(aiScene* scene, const tTreeMesh& treeMesh) {
	int numTreeMeshes = 0;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId2)) numTreeMeshes++;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId3)) numTreeMeshes++;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId4)) numTreeMeshes++;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId5)) numTreeMeshes++;
	if (numTreeMeshes <= 0) return nullptr;

	auto node = new aiNode();
	node->mName = "TreeMesh" + std::to_string(&treeMesh - &aTreeMeshes[0]);
	node->mMeshes = new uint32_t[numTreeMeshes];
	node->mNumMeshes = numTreeMeshes;

	int i = 0;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId2)) node->mMeshes[i++] = aSurfaces[treeMesh.nSurfaceId2]._nFBXModelId;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId3)) node->mMeshes[i++] = aSurfaces[treeMesh.nSurfaceId3]._nFBXModelId;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId4)) node->mMeshes[i++] = aSurfaces[treeMesh.nSurfaceId4]._nFBXModelId;
	if (IsSurfaceValidAndExportable(treeMesh.nSurfaceId5)) node->mMeshes[i++] = aSurfaces[treeMesh.nSurfaceId5]._nFBXModelId;
	return node;
}

aiScene GenerateScene() {
	aiScene scene;
	scene.mRootNode = new aiNode();

	// materials
	scene.mMaterials = new aiMaterial*[aMaterials.size()];
	for (int i = 0; i < aMaterials.size(); i++) {
		scene.mMaterials[i] = new aiMaterial();

		auto& src = aMaterials[i];
		scene.mMaterials[i] = new aiMaterial();
		auto dest = scene.mMaterials[i];
		aiString matName(src.sName);
		dest->AddProperty( &matName, AI_MATKEY_NAME );
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
	for (auto& surface : aSurfaces) {
		if (CanSurfaceBeExported(&surface)) numSurfaces++;
	}
	WriteConsole(std::to_string(numSurfaces) + " surfaces of " + std::to_string(aSurfaces.size()) + " can be exported");

	scene.mMeshes = new aiMesh*[numSurfaces];
	scene.mNumMeshes = numSurfaces;

	int counter = 0;
	for (auto& src : aSurfaces) {
		if (!CanSurfaceBeExported(&src)) continue;

		int i = counter++;
		src._nFBXModelId = i;

		scene.mMeshes[i] = new aiMesh();

		auto dest = scene.mMeshes[i];
		dest->mName = "Surface" + std::to_string(&src - &aSurfaces[0]);
		dest->mMaterialIndex = src.nMaterialId;

		auto vBuf = FindVertexBuffer(src.nStreamId[0]);
		auto iBuf = FindIndexBuffer(src.nStreamId[1]);

		auto stride = vBuf->vertexSize;
		uintptr_t vertexData = ((uintptr_t)vBuf->data) + src.nStreamOffset[0];
		uintptr_t indexData = ((uintptr_t)iBuf->data) + src.nStreamOffset[1];

		uint32_t baseVertexOffset = src.nStreamOffset[0] / vBuf->vertexSize;

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
		if ((vBuf->flags & VERTEX_NORMAL) != 0) {
			dest->mNormals = new aiVector3D[src.nVertexCount];
		}

		if (nImportMapVersion >= 0x20002) {
			for (int j = 0; j < src.nVertexCount; j++) {
				auto vertices = (int16_t*)vertexData;

				// uvs always seem to be the last 2 or 4 values in the vertex buffer
				auto uvOffset = stride - 4;
				if ((vBuf->flags & VERTEX_UV2) != 0) uvOffset -= 4;
				auto uvs = (int16_t*)(vertexData + uvOffset);

				dest->mVertices[j].x = vertices[0];
				dest->mVertices[j].y = vertices[1];
				dest->mVertices[j].z = vertices[2];
				dest->mVertices[j].x += src.foucVertexMultiplier[0];
				dest->mVertices[j].y += src.foucVertexMultiplier[1];
				dest->mVertices[j].z += src.foucVertexMultiplier[2];
				dest->mVertices[j].x *= src.foucVertexMultiplier[3];
				dest->mVertices[j].y *= src.foucVertexMultiplier[3];
				dest->mVertices[j].z *= -src.foucVertexMultiplier[3];
				vertices += 3;

				if ((vBuf->flags & VERTEX_NORMAL) != 0) {
					dest->mNormals[j].x = vertices[0] / 32767.0;
					dest->mNormals[j].y = vertices[1] / 32767.0;
					dest->mNormals[j].z = -vertices[2] / 32767.0;
					vertices += 3; // 3 floats
				}
				if ((vBuf->flags & VERTEX_COLOR) != 0) vertices += 1; // 1 int32
				if ((vBuf->flags & VERTEX_UV) != 0 || (vBuf->flags & VERTEX_UV2) != 0) {
					dest->mTextureCoords[0][j].x = uvs[0] / 2048.0;
					dest->mTextureCoords[0][j].y = 1 - (uvs[1] / 2048.0);
					dest->mTextureCoords[0][j].z = 0;
				}
				if ((vBuf->flags & VERTEX_UV2) != 0) {
					dest->mTextureCoords[1][j].x = uvs[2] / 2048.0;
					dest->mTextureCoords[1][j].y = 1 - (uvs[3] / 2048.0);
					dest->mTextureCoords[1][j].z = 0;
				}
				vertexData += stride;
			}
		}
		else {
			for (int j = 0; j < src.nVertexCount; j++) {
				auto vertices = (float*)vertexData;
				dest->mVertices[j].x = vertices[0];
				dest->mVertices[j].y = vertices[1];
				dest->mVertices[j].z = -vertices[2];
				vertices += 3;

				if ((vBuf->flags & VERTEX_NORMAL) != 0) {
					dest->mNormals[j].x = vertices[0];
					dest->mNormals[j].y = vertices[1];
					dest->mNormals[j].z = -vertices[2];
					vertices += 3; // 3 floats
				}
				if ((vBuf->flags & VERTEX_COLOR) != 0) vertices += 1; // 1 int32
				if ((vBuf->flags & VERTEX_UV) != 0 || (vBuf->flags & VERTEX_UV2) != 0) {
					dest->mTextureCoords[0][j].x = vertices[0];
					dest->mTextureCoords[0][j].y = 1 - vertices[1];
					dest->mTextureCoords[0][j].z = 0;
					vertices += 2;
				}
				if ((vBuf->flags & VERTEX_UV2) != 0) {
					dest->mTextureCoords[1][j].x = vertices[0];
					dest->mTextureCoords[1][j].y = 1 - vertices[1];
					dest->mTextureCoords[1][j].z = 0;
					vertices += 2;
				}
				vertexData += stride;
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
				if (indices[0] < 0 || indices[0] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[0])); exit(0); }
				if (indices[1] < 0 || indices[1] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[1])); exit(0); }
				if (indices[2] < 0 || indices[2] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[2])); exit(0); }
				dest->mFaces[j].mIndices = new uint32_t[3];
				if (bFlip) {
					dest->mFaces[j].mIndices[0] = indices[2];
					dest->mFaces[j].mIndices[1] = indices[1];
					dest->mFaces[j].mIndices[2] = indices[0];
				}
				else {
					dest->mFaces[j].mIndices[0] = indices[0];
					dest->mFaces[j].mIndices[1] = indices[1];
					dest->mFaces[j].mIndices[2] = indices[2];
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
				if (indices[0] < 0 || indices[0] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[0])); exit(0); }
				if (indices[1] < 0 || indices[1] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[1])); exit(0); }
				if (indices[2] < 0 || indices[2] >= src.nVertexCount) { WriteConsole("Index out of bounds: " + std::to_string(indices[2])); exit(0); }
				dest->mFaces[j].mIndices = new uint32_t[3];
				dest->mFaces[j].mIndices[0] = indices[0];
				dest->mFaces[j].mIndices[1] = indices[1];
				dest->mFaces[j].mIndices[2] = indices[2];
				dest->mFaces[j].mNumIndices = 3;
				indexData += 2 * 3;
			}
		}
	}

	if (auto node = new aiNode()) {
		int numStaticMeshes = 0;
		for (auto& batch : aStaticBatches) {
			if (IsSurfaceValidAndExportable(batch.nSurfaceId)) numStaticMeshes++;
		}

		node->mName = "StaticBatch";
		scene.mRootNode->addChildren(1, &node);
		node->mMeshes = new uint32_t[numStaticMeshes];
		node->mNumMeshes = numStaticMeshes;
		int i = 0;
		for (auto &batch: aStaticBatches) {
			if (!IsSurfaceValidAndExportable(batch.nSurfaceId)) continue;
			node->mMeshes[i++] = aSurfaces[batch.nSurfaceId]._nFBXModelId;
		}
	}

	if (auto node = new aiNode()) {
		node->mName = "TreeMesh";
		scene.mRootNode->addChildren(1, &node);
		for (auto& treeMesh : aTreeMeshes) {
			if (auto treeNode = CreateNodeForTreeMesh(&scene, treeMesh)) {
				node->addChildren(1, &treeNode);
			}
		}
	}

	if (auto node = new aiNode()) {
		node->mName = "Objects";
		scene.mRootNode->addChildren(1, &node);
		for (auto& object : aObjects) {
			auto objectNode = new aiNode();
			node->addChildren(1, &objectNode);

			objectNode->mName = object.sName1;
			objectNode->mTransformation.a1 = object.mMatrix[0];
			objectNode->mTransformation.b1 = object.mMatrix[1];
			objectNode->mTransformation.c1 = -object.mMatrix[2];
			objectNode->mTransformation.d1 = object.mMatrix[3];
			objectNode->mTransformation.a2 = object.mMatrix[4];
			objectNode->mTransformation.b2 = object.mMatrix[5];
			objectNode->mTransformation.c2 = -object.mMatrix[6];
			objectNode->mTransformation.d2 = object.mMatrix[7];
			objectNode->mTransformation.a3 = -object.mMatrix[8];
			objectNode->mTransformation.b3 = -object.mMatrix[9];
			objectNode->mTransformation.c3 = object.mMatrix[10];
			objectNode->mTransformation.d3 = object.mMatrix[11];
			objectNode->mTransformation.a4 = object.mMatrix[12];
			objectNode->mTransformation.b4 = object.mMatrix[13];
			objectNode->mTransformation.c4 = -object.mMatrix[14];
			objectNode->mTransformation.d4 = object.mMatrix[15];
		}
	}

	if (auto node = new aiNode()) {
		node->mName = "CompactMesh";
		scene.mRootNode->addChildren(1, &node);
		for (auto& compactMesh : aCompactMeshes) {
			auto meshNode = new aiNode();
			meshNode->mName = compactMesh.sName1;
			meshNode->mTransformation.a1 = compactMesh.mMatrix[0];
			meshNode->mTransformation.b1 = compactMesh.mMatrix[1];
			meshNode->mTransformation.c1 = -compactMesh.mMatrix[2];
			meshNode->mTransformation.d1 = compactMesh.mMatrix[3];
			meshNode->mTransformation.a2 = compactMesh.mMatrix[4];
			meshNode->mTransformation.b2 = compactMesh.mMatrix[5];
			meshNode->mTransformation.c2 = -compactMesh.mMatrix[6];
			meshNode->mTransformation.d2 = compactMesh.mMatrix[7];
			meshNode->mTransformation.a3 = -compactMesh.mMatrix[8];
			meshNode->mTransformation.b3 = -compactMesh.mMatrix[9];
			meshNode->mTransformation.c3 = compactMesh.mMatrix[10];
			meshNode->mTransformation.d3 = compactMesh.mMatrix[11];
			meshNode->mTransformation.a4 = compactMesh.mMatrix[12];
			meshNode->mTransformation.b4 = compactMesh.mMatrix[13];
			meshNode->mTransformation.c4 = -compactMesh.mMatrix[14];
			meshNode->mTransformation.d4 = compactMesh.mMatrix[15];
			node->addChildren(1, &meshNode);

			// not loading the damaged or lod parts here
			if (!compactMesh.aLODMeshIds.empty()) {
				auto model = aModels[compactMesh.aLODMeshIds[0]];
				std::vector<int> aMeshes;
				for (auto& surfaceId : model.aSurfaces) {
					if (!IsSurfaceValidAndExportable(surfaceId)) continue;
					aMeshes.push_back(aSurfaces[surfaceId]._nFBXModelId);
				}
				meshNode->mMeshes = new uint32_t[aMeshes.size()];
				memcpy(meshNode->mMeshes, &aMeshes[0], aMeshes.size() * sizeof(uint32_t));
				meshNode->mNumMeshes = aMeshes.size();
			}
		}
	}

	return scene;
}

void WriteW32ToFBX() {
	WriteConsole("Writing model file...");

	auto scene = GenerateScene();

	Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
	Assimp::DefaultLogger::create("export_log.txt",severity, aiDefaultLogStream_FILE);

	Assimp::Exporter exporter;
	if (exporter.Export(&scene, "fbx", sFileNameNoExt + "_out.fbx") != aiReturn_SUCCESS) {
		WriteConsole("Model export failed!");
	}
	else {
		WriteConsole("Model export finished");
	}
}