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

	int type = buf.isVegetation ? 3 : 1;
	file.write((char*)&type, 4);
	file.write((char*)&buf.foucExtraFormat, 4);
	file.write((char*)&buf.vertexCount, 4);
	file.write((char*)&buf.vertexSize, 4);
	if (!buf.isVegetation) file.write((char*)&buf.flags, 4);
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
			WriteConsole("Write mismatch!", LOG_ERRORS);
			WriteConsole(std::to_string(buf.vertexCount * (buf.vertexSize / 4)), LOG_ERRORS);
			WriteConsole(std::to_string(numWritten), LOG_ERRORS);
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
		file.write((char*)surface.vCenter, 12);
		file.write((char*)surface.vRadius, 12);
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
	file.write((char*)&staticBatch.nId1, 4);
	file.write((char*)&staticBatch.nBVHId1, 4);
	file.write((char*)&staticBatch.nBVHId2, 4);
	if (nExportFileVersion >= 0x20000) {
		file.write((char*)staticBatch.vCenter, 12);
		file.write((char*)staticBatch.vRadius, 12);
	}
	else {
		file.write((char*)&staticBatch.nUnk, 4);
	}
}

void WriteTreeMeshToFile(std::ofstream& file, const tTreeMesh& treeMesh) {
	file.write((char*)&treeMesh.nIsBush, 4);
	file.write((char*)&treeMesh.nUnk2Unused, 4);
	file.write((char*)&treeMesh.nBVHId1, 4);
	file.write((char*)&treeMesh.nBVHId2, 4);
	file.write((char*)treeMesh.mMatrix, sizeof(treeMesh.mMatrix));
	file.write((char*)treeMesh.fScale, sizeof(treeMesh.fScale));
	if (bIsFOUCModel) {
		file.write((char*)&treeMesh.foucTrunk, sizeof(treeMesh.foucTrunk));
		file.write((char*)&treeMesh.foucBranch, sizeof(treeMesh.foucBranch));
		file.write((char*)&treeMesh.foucLeaf, sizeof(treeMesh.foucLeaf));
		file.write((char*)treeMesh.foucExtraData4, sizeof(treeMesh.foucExtraData4));
	}
	else {
		file.write((char*)&treeMesh.nTrunkSurfaceId, 4);
		file.write((char*)&treeMesh.nBranchSurfaceId, 4);
		file.write((char*)&treeMesh.nLeafSurfaceId, 4);
		file.write((char*)&treeMesh.nColorId, 4);
		file.write((char*)&treeMesh.nLodId, 4);
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

void WriteObjectToFile(std::ofstream& file, tObject& object) {
	if (bImportPropsFromFBX) {
		if (auto fbx = FindFBXNodeForObject(object.sName1)) {
			FBXMatrixToFO2Matrix(GetFullMatrixFromDummyObject(fbx), object.mMatrix);
		}
	}

	file.write((char*)&object.identifier, 4);
	file.write(object.sName1.c_str(), object.sName1.length() + 1);
	file.write(object.sName2.c_str(), object.sName2.length() + 1);
	file.write((char*)&object.nFlags, 4);
	file.write((char*)object.mMatrix, sizeof(object.mMatrix));
}

void WriteCollidableModelToFile(std::ofstream& file, const tCollidableModel& col) {
	int numModels = col.aModels.size();
	file.write((char*)&numModels, 4);
	for (auto& model : col.aModels) {
		file.write((char*)&model, 4);
	}
	file.write((char*)col.vCenter, sizeof(col.vCenter));
	file.write((char*)col.vRadius, sizeof(col.vRadius));
}

void WriteMeshDamageAssocToFile(std::ofstream& file, const tMeshDamageAssoc& assoc) {
	file.write(assoc.sName.c_str(), assoc.sName.length() + 1);
	file.write((char*)assoc.nIds, sizeof(assoc.nIds));
}

void WriteCompactMeshToFile(std::ofstream& file, tCompactMesh& mesh) {
	if (bEnableAllProps && (mesh.nFlags == 0x8000 || mesh.nFlags == 0x4000)) mesh.nFlags = 0x2000;
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
		file.write((char*)&mesh.nDamageAssocId, 4);
	}
	else {
		int numLODs = mesh.aModels.size();
		file.write((char*)&numLODs, 4);
		for (auto model : mesh.aModels) {
			file.write((char*)&model, 4);
		}
	}
}

void WriteBGMMeshToFile(std::ofstream& file, tCompactMesh& mesh) {
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

void ClampMeshUVs(aiMesh* mesh, int uvChannel) {
	if (mesh->HasTextureCoords(uvChannel)) {
		float uvMin[2] = {99999, 99999};
		float uvMax[2] = {-99999,-99999};
		float uvOffset[2] = {0, 0};

		for (int i = 0; i < mesh->mNumVertices; i++) {
			auto vert = mesh->mTextureCoords[uvChannel][i];
			uvMin[0] = std::min(vert.x, uvMin[0]);
			uvMin[1] = std::min(vert.y, uvMin[1]);
			uvMax[0] = std::max(vert.x, uvMax[0]);
			uvMax[1] = std::max(vert.y, uvMax[1]);
		}
		while (uvMax[0] + uvOffset[0] < 0) uvOffset[0] += 1;
		while (uvMax[1] + uvOffset[1] < 0) uvOffset[1] += 1;
		while (uvMin[0] + uvOffset[0] > 1) uvOffset[0] -= 1;
		while (uvMin[1] + uvOffset[1] > 1) uvOffset[1] -= 1;
		for (int i = 0; i < mesh->mNumVertices; i++) {
			auto vert = mesh->mTextureCoords[uvChannel][i];
			vert.x += uvOffset[0];
			vert.y += uvOffset[1];
		}
		if (uvOffset[0] != 0) {
			WriteConsole("Moving UV of " + (std::string)mesh->mName.C_Str() + " by " + std::to_string(uvOffset[0]), LOG_ALL);
		}
		if (uvOffset[1] != 0) {
			WriteConsole("Moving UV of " + (std::string)mesh->mName.C_Str() + " by " + std::to_string(uvOffset[1]), LOG_ALL);
		}
	}
}

void FBXNormalsToFOUCNormals(const aiVector3D* fbx, uint8_t* fouc, bool treeHack) {
	auto normals = *fbx;
	if (normals[0] > 1.0) normals[0] = 1.0;
	if (normals[1] > 1.0) normals[1] = 1.0;
	if (normals[2] > 1.0) normals[2] = 1.0;
	if (normals[0] < -1.0) normals[0] = -1.0;
	if (normals[1] < -1.0) normals[1] = -1.0;
	if (normals[2] < -1.0) normals[2] = -1.0;
	if (treeHack) {
		normals.x = 0;
		normals.y = 1;
		normals.z = 0;
	}

	double tmp = (-normals.z + 1) * 127.0;
	fouc[0] = tmp;
	tmp = (normals.y + 1) * 127.0;
	fouc[1] = tmp;
	tmp = (normals.x + 1) * 127.0;
	fouc[2] = tmp;
	fouc[3] = 0xFF;
}

aiMatrix4x4* pStaticTransformationMatrix = nullptr;
aiMatrix4x4* pStaticRotationMatrix = nullptr;

bool bTmpModelsDoubleSided = false;
void CreateStreamsFromFBX(aiMesh* mesh, uint32_t flags, uint32_t vertexSize, float foucOffset1 = 0, float foucOffset2 = 0, float foucOffset3 = 0, float foucOffset4 = fFOUCBGMScaleMultiplier, bool treeHack = false) {
	int id = aVertexBuffers.size() + aIndexBuffers.size();

	if (bNoTreeHack) treeHack = false;

	if ((flags & VERTEX_UV2) != 0 && !mesh->HasTextureCoords(1)) {
		WriteConsole("WARNING: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have 2 sets of UVs!", LOG_MINOR_WARNINGS);
		//WaitAndExitOnFail();
	}

	float foucOffsets[] = {foucOffset1, foucOffset2, foucOffset3, foucOffset4};

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
		WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " has more than 65535 vertices! Split the mesh!", LOG_ERRORS);
		WaitAndExitOnFail();
	}
	if (bIsFOUCModel) {
		// adjust UV coords to fit into the 2048 grid
		ClampMeshUVs(mesh, 0);
		ClampMeshUVs(mesh, 1);

		vBuf.flags |= VERTEX_INT16;
		vBuf.data = new float[mesh->mNumVertices * (vertexSize / 4)];
		memset(vBuf.data, 0, mesh->mNumVertices * (vertexSize / 4) * sizeof(float));
		auto vertexData = vBuf.data;
		for (int i = 0; i < mesh->mNumVertices; i++) {
			auto data = (tVertexDataFOUC*)vertexData;
			auto srcVerts = mesh->mVertices[i];
			if (pStaticTransformationMatrix) srcVerts *= *pStaticTransformationMatrix;

			srcVerts /= foucOffsets[3];
			srcVerts.x -= foucOffsets[0];
			srcVerts.y -= foucOffsets[1];
			srcVerts.z += foucOffsets[2];
			data->vPos[0] = srcVerts.x;
			data->vPos[1] = srcVerts.y;
			data->vPos[2] = -srcVerts.z;

			data->nUnk32 = 0x0400;
			data->vTangents[0] = 0x00;
			data->vTangents[1] = 0x00;
			data->vTangents[2] = 0x00;
			data->vTangents[3] = 0xFF;
			data->vBitangents[0] = 0x00;
			data->vBitangents[1] = 0x00;
			data->vBitangents[2] = 0x00;
			data->vBitangents[3] = 0xFF;
			data->vNormals[0] = 0x00;
			data->vNormals[1] = 0x00;
			data->vNormals[2] = 0x00;
			data->vNormals[3] = 0xFF;

			// normals
			if (mesh->HasNormals()) {
				if (!mesh->HasTangentsAndBitangents()) {
					WriteConsole("WARNING: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have tangents!", LOG_ERRORS);
					//WaitAndExitOnFail();

					auto normal = mesh->mNormals[i];
					FBXNormalsToFOUCNormals(&normal, data->vTangents, false);
					FBXNormalsToFOUCNormals(&normal, data->vBitangents, false);
					FBXNormalsToFOUCNormals(&normal, data->vNormals, treeHack);
				}
				else {
					auto tangent = mesh->mTangents[i];
					auto bitangent = mesh->mBitangents[i];
					auto normal = mesh->mNormals[i];
					if (pStaticRotationMatrix) {
						tangent *= *pStaticRotationMatrix;
						bitangent *= *pStaticRotationMatrix;
						normal *= *pStaticRotationMatrix;
					}

					FBXNormalsToFOUCNormals(&tangent, data->vTangents, false);
					FBXNormalsToFOUCNormals(&bitangent, data->vBitangents, false);
					FBXNormalsToFOUCNormals(&normal, data->vNormals, treeHack);
				}
			}
			else {
				WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have normals!", LOG_ERRORS);
				WaitAndExitOnFail();
			}
			// vertex color
			if (!bNoVertexColors && mesh->HasVertexColors(0)) {
				uint8_t tmp[4] = {0, 0, 0, 0xFF};
				tmp[0] = mesh->mColors[0][i].b * 255.0;
				tmp[1] = mesh->mColors[0][i].g * 255.0;
				tmp[2] = mesh->mColors[0][i].r * 255.0;
				tmp[3] = mesh->mColors[0][i].a * 255.0;
				*(uint32_t*)data->vVertexColors = *(uint32_t*)tmp;
			}
			else {
				*(uint32_t*)data->vVertexColors = 0xFFFFFFFF;
			}
			// UV1
			if (mesh->HasTextureCoords(0)) {
				data->vUV1[0] = mesh->mTextureCoords[0][i].x * 2048.0;
				data->vUV1[1] = 1 - mesh->mTextureCoords[0][i].y * 2048.0;
			}
			else {
				WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have UVs!", LOG_ERRORS);
				WaitAndExitOnFail();
			}
			// UV2
			if (mesh->HasTextureCoords(1)) {
				data->vUV2[0] = mesh->mTextureCoords[1][i].x * 2048.0;
				data->vUV2[1] = 1 - mesh->mTextureCoords[1][i].y * 2048.0;
			}
			else {
				data->vUV2[0] = 0;
				data->vUV2[1] = 0;
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
			auto srcVerts = mesh->mVertices[i];
			if (pStaticTransformationMatrix) srcVerts *= *pStaticTransformationMatrix;

			vertices[0] = srcVerts.x;
			vertices[1] = srcVerts.y;
			vertices[2] = -srcVerts.z;
			vertices += 3;

			if ((flags & VERTEX_NORMAL) != 0) {
				if (!mesh->HasNormals()) {
					WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have normals!", LOG_ERRORS);
					WaitAndExitOnFail();
				}

				auto normals = mesh->mNormals[i];
				if (pStaticRotationMatrix) normals *= *pStaticRotationMatrix;

				if (normals[0] > 1.0) normals[0] = 1.0;
				if (normals[1] > 1.0) normals[1] = 1.0;
				if (normals[2] > 1.0) normals[2] = 1.0;
				if (normals[0] < -1.0) normals[0] = -1.0;
				if (normals[1] < -1.0) normals[1] = -1.0;
				if (normals[2] < -1.0) normals[2] = -1.0;

				vertices[0] = normals.x;
				vertices[1] = normals.y;
				vertices[2] = -normals.z;
				vertices += 3; // 3 floats
			}
			if ((flags & VERTEX_COLOR) != 0) {
				if (!bNoVertexColors && mesh->HasVertexColors(0)) {
					uint8_t tmp[4] = {0, 0, 0, 0xFF};
					tmp[0] = mesh->mColors[0][i].r * 255.0;
					tmp[1] = mesh->mColors[0][i].g * 255.0;
					tmp[2] = mesh->mColors[0][i].b * 255.0;
					if (bIsBGMModel) tmp[3] = mesh->mColors[0][i].a * 255.0;
					else tmp[3] = 255; // terrain vertex color uses a lookup table if it's not 255, this would be BAD
					*(uint32_t*)&vertices[0] = *(uint32_t*)tmp;
				}
				else {
					*(uint32_t*)&vertices[0] = 0xFFFFFFFF;
				}
				vertices += 1; // 1 int32
			}
			if ((flags & VERTEX_UV) != 0 || (flags & VERTEX_UV2) != 0) {
				if (!mesh->HasTextureCoords(0)) {
					WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have UVs!", LOG_ERRORS);
					WaitAndExitOnFail();
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

	tIndexBuffer iBuf;
	iBuf.id = id + 1;
	iBuf.indexCount = mesh->mNumFaces * 3;
	if (bTmpModelsDoubleSided || bMakeAllDoubleSided) iBuf.indexCount *= 2;
	iBuf.data = new uint16_t[iBuf.indexCount];
	auto indexData = iBuf.data;
	for (int i = 0; i < mesh->mNumFaces; i++) {
		auto& face = mesh->mFaces[i];
		if (face.mNumIndices != 3) {
			WriteConsole("ERROR: Non-tri found in FBX mesh while exporting!", LOG_ERRORS);
			continue;
		}
		indexData[0] = face.mIndices[2];
		indexData[1] = face.mIndices[1];
		indexData[2] = face.mIndices[0];
		indexData += 3;
		if (bTmpModelsDoubleSided || bMakeAllDoubleSided) {
			indexData[0] = face.mIndices[0];
			indexData[1] = face.mIndices[1];
			indexData[2] = face.mIndices[2];
			indexData += 3;
		}
	}
	aVertexBuffers.push_back(vBuf);
	aIndexBuffers.push_back(iBuf);
}

void FixNameExtensions(std::string& name) {
	for (int i = 0; i < 999; i++) {
		if (name.ends_with(std::format(".{:03}", i))) {
			for (int j = 0; j < 4; j++) {
				name.pop_back();
			}
			return;
		}
	}
}

struct tShaderConfig {
	struct tShaderMatchup {
		std::string name;
		int shaderId = -1;
		bool forceAlphaOn = false;
		bool forceAlphaOff = false;
		bool treeHack = false;
		bool isWater = false;
		bool isTire = false;
		bool isRim = false;
		bool isMallFloor = false;

		void ApplyToMaterial(tMaterial* mat) {
			if (isWater) {
				// water is a window in fo1/fo2, lol
				if (bIsFOUCModel) {
					mat->sTextureNames[0] = "puddle_normal.tga";
					mat->nShaderId = 45; // puddle
					mat->nAlpha = 1;
				}
				else {
					mat->sTextureNames[0] = "alpha_windowshader.tga";
					mat->nShaderId = 34; // reflecting window shader (static)
					mat->nAlpha = 1;
				}
			}
			if (isTire) {
				mat->nShaderId = bIsFOUCModel ? 44 : 7; // fo2: car diffuse, uc: car tire
				if (bCarForceTireAlpha) mat->nAlpha = 1;
			}
			if (isRim) {
				mat->nShaderId = 9; // fo2: car tire, uc: car tire rim
				if (!bIsFOUCModel) mat->nAlpha = 1;
				if (bCarNoRimAlpha) mat->nAlpha = 0;
				if (bCarForceRimAlpha) mat->nAlpha = 1;
			}
			if (bIsFOUCModel && isMallFloor) mat->nShaderId = 49; // lightmapped planar reflection
			if (treeHack) {
				mat->nAlpha = 1;
				mat->_bIsCustomFOUCTree = true;
			}
			if (shaderId >= 0) {
				mat->nShaderId = shaderId;
				// car lights
				if (shaderId == 10) {
					mat->v92 = 2;
				}
			}
			if (forceAlphaOn) mat->nAlpha = 1;
			if (forceAlphaOff) mat->nAlpha = 0;
		}
	};
	std::vector<tShaderMatchup> name;
	std::vector<tShaderMatchup> name_prefix;
	std::vector<tShaderMatchup> name_suffix;
	std::vector<tShaderMatchup> texture;
	std::vector<tShaderMatchup> texture_prefix;
	std::vector<tShaderMatchup> texture_suffix;

	void ApplyToMaterial(tMaterial* mat) {
		auto nameLower = mat->sName;
		auto texLower = mat->sTextureNames[0];
		std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](unsigned char c){ return std::tolower(c); });
		std::transform(texLower.begin(), texLower.end(), texLower.begin(), [](unsigned char c){ return std::tolower(c); });
		for (auto& matchup : name) {
			if (nameLower == matchup.name) {
				matchup.ApplyToMaterial(mat);
			}
		}
		for (auto& matchup : name_prefix) {
			if (nameLower.starts_with(matchup.name)) {
				matchup.ApplyToMaterial(mat);
			}
		}
		for (auto& matchup : name_suffix) {
			if (nameLower.ends_with(matchup.name)) {
				matchup.ApplyToMaterial(mat);
			}
		}
		for (auto& matchup : texture) {
			if (texLower == matchup.name) {
				matchup.ApplyToMaterial(mat);
			}
		}
		for (auto& matchup : texture_prefix) {
			if (texLower.starts_with(matchup.name)) {
				matchup.ApplyToMaterial(mat);
			}
		}
		for (auto& matchup : texture_suffix) {
			if (texLower.ends_with(matchup.name)) {
				matchup.ApplyToMaterial(mat);
			}
		}
	}
};
tShaderConfig gBGMShaders, gW32StaticShaders, gW32DynamicShaders;

void InitShaderConfig() {
	static bool bInited = false;
	if (bInited) return;
	bInited = true;

	tShaderConfig* config = nullptr;
	std::vector<tShaderConfig::tShaderMatchup>* matchups = nullptr;

	auto input = std::ifstream("shaders.txt");
	if (!input.is_open()) return;
	for (std::string line; std::getline(input, line);) {
		if (line.starts_with("!!BGM!!")) { config = &gBGMShaders; continue; }
		if (line.starts_with("!!W32 STATIC!!")) { config = &gW32StaticShaders; continue; }
		if (line.starts_with("!!W32 DYNAMIC!!")) { config = &gW32DynamicShaders; continue; }

		if (!config) continue;
		if (line.starts_with("[name]")) { matchups = &config->name; continue; }
		if (line.starts_with("[name_prefix]")) { matchups = &config->name_prefix; continue; }
		if (line.starts_with("[name_suffix]")) { matchups = &config->name_suffix; continue; }
		if (line.starts_with("[texture]")) { matchups = &config->texture; continue; }
		if (line.starts_with("[texture_prefix]")) { matchups = &config->texture_prefix; continue; }
		if (line.starts_with("[texture_suffix]")) { matchups = &config->texture_suffix; continue; }
		if (!matchups) continue;
		if (line.starts_with("#")) continue;

		auto equals = line.find('=');
		if (equals == std::string::npos) continue;

		auto name = line.substr(0, equals);
		std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });
		tShaderConfig::tShaderMatchup* matchup = nullptr;
		for (auto& arr : *matchups) {
			if (arr.name == name) {
				matchup = &arr;
				break;
			}
		}
		if (!matchup) {
			matchups->push_back(tShaderConfig::tShaderMatchup());
			matchup = &(*matchups)[matchups->size()-1];
			matchup->name = name;
		}

		auto data = line.substr(equals+1);
		if (data == "FORCEALPHA") matchup->forceAlphaOn = true;
		else if (data == "FORCENOALPHA") matchup->forceAlphaOff = true;
		else if (data == "TREEHACK") matchup->treeHack = true;
		else if (data == "WATER") matchup->isWater = true;
		else if (data == "TIRE") matchup->isTire = true;
		else if (data == "RIM") matchup->isRim = true;
		else if (data == "MALLFLOOR") matchup->isMallFloor = true;
		else matchup->shaderId = std::stoi(data);
	}
}

struct tTextureMatchupConfig {
	std::string materialName;
	std::string textureName;
};
std::vector<tTextureMatchupConfig> aTextureMatchups;

void InitTextureMatchupConfig() {
	static bool bInited = false;
	if (bInited) return;
	bInited = true;

	auto input = std::ifstream("textures.txt");
	if (!input.is_open()) return;
	for (std::string line; std::getline(input, line);) {
		if (line.starts_with("#")) continue;

		auto equals = line.find('=');
		if (equals == std::string::npos) continue;

		auto name = line.substr(0, equals);
		tTextureMatchupConfig matchup;
		matchup.materialName = name;
		if (matchup.materialName.empty()) continue;
		matchup.textureName = line.substr(equals+1);
		if (matchup.textureName.empty()) continue;
		std::transform(matchup.materialName.begin(), matchup.materialName.end(), matchup.materialName.begin(), [](unsigned char c){ return std::tolower(c); });
		std::transform(matchup.textureName.begin(), matchup.textureName.end(), matchup.textureName.begin(), [](unsigned char c){ return std::tolower(c); });
		aTextureMatchups.push_back(matchup);
	}
}

struct tTextureReplacementConfig {
	std::string textureNameIn;
	std::string textureNameOut;
};
std::vector<tTextureReplacementConfig> aTextureReplacements;

void InitTextureReplacementConfig() {
	static bool bInited = false;
	if (bInited) return;
	bInited = true;

	auto input = std::ifstream("textures_replace.txt");
	if (!input.is_open()) return;
	for (std::string line; std::getline(input, line);) {
		if (line.starts_with("#")) continue;

		auto equals = line.find('=');
		if (equals == std::string::npos) continue;

		auto name = line.substr(0, equals);
		tTextureReplacementConfig matchup;
		matchup.textureNameIn = name;
		if (matchup.textureNameIn.empty()) continue;
		matchup.textureNameOut = line.substr(equals+1);
		if (matchup.textureNameOut.empty()) continue;
		std::transform(matchup.textureNameIn.begin(), matchup.textureNameIn.end(), matchup.textureNameIn.begin(), [](unsigned char c){ return std::tolower(c); });
		std::transform(matchup.textureNameOut.begin(), matchup.textureNameOut.end(), matchup.textureNameOut.begin(), [](unsigned char c){ return std::tolower(c); });
		aTextureReplacements.push_back(matchup);
	}
}

bool hasEnding(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

void FixupFBXCarMaterial(tMaterial& mat) {
	InitShaderConfig();
	InitTextureMatchupConfig();
	InitTextureReplacementConfig();

	mat.nShaderId = 8; // car metal
	FixNameExtensions(mat.sName);

	if (bRallyTrophyShaderFixup) {
		if (mat.sTextureNames[0] == "body.tga" || mat.sTextureNames[0] == "Body.tga") {
			mat.nShaderId = 5; // car body
			mat.sName = "body";
		}
		if (mat.sTextureNames[0] == "interior_lo.tga") mat.sTextureNames[0] = "interior.tga";
		if (mat.sName == "collision" || mat.sName == "Collision" || mat.sTextureNames[0].starts_with("null")) mat.nShaderId = 13; // shadow project
		if (mat.sName == "#E1 window front") mat.sName = "window_front";
		if (mat.sName == "#E1 window leftside") mat.sName = "window_left";
		if (mat.sName == "#E1 window rightside") mat.sName = "window_right";
		if (mat.sName.starts_with("tire alpha")) mat.sTextureNames[0] = bIsFOUCModel ? "rim.tga" : "tire_01.tga";
		else if (mat.sName.starts_with("tire")) mat.sTextureNames[0] = bIsFOUCModel ? "tire.tga" : "tire_01.tga";

		if (mat.sName.ends_with(" alpha")) mat.nAlpha = 1;
	}

	if (bRetroDemoCarFixup) {
		if (mat.sName.starts_with("tire") || mat.sName.starts_with("tyre")) mat.sTextureNames[0] = bIsFOUCModel ? "tire.tga" : "tire_01.tga";
		if (mat.sTextureNames[0] == "interior_lo.tga" || mat.sTextureNames[0] == "Interior_Lo.tga") mat.sTextureNames[0] = "interior.tga";
	}

	// shadow project has no texture
	if (mat.sName.starts_with("shadow")) mat.sTextureNames[0] = "";
	// body always uses skin1.tga
	if (mat.sName.starts_with("body")) mat.sTextureNames[0] = "skin1.tga";
	// fouc tire_01 hack
	if (bIsFOUCModel && (mat.sTextureNames[0].starts_with("tire_0") || mat.sTextureNames[0].starts_with("tire_1"))) mat.sTextureNames[0] = "tire.tga";

	if (mat.sTextureNames[0].empty()) {
		auto name = mat.sName;
		if (hasEnding(name, ".tga") || hasEnding(name, ".png") || hasEnding(name, ".dds") || hasEnding(name, ".bmp")) {
			name.pop_back();
			name.pop_back();
			name.pop_back();
			name.pop_back();
		}
		mat.sTextureNames[0] = name + ".tga";
	}

	auto matNameLower = mat.sName;
	std::transform(matNameLower.begin(), matNameLower.end(), matNameLower.begin(), [](unsigned char c){ return std::tolower(c); });
	for (auto& matchup : aTextureMatchups) {
		if (matNameLower == matchup.materialName) mat.sTextureNames[0] = matchup.textureName;
	}

	auto texNameLower = mat.sTextureNames[0];
	std::transform(texNameLower.begin(), texNameLower.end(), texNameLower.begin(), [](unsigned char c){ return std::tolower(c); });
	for (auto& matchup : aTextureReplacements) {
		if (texNameLower == matchup.textureNameIn) mat.sTextureNames[0] = matchup.textureNameOut;
	}

	gBGMShaders.ApplyToMaterial(&mat);

	// custom alpha suffix
	if (mat.sName.ends_with("_alpha")) mat.nAlpha = 1;
	if (mat.sName.ends_with("_noalpha")) mat.nAlpha = 0;
}

void FixupFBXMapMaterial(tMaterial& mat, bool isStaticModel, bool disallowTrees) {
	InitShaderConfig();
	InitTextureMatchupConfig();
	InitTextureReplacementConfig();

	if (bRallyTrophyShaderFixup) {
		mat.nAlpha = 0;
		mat.nShaderId = 0; // static prelit
		if (!isStaticModel) {
			mat.nShaderId = 3; // dynamic diffuse
		}

		FixNameExtensions(mat.sName);

		if (mat.sTextureNames[0].starts_with("sweden_audience")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("kenya_audience")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("kenya_roundhut_pale")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("audience_")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("fence_")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("fin_fence")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("village_fence")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("power_line")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("sign_railroad")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("sign_trunks_reverse")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("skidmark")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("string_")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("fin_powerline")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("fin_pole_wires")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("fin_string")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("indicators")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("rope")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("streetlight_glass")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].starts_with("waterfall")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].ends_with("_alpha.tga")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].ends_with("_wire.tga")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].ends_with("_alpha.TGA")) mat.nAlpha = 1;
		if (mat.sTextureNames[0].ends_with("_wire.TGA")) mat.nAlpha = 1;

		const char* aTreeTextures[] = {
			"russian_tree_plane",
			"kenya_tree_plane",
			"mountain_tree_plane",
			"sweden_tree_plane",
			"felling_tree",
			"fin_trees",
			"fin_forest",
			"fin_roadside_bush",
			"plane_bush",
			"forest_mixed_",
			"forest_snow_",
			"grass",
			"bush_",
		};
		for (auto& tex : aTreeTextures) {
			if (mat.sTextureNames[0].starts_with(tex)) {
				mat.nAlpha = 1;
				mat._bIsCustomFOUCTree = true;
			}
		}

		if (mat.sTextureNames[0].starts_with("tree_") && mat.sTextureNames[0].find("trunk") == std::string::npos) {
			mat.nAlpha = 1;
			mat._bIsCustomFOUCTree = true;
		}
		if (mat.sTextureNames[0].starts_with("kenya_tree") && mat.sTextureNames[0].find("trunk") == std::string::npos) {
			mat.nAlpha = 1;
			mat._bIsCustomFOUCTree = true;
		}
		// should plane2_bush be here?

		if (mat.sName.starts_with("river") || mat.sName.starts_with("Water")) {
			if (bIsFOUCModel) {
				mat.sTextureNames[0] = "puddle_normal.tga";
				mat.nShaderId = 45; // puddle
				mat.nAlpha = 1;
			}
			else {
				mat.sTextureNames[0] = "alpha_windowshader.tga";
				mat.nShaderId = 34; // reflecting window shader (static)
				mat.nAlpha = 1;
			}
		}

		return;
	}

	mat.nAlpha = mat.sName.starts_with("alpha") || mat.sName.starts_with("Alpha");
	if (isStaticModel) {
		mat.nShaderId = 0; // static prelit
		FixNameExtensions(mat.sName);
	}
	else {
		mat.nShaderId = 3; // dynamic diffuse
	}
	FixNameExtensions(mat.sName);

	if (mat.sTextureNames[0] == "colormap.tga" || mat.sTextureNames[0] == "Colormap.tga") {
		mat.nUseColormap = 1;
		if (mat.nShaderId == 0) mat.nShaderId = 1; // terrain shader

		// hack to load colormapped textures properly when the fbx has texture2 stripped
		if (mat.sTextureNames[1].empty()) {
			mat.sTextureNames[1] = mat.sName + ".tga";
			mat.nNumTextures = 2;
		}
	}

	if (mat.sTextureNames[0].starts_with("alpha") || mat.sTextureNames[0].starts_with("Alpha")) mat.nAlpha = 1;
	if (bToughTrucksStadiumScreen && mat.sTextureNames[0] == "stadion_screen.tga") {
		mat.sName = isStaticModel ? "screenmaterial_static" : "screenmaterial";
		mat.nShaderId = isStaticModel ? 40 : 41; // static nonlit : dynamic nonlit
	}

	if (mat.sName.starts_with("treehack_")) {
		mat.nAlpha = 1;
		mat._bIsCustomFOUCTree = true;
	}

	if (mat.sTextureNames[0].empty()) {
		auto name = mat.sName;
		if (hasEnding(name, ".tga") || hasEnding(name, ".png") || hasEnding(name, ".dds") || hasEnding(name, ".bmp")) {
			name.pop_back();
			name.pop_back();
			name.pop_back();
			name.pop_back();
		}
		mat.sTextureNames[0] = name + ".tga";
	}
	
	if (mat.sTextureNames[0] == "null.tga") {
		mat.sTextureNames[0] = "";
		mat.nShaderId = 34; // reflecting window shader (static)
	}

	auto matNameLower = mat.sName;
	std::transform(matNameLower.begin(), matNameLower.end(), matNameLower.begin(), [](unsigned char c){ return std::tolower(c); });
	for (auto& matchup : aTextureMatchups) {
		if (matNameLower == matchup.materialName) mat.sTextureNames[0] = matchup.textureName;
	}

	auto texNameLower = mat.sTextureNames[0];
	std::transform(texNameLower.begin(), texNameLower.end(), texNameLower.begin(), [](unsigned char c){ return std::tolower(c); });
	for (auto& matchup : aTextureReplacements) {
		if (texNameLower == matchup.textureNameIn) mat.sTextureNames[0] = matchup.textureNameOut;
	}

	if (isStaticModel) {
		gW32StaticShaders.ApplyToMaterial(&mat);
	}
	else {
		gW32DynamicShaders.ApplyToMaterial(&mat);
	}

	// Trees outside of TreeMesh don't draw, replace their shaders to get around this
	if (disallowTrees) {
		if (mat.nShaderId == 19 || mat.nShaderId == 20 || mat.nShaderId == 21) {
			if (mat.nShaderId != 19) mat._bIsCustomFOUCTree = true;
			mat.nShaderId = 0;
		}
	}

	// terrain -> static prelit
	if (bNeverUseTerrainShader && (mat.nShaderId == 1 || mat.nShaderId == 2)) {
		mat.nShaderId = 0;
	}

	if (mat.sName.ends_with("_alpha")) mat.nAlpha = 1;
	if (mat.sName.ends_with("_noalpha")) mat.nAlpha = 0;
}

