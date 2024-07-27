void WriteMaterialToFile(std::ofstream& file, const tMaterial& material) {
	file.write((char*)&material.identifier, 4);
	file.write(material.sName.c_str(), material.sName.length() + 1);
	file.write((char*)&material.nAlpha, 4);
	file.write((char*)&material.v92, 4);
	file.write((char*)&material.nNumTextures, 4);
	file.write((char*)&material.nShaderId, 4);
	file.write((char*)&material.nUseColormap, 4);
	file.write((char*)&material.v74, 4);
	file.write((char*)material.v108, sizeof(material.v108));
	file.write((char*)material.v109, sizeof(material.v109));
	file.write((char*)material.v98, sizeof(material.v98));
	file.write((char*)material.v99, sizeof(material.v99));
	file.write((char*)material.v100, sizeof(material.v100));
	file.write((char*)material.v101, sizeof(material.v101));
	file.write((char*)&material.v102, 4);
	for (int i = 0; i < 3; i++) {
		file.write(material.sTextureNames[i].c_str(), material.sTextureNames[i].length() + 1);
	}
}

// FO1 doesn't support the vertex color + vertex normal combo and so crashes
// for exporting to FO1, i adjust the vertex buffers to not have normals if they also have vertex colors
// this could prolly be fixed with a plugin too to actually get rid of the issue
bool IsBufferReductionRequiredForFO1(uint32_t flags) {
	if (bIsBGMModel) return false;
	if ((flags & 0x10) == 0) return false;
	if ((flags & 0x40) == 0) return false;
	return true;
}

void WriteVertexBufferToFile(std::ofstream& file, tVertexBuffer& buf) {
	bool bRemoveNormals = false;
	if (nExportFileVersion < 0x20000 && nExportFileVersion != nImportFileVersion && IsBufferReductionRequiredForFO1(buf.flags)) {
		buf._vertexSizeBeforeFO1 = buf.vertexSize;
		buf.flags -= 0x10;
		buf.vertexSize -= 0xC;
		bRemoveNormals = true;
	}

	int type = 1;
	file.write((char*)&type, 4);
	file.write((char*)&buf.foucExtraFormat, 4);
	file.write((char*)&buf.vertexCount, 4);
	file.write((char*)&buf.vertexSize, 4);
	file.write((char*)&buf.flags, 4);
	if (bRemoveNormals) {
		int numWritten = 0;

		auto dataSize = buf.vertexCount * (buf._vertexSizeBeforeFO1 / sizeof(float));
		size_t j = 0;
		while (j < dataSize) {
			for (int k = 0; k < buf._vertexSizeBeforeFO1 / sizeof(float); k++) {
				if (k == 3 || k == 4 || k == 5) {
					j++;
					continue;
				}
				file.write((char*)&buf.data[j], 4);
				j++;
				numWritten++;
			}
		}

		if (numWritten != buf.vertexCount * (buf.vertexSize / 4)) {
			WriteConsole("Write mismatch!");
			WriteConsole(std::to_string(buf.vertexCount * (buf.vertexSize / 4)));
			WriteConsole(std::to_string(numWritten));
		}
	}
	else {
		if (buf.origDataForFOUCExport) {
			file.write((char*)buf.origDataForFOUCExport, buf.vertexCount * buf.vertexSize);
		}
		else {
			int vertexSize = buf.vertexSize;
			if (buf.foucExtraFormat == 22) vertexSize = 32;
			file.write((char*)buf.data, buf.vertexCount * vertexSize);
		}
	}
}

void WriteIndexBufferToFile(std::ofstream& file, const tIndexBuffer& buf) {
	int type = 2;
	file.write((char*)&type, 4);
	file.write((char*)&buf.foucExtraFormat, 4);
	file.write((char*)&buf.indexCount, 4);
	file.write((char*)buf.data, buf.indexCount * 2);
}

void WriteVegVertexBufferToFile(std::ofstream& file, const tVegVertexBuffer& buf) {
	int type = 3;
	file.write((char*)&type, 4);
	file.write((char*)&buf.foucExtraFormat, 4);
	file.write((char*)&buf.vertexCount, 4);
	file.write((char*)&buf.vertexSize, 4);
	file.write((char*)buf.data, buf.vertexCount * buf.vertexSize);
}

void WriteSurfaceToFile(std::ofstream& file, tSurface& surface) {
	if (nExportFileVersion < 0x20000 && nExportFileVersion != nImportFileVersion && IsBufferReductionRequiredForFO1(surface.nFlags)) {
		surface.nFlags -= 0x10;
		auto stream = FindVertexBuffer(surface.nStreamId[0]);
		auto vertexSizeBefore = stream->_vertexSizeBeforeFO1;
		auto vertexSizeAfter = stream->vertexSize;
		surface.nStreamOffset[0] /= vertexSizeBefore;
		surface.nStreamOffset[0] *= vertexSizeAfter;
	}

	file.write((char*)&surface.nIsVegetation, 4);
	file.write((char*)&surface.nMaterialId, 4);
	file.write((char*)&surface.nVertexCount, 4);
	file.write((char*)&surface.nFlags, 4);
	file.write((char*)&surface.nPolyCount, 4);
	file.write((char*)&surface.nPolyMode, 4);
	file.write((char*)&surface.nNumIndicesUsed, 4);
	if (nExportFileVersion < 0x20000) {
		file.write((char*)surface.vAbsoluteCenter, 12);
		file.write((char*)surface.vRelativeCenter, 12);
	}
	if (bIsFOUCModel) {
		file.write((char*)surface.foucVertexMultiplier, sizeof(surface.foucVertexMultiplier));
	}
	file.write((char*)&surface.nNumStreamsUsed, 4);
	for (int j = 0; j < surface.nNumStreamsUsed; j++) {
		file.write((char*)&surface.nStreamId[j], 4);
		file.write((char*)&surface.nStreamOffset[j], 4);
	}
}