tMaterial GetMaterialFromFBX(aiMaterial* fbxMaterial) {
	tMaterial mat;
	mat.sName = mat._sFBXName = fbxMaterial->GetName().C_Str();
	mat.nNumTextures = fbxMaterial->GetTextureCount(aiTextureType_DIFFUSE);
	if (mat.nNumTextures > 3) mat.nNumTextures = 3;
	for (int i = 0; i < mat.nNumTextures; i++) {
		auto texName= GetFBXTextureInFO2Style(fbxMaterial, i);
		mat.sTextureNames[i] = texName;
	}
	return mat;
}

tMaterial GetCarMaterialFromFBX(aiMaterial* fbxMaterial) {
	auto mat = GetMaterialFromFBX(fbxMaterial);
	FixupFBXCarMaterial(mat);
	WriteConsole("Creating new material " + mat.sName + " with shader " + GetShaderName(mat.nShaderId, nExportFileVersion), LOG_ALL);
	return mat;
}

tMaterial GetMapMaterialFromFBX(aiMaterial* fbxMaterial, bool isStaticModel, bool disallowTrees) {
	auto mat = GetMaterialFromFBX(fbxMaterial);
	FixupFBXMapMaterial(mat, isStaticModel, disallowTrees);
	WriteConsole("Creating new material " + mat.sName + " with shader " + GetShaderName(mat.nShaderId, nExportFileVersion), LOG_ALL);
	return mat;
}

int GetMaterialSortPriority(aiMaterial* mat) {
	auto name = (std::string)mat->GetName().C_Str();
	FixNameExtensions(name);

	static auto config = toml::parse_file("FlatOutW32BGMTool_gcp.toml");
	return config["material_priorities"][name].value_or(0);
}

int GetBGMMaterialID(const std::string& name, const std::string& path) {
	for (auto& material : aMaterials) {
		if (material._sFBXName == name) return &material - &aMaterials[0];
	}
	// fallback for name only
	for (auto& material : aMaterials) {
		if (material.sName == name) return &material - &aMaterials[0];
	}
	// fallback for menucar material combining
	for (auto& material : aMaterials) {
		if (material.sTextureNames[0] == path) return &material - &aMaterials[0];
	}
	WriteConsole(std::format("WARNING: Failed to find material {}", name), LOG_WARNINGS);
	return 0;
}

int GetBGMMaterialID(aiMaterial* material) {
	auto mat = GetMaterialFromFBX(material);
	FixupFBXCarMaterial(mat);

	return GetBGMMaterialID(mat.sName, mat.sTextureNames[0]);
}

void CreateBGMSurfaceFromFBX(aiNode* node, int meshId, bool isCrash) {
	auto mesh = pParsedFBXScene->mMeshes[node->mMeshes[meshId]];
	int fbxMeshId = node->mMeshes[meshId];

	tSurface surface;
	surface.nIsVegetation = 0;
	surface.nVertexCount = mesh->mNumVertices;
	surface.nPolyCount = mesh->mNumFaces;
	surface.nNumIndicesUsed = mesh->mNumFaces * 3;
	if (bMakeAllDoubleSided) {
		surface.nPolyCount *= 2;
		surface.nNumIndicesUsed *= 2;
	}
	surface.nFlags = aVertexBuffers[node->mMeshes[meshId]].flags;
	surface.nMaterialId = GetBGMMaterialID(pParsedFBXScene->mMaterials[mesh->mMaterialIndex]);
	surface.nNumStreamsUsed = 2;
	surface.nPolyMode = 4;
	surface.nStreamId[0] = fbxMeshId * 2;
	surface.nStreamId[1] = (fbxMeshId * 2) + 1;
	surface.nStreamOffset[0] = 0;
	surface.nStreamOffset[1] = 0;

	surface.foucVertexMultiplier[0] = 0;
	surface.foucVertexMultiplier[1] = 0;
	surface.foucVertexMultiplier[2] = 0;
	surface.foucVertexMultiplier[3] = fFOUCBGMScaleMultiplier;

	if (isCrash) aCrashSurfaces.push_back(surface);
	else aSurfaces.push_back(surface);
}