void WriteStaticBatchToFile(std::ofstream& file, const tStaticBatch& staticBatch) {
	file.write((char*)&staticBatch.nCenterId1, 4);
	file.write((char*)&staticBatch.nCenterId2, 4);
	file.write((char*)&staticBatch.nSurfaceId, 4);
	if (nExportFileVersion >= 0x20000) {
		file.write((char*)staticBatch.vAbsoluteCenter, 12);
		file.write((char*)staticBatch.vRelativeCenter, 12);
	}
	else {
		file.write((char*)&staticBatch.nUnk, 4);
	}
}

void WriteTreeMeshToFile(std::ofstream& file, const tTreeMesh& treeMesh) {
	file.write((char*)&treeMesh.nUnk1, 4);
	file.write((char*)&treeMesh.nUnk2Unused, 4);
	file.write((char*)&treeMesh.nSurfaceId1Unused, 4);
	file.write((char*)&treeMesh.nSurfaceId2, 4);
	file.write((char*)treeMesh.fUnk, sizeof(treeMesh.fUnk));
	if (bIsFOUCModel) {
		file.write((char*)treeMesh.foucExtraData1, sizeof(treeMesh.foucExtraData1));
		file.write((char*)treeMesh.foucExtraData2, sizeof(treeMesh.foucExtraData2));
		file.write((char*)treeMesh.foucExtraData3, sizeof(treeMesh.foucExtraData3));
		file.write((char*)&treeMesh.nSurfaceId3, 4);
		file.write((char*)&treeMesh.nSurfaceId4, 4);
		file.write((char*)&treeMesh.nSurfaceId5, 4);
	}
	else {
		file.write((char*)&treeMesh.nSurfaceId3, 4);
		file.write((char*)&treeMesh.nSurfaceId4, 4);
		file.write((char*)&treeMesh.nSurfaceId5, 4);
		file.write((char*)&treeMesh.nIdInUnkArray1, 4);
		file.write((char*)&treeMesh.nIdInUnkArray2, 4);
		file.write((char*)&treeMesh.nMaterialId, 4);
	}
}

void WriteModelToFile(std::ofstream& file, const tModel& model) {
	file.write((char*)&model.identifier, 4);
	file.write((char*)&model.nUnk, 4);
	file.write(model.sName.c_str(), model.sName.length() + 1);
	file.write((char*)model.vCenter, sizeof(model.vCenter));
	file.write((char*)model.vRadius, sizeof(model.vRadius));
	file.write((char*)&model.fRadius, 4);
	int numSurfaces = model.aSurfaces.size();
	file.write((char*)&numSurfaces, 4);
	for (auto& surface : model.aSurfaces) {
		file.write((char*)&surface, 4);
	}
}

void WriteObjectToFile(std::ofstream& file, const tObject& object) {
	file.write((char*)&object.identifier, 4);
	file.write(object.sName1.c_str(), object.sName1.length() + 1);
	file.write(object.sName2.c_str(), object.sName2.length() + 1);
	file.write((char*)&object.nFlags, 4);
	file.write((char*)object.mMatrix, sizeof(object.mMatrix));
}

void WriteBoundingBoxToFile(std::ofstream& file, const tBoundingBox& bbox) {
	int numModels = bbox.aModels.size();
	file.write((char*)&numModels, 4);
	for (auto& model : bbox.aModels) {
		file.write((char*)&model, 4);
	}
	file.write((char*)bbox.vCenter, sizeof(bbox.vCenter));
	file.write((char*)bbox.vRadius, sizeof(bbox.vRadius));
}

void WriteBoundingBoxMeshAssocToFile(std::ofstream& file, const tBoundingBoxMeshAssoc& assoc) {
	file.write(assoc.sName.c_str(), assoc.sName.length() + 1);
	file.write((char*)assoc.nIds, sizeof(assoc.nIds));
}

void WriteCompactMeshToFile(std::ofstream& file, tCompactMesh& mesh) {
	if (bEnableAllProps && mesh.nFlags == 0x8000) mesh.nFlags = 0x2000;
	if (bImportPropsFromFBX) {
		if (auto fbx = FindFBXNodeForCompactMesh(mesh.sName1)) {
			float oldMatrix[4*4];
			memcpy(oldMatrix, mesh.mMatrix, sizeof(oldMatrix));
			FBXMatrixToFO2Matrix( GetFullMatrixFromCompactMeshObject(fbx), mesh.mMatrix);
			if (bUngroupMovedPropsFromFBX) {
				if (std::abs(oldMatrix[12] - mesh.mMatrix[12]) > 1 || std::abs(oldMatrix[13] - mesh.mMatrix[13]) > 1 || std::abs(oldMatrix[14] - mesh.mMatrix[14]) > 1) {
					mesh.nGroup = -1;
				}
			}
		}
	}

	file.write((char*)&mesh.identifier, 4);
	file.write(mesh.sName1.c_str(), mesh.sName1.length() + 1);
	file.write(mesh.sName2.c_str(), mesh.sName2.length() + 1);
	file.write((char*)&mesh.nFlags, 4);
	file.write((char*)&mesh.nGroup, 4);
	file.write((char*)mesh.mMatrix, sizeof(mesh.mMatrix));
	if (nExportFileVersion >= 0x20000) {
		file.write((char*)&mesh.nUnk1, 4);
		file.write((char*)&mesh.nBBoxAssocId, 4);
	}
	else {
		int numLODs = mesh.aLODMeshIds.size();
		file.write((char*)&numLODs, 4);
		for (auto model : mesh.aLODMeshIds) {
			file.write((char*)&model, 4);
		}
	}
}