bool HasSimilarMaterialForMenuCar(int id) {
	auto mat = GetMaterialFromFBX(pParsedFBXScene->mMaterials[id]);
	FixupFBXCarMaterial(mat);

	// find material with same texture
	for (auto& material : aMaterials) {
		if (material.nAlpha == mat.nAlpha && material.sTextureNames[0] == mat.sTextureNames[0]) {
			return true;
		}
	}
	return false;
}

void FillBGMFromFBX() {
	WriteConsole("Creating BGM data...", LOG_ALWAYS);

	WriteConsole("Creating materials...", LOG_ALWAYS);

	// create materials
	if (bNoMaterialPriorities) {
		for (int i = 0; i < pParsedFBXScene->mNumMaterials; i++) {
			auto mat = pParsedFBXScene->mMaterials[i];
			if (bMenuCarCombineMaterials && HasSimilarMaterialForMenuCar(i)) continue;
			aMaterials.push_back(GetCarMaterialFromFBX(pParsedFBXScene->mMaterials[i]));
		}
	}
	else {
		for (int j = 0; j < 256; j++) {
			for (int i = 0; i < pParsedFBXScene->mNumMaterials; i++) {
				auto mat = pParsedFBXScene->mMaterials[i];
				if (GetMaterialSortPriority(mat) != j) continue;
				if (bMenuCarCombineMaterials && HasSimilarMaterialForMenuCar(i)) continue;
				aMaterials.push_back(GetCarMaterialFromFBX(pParsedFBXScene->mMaterials[i]));
			}
		}
	}

	if (bAddLightMaterials) {
		const char* aLightMaterials[] = {
			"light_front_l",
			"light_front_r",
			"light_brake_l",
			"light_brake_r",
			"light_reverse_l",
			"light_reverse_r",
		};

		for (auto& name : aLightMaterials) {
			tMaterial mat;
			mat.sName = mat._sFBXName = name;
			mat.nNumTextures = 1;
			mat.sTextureNames[0] = "lights.tga";
			mat.v92 = 2;
			mat.nShaderId = 10; // car lights
			mat.nAlpha = 1;
			aMaterials.push_back(mat);
		}
	}

	if (bMenuCarCombineMaterials && aMaterials.size() > 16) {
		WriteConsole("ERROR: Failed to combine enough materials to fit within the menucar limit! Mesh has " + std::to_string(aMaterials.size()) + " materials, max is 16", LOG_ERRORS);
		WaitAndExitOnFail();
	}

	WriteConsole("Creating streams...", LOG_ALWAYS);

	// create streams
	for (int i = 0; i < pParsedFBXScene->mNumMeshes; i++) {
		auto src = pParsedFBXScene->mMeshes[i];
		auto material = &aMaterials[GetBGMMaterialID(pParsedFBXScene->mMaterials[src->mMaterialIndex])];
		if (bIsFOUCModel) {
			CreateStreamsFromFBX(src, VERTEX_POSITION | VERTEX_COLOR | VERTEX_UV2, 36);
		}
		else if (material->nShaderId == 5 || material->nShaderId == 26) { // car body or ragdoll skin, has vertex colors
			CreateStreamsFromFBX(src, VERTEX_POSITION | VERTEX_NORMAL | VERTEX_COLOR | VERTEX_UV, 36);
		}
		else if (material->nShaderId == 13) { // shadow project, position only
			CreateStreamsFromFBX(src, VERTEX_POSITION, 12);
		}
		else { // regular meshes, only normal and uv
			CreateStreamsFromFBX(src, VERTEX_POSITION | VERTEX_NORMAL | VERTEX_UV, 32);
		}
	}

	WriteConsole("Creating meshes & surfaces...", LOG_ALWAYS);

	auto bgmMeshArray = GetFBXNodeForBGMMeshArray();
	for (int i = 0; i < bgmMeshArray->mNumChildren; i++) {
		auto bodyNode = bgmMeshArray->mChildren[i]; // body

		tCompactMesh bgmMesh;
		bgmMesh.sName1 = bodyNode->mName.C_Str();
		FixNameExtensions(bgmMesh.sName1);
		if (bgmMesh.sName1.ends_with("_crash")) continue;

		bgmMesh.nFlags = 0x0;
		bgmMesh.nGroup = -1;
		FBXMatrixToFO2Matrix(bodyNode->mTransformation, bgmMesh.mMatrix);

		tModel model;
		model.sName = bodyNode->mName.C_Str();
		FixNameExtensions(model.sName);

		float aabbMin[3] = {9999,9999,9999};
		float aabbMax[3] = {-9999,-9999,-9999};

		for (int j = -1; j < (int)bodyNode->mNumChildren; j++) {
			aiNode* body001 = nullptr;
			if (j < 0) body001 = bodyNode;
			else body001 = bodyNode->mChildren[j]; // body.001

			struct tmp {
				aiNode* node;
				int meshId;
			};
			std::vector<tmp> aMeshes;

			for (int k = 0; k < body001->mNumMeshes; k++) {
				aMeshes.push_back({body001, k});
			}
			if (j >= 0) {
				for (int k = 0; k < body001->mNumChildren; k++) {
					auto surface = body001->mChildren[k]; // Surface1
					for (int l = 0; l < surface->mNumMeshes; l++) {
						aMeshes.push_back({surface, l});
					}
				}
			}
			if (aMeshes.empty()) continue;

			for (int p = 0; p < 255; p++) {
				for (auto& data: aMeshes) {
					// Z inversion doesn't matter here - we're calculating a radius anyway
					auto mesh = pParsedFBXScene->mMeshes[data.node->mMeshes[data.meshId]];
					if (GetMaterialSortPriority(pParsedFBXScene->mMaterials[mesh->mMaterialIndex]) != p) continue;

					auto nodeName = (std::string) data.node->mName.C_Str();
					auto meshName = (std::string) mesh->mName.C_Str();
					if (nodeName.ends_with("_crash") || meshName.ends_with("_crash")) {
						model.aCrashSurfaces.push_back(aCrashSurfaces.size());
						CreateBGMSurfaceFromFBX(data.node, data.meshId, true);
						continue;
					}
					if (aabbMin[0] > mesh->mAABB.mMin.x) aabbMin[0] = mesh->mAABB.mMin.x;
					if (aabbMin[1] > mesh->mAABB.mMin.y) aabbMin[1] = mesh->mAABB.mMin.y;
					if (aabbMin[2] > mesh->mAABB.mMin.z) aabbMin[2] = mesh->mAABB.mMin.z;
					if (aabbMax[0] < mesh->mAABB.mMax.x) aabbMax[0] = mesh->mAABB.mMax.x;
					if (aabbMax[1] < mesh->mAABB.mMax.y) aabbMax[1] = mesh->mAABB.mMax.y;
					if (aabbMax[2] < mesh->mAABB.mMax.z) aabbMax[2] = mesh->mAABB.mMax.z;

					model.aSurfaces.push_back(aSurfaces.size());
					CreateBGMSurfaceFromFBX(data.node, data.meshId, false);
				}
			}
		}

		if (!model.aCrashSurfaces.empty() && model.aCrashSurfaces.size() != model.aSurfaces.size()) {
			WriteConsole("ERROR: " + model.sName + " has crash models but not for every mesh! Skipping", LOG_ERRORS);
			model.aCrashSurfaces.clear();
		}
		model.vCenter[0] = (aabbMax[0] + aabbMin[0]) * 0.5;
		model.vCenter[1] = (aabbMax[1] + aabbMin[1]) * 0.5;
		model.vCenter[2] = (aabbMax[2] + aabbMin[2]) * -0.5;
		model.vRadius[0] = std::abs(aabbMax[0] - aabbMin[0]) * 0.5;
		model.vRadius[1] = std::abs(aabbMax[1] - aabbMin[1]) * 0.5;
		model.vRadius[2] = std::abs(aabbMax[2] - aabbMin[2]) * 0.5;
		// fRadius isn't required, game doesn't read it
		bgmMesh.aModels.push_back(aModels.size());
		aModels.push_back(model);

		if (bRetroDemoCarFixup && bgmMesh.sName1.starts_with("engine")) {
			tObject obj;
			obj.sName1 = "engine_smoke_dummy";
			memcpy(obj.mMatrix, bgmMesh.mMatrix, sizeof(obj.mMatrix));
			obj.nFlags = 0xE0F9;
			aObjects.push_back(obj);
			obj.sName1 = "engine_fire_dummy";
			aObjects.push_back(obj);
		}

		if (bgmMesh.aModels.empty()) {
			WriteConsole(std::format("WARNING: BGMMesh {} is empty", bgmMesh.sName1), LOG_WARNINGS);
			continue;
		}
		aCompactMeshes.push_back(bgmMesh);
	}

	for (auto& mesh : aCompactMeshes) {
		auto crash = FindFBXNodeForBGMMesh(mesh.sName1 + "_crash");
		if (!crash) continue;

		auto model = &aModels[mesh.aModels[0]];

		int counter = 0;
		for (int j = -1; j < (int)crash->mNumChildren; j++) {
			aiNode* body001 = nullptr;
			if (j < 0) body001 = crash;
			else body001 = crash->mChildren[j]; // body.001
			auto name = (std::string)body001->mName.C_Str();
			FixNameExtensions(name);

			struct tmp {
				aiNode* node;
				int meshId;
			};
			std::vector<tmp> aMeshes;

			for (int k = 0; k < body001->mNumMeshes; k++) {
				aMeshes.push_back({body001, k});
			}
			if (j >= 0) {
				for (int k = 0; k < body001->mNumChildren; k++) {
					auto surface = body001->mChildren[k]; // Surface1
					for (int l = 0; l < surface->mNumMeshes; l++) {
						aMeshes.push_back({surface, l});
					}
				}
			}

			if (aMeshes.empty()) continue;
			//if (counter >= mesh.aModels.size()) continue;

			//auto model = &aModels[mesh.aModels[counter++]];

			for (int p = 0; p < 255; p++) {
				for (auto& data: aMeshes) {
					auto mesh = pParsedFBXScene->mMeshes[data.node->mMeshes[data.meshId]];
					if (GetMaterialSortPriority(pParsedFBXScene->mMaterials[mesh->mMaterialIndex]) != p) continue;

					model->aCrashSurfaces.push_back(aCrashSurfaces.size());
					CreateBGMSurfaceFromFBX(data.node, data.meshId, true);
				}
			}
		}

		if (!model->aCrashSurfaces.empty() && model->aCrashSurfaces.size() != model->aSurfaces.size()) {
			WriteConsole("ERROR: " + model->sName + " has crash models but not for every mesh! Skipping", LOG_ERRORS);
			model->aCrashSurfaces.clear();
		}
	}

	if (bMenuCarCombineMaterials && aSurfaces.size() > 16) {
		WriteConsole("WARNING: Menucar surface limit exceeded! Mesh has " + std::to_string(aSurfaces.size()) + " surfaces, max is 16", LOG_WARNINGS);
	}

	WriteConsole("Creating object dummies...", LOG_ALWAYS);

	auto objectsArray = GetFBXNodeForObjectsArray();
	for (int i = 0; i < objectsArray->mNumChildren; i++) {
		auto objectNode = objectsArray->mChildren[i];

		tObject object;
		object.sName1 = objectNode->mName.C_Str();
		object.nFlags = 0xE0F9;
		FBXMatrixToFO2Matrix(objectNode->mTransformation, object.mMatrix);
		aObjects.push_back(object);
	}

	WriteConsole("BGM data created", LOG_ALWAYS);
}

void ImportSurfaceFromFBX(tSurface* surface, aiNode* node, bool isStaticModel, aiMesh* mesh) {
	WriteConsole("Exporting " + (std::string)node->mName.C_Str() + " into surface " + std::to_string(surface - &aSurfaces[0]), LOG_ALL);

	auto fbxMaterial = pParsedFBXScene->mMaterials[mesh->mMaterialIndex];
	auto material = FindMaterialIDByName(fbxMaterial->GetName().C_Str(), bNoMaterialReuse);
	if (material < 0) {
		material = aMaterials.size();
		bool isTree = false;
		if (surface->_nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_3] > 0) isTree = true;
		if (surface->_nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_4] > 0) isTree = true;
		if (surface->_nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_5] > 0) isTree = true;
		auto newMat = GetMapMaterialFromFBX(fbxMaterial, isStaticModel, !isTree);
		newMat._bIsCustom = true;
		aMaterials.push_back(newMat);
	}
	WriteConsole("Assigning material " + aMaterials[material].sName + " to " + node->mName.C_Str(), LOG_ALL);
	surface->nMaterialId = material;

	uint32_t bufFlags;
	uint32_t vertexSize;
	if (bIsFOUCModel) {
		bufFlags = 0x2242;
		vertexSize = 32;
	}
	else {
		auto& mat = aMaterials[surface->nMaterialId];
		if (mat.nShaderId == 1) { // terrain
			// DoubleUVMap
			bufFlags = 0x202;
			vertexSize = 28;
		}
		else if (mat.nShaderId == 2) { // terrain specular
			// Normals + DoubleUVMap
			bufFlags = 0x212;
			vertexSize = 40;
		}
		else if (mat.nShaderId == 34 || mat.nShaderId == 36) { // reflecting window shader (static)
			// Normals + VertexColor + UVMap
			bufFlags = 0x152;
			vertexSize = 36;
		}
		else if (mat.nShaderId == 3 || mat.nShaderId == 4) { // dynamic diffuse, dynamic specular
			// Normals + UVMap
			bufFlags = 0x112;
			vertexSize = 32;
		}
		else {
			// VertexColor + UVMap
			bufFlags = 0x142;
			vertexSize = 24;
		}
	}

	NyaVec3 vCenter;
	NyaVec3 vRadius;
	vCenter[0] = (mesh->mAABB.mMax.x + mesh->mAABB.mMin.x) * 0.5;
	vCenter[1] = (mesh->mAABB.mMax.y + mesh->mAABB.mMin.y) * 0.5;
	vCenter[2] = (mesh->mAABB.mMax.z + mesh->mAABB.mMin.z) * -0.5;
	vRadius[0] = std::abs(mesh->mAABB.mMax.x - mesh->mAABB.mMin.x);
	vRadius[1] = std::abs(mesh->mAABB.mMax.y - mesh->mAABB.mMin.y);
	vRadius[2] = std::abs(mesh->mAABB.mMax.z - mesh->mAABB.mMin.z);
	if (pStaticTransformationMatrix) {
		vCenter.x += pStaticTransformationMatrix->a4;
		vCenter.y += pStaticTransformationMatrix->b4;
		vCenter.z -= pStaticTransformationMatrix->c4;
	}
	if (pStaticRotationMatrix) {
		aiVector3D rad = {vRadius[0], vRadius[1], vRadius[2]};
		rad *= *pStaticRotationMatrix;
		vRadius[0] = std::abs(rad[0]);
		vRadius[1] = std::abs(rad[1]);
		vRadius[2] = std::abs(rad[2]);
	}

	uint32_t streamCount = aVertexBuffers.size() + aIndexBuffers.size();
	if (bIsFOUCModel) {
		float fFOUCRadius = vRadius.length() / 32767;
		NyaVec3 vFOUCCenter = vCenter / fFOUCRadius;
		if (!isStaticModel) {
			fFOUCRadius = fFOUCBGMScaleMultiplier;
			vFOUCCenter = {0, 0, 0};
		}

		CreateStreamsFromFBX(mesh, bufFlags, vertexSize, vFOUCCenter.x, vFOUCCenter.y, vFOUCCenter.z, fFOUCRadius, aMaterials[surface->nMaterialId]._bIsCustomFOUCTree);
		surface->foucVertexMultiplier[0] = vFOUCCenter[0];
		surface->foucVertexMultiplier[1] = vFOUCCenter[1];
		surface->foucVertexMultiplier[2] = vFOUCCenter[2];
		surface->foucVertexMultiplier[3] = fFOUCRadius;
	}
	else {
		CreateStreamsFromFBX(mesh, bufFlags, vertexSize);
	}

	surface->vCenter[0] = vCenter[0];
	surface->vCenter[1] = vCenter[1];
	surface->vCenter[2] = vCenter[2];
	surface->vRadius[0] = vRadius[0] * 0.5;
	surface->vRadius[1] = vRadius[1] * 0.5;
	surface->vRadius[2] = vRadius[2] * 0.5;
	surface->nFlags = bufFlags;
	surface->nVertexCount = mesh->mNumVertices;
	surface->nPolyCount = mesh->mNumFaces;
	surface->nNumIndicesUsed = mesh->mNumFaces * 3;
	if (bTmpModelsDoubleSided || bMakeAllDoubleSided) {
		surface->nPolyCount *= 2;
		surface->nNumIndicesUsed *= 2;
	}
	surface->nPolyMode = 4;
	surface->nNumStreamsUsed = 2;
	surface->nStreamOffset[0] = surface->nStreamOffset[1] = 0;
	surface->nStreamId[0] = streamCount;
	surface->nStreamId[1] = streamCount + 1;
	surface->_bIsReplacedMapSurface = true;
	surface->_pReplacedMapSurfaceMesh = mesh;

	// update bvh for associated nodes
	for (auto& batch : aStaticBatches) {
		if (batch.nBVHId1 == surface - &aSurfaces[0]) {
			memcpy(batch.vCenter, surface->vCenter, sizeof(batch.vCenter));
			memcpy(batch.vRadius, surface->vRadius, sizeof(batch.vRadius));
		}
	}
}

tCompactMesh* GetCompactMeshByName(const std::string& name) {
	for (auto& mesh : aCompactMeshes) {
		if (mesh.sName1 == name) return &mesh;
	}
	return nullptr;
}

bool ShouldSurfaceMeshBeImported(aiNode* node) {
	if (!node) return false;
	auto name = (std::string)node->mName.C_Str();
	if (!bImportAllSurfacesFromFBX && !name.ends_with("_export")) return false;
	if (node->mNumMeshes != 1) {
		WriteConsole("ERROR: " + name + " has more than one mesh or material!", LOG_ERRORS);
		return false;
	}
	return true;
}

struct tMeshNodeAssoc {
	aiNode* node;
	aiMesh* mesh;
};
std::vector<tMeshNodeAssoc> aUnorderedMeshImports;

void DeleteSurfaceByEmptying(tSurface* surface) {
	surface->nPolyCount = 0;
	surface->nVertexCount = 0;
	surface->nNumIndicesUsed = 0;
	surface->foucVertexMultiplier[3] = 0;
	surface->_bIsReplacedMapSurface = true;
	surface->_pReplacedMapSurfaceMesh = nullptr;
	WriteConsole("Deleting surface " + std::to_string(surface - &aSurfaces[0]), LOG_ALL);
}

int ReplaceNextAvailableSurface(aiNode* node, aiMesh* mesh, bool allowModelSurfaces) {
	for (auto& surface: aSurfaces) {
		if (!surface._bIsReplacedMapSurface) continue;
		if (surface._pReplacedMapSurfaceMesh == mesh) return &surface - &aSurfaces[0];
	}

	for (auto& surface: aSurfaces) {
		if (!allowModelSurfaces && surface._nNumReferencesByType[SURFACE_REFERENCE_MODEL] > 0) continue;
		if (surface.nNumStreamsUsed != 2) continue;
		if (surface._bIsReplacedMapSurface) continue;

		ImportSurfaceFromFBX(&surface, node, !allowModelSurfaces, mesh);
		return &surface - &aSurfaces[0];
	}

	if (bCreateW32FromFBX) {
		tSurface tmp;
		aSurfaces.push_back(tmp);
		auto& surface = aSurfaces[aSurfaces.size() - 1];
		ImportSurfaceFromFBX(&surface, node, !allowModelSurfaces, mesh);
		return &surface - &aSurfaces[0];
	}
	return -1;
}

void WalkFBXTreeForMeshes(aiNode* node) {
	for (int i = 0; i < node->mNumMeshes; i++) {
		aUnorderedMeshImports.push_back({node, pParsedFBXScene->mMeshes[node->mMeshes[i]]});
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		WalkFBXTreeForMeshes(node->mChildren[i]);
	}
}