void WriteCarMeshToFile(std::ofstream& file, tCarMesh& mesh) {
	file.write((char*)&mesh.identifier, 4);
	file.write(mesh.sName1.c_str(), mesh.sName1.length() + 1);
	file.write(mesh.sName2.c_str(), mesh.sName2.length() + 1);
	file.write((char*)&mesh.nFlags, 4);
	file.write((char*)&mesh.nGroup, 4);
	file.write((char*)mesh.mMatrix, sizeof(mesh.mMatrix));
	int numModels = mesh.aModels.size();
	file.write((char*)&numModels, 4);
	for (auto model : mesh.aModels) {
		file.write((char*)&model, 4);
	}
}

void CreateStreamsFromFBX(aiMesh* mesh, uint32_t flags, uint32_t vertexSize) {
	int id = aVertexBuffers.size() + aVegVertexBuffers.size() + aIndexBuffers.size();

	if ((flags & VERTEX_UV2) != 0 && !mesh->HasTextureCoords(1)) {
		WriteConsole("WARNING: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have 2 sets of UVs!");
		//exit(0);
	}

	tVertexBuffer vBuf;
	vBuf.id = id;
	vBuf.flags = flags;
	if (bIsFOUCModel) {
		vBuf.foucExtraFormat = 22;
		vertexSize = 32;
	}
	vBuf.vertexSize = vertexSize;
	vBuf.vertexCount = mesh->mNumVertices;
	if (vBuf.vertexCount > 65535) {
		WriteConsole("WARNING: " + (std::string)mesh->mName.C_Str() + " has more than 65535 vertices! Split the mesh or it won't render properly!");
	}
	if (bIsFOUCModel) {
		vBuf.flags |= VERTEX_INT16;
		vBuf.data = new float[mesh->mNumVertices * (vertexSize / 4)];
		memset(vBuf.data, 0, mesh->mNumVertices * (vertexSize / 4) * sizeof(float));
		auto vertexData = vBuf.data;
		for (int i = 0; i < mesh->mNumVertices; i++) {
			auto vertices = (uint16_t*)vertexData;
			vertices[0] = mesh->mVertices[i].x / fFOUCCarMultiplier;
			vertices[1] = mesh->mVertices[i].y / fFOUCCarMultiplier;
			vertices[2] = -mesh->mVertices[i].z / fFOUCCarMultiplier;
			vertices += 3;

			if ((flags & VERTEX_NORMAL) != 0 || bIsFOUCModel) {
				if (!mesh->HasNormals()) {
					WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have normals!");
					exit(0);
				}

				// scalar + bumpmap strength i believe
				vertices[0] = 0x0400;
				vertices[1] = 0;
				vertices[2] = 0;
				vertices[3] = 0;
				vertices += 4; // 1 int, 3 floats

				auto int8Vertices = (uint8_t*)vertices;
				int8Vertices[0] = 0;
				int8Vertices[1] = 0;

				double tmp = (-mesh->mNormals[i].z + 1) * 127.0;
				int8Vertices[2] = tmp;
				tmp = (mesh->mNormals[i].y + 1) * 127.0;
				int8Vertices[3] = tmp;
				tmp = (mesh->mNormals[i].x + 1) * 127.0;
				int8Vertices[4] = tmp;
				int8Vertices[5] = 0;
				vertices += 3; // 3 floats
			}
			if ((flags & VERTEX_COLOR) != 0) {
				if (mesh->HasVertexColors(0)) {
					uint8_t tmp[4] = {0, 0, 0, 0xFF};
					tmp[0] = mesh->mColors[0][i].r * 255.0;
					tmp[1] = mesh->mColors[0][i].g * 255.0;
					tmp[2] = mesh->mColors[0][i].b * 255.0;
					*(uint32_t*)&vertices[0] = *(uint32_t*)tmp;
				}
				else {
					*(uint32_t*)&vertices[0] = 0xFFFFFFFF;
				}
				vertices += 2; // 1 int32
			}
			if ((flags & VERTEX_UV) != 0 || (flags & VERTEX_UV2) != 0) {
				if (!mesh->HasTextureCoords(0)) {
					WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have UVs!");
					exit(0);
				}

				vertices[0] = mesh->mTextureCoords[0][i].x * 2048.0;
				vertices[1] = 1 - mesh->mTextureCoords[0][i].y * 2048.0;
				vertices += 2; // 2 floats
			}
			if ((flags & VERTEX_UV2) != 0) {
				if (mesh->HasTextureCoords(1)) {
					vertices[0] = mesh->mTextureCoords[1][i].x * 2048.0;
					vertices[1] = 1 - mesh->mTextureCoords[1][i].y * 2048.0;
				}
				else {
					vertices[0] = 0;
					vertices[1] = 0;
				}
				vertices += 2; // 2 floats
			}
			vertexData += vertexSize / 4;
		}
	}
	else {
		vBuf.data = new float[mesh->mNumVertices * (vertexSize / 4)];
		memset(vBuf.data, 0, mesh->mNumVertices * (vertexSize / 4) * sizeof(float));
		auto vertexData = vBuf.data;
		for (int i = 0; i < mesh->mNumVertices; i++) {
			auto vertices = (float*)vertexData;
			vertices[0] = mesh->mVertices[i].x;
			vertices[1] = mesh->mVertices[i].y;
			vertices[2] = -mesh->mVertices[i].z;
			vertices += 3;

			if ((flags & VERTEX_NORMAL) != 0) {
				if (!mesh->HasNormals()) {
					WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have normals!");
					exit(0);
				}

				vertices[0] = mesh->mNormals[i].x;
				vertices[1] = mesh->mNormals[i].y;
				vertices[2] = -mesh->mNormals[i].z;
				vertices += 3; // 3 floats
			}
			if ((flags & VERTEX_COLOR) != 0) {
				if (mesh->HasVertexColors(0)) {
					uint8_t tmp[4] = {0, 0, 0, 0xFF};
					tmp[0] = mesh->mColors[0][i].r * 255.0;
					tmp[1] = mesh->mColors[0][i].g * 255.0;
					tmp[2] = mesh->mColors[0][i].b * 255.0;
					*(uint32_t*)&vertices[0] = *(uint32_t*)tmp;
				}
				else {
					*(uint32_t*)&vertices[0] = 0xFFFFFFFF;
				}
				vertices += 1; // 1 int32
			}
			if ((flags & VERTEX_UV) != 0 || (flags & VERTEX_UV2) != 0) {
				if (!mesh->HasTextureCoords(0)) {
					WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have UVs!");
					exit(0);
				}

				vertices[0] = mesh->mTextureCoords[0][i].x;
				vertices[1] = 1 - mesh->mTextureCoords[0][i].y;
				vertices += 2; // 2 floats
			}
			if ((flags & VERTEX_UV2) != 0) {
				if (mesh->HasTextureCoords(1)) {
					vertices[0] = mesh->mTextureCoords[1][i].x;
					vertices[1] = 1 - mesh->mTextureCoords[1][i].y;
				}
				else {
					vertices[0] = 0;
					vertices[1] = 0;
				}
				vertices += 2; // 2 floats
			}
			vertexData += vertexSize / 4;
		}
	}
	aVertexBuffers.push_back(vBuf);

	tIndexBuffer iBuf;
	iBuf.id = id + 1;
	iBuf.indexCount = mesh->mNumFaces * 3;
	iBuf.data = new uint16_t[iBuf.indexCount];
	auto indexData = iBuf.data;
	for (int i = 0; i < mesh->mNumFaces; i++) {
		auto& face = mesh->mFaces[i];
		if (face.mNumIndices != 3) {
			WriteConsole("ERROR: Non-tri found in FBX mesh while exporting!");
			continue;
		}
		indexData[0] = face.mIndices[2];
		indexData[1] = face.mIndices[1];
		indexData[2] = face.mIndices[0];
		indexData += 3;
	}
	aIndexBuffers.push_back(iBuf);
}

tMaterial GetCarMaterialFromFBX(aiMaterial* fbxMaterial) {
	tMaterial mat;
	auto matName = fbxMaterial->GetName().C_Str();
	mat.sName = matName;
	mat.nShaderId = 8; // car metal
	if (mat.sName.starts_with("body")) mat.nShaderId = 5; // car body
	if (mat.sName.starts_with("interior")) mat.nShaderId = 7; // car diffuse
	if (mat.sName.starts_with("shadow")) mat.nShaderId = 13; // shadow project
	if (mat.sName.starts_with("window")) mat.nShaderId = 6; // car window
	if (mat.sName.starts_with("shear")) mat.nShaderId = 11; // car shear
	if (mat.sName.starts_with("scale")) mat.nShaderId = 12; // car scale
	if (bIsFOUCModel && mat.sName.starts_with("tire")) mat.nShaderId = 44; // car tire
	if (bIsFOUCModel && mat.sName.starts_with("rim")) mat.nShaderId = 9; // car tire rim
	if (mat.sName.starts_with("terrain") || mat.sName.starts_with("groundplane")) mat.nShaderId = 7; // car diffuse
	if (mat.sName.starts_with("light")) {
		mat.v92 = 2;
		mat.nShaderId = 10; // car lights
	}
	if (mat.sName.ends_with(".001")) {
		for (int i = 0; i < 4; i++) {
			mat.sName.pop_back();
		}
	}

	mat.nNumTextures = fbxMaterial->GetTextureCount(aiTextureType_DIFFUSE);
	if (mat.nNumTextures > 3) mat.nNumTextures = 3;
	for (int i = 0; i < mat.nNumTextures; i++) {
		auto texName= GetFBXTextureInFO2Style(fbxMaterial, i);
		mat.sTextureNames[i] = texName;
		if (texName == "lights.tga" || texName == "windows.tga" || texName == "shock.tga") {
			mat.nAlpha = 1;
		}
	}
	// shadow project has no texture
	if (mat.sName.starts_with("shadow")) {
		mat.sTextureNames[0] = "";
	}
	return mat;
}

void CreateBGMSurfaceFromFBX(aiNode* node, int meshId) {
	auto mesh = pParsedFBXScene->mMeshes[node->mMeshes[meshId]];

	tSurface surface;
	surface.nIsVegetation = 0;
	surface.nVertexCount = mesh->mNumVertices;
	surface.nPolyCount = mesh->mNumFaces;
	surface.nNumIndicesUsed = mesh->mNumFaces * 3;
	surface.nFlags = aVertexBuffers[node->mMeshes[meshId]].flags;
	surface.nMaterialId = mesh->mMaterialIndex;
	surface.nNumStreamsUsed = 2;
	surface.nPolyMode = 4;
	surface.nStreamId[0] = node->mMeshes[meshId] * 2;
	surface.nStreamId[1] = (node->mMeshes[meshId] * 2) + 1;
	surface.nStreamOffset[0] = 0;
	surface.nStreamOffset[1] = 0;

	surface.foucVertexMultiplier[0] = 0;
	surface.foucVertexMultiplier[1] = 0;
	surface.foucVertexMultiplier[2] = 0;
	surface.foucVertexMultiplier[3] = fFOUCCarMultiplier;

	aSurfaces.push_back(surface);
}