tModel* CreateModelFromMesh(aiNode* node) {
	if (bRallyTrophyNodes) {
		auto nodeName = (std::string) node->mName.C_Str();
		if (nodeName.starts_with("split_") || nodeName.starts_with("start")) {
			return nullptr;
		}
	}

	std::vector<aiMesh*> aMeshes;
	for (int i = 0; i < node->mNumMeshes; i++) {
		aMeshes.push_back(pParsedFBXScene->mMeshes[node->mMeshes[i]]);
	}
	for (int i = 0; i < node->mNumChildren; i++) {
		auto child = node->mChildren[i];
		for (int j = 0; j < child->mNumMeshes; j++) {
			aMeshes.push_back(pParsedFBXScene->mMeshes[child->mMeshes[j]]);
		}
	}
	if (aMeshes.empty()) {
		WriteConsole("WARNING: Prop " + (std::string)node->mName.C_Str() + " has no meshes! Ignoring...", LOG_WARNINGS);
		return nullptr;
	}

	aiAABB aabb;

	std::vector<int> surfaces;
	for (auto& mesh : aMeshes) {
		aabb.mMin.x = std::min(aabb.mMin.x, mesh->mAABB.mMin.x);
		aabb.mMin.y = std::min(aabb.mMin.y, mesh->mAABB.mMin.y);
		aabb.mMin.z = std::min(aabb.mMin.z, mesh->mAABB.mMin.z);
		aabb.mMax.x = std::max(aabb.mMax.x, mesh->mAABB.mMax.x);
		aabb.mMax.y = std::max(aabb.mMax.y, mesh->mAABB.mMax.y);
		aabb.mMax.z = std::max(aabb.mMax.z, mesh->mAABB.mMax.z);

		auto surface = ReplaceNextAvailableSurface(node, mesh, true);
		if (surface < 0) continue;
		surfaces.push_back(surface);
	}
	if (surfaces.empty()) {
		WriteConsole("WARNING: The W32 doesn't have any surfaces left for prop " + (std::string)node->mName.C_Str() + "!", LOG_WARNINGS);
		return nullptr;
	}
	if (surfaces.size() < aMeshes.size()) {
		WriteConsole("WARNING: The W32 doesn't have enough surfaces for prop " + (std::string)node->mName.C_Str() + ", " + std::to_string(aMeshes.size() - surfaces.size()) + " FBX surfaces will be skipped!", LOG_WARNINGS);
	}

	tModel model;
	model.nUnk = 4;
	model.sName = node->mName.C_Str();
	model.vCenter[0] = (aabb.mMax[0] + aabb.mMin[0]) * 0.5;
	model.vCenter[1] = (aabb.mMax[1] + aabb.mMin[1]) * 0.5;
	model.vCenter[2] = (aabb.mMax[2] + aabb.mMin[2]) * -0.5;
	model.vRadius[0] = std::abs(aabb.mMax[0] - aabb.mMin[0]) * 0.5;
	model.vRadius[1] = std::abs(aabb.mMax[1] - aabb.mMin[1]) * 0.5;
	model.vRadius[2] = std::abs(aabb.mMax[2] - aabb.mMin[2]) * 0.5;
	// hack for downtown maps
	if (model.sName.starts_with("dyn_streetlight_largepole_")) {
		//model.vCenter[0] = 1.990231;
		model.vCenter[1] = 0.380852;
		model.vCenter[2] = model.vCenter[0] = -0.005263;
		//model.vRadius[0] = 2.124183;
		model.vRadius[1] = 5.630850;
		model.vRadius[2] = model.vRadius[0] = 0.146277;
		model.fRadius = 6.019969;
	}
	for (auto& surface : surfaces) {
		model.aSurfaces.push_back(surface);
	}
	aModels.push_back(model);
	return &aModels[aModels.size() - 1];
}

void CreateW32ObjectsFromFBX() {
	auto objectsArray = GetFBXNodeForObjectsArray();
	for (int i = 0; i < objectsArray->mNumChildren; i++) {
		auto prop = objectsArray->mChildren[i];
		tObject object;
		object.sName1 = prop->mName.C_Str();
		object.nFlags = 0xE0F9;
		FBXMatrixToFO2Matrix(GetFullMatrixFromDummyObject(prop), object.mMatrix);
		aObjects.push_back(object);
	}
}

struct tPropConfig {
	struct tPropNameMatchup {
		std::string name;
		std::string dynamicName;
	};
	std::vector<tPropNameMatchup> name;
	std::vector<tPropNameMatchup> name_prefix;
	std::vector<tPropNameMatchup> name_suffix;

	bool ApplyToProp(tCompactMesh* mesh) {
		auto nameLower = mesh->sName1;
		std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), [](unsigned char c){ return std::tolower(c); });
		for (auto& matchup : name) {
			if (nameLower == matchup.name) {
				mesh->sName2 = matchup.dynamicName;
				return true;
			}
		}
		for (auto& matchup : name_prefix) {
			if (nameLower.starts_with(matchup.name)) {
				mesh->sName2 = matchup.dynamicName;
				return true;
			}
		}
		for (auto& matchup : name_suffix) {
			if (nameLower.ends_with(matchup.name)) {
				mesh->sName2 = matchup.dynamicName;
				return true;
			}
		}
		return false;
	}
};
tPropConfig gPropConfig, gRallyTrophyPropConfig;

void InitPropConfig() {
	static bool bInited = false;
	if (bInited) return;
	bInited = true;

	tPropConfig* config = nullptr;
	std::vector<tPropConfig::tPropNameMatchup>* matchups = nullptr;

	auto input = std::ifstream("prop_overrides.txt");
	if (!input.is_open()) return;
	for (std::string line; std::getline(input, line);) {
		if (line.starts_with("!!FLATOUT!!")) { config = &gPropConfig; continue; }
		if (line.starts_with("!!RALLYTROPHY!!")) { config = &gRallyTrophyPropConfig; continue; }

		if (!config) continue;
		if (line.starts_with("[name]")) { matchups = &config->name; continue; }
		if (line.starts_with("[name_prefix]")) { matchups = &config->name_prefix; continue; }
		if (line.starts_with("[name_suffix]")) { matchups = &config->name_suffix; continue; }
		if (!matchups) continue;
		if (line.starts_with("#")) continue;

		auto equals = line.find('=');
		if (equals == std::string::npos) continue;

		auto name = line.substr(0, equals);
		std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return std::tolower(c); });
		tPropConfig::tPropNameMatchup matchup;
		matchup.name = name;
		matchup.dynamicName = line.substr(equals+1);
		matchups->push_back(matchup);
	}
}

void AddCompactMeshFromFBXNode(aiNode* prop, int defaultGroup) {
	InitPropConfig();

	if (auto model = CreateModelFromMesh(prop)) {
		std::string dynamicType;
		int group = defaultGroup;
		for (int j = 0; j < prop->mNumChildren; j++) {
			auto child = prop->mChildren[j];
			auto name = (std::string)child->mName.C_Str();
			if (name.find("TYPE_") != std::string::npos) {
				name.erase(name.begin(), name.begin() + name.find("TYPE_") + 5);
				FixNameExtensions(name);
				dynamicType = name;
			}
			else if (name.find("GROUP_") != std::string::npos) {
				name.erase(name.begin(), name.begin() + name.find("GROUP_") + 6);
				FixNameExtensions(name);
				group = std::stoi(name);
			}
		}
		if (group >= nCompactMeshGroupCount) nCompactMeshGroupCount = group + 1;

		tCompactMesh mesh;
		mesh.sName1 = prop->mName.C_Str();
		if (dynamicType.empty() && !bAllowEmptyPropTypes) {
			std::string defType = "metal_light";

			bool applied = false;
			if (bRallyTrophyShaderFixup && gRallyTrophyPropConfig.ApplyToProp(&mesh)) {
				applied = true;
			}
			if (!bRallyTrophyShaderFixup && gPropConfig.ApplyToProp(&mesh)) {
				applied = true;
			}

			if (!applied) {
				WriteConsole("WARNING: Prop " + mesh.sName1 + " has no dynamic type! Defaulting to " + defType, LOG_WARNINGS);
				mesh.sName2 = defType;
			}
		}
		else {
			mesh.sName2 = dynamicType;
		}
		FBXMatrixToFO2Matrix(GetFullMatrixFromCompactMeshObject(prop), mesh.mMatrix);
		mesh.nGroup = group;
		mesh.nFlags = 0xE000;
		mesh.nUnk1 = 1;
		mesh.aModels.push_back(model - &aModels[0]);
		mesh.nDamageAssocId = aMeshDamageAssoc.size();
		aCompactMeshes.push_back(mesh);

		tMeshDamageAssoc assoc;
		assoc.sName = mesh.sName1;
		assoc.nIds[0] = aCollidableModels.size();
		assoc.nIds[1] = -1;
		aMeshDamageAssoc.push_back(assoc);

		tCollidableModel col;
		memcpy(col.vRadius, model->vRadius, sizeof(col.vRadius));
		memcpy(col.vCenter, model->vCenter, sizeof(col.vCenter));
		col.aModels.push_back(model - &aModels[0]);
		aCollidableModels.push_back(col);

		WriteConsole("Created new prop " + mesh.sName1 + " in group " + std::to_string(mesh.nGroup) + " with properties from " + mesh.sName2, LOG_ALL);
	}
}

void CreateW32CompactMeshesFromFBX() {
	nCompactMeshGroupCount = 0;

	auto node = GetFBXNodeForCompactMeshArray();
	for (int i = 0; i < node->mNumChildren; i++) {
		AddCompactMeshFromFBXNode(node->mChildren[i], -1);
	}
}

void ReadSplinesFromFBX(aiNode* node) {
	auto dest = GetAISplineByName(node->mName.C_Str());
	for (int i = 0; i < 9999; i++) {
		for (int j = 0; j < node->mNumChildren; j++) {
			auto child = node->mChildren[j];
			auto targetNodeName1 = std::format("{}_Node{}", node->mName.C_Str(), i-1);
			auto targetNodeName2 = std::format("{}_Node1.{:03}", node->mName.C_Str(), i-1); // support default blender duplication naming
			if (child->mName.C_Str() == targetNodeName1 || child->mName.C_Str() == targetNodeName2) {
				aiVector3D v;
				v.x = child->mTransformation.a4;
				v.y = child->mTransformation.b4;
				v.z = -child->mTransformation.c4;
				dest->values.push_back(v);
			}
		}
	}
}

bool AddClonedPropFromFBX(aiNode* child, int groupId) {
	if (GetCompactMeshByName(child->mName.C_Str())) return false;
	auto tmpName = (std::string)child->mName.C_Str();
	while (!GetCompactMeshByName(tmpName) && !tmpName.empty()) {
		tmpName.pop_back();
	}
	if (tmpName.empty()) return false;
	auto baseMesh = GetCompactMeshByName(tmpName);
	if (!baseMesh) return false;
	auto baseName = baseMesh->sName1; // not copying this out beforehand for the console log makes the tool crash????????

	tCompactMesh newMesh = *baseMesh;
	newMesh.nGroup = groupId;
	newMesh.sName1 = child->mName.C_Str();
	FBXMatrixToFO2Matrix(GetFullMatrixFromCompactMeshObject(child), newMesh.mMatrix);
	aCompactMeshes.push_back(newMesh);

	WriteConsole("Cloning " + baseName + " in group " + std::to_string(groupId) + " for new prop placement " + newMesh.sName1, LOG_ALL);
	return true;
}