void FillBGMFromFBX() {
	WriteConsole("Creating BGM data...");

	WriteConsole("Creating materials...");

	// create materials
	for (int i = 0; i < pParsedFBXScene->mNumMaterials; i++) {
		auto src = pParsedFBXScene->mMaterials[i];
		auto material = GetCarMaterialFromFBX(src);
		aMaterials.push_back(material);
	}

	WriteConsole("Creating streams...");

	// create streams
	for (int i = 0; i < pParsedFBXScene->mNumMeshes; i++) {
		auto src = pParsedFBXScene->mMeshes[i];
		WriteConsole("Exporting " + (std::string)src->mName.C_Str());

		auto material = &aMaterials[src->mMaterialIndex];
		if (bIsFOUCModel) {
			CreateStreamsFromFBX(src, VERTEX_POSITION | VERTEX_COLOR | VERTEX_UV2, 36);
		}
		else if (material->nShaderId == 5) { // car body, has vertex colors
			CreateStreamsFromFBX(src, VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_UV, 36);
		}
		else if (material->nShaderId == 13) { // shadow project, position only
			CreateStreamsFromFBX(src, VERTEX_POSITION, 12);
		}
		else { // regular meshes, only normal and uv
			CreateStreamsFromFBX(src, VERTEX_POSITION | VERTEX_NORMAL | VERTEX_UV, 32);
		}
	}

	WriteConsole("Creating car meshes & surfaces...");

	auto carMeshArray = GetFBXNodeForCarMeshArray();
	for (int i = 0; i < carMeshArray->mNumChildren; i++) {
		auto bodyNode = carMeshArray->mChildren[i]; // body

		tCarMesh carMesh;
		carMesh.sName1 = bodyNode->mName.C_Str();
		carMesh.nFlags = 0x0;
		carMesh.nGroup = -1;
		FBXMatrixToFO2Matrix(bodyNode->mTransformation, carMesh.mMatrix);

		for (int j = 0; j < bodyNode->mNumChildren; j++) {
			auto body001 = bodyNode->mChildren[j]; // body.001

			tModel model;
			model.sName = body001->mName.C_Str();

			float aabbMin[3] = {0, 0, 0};
			float aabbMax[3] = {0, 0, 0};

			for (int k = 0; k < body001->mNumMeshes; k++) {
				// Z inversion doesn't matter here - we're calculating a radius anyway
				auto mesh = pParsedFBXScene->mMeshes[body001->mMeshes[k]];
				if (aabbMin[0] > mesh->mAABB.mMin.x) aabbMin[0] = mesh->mAABB.mMin.x;
				if (aabbMin[1] > mesh->mAABB.mMin.y) aabbMin[1] = mesh->mAABB.mMin.y;
				if (aabbMin[2] > mesh->mAABB.mMin.z) aabbMin[2] = mesh->mAABB.mMin.z;
				if (aabbMax[0] < mesh->mAABB.mMax.x) aabbMax[0] = mesh->mAABB.mMax.x;
				if (aabbMax[1] < mesh->mAABB.mMax.y) aabbMax[1] = mesh->mAABB.mMax.y;
				if (aabbMax[2] < mesh->mAABB.mMax.z) aabbMax[2] = mesh->mAABB.mMax.z;

				model.aSurfaces.push_back(aSurfaces.size());

				CreateBGMSurfaceFromFBX(body001, k);
			}
			for (int k = 0; k < body001->mNumChildren; k++) {
				auto surface = body001->mChildren[k]; // Surface1
				for (int l = 0; l < surface->mNumMeshes; l++) {

					// Z inversion doesn't matter here - we're calculating a radius anyway
					auto mesh = pParsedFBXScene->mMeshes[surface->mMeshes[l]];
					if (aabbMin[0] > mesh->mAABB.mMin.x) aabbMin[0] = mesh->mAABB.mMin.x;
					if (aabbMin[1] > mesh->mAABB.mMin.y) aabbMin[1] = mesh->mAABB.mMin.y;
					if (aabbMin[2] > mesh->mAABB.mMin.z) aabbMin[2] = mesh->mAABB.mMin.z;
					if (aabbMax[0] < mesh->mAABB.mMax.x) aabbMax[0] = mesh->mAABB.mMax.x;
					if (aabbMax[1] < mesh->mAABB.mMax.y) aabbMax[1] = mesh->mAABB.mMax.y;
					if (aabbMax[2] < mesh->mAABB.mMax.z) aabbMax[2] = mesh->mAABB.mMax.z;

					model.aSurfaces.push_back(aSurfaces.size());

					CreateBGMSurfaceFromFBX(surface, l);
				}
			}

			if (model.sName.ends_with(".001")) {
				for (int k = 0; k < 4; k++) {
					model.sName.pop_back();
				}
			}

			model.vCenter[0] = (aabbMax[0] + aabbMin[0]) * 0.5;
			model.vCenter[1] = (aabbMax[1] + aabbMin[1]) * 0.5;
			model.vCenter[2] = (aabbMax[2] + aabbMin[2]) * -0.5;
			model.vRadius[0] = std::abs(aabbMax[0] - aabbMin[0]) * 0.5;
			model.vRadius[1] = std::abs(aabbMax[1] - aabbMin[1]) * 0.5;
			model.vRadius[2] = std::abs(aabbMax[2] - aabbMin[2]) * 0.5;
			// fRadius isn't required, game doesn't read it
			carMesh.aModels.push_back(aModels.size());
			aModels.push_back(model);
		}

		aCarMeshes.push_back(carMesh);
	}

	WriteConsole("Creating object dummies...");

	auto objectsArray = GetFBXNodeForObjectsArray();
	for (int i = 0; i < objectsArray->mNumChildren; i++) {
		auto objectNode = objectsArray->mChildren[i];

		tObject object;
		object.sName1 = objectNode->mName.C_Str();
		object.nFlags = 0xE0F9;
		FBXMatrixToFO2Matrix(objectNode->mTransformation, object.mMatrix);
		aObjects.push_back(object);
	}

	WriteConsole("BGM data created");
}

void WriteW32(uint32_t exportMapVersion) {
	WriteConsole("Writing output w32 file...");

	nExportFileVersion = exportMapVersion;
	if ((nExportFileVersion == 0x20002 || nImportFileVersion == 0x20002 || bIsFOUCModel) && nImportFileVersion != nExportFileVersion) {
		WriteConsole("ERROR: FOUC conversions are currently not supported!");
		return;
	}

	std::ofstream file(sFileNameNoExt + "_out.w32", std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	file.write((char*)&nExportFileVersion, 4);
	if (nExportFileVersion >= 0x20000) file.write((char*)&nSomeMapValue, 4);

	uint32_t streamCount = aVertexBuffers.size() + aVegVertexBuffers.size() + aIndexBuffers.size();
	if (bLoadFBX && bImportSurfacesFromFBX && !bIsFOUCModel) { // surface exports only supported for FO1 and FO2 currently
		for (auto& surface : aSurfaces) {
			if (auto node = FindFBXNodeForSurface(&surface - &aSurfaces[0])) {
				if (ShouldSurfaceMeshBeImported(node)) {
					WriteConsole("Exporting surface " + (std::string)node->mName.C_Str());

					auto buf = FindVertexBuffer(surface.nStreamId[0]);
					auto mesh = pParsedFBXScene->mMeshes[node->mMeshes[0]];
					uint32_t bufFlags = VERTEX_POSITION;
					uint32_t vertexSize = 3 * sizeof(float);
					if ((buf->flags & VERTEX_NORMAL) != 0) {
						bufFlags += VERTEX_NORMAL;
						vertexSize += 3 * sizeof(float);
					}
					if ((buf->flags & VERTEX_COLOR) != 0) {
						bufFlags += VERTEX_COLOR;
						vertexSize += 1 * sizeof(uint32_t);
					}
					if (mesh->HasTextureCoords(0) && mesh->HasTextureCoords(1)) {
						bufFlags += VERTEX_UV2;
						vertexSize += 4 * sizeof(float);
					}
					else if (mesh->HasTextureCoords(0)) {
						bufFlags += VERTEX_UV;
						vertexSize += 2 * sizeof(float);
					}

					// todo this seems to be broken - mMaterialIndex isn't correct here for some reason
					if (bImportSurfaceMaterialsFromFBX) {
						auto fbxMaterial = pParsedFBXScene->mMaterials[mesh->mMaterialIndex];
						auto matName = fbxMaterial->GetName().C_Str();
						auto material = FindMaterialIDByName(matName);
						if (material < 0) {
							material = aMaterials.size();
							tMaterial mat;
							mat.sName = matName;
							mat.nAlpha = mat.sName.starts_with("alpha") || mat.sName.starts_with("Alpha");
							mat.nNumTextures = fbxMaterial->GetTextureCount(aiTextureType_DIFFUSE);
							if (mat.nNumTextures > 3) mat.nNumTextures = 3;
							for (int i = 0; i < mat.nNumTextures; i++) {
								mat.sTextureNames[i] = GetFBXTextureInFO2Style(fbxMaterial, i);
								if (i == 0 && mat.sTextureNames[i] == "colormap.tga") {
									mat.nShaderId = 2; // terrain specular
									mat.nUseColormap = 1;

									// hack to load colormapped textures properly when the fbx has texture2 stripped
									if (mat.nNumTextures == 1) {
										mat.sTextureNames[1] = mat.sName + ".tga";
										mat.nNumTextures = 2;
										break;
									}
								}
							}
							WriteConsole("Creating new material " + mat.sName);
							aMaterials.push_back(mat);
						}
						WriteConsole("Assigning material " + aMaterials[material].sName + " to surface " + node->mName.C_Str());
						surface.nMaterialId = material;
					}

					CreateStreamsFromFBX(mesh, bufFlags, vertexSize);
					surface.nFlags = bufFlags;
					surface.nVertexCount = mesh->mNumVertices;
					surface.nPolyCount = mesh->mNumFaces;
					surface.nNumIndicesUsed = mesh->mNumFaces * 3;
					surface.nPolyMode = 4;
					surface.nStreamOffset[0] = surface.nStreamOffset[1] = 0;
					surface.nStreamId[0] = streamCount;
					surface.nStreamId[1] = streamCount + 1;
					streamCount += 2;
				}
			}
		}
	}

	uint32_t materialCount = aMaterials.size();
	file.write((char*)&materialCount, 4);
	for (auto& material : aMaterials) {
		WriteMaterialToFile(file, material);
	}

	file.write((char*)&streamCount, 4);
	for (int i = 0; i < streamCount; i++) {
		for (auto& buf : aVertexBuffers) {
			if (buf.id == i) {
				WriteVertexBufferToFile(file, buf);
			}
		}
		for (auto& buf : aVegVertexBuffers) {
			if (buf.id == i) {
				WriteVegVertexBufferToFile(file, buf);
			}
		}
		for (auto& buf : aIndexBuffers) {
			if (buf.id == i) {
				WriteIndexBufferToFile(file, buf);
			}
		}
	}

	uint32_t surfaceCount = aSurfaces.size();
	file.write((char*)&surfaceCount, 4);
	for (auto& surface : aSurfaces) {
		if (bImportDeletionFromFBX && !FindFBXNodeForSurface(&surface - &aSurfaces[0]) && !surface._nNumReferencesByType[SURFACE_REFERENCE_MODEL]) {
			surface.nPolyCount = 0;
			surface.nVertexCount = 0;
			surface.nNumIndicesUsed = 0;
		}

		WriteSurfaceToFile(file, surface);
	}

	uint32_t staticBatchCount = aStaticBatches.size();
	file.write((char*)&staticBatchCount, 4);
	for (auto& staticBatch : aStaticBatches) {
		WriteStaticBatchToFile(file, staticBatch);
	}

	if (!bIsFOUCModel) {
		uint32_t unk1Count = aUnknownArray1.size();
		file.write((char*)&unk1Count, 4);
		for (auto &data: aUnknownArray1) {
			file.write((char*)&data, 4);
		}
	}

	uint32_t unk2Count = aUnknownArray2.size();
	file.write((char*)&unk2Count, 4);
	for (auto& data : aUnknownArray2) {
		file.write((char*)data.vPos, sizeof(data.vPos));
		file.write((char*)data.fValues, sizeof(data.fValues));
		file.write((char*)data.nValues, sizeof(data.nValues));
	}

	uint32_t treeMeshCount = aTreeMeshes.size();
	file.write((char*)&treeMeshCount, 4);
	for (auto& mesh : aTreeMeshes) {
		WriteTreeMeshToFile(file, mesh);
	}

	if (nExportFileVersion >= 0x10004) {
		for (int i = 0; i < 16; i++) {
			file.write((char*)&aUnknownArray3[i], 4);
		}
	}

	uint32_t modelCount = aModels.size();
	file.write((char*)&modelCount, 4);
	for (auto& model : aModels) {
		WriteModelToFile(file, model);
	}

	if (bDisableObjects) {
		uint32_t tmpCount = 0;
		file.write((char*)&tmpCount, 4); // objects
	}
	else {
		uint32_t objectCount = aObjects.size();
		file.write((char*)&objectCount, 4);
		for (auto& object : aObjects) {
			WriteObjectToFile(file, object);
		}
	}

	if (bDisableProps) {
		uint32_t tmpCount = 0;
		if (nExportFileVersion >= 0x20000) {
			file.write((char*)&tmpCount, 4); // bbox
			file.write((char*)&tmpCount, 4); // bbox assoc
		}
		file.write((char*)&tmpCount, 4); // compactmesh groups
		file.write((char*)&tmpCount, 4); // compactmesh
	}
	else {
		if (nExportFileVersion >= 0x20000) {
			uint32_t boundingBoxCount = aBoundingBoxes.size();
			file.write((char*)&boundingBoxCount, 4);
			for (auto& bbox : aBoundingBoxes) {
				WriteBoundingBoxToFile(file, bbox);
			}

			uint32_t boundingBoxAssocCount = aBoundingBoxMeshAssoc.size();
			file.write((char*)&boundingBoxAssocCount, 4);
			for (auto& bboxAssoc : aBoundingBoxMeshAssoc) {
				WriteBoundingBoxMeshAssocToFile(file, bboxAssoc);
			}
		}

		uint32_t compactMeshCount = aCompactMeshes.size();
		if (bImportDeletionFromFBX) {
			for (auto& mesh : aCompactMeshes) {
				if (!FindFBXNodeForCompactMesh(mesh.sName1)) compactMeshCount--;
			}
		}

		file.write((char*)&nCompactMeshGroupCount, 4);
		file.write((char*)&compactMeshCount, 4);
		for (auto& mesh : aCompactMeshes) {
			if (bImportDeletionFromFBX && !FindFBXNodeForCompactMesh(mesh.sName1)) continue;
			WriteCompactMeshToFile(file, mesh);
		}
	}

	file.flush();

	WriteConsole("W32 export finished");
}

std::vector<tVertexBuffer> aConversionVertexBuffers;
std::vector<tIndexBuffer> aConversionIndexBuffers;
void ConvertFOUCSurfaceToFO2(tSurface& surface) {
	auto vBuf = FindVertexBuffer(surface.nStreamId[0]);
	auto iBuf = FindIndexBuffer(surface.nStreamId[1]);

	auto stride = vBuf->vertexSize;
	uintptr_t vertexData = ((uintptr_t)vBuf->data) + surface.nStreamOffset[0];
	uintptr_t indexData = ((uintptr_t)iBuf->data) + surface.nStreamOffset[1];

	uint32_t baseVertexOffset = surface.nStreamOffset[0] / vBuf->vertexSize;

	uint32_t flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_UV;
	uint32_t numVertexValues = 8;
	auto material = &aMaterials[surface.nMaterialId];
	if (material->nShaderId == 5) { // car body, has vertex colors
		flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_UV;
		numVertexValues = 9;
	}

	auto newBuffer = new float[surface.nVertexCount * numVertexValues];
	auto newVertexData = (uintptr_t)newBuffer;
	auto newStride = numVertexValues * 4;
	for (int i = 0; i < surface.nVertexCount; i++) {
		auto src = (int16_t*)vertexData;
		auto dest = (float*)newVertexData;

		// uvs always seem to be the last 2 or 4 values in the vertex buffer
		auto uvOffset = stride - 4;
		if ((vBuf->flags & VERTEX_UV2) != 0) uvOffset -= 4;
		auto uvs = (int16_t*)(vertexData + uvOffset);

		dest[0] = src[0];
		dest[1] = src[1];
		dest[2] = src[2];
		dest[0] += surface.foucVertexMultiplier[0];
		dest[1] += surface.foucVertexMultiplier[1];
		dest[2] += surface.foucVertexMultiplier[2];
		dest[0] *= surface.foucVertexMultiplier[3];
		dest[1] *= surface.foucVertexMultiplier[3];
		dest[2] *= surface.foucVertexMultiplier[3];
		src += 3;

		// normals
		{
			src += 4;
			auto int8Vertices = (uint8_t*)src;
			dest[5] = (int8Vertices[2] / 127.0) - 1;
			dest[4] = (int8Vertices[3] / 127.0) - 1;
			dest[3] = (int8Vertices[4] / 127.0) - 1;
		}

		if ((flags & VERTEX_COLOR) != 0) {
			*(uint32_t*)&dest[6] = 0xFFFFFFFF; // vertex color
			dest++;
		}

		if ((vBuf->flags & VERTEX_COLOR) != 0) src += 9;
		if ((vBuf->flags & VERTEX_UV) != 0 || (vBuf->flags & VERTEX_UV2) != 0) {
			dest[6] = uvs[0] / 2048.0;
			dest[7] = uvs[1] / 2048.0;
		}
		//if ((vBuf->flags & VERTEX_UV2) != 0) {
		//}
		vertexData += stride;
		newVertexData += newStride;
	}

	surface.nStreamOffset[0] = 0;
	surface.nStreamOffset[1] = 0;

	tVertexBuffer newVertexBuffer;
	surface.nStreamId[0] = newVertexBuffer.id = (aConversionVertexBuffers.size() + aConversionIndexBuffers.size());
	surface.nFlags = newVertexBuffer.flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_UV;
	newVertexBuffer.vertexSize = numVertexValues * 4;
	newVertexBuffer.vertexCount = surface.nVertexCount;
	newVertexBuffer.data = newBuffer;
	aConversionVertexBuffers.push_back(newVertexBuffer);

	auto newIndices = new uint16_t[surface.nNumIndicesUsed];
	for (int i = 0; i < surface.nNumIndicesUsed; i++) {
		auto tmp = *(uint16_t*)indexData;
		newIndices[i] = tmp - baseVertexOffset;
		if (newIndices[i] < 0 || newIndices[i] >= surface.nVertexCount) {
			WriteConsole("Index out of bounds: " + std::to_string(newIndices[i]));
			exit(0);
		}
		indexData += 2;
	}

	tIndexBuffer newIndexBuffer;
	surface.nStreamId[1] = newIndexBuffer.id = (aConversionVertexBuffers.size() + aConversionIndexBuffers.size());
	newIndexBuffer.indexCount = surface.nNumIndicesUsed;
	newIndexBuffer.data = newIndices;
	aConversionIndexBuffers.push_back(newIndexBuffer);
}

void WriteBGM(uint32_t exportMapVersion) {
	WriteConsole("Writing output bgm file...");

	nExportFileVersion = exportMapVersion;

	std::ofstream file(sFileNameNoExt + "_out.bgm", std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	file.write((char*)&nExportFileVersion, 4);

	uint32_t materialCount = aMaterials.size();
	file.write((char*)&materialCount, 4);
	for (auto& material : aMaterials) {
		// replace car interior shader with car diffuse
		if (bIsFOUCModel && (bConvertToFO1 || bConvertToFO2) && material.nShaderId == 43) {
			material.nShaderId = 7;
		}
		WriteMaterialToFile(file, material);
	}

	if (bIsFOUCModel && (bConvertToFO1 || bConvertToFO2)) {
		for (auto& surface : aSurfaces) {
			if (surface.nFlags != 0x2242) {
				WriteConsole("Unexpected flags value for surface! Can't convert");
				return;
			}
			ConvertFOUCSurfaceToFO2(surface);
		}

		aVertexBuffers = aConversionVertexBuffers;
		aIndexBuffers = aConversionIndexBuffers;
		aVegVertexBuffers.clear();
	}

	if (bConvertToFO1 || bConvertToFO2) bIsFOUCModel = false;

	uint32_t streamCount = aVertexBuffers.size() + aVegVertexBuffers.size() + aIndexBuffers.size();
	file.write((char*)&streamCount, 4);
	for (int i = 0; i < streamCount; i++) {
		for (auto& buf : aVertexBuffers) {
			if (buf.id == i) {
				WriteVertexBufferToFile(file, buf);
			}
		}
		for (auto& buf : aVegVertexBuffers) {
			if (buf.id == i) {
				WriteVegVertexBufferToFile(file, buf);
			}
		}
		for (auto& buf : aIndexBuffers) {
			if (buf.id == i) {
				WriteIndexBufferToFile(file, buf);
			}
		}
	}

	uint32_t surfaceCount = aSurfaces.size();
	file.write((char*)&surfaceCount, 4);
	for (auto& surface : aSurfaces) {
		WriteSurfaceToFile(file, surface);
	}

	uint32_t modelCount = aModels.size();
	file.write((char*)&modelCount, 4);
	for (auto& model : aModels) {
		WriteModelToFile(file, model);
	}

	uint32_t carMeshCount = aCarMeshes.size();
	file.write((char*)&carMeshCount, 4);
	for (auto& mesh : aCarMeshes) {
		WriteCarMeshToFile(file, mesh);
	}

	uint32_t objectCount = aObjects.size();
	file.write((char*)&objectCount, 4);
	for (auto& object : aObjects) {
		WriteObjectToFile(file, object);
	}

	file.flush();

	WriteConsole("BGM export finished");
}