void WriteW32(uint32_t exportMapVersion) {
	WriteConsole("Writing output w32 file...", LOG_ALWAYS);

	nExportFileVersion = exportMapVersion;
	if ((nExportFileVersion == 0x20002 || nImportFileVersion == 0x20002 || bIsFOUCModel) && nImportFileVersion != nExportFileVersion) {
		WriteConsole("ERROR: FOUC conversions are currently not supported!", LOG_ERRORS);
		return;
	}
	if (nExportFileVersion <= 0x10003 || nImportFileVersion <= 0x10003) {
		WriteConsole("ERROR: Retro Demo / Tough Trucks conversions are currently not supported!", LOG_ERRORS);
		return;
	}

	auto outFileName = sFileNameNoExt.string() + "_out.w32";
	if (bUseVanillaNames) outFileName = sFileFolder.string() + (bIsFOUCModel ? "track_geom_w2.w32" : "track_geom.w32");
	std::ofstream file(outFileName, std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	file.write((char*)&nExportFileVersion, 4);
	if (nExportFileVersion >= 0x20000) {
		for (int i = 0; i < nSomeMapValue; i++) {
			file.write((char*)&nSomeMapValue, 4);
		}
	}

	if (auto node = GetFBXNodeForSplitpointsArray()) {
		aSplitpoints.clear();
		for (int i = 0; i < 9999; i++) {
			auto pos = FindFBXNodeForSplitpointPos(i);
			auto left = FindFBXNodeForSplitpointLeft(i);
			auto right = FindFBXNodeForSplitpointRight(i);
			if (!pos) continue;
			if (!left) continue;
			if (!right) continue;

			tSplitpoint splitPoint;
			splitPoint.pos.x = pos->mTransformation.a4;
			splitPoint.pos.y = pos->mTransformation.b4;
			splitPoint.pos.z = -pos->mTransformation.c4;
			splitPoint.left.x = left->mTransformation.a4;
			splitPoint.left.y = left->mTransformation.b4;
			splitPoint.left.z = -left->mTransformation.c4;
			splitPoint.right.x = right->mTransformation.a4;
			splitPoint.right.y = right->mTransformation.b4;
			splitPoint.right.z = -right->mTransformation.c4;
			aSplitpoints.push_back(splitPoint);
		}
	}

	if (bRallyTrophyNodes) {
		if (auto node = FindFBXNodeForCompactMesh("start")) {
			tStartpoint startPoint;
			FBXMatrixToFO2Matrix(node->mTransformation, startPoint.mMatrix);

			startPoint.mMatrix[0] = 1;
			startPoint.mMatrix[1] = 0;
			startPoint.mMatrix[2] = 0;
			startPoint.mMatrix[4] = 0;
			startPoint.mMatrix[5] = 1;
			startPoint.mMatrix[6] = 0;
			startPoint.mMatrix[8] = 0;
			startPoint.mMatrix[9] = 0;
			startPoint.mMatrix[10] = 1;

			aStartpoints.push_back(startPoint);
		}
		for (int i = 0; i < 64; i++) {
			auto node = FindFBXNodeForCompactMesh(std::format("start{:02}", i));
			if (!node) continue;

			tStartpoint startPoint;
			FBXMatrixToFO2Matrix(node->mTransformation, startPoint.mMatrix);

			startPoint.mMatrix[0] = 1;
			startPoint.mMatrix[1] = 0;
			startPoint.mMatrix[2] = 0;
			startPoint.mMatrix[4] = 0;
			startPoint.mMatrix[5] = 1;
			startPoint.mMatrix[6] = 0;
			startPoint.mMatrix[8] = 0;
			startPoint.mMatrix[9] = 0;
			startPoint.mMatrix[10] = 1;

			aStartpoints.push_back(startPoint);
		}

		for (int i = 0; i < 256; i++) {
			auto node = FindFBXNodeForCompactMesh(std::format("split_{:02}", i));
			if (!node) continue;

			tSplitpoint splitPoint;
			float tmp[4*4];
			FBXMatrixToFO2Matrix(node->mTransformation, tmp);
			splitPoint.pos.x = tmp[(4*3)+0];
			splitPoint.pos.y = tmp[(4*3)+1];
			splitPoint.pos.z = tmp[(4*3)+2];
			splitPoint.left = splitPoint.pos;
			splitPoint.right = splitPoint.pos;
			splitPoint.left.x -= tmp[0] * 50;
			splitPoint.left.y -= tmp[1] * 50;
			splitPoint.left.z -= tmp[2] * 50;
			splitPoint.right.x += tmp[0] * 50;
			splitPoint.right.y += tmp[1] * 50;
			splitPoint.right.z += tmp[2] * 50;
			aSplitpoints.push_back(splitPoint);
		}
	}

	if (auto node = GetFBXNodeForStartpointsArray()) {
		aStartpoints.clear();
		for (int i = 0; i < 9999; i++) {
			auto pos = FindFBXNodeForStartpoint(i);
			if (!pos) continue;

			tStartpoint startPoint;
			FBXMatrixToFO2Matrix(pos->mTransformation, startPoint.mMatrix);
			aStartpoints.push_back(startPoint);
		}
	}

	if (auto node = GetFBXNodeForSplinesArray()) {
		for (int i = 0; i < node->mNumChildren; i++) {
			auto child = node->mChildren[i];
			ReadSplinesFromFBX(child);
		}
	}

	if (bImportSurfacesFromFBX) {
		if (bImportAndAutoMatchAllMeshesFromFBX || bImportAndAutoMatchAllSurfacesFromFBX) {
			auto materialsBackup = aMaterials;

			// FO1 can't handle too many materials, clear them first
			if (nExportFileVersion < 0x20000 || bClearOriginalMaterials) aMaterials.clear();

			if (bImportAndAutoMatchAllMeshesFromFBX) {
				WalkFBXTreeForMeshes(pParsedFBXScene->mRootNode);
			}
			else {
				auto nodes = GetAllFBXSurfaceNodes();
				for (auto node : nodes) {
					if (node->mNumMeshes == 0) continue;
					for (int j = 0; j < node->mNumMeshes; j++) {
						aUnorderedMeshImports.push_back({node, pParsedFBXScene->mMeshes[node->mMeshes[j]]});
					}
				}
			}

			// match up fbx meshes and delete every leftover non-prop w32 surface
			int unmatchedCount = 0;
			for (auto& data : aUnorderedMeshImports) {
				if (ReplaceNextAvailableSurface(data.node, data.mesh, false) < 0) unmatchedCount++;
			}

			if (unmatchedCount > 0) {
				WriteConsole("WARNING: The W32 only has " + std::to_string(aUnorderedMeshImports.size() - unmatchedCount) + " usable surfaces but the FBX has " + std::to_string(aUnorderedMeshImports.size()) + ", " + std::to_string(unmatchedCount) + " FBX surfaces will be skipped!", LOG_WARNINGS);
			}

			if (nExportFileVersion < 0x20000 || bClearOriginalMaterials) {
				for (int i = aMaterials.size(); i < materialsBackup.size(); i++) {
					aMaterials.push_back(materialsBackup[i]);
				}
			}
		}
		else {
			for (auto& surface: aSurfaces) {
				if (auto node = FindFBXNodeForSurface(&surface - &aSurfaces[0])) {
					if (node->mNumMeshes <= 0) continue;
					if (ShouldSurfaceMeshBeImported(node)) {
						if (node->mNumMeshes > 1) {
							WriteConsole("WARNING: " + (std::string)node->mName.C_Str() + " has more than 1 mesh or material, only the first one will be imported!", LOG_WARNINGS);
						}
						ImportSurfaceFromFBX(&surface, node, surface._nNumReferencesByType[SURFACE_REFERENCE_MODEL] <= 0, pParsedFBXScene->mMeshes[node->mMeshes[0]]);
					}
				}
			}
		}
	}

	if (bDisableObjects) {
		aObjects.clear();
	}

	if (bImportAllObjectsFromFBX) {
		aObjects.clear();
		CreateW32ObjectsFromFBX();
	}

	if (bImportAllPropsFromFBX) {
		aMeshDamageAssoc.clear();
		aCollidableModels.clear();
		aCompactMeshes.clear();
		CreateW32CompactMeshesFromFBX();
	}

	if (bImportAndAutoMatchAllSurfacesFromFBX || bImportAndAutoMatchAllMeshesFromFBX) {
		for (auto& surface : aSurfaces) {
			if (!bImportAllPropsFromFBX && surface._nNumReferencesByType[SURFACE_REFERENCE_MODEL] > 0) continue;
			if (!surface._bIsReplacedMapSurface) DeleteSurfaceByEmptying(&surface);
		}
	}

	uint32_t materialCount = aMaterials.size();
	file.write((char*)&materialCount, 4);
	for (auto& material : aMaterials) {
		WriteMaterialToFile(file, material);
	}

	uint32_t streamCount = aVertexBuffers.size() + aIndexBuffers.size();
	file.write((char*)&streamCount, 4);
	for (int i = 0; i < streamCount; i++) {
		for (auto& buf : aVertexBuffers) {
			if (buf.id == i) {
				WriteVertexBufferToFile(file, buf);
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
		if (!bImportAndAutoMatchAllSurfacesFromFBX && bImportDeletionFromFBX && CanSurfaceBeExported(&surface) && !FindFBXNodeForSurface(&surface - &aSurfaces[0]) && !surface._nNumReferencesByType[SURFACE_REFERENCE_MODEL]) {
			DeleteSurfaceByEmptying(&surface);
		}

		WriteSurfaceToFile(file, surface);
	}

	uint32_t staticBatchCount = aStaticBatches.size();
	file.write((char*)&staticBatchCount, 4);
	for (auto& staticBatch : aStaticBatches) {
		WriteStaticBatchToFile(file, staticBatch);
	}

	if (!bIsFOUCModel) {
		uint32_t treeColorCount = aTreeColors.size();
		file.write((char*)&treeColorCount, 4);
		for (auto& data: aTreeColors) {
			file.write((char*)&data, 4);
		}
	}

	uint32_t treeLodCount = aTreeLODs.size();
	file.write((char*)&treeLodCount, 4);
	for (auto& data : aTreeLODs) {
		file.write((char*)data.vPos, sizeof(data.vPos));
		file.write((char*)data.fScale, sizeof(data.fScale));
		file.write((char*)data.nValues, sizeof(data.nValues));
	}

	uint32_t treeMeshCount = aTreeMeshes.size();
	file.write((char*)&treeMeshCount, 4);
	for (auto& mesh : aTreeMeshes) {
		WriteTreeMeshToFile(file, mesh);
	}

	if (nExportFileVersion >= 0x10004) {
		if (bDisableCarCollisions) memset(aTrackCollisionOffsetMatrix, 0, sizeof(aTrackCollisionOffsetMatrix));
		file.write((char*)&aTrackCollisionOffsetMatrix, sizeof(aTrackCollisionOffsetMatrix));
	}

	uint32_t modelCount = aModels.size();
	file.write((char*)&modelCount, 4);
	for (auto& model : aModels) {
		WriteModelToFile(file, model);
	}

	uint32_t objectCount = aObjects.size();
	file.write((char*)&objectCount, 4);
	for (auto& object : aObjects) {
		WriteObjectToFile(file, object);
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
			uint32_t colCount = aCollidableModels.size();
			file.write((char*)&colCount, 4);
			for (auto& bbox : aCollidableModels) {
				WriteCollidableModelToFile(file, bbox);
			}

			uint32_t meshDamageCount = aMeshDamageAssoc.size();
			file.write((char*)&meshDamageCount, 4);
			for (auto& bboxAssoc : aMeshDamageAssoc) {
				WriteMeshDamageAssocToFile(file, bboxAssoc);
			}
		}

		uint32_t compactMeshCount = aCompactMeshes.size();
		if (bImportDeletionFromFBX) {
			for (auto& mesh : aCompactMeshes) {
				if (!FindFBXNodeForCompactMesh(mesh.sName1)) compactMeshCount--;
			}
		}
		if (bImportClonedPropsFromFBX) {
			auto node = GetFBXNodeForCompactMeshArray();
			for (int i = 0; i < node->mNumChildren; i++) {
				if (AddClonedPropFromFBX(node->mChildren[i], -1)) compactMeshCount++;
			}
		}

		file.write((char*)&nCompactMeshGroupCount, 4);
		file.write((char*)&compactMeshCount, 4);
		for (auto& mesh : aCompactMeshes) {
			if (bImportDeletionFromFBX && !FindFBXNodeForCompactMesh(mesh.sName1)) continue;
			WriteCompactMeshToFile(file, mesh);
		}
	}

	if (bImportSurfacesFromFBX && !bImportAndAutoMatchAllSurfacesFromFBX) {
		for (int i = aSurfaces.size(); i < 9999; i++) {
			if (auto node = FindFBXNodeForSurface(i)) {
				if (node->mNumMeshes <= 0) continue;
				WriteConsole("WARNING: Found " + (std::string)node->mName.C_Str() + " but no such surface exists in the w32! Ignoring...", LOG_WARNINGS);
			}
		}
	}

	file.flush();

	WriteConsole("W32 export finished", LOG_ALWAYS);

	UpdateTrackBVH();
	WriteTrackBVH();
	WriteSplitpoints();
	WriteStartpoints();
	WriteSplines();
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
	if (material->nShaderId == 5 || material->nShaderId == 26) { // car body or ragdoll skin, has vertex colors
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
			src += 3;
		}

		if ((flags & VERTEX_COLOR) != 0) {
			*(uint32_t*)&dest[6] = *(uint32_t*)src; // vertex color
			dest += 1;
		}

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
			WriteConsole("ERROR: Index out of bounds: " + std::to_string(newIndices[i]) + " for surface " + std::to_string(&surface - &aSurfaces[0]), LOG_ERRORS);
			WaitAndExitOnFail();
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
	WriteConsole("Writing output bgm file...", LOG_ALWAYS);

	nExportFileVersion = exportMapVersion;

	std::ofstream file(sFileNameNoExt.string() + "_out.bgm", std::ios::out | std::ios::binary );
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
		// todo add direct crash.dat conversion support, currently just clearing it
		aCrashData.clear();
		aCrashSurfaces.clear();

		for (auto& surface : aSurfaces) {
			if (surface.nFlags != 0x2242) {
				WriteConsole("ERROR: Unexpected flags value! Failed to convert surfaces", LOG_ERRORS);
				return;
			}
			ConvertFOUCSurfaceToFO2(surface);
		}

		aVertexBuffers = aConversionVertexBuffers;
		aIndexBuffers = aConversionIndexBuffers;
	}

	if (bConvertToFO1 || bConvertToFO2) bIsFOUCModel = false;

	uint32_t streamCount = aVertexBuffers.size() + aIndexBuffers.size();
	file.write((char*)&streamCount, 4);
	for (int i = 0; i < streamCount; i++) {
		for (auto& buf : aVertexBuffers) {
			if (buf.id == i) {
				WriteVertexBufferToFile(file, buf);
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

	uint32_t carMeshCount = aCompactMeshes.size();
	file.write((char*)&carMeshCount, 4);
	for (auto& mesh : aCompactMeshes) {
		WriteBGMMeshToFile(file, mesh);
	}

	uint32_t objectCount = aObjects.size();
	file.write((char*)&objectCount, 4);
	for (auto& object : aObjects) {
		WriteObjectToFile(file, object);
	}

	file.flush();

	WriteConsole("BGM export finished", LOG_ALWAYS);
}

void WriteCrashDat(uint32_t exportVersion) {
	int numModels = 0;
	for (auto& model : aModels) {
		if (!model.aCrashSurfaces.empty()) numModels++;
	}

	if (numModels <= 0) {
		WriteConsole("No crash data found, skipping crash.dat export", LOG_ALWAYS);
		return;
	}

	WriteConsole("Writing output crash.dat file...", LOG_ALWAYS);

	nExportFileVersion = exportVersion;

	auto fileName = sFileNameNoExt.string() + "_out_crash.dat";
	if (bUseVanillaNames) fileName = sFileFolder.string() + "crash.dat";
	std::ofstream file(fileName, std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	file.write((char*)&numModels, 4);
	for (auto& model : aModels) {
		if (model.aCrashSurfaces.empty()) continue;

		auto name = model.sName;
		name += "_crash";
		file.write(name.c_str(), name.length() + 1);

		uint32_t numSurfaces = model.aCrashSurfaces.size();
		file.write((char*)&numSurfaces, 4);
		for (auto& surfaceId : model.aCrashSurfaces) {
			bool noDamage = false;
			auto baseSurface = aSurfaces[model.aSurfaces[&surfaceId - &model.aCrashSurfaces[0]]];
			auto crashSurface = aCrashSurfaces[surfaceId];
			if (baseSurface.nVertexCount != crashSurface.nVertexCount) {
				WriteConsole("ERROR: " + model.sName + " has damage model with a mismatching vertex count, no damage will be exported! (" +std::to_string(baseSurface.nVertexCount) + "/" + std::to_string(crashSurface.nVertexCount) + ")", LOG_ERRORS);
				noDamage = true;
			}
			auto baseVBuffer = FindVertexBuffer(baseSurface.nStreamId[0]);
			if (!baseVBuffer) {
				WriteConsole("ERROR: Failed to find vertex buffer for " + model.sName, LOG_ERRORS);
				WaitAndExitOnFail();
			}
			auto crashVBuffer = FindVertexBuffer(crashSurface.nStreamId[0]);
			if (!crashVBuffer) {
				WriteConsole("ERROR: Failed to find damage vertex buffer for " + model.sName, LOG_ERRORS);
				WaitAndExitOnFail();
			}
			if (baseVBuffer->vertexSize != crashVBuffer->vertexSize) {
				WriteConsole("ERROR: " + model.sName + " has damage model with a mismatching vertex size!", LOG_ERRORS);
				WaitAndExitOnFail();
			}

			uint32_t numVerts = baseSurface.nVertexCount;
			file.write((char*)&numVerts, 4);
			if (!bIsFOUCModel) {
				uint32_t numVertsBytes = baseSurface.nVertexCount * baseVBuffer->vertexSize;
				file.write((char*)&numVertsBytes, 4);
				file.write((char*)baseVBuffer->data, numVertsBytes);
			}
			auto baseVerts = (uintptr_t)baseVBuffer->data;
			auto crashVerts = (uintptr_t)crashVBuffer->data;
			if (noDamage) crashVerts = baseVerts;
			for (int i = 0; i < numVerts; i++) {
				if (bIsFOUCModel) {
					// verts
					file.write((char*)baseVerts, 3 * sizeof(uint16_t));
					file.write((char*)crashVerts, 3 * sizeof(uint16_t));
					// bumpmap related
					file.write((char*)(baseVerts + 4 * sizeof(uint16_t)), 4);
					file.write((char*)(crashVerts + 4 * sizeof(uint16_t)), 4);
					// bumpmap related
					file.write((char*)(baseVerts + 6 * sizeof(uint16_t)), 4);
					file.write((char*)(crashVerts + 6 * sizeof(uint16_t)), 4);
					// normals
					file.write((char*)(baseVerts + 8 * sizeof(uint16_t)), 4);
					file.write((char*)(crashVerts + 8 * sizeof(uint16_t)), 4);
					// uvs, no crash variant
					file.write((char*)(baseVerts + 12 * sizeof(uint16_t)), 4);
				}
				else {
					// base verts, crash verts, base normals, crash normals
					file.write((char*)baseVerts, 3 * sizeof(float));
					file.write((char*)crashVerts, 3 * sizeof(float));
					file.write((char*)(baseVerts + 3 * sizeof(float)), 3 * sizeof(float));
					file.write((char*)(crashVerts + 3 * sizeof(float)), 3 * sizeof(float));
				}

				baseVerts += baseVBuffer->vertexSize;
				crashVerts += crashVBuffer->vertexSize;
			}
		}
	}

	file.flush();

	WriteConsole("crash.dat export finished", LOG_ALWAYS);
}

void ImportNewStaticBatchFromFBX(aiNode* node, int meshId) {
	auto pMesh = pParsedFBXScene->mMeshes[node->mMeshes[meshId]];
	if (bFBXNoBushes) {
		auto mat = pParsedFBXScene->mMaterials[pMesh->mMaterialIndex];
		auto matName = (std::string)mat->GetName().C_Str();
		FixNameExtensions(matName);
		if (matName == "alpha_bushbranch") return;
	}

	if (bApplyStaticTransforms) {
		pStaticTransformationMatrix = new aiMatrix4x4;
		pStaticRotationMatrix = new aiMatrix4x4;
		*pStaticTransformationMatrix = GetFullMatrixFromStaticBatchObject(node);

		*pStaticRotationMatrix = *pStaticTransformationMatrix;
		pStaticRotationMatrix->a4 = 0;
		pStaticRotationMatrix->b4 = 0;
		pStaticRotationMatrix->c4 = 0;
	}

	tSurface tmp;
	aSurfaces.push_back(tmp);
	auto& surface = aSurfaces[aSurfaces.size() - 1];
	ImportSurfaceFromFBX(&surface, node, true, pMesh);
	tStaticBatch batch;
	batch.nBVHId1 = batch.nBVHId2 = batch.nId1 = aStaticBatches.size();
	memcpy(batch.vCenter, surface.vCenter, sizeof(batch.vCenter));
	memcpy(batch.vRadius, surface.vRadius, sizeof(batch.vRadius));
	aStaticBatches.push_back(batch);

	if (pStaticTransformationMatrix) {
		delete pStaticTransformationMatrix;
		pStaticTransformationMatrix = nullptr;
	}
	if (pStaticRotationMatrix) {
		delete pStaticRotationMatrix;
		pStaticRotationMatrix = nullptr;
	}
}

void ImportStaticBatchesFromFBXTree(aiNode* node) {
	for (int i = 0; i < node->mNumChildren; i++) {
		ImportStaticBatchesFromFBXTree(node->mChildren[i]);
	}
	for (int i = 0; i < node->mNumMeshes; i++) {
		ImportNewStaticBatchFromFBX(node, i);
	}
}

void FillW32FromFBX() {
	WriteConsole("Creating W32 data...", LOG_ALWAYS);

	WriteConsole("Creating static batches...", LOG_ALWAYS);

	ImportStaticBatchesFromFBXTree(GetFBXNodeForStaticBatchArray());
	// todo actual tree meshes, this'll require tree colors and leaves to work
	bTmpModelsDoubleSided = true;
	ImportStaticBatchesFromFBXTree(GetFBXNodeForTreeMeshArray());
	bTmpModelsDoubleSided = false;

	WriteConsole("Creating object dummies...", LOG_ALWAYS);
	CreateW32ObjectsFromFBX();

	WriteConsole("Creating compact meshes...", LOG_ALWAYS);
	CreateW32CompactMeshesFromFBX();

	aTrackCollisionOffsetMatrix[0] = 0.560662;
	aTrackCollisionOffsetMatrix[1] = 0.000000;
	aTrackCollisionOffsetMatrix[2] = 0.828045;
	aTrackCollisionOffsetMatrix[3] = 0.000000;
	aTrackCollisionOffsetMatrix[4] = 0.000000;
	aTrackCollisionOffsetMatrix[5] = 1.000000;
	aTrackCollisionOffsetMatrix[6] = 0.000000;
	aTrackCollisionOffsetMatrix[7] = 0.000000;
	aTrackCollisionOffsetMatrix[8] = -0.828045;
	aTrackCollisionOffsetMatrix[9] = 0.000000;
	aTrackCollisionOffsetMatrix[10] = 0.560662;
	aTrackCollisionOffsetMatrix[11] = 0.000000;
	aTrackCollisionOffsetMatrix[12] = 0.000000;
	aTrackCollisionOffsetMatrix[13] = 0.000000;
	aTrackCollisionOffsetMatrix[14] = 0.000000;
	aTrackCollisionOffsetMatrix[15] = 1.000000;

	WriteConsole("W32 data created", LOG_ALWAYS);
}

void WriteXboxBuffers() {
	if (aXboxCPUBuffers.empty() || aXboxGPUBuffers.empty()) return;

	std::ofstream cpu(sFileNameNoExt.string() + "_cpu.dat", std::ios::out | std::ios::binary );
	if (!cpu.is_open()) return;
	cpu.write((char*)aXboxCPUBuffers[0].data, aXboxCPUBuffers[0].count);

	std::ofstream gpu(sFileNameNoExt.string() + "_gpu.dat", std::ios::out | std::ios::binary );
	if (!gpu.is_open()) return;
	gpu.write((char*)aXboxGPUBuffers[0].data, aXboxGPUBuffers[0].count);
}