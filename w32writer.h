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
		file.write((char*)treeMesh.foucExtraData1, sizeof(treeMesh.foucExtraData1));
		file.write((char*)treeMesh.foucExtraData2, sizeof(treeMesh.foucExtraData2));
		file.write((char*)treeMesh.foucExtraData3, sizeof(treeMesh.foucExtraData3));
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
		int numLODs = mesh.aLODMeshIds.size();
		file.write((char*)&numLODs, 4);
		for (auto model : mesh.aLODMeshIds) {
			file.write((char*)&model, 4);
		}
	}
}

void WriteBGMMeshToFile(std::ofstream& file, tBGMMesh& mesh) {
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

void CreateStreamsFromFBX(aiMesh* mesh, uint32_t flags, uint32_t vertexSize, float foucOffset1 = 0, float foucOffset2 = 0, float foucOffset3 = 0, float foucOffset4 = fFOUCBGMScaleMultiplier, bool treeHack = false) {
	int id = aVertexBuffers.size() + aIndexBuffers.size();

	if (bNoTreeHack) treeHack = false;

	if ((flags & VERTEX_UV2) != 0 && !mesh->HasTextureCoords(1)) {
		WriteConsole("WARNING: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have 2 sets of UVs!", LOG_MINOR_WARNINGS);
		//exit(0);
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
		exit(0);
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
			srcVerts.x /= foucOffsets[3];
			srcVerts.y /= foucOffsets[3];
			srcVerts.z /= foucOffsets[3];
			srcVerts.x -= foucOffsets[0];
			srcVerts.y -= foucOffsets[1];
			srcVerts.z += foucOffsets[2];
			data->vPos[0] = srcVerts.x;
			data->vPos[1] = srcVerts.y;
			data->vPos[2] = -srcVerts.z;

			// scalar + bumpmap strength i believe
			data->nUnk32 = 0x0400;
			data->vUnknownProllyBumpmaps[0] = 0x00;
			data->vUnknownProllyBumpmaps[1] = 0x00;
			data->vUnknownProllyBumpmaps[2] = 0x00;
			data->vUnknownProllyBumpmaps[3] = 0xFF;
			data->vUnknownProllyBumpmaps2[0] = 0x00;
			data->vUnknownProllyBumpmaps2[1] = 0x00;
			data->vUnknownProllyBumpmaps2[2] = 0x00;
			data->vUnknownProllyBumpmaps2[3] = 0xFF;

			// normals
			if (mesh->HasNormals()) {
				auto normals = mesh->mNormals[i];
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
				data->vNormals[0] = tmp;
				tmp = (normals.y + 1) * 127.0;
				data->vNormals[1] = tmp;
				tmp = (normals.x + 1) * 127.0;
				data->vNormals[2] = tmp;
				data->vNormals[3] = 0xFF;
			}
			else {
				WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have normals!", LOG_ERRORS);
				exit(0);
			}
			// vertex color
			if (mesh->HasVertexColors(0)) {
				uint8_t tmp[4] = {0, 0, 0, 0xFF};
				tmp[0] = mesh->mColors[0][i].r * 255.0;
				tmp[1] = mesh->mColors[0][i].g * 255.0;
				tmp[2] = mesh->mColors[0][i].b * 255.0;
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
				exit(0);
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
			vertices[0] = mesh->mVertices[i].x;
			vertices[1] = mesh->mVertices[i].y;
			vertices[2] = -mesh->mVertices[i].z;
			vertices += 3;

			if ((flags & VERTEX_NORMAL) != 0) {
				if (!mesh->HasNormals()) {
					WriteConsole("ERROR: " + (std::string)mesh->mName.C_Str() + " uses a shader required to have normals!", LOG_ERRORS);
					exit(0);
				}

				auto normals = mesh->mNormals[i];
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
				if (mesh->HasVertexColors(0)) {
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

	tIndexBuffer iBuf;
	iBuf.id = id + 1;
	iBuf.indexCount = mesh->mNumFaces * 3;
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
	}
	aVertexBuffers.push_back(vBuf);
	aIndexBuffers.push_back(iBuf);
}

tMaterial GetCarMaterialFromFBX(aiMaterial* fbxMaterial) {
	tMaterial mat;
	auto matName = fbxMaterial->GetName().C_Str();
	mat.sName = matName;
	mat.nShaderId = 8; // car metal
	if (mat.sName.starts_with("male")) mat.nShaderId = 26; // skinning
	if (mat.sName.starts_with("female")) mat.nShaderId = 26; // skinning
	if (mat.sName.starts_with("body")) mat.nShaderId = 5; // car body
	if (mat.sName.starts_with("interior")) mat.nShaderId = 7; // car diffuse
	if (mat.sName.starts_with("grille")) mat.nShaderId = 7; // car diffuse
	if (mat.sName.starts_with("shadow")) mat.nShaderId = 13; // shadow project
	if (mat.sName.starts_with("window")) mat.nShaderId = 6; // car window
	if (mat.sName.starts_with("shear")) mat.nShaderId = 11; // car shear
	if (mat.sName.starts_with("scale")) mat.nShaderId = 12; // car scale
	if (mat.sName.starts_with("tire")) mat.nShaderId = bIsFOUCModel ? 44 : 7; // fo2: car diffuse, uc: car tire
	if (mat.sName.starts_with("rim")) mat.nShaderId = 9; // fo2: car tire, uc: car tire rim
	if (mat.sName.starts_with("terrain") || mat.sName.starts_with("groundplane")) mat.nShaderId = 7; // car diffuse
	if (mat.sName.starts_with("massdoubler_texture")) mat.nShaderId = 3; // dynamic diffuse
	if (mat.sName.starts_with("bomb_texture")) mat.nShaderId = 3; // dynamic diffuse
	if (mat.sName.starts_with("powerarmour_texture")) mat.nShaderId = 3; // dynamic diffuse
	if (mat.sName.starts_with("scoredoubler_texture")) mat.nShaderId = 3; // dynamic diffuse
	if (mat.sName.starts_with("infinitro_texture")) mat.nShaderId = 3; // dynamic diffuse
	if (mat.sName.starts_with("repair_texture")) mat.nShaderId = 3; // dynamic diffuse
	if (mat.sName.starts_with("powerram_texture")) mat.nShaderId = 3; // dynamic diffuse
	if (mat.sName.starts_with("massdoubler_texture")) mat.nShaderId = 3; // dynamic diffuse
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
		if (texName == "Sue.tga" || texName == "Jack.tga") {
			mat.nShaderId = 26;
		}
	}
	// shadow project has no texture
	if (mat.sName.starts_with("shadow")) mat.sTextureNames[0] = "";
	// scaleshock and shearhock have no alpha
	if (mat.sName.starts_with("scaleshock")) mat.nAlpha = 0;
	if (mat.sName.starts_with("shearhock")) mat.nAlpha = 0;
	// fouc tire_01 hack
	if (bIsFOUCModel && mat.sTextureNames[0] == "tire_01.tga") mat.sTextureNames[0] = "tire.tga";
	// custom alpha suffix
	if (mat.sName.ends_with("_alpha")) mat.nAlpha = 1;
	WriteConsole("Creating new material " + mat.sName + " with shader " + GetShaderName(mat.nShaderId), LOG_ALL);
	return mat;
}

tMaterial GetMapMaterialFromFBX(aiMaterial* fbxMaterial, bool isStaticModel) {
	tMaterial mat;
	auto matName = fbxMaterial->GetName().C_Str();
	mat.sName = matName;
	mat.nAlpha = mat.sName.starts_with("alpha") || mat.sName.starts_with("Alpha") || mat.sName.starts_with("wirefence_");
	if (isStaticModel) {
		mat.nShaderId = 0; // static prelit
		if (mat.sName.starts_with("dm_")) mat.nShaderId = 1; // terrain
		if (mat.sName.starts_with("terrain_")) mat.nShaderId = 1; // terrain
		if (mat.sName.starts_with("sdm_")) mat.nShaderId = 2; // terrain specular
		if (mat.sName.starts_with("treetrunk")) mat.nShaderId = 19; // tree trunk
		if (mat.sName.starts_with("alpha_treebranch")) mat.nShaderId = 20; // tree branch
		if (mat.sName.starts_with("alpha_bushbranch")) mat.nShaderId = 20; // tree branch
		if (mat.sName.starts_with("alpha_treelod")) mat.nShaderId = 21; // tree leaf
		if (mat.sName.starts_with("alpha_treesprite")) mat.nShaderId = 21; // tree leaf
		if (mat.sName.starts_with("alpha_bushlod")) mat.nShaderId = 21; // tree leaf
		if (mat.sName.starts_with("alpha_bushsprite")) mat.nShaderId = 21; // tree leaf
		if (mat.sName.starts_with("puddle")) mat.nShaderId = bIsFOUCModel ? 45 : 25; // puddle : water
		if (mat.sName.ends_with(".001")) {
			for (int i = 0; i < 4; i++) {
				mat.sName.pop_back();
			}
		}
		if (mat.sName == "water") mat.nShaderId = bIsFOUCModel ? 45 : 25; // puddle : water
	}
	else {
		mat.nShaderId = 3; // dynamic diffuse
		if (mat.sName.ends_with(".001")) {
			for (int i = 0; i < 4; i++) {
				mat.sName.pop_back();
			}
		}
		if (mat.sName.ends_with("_specular")) mat.nShaderId = 4; // dynamic specular, custom suffix for manual use
	}

	mat.nNumTextures = fbxMaterial->GetTextureCount(aiTextureType_DIFFUSE);
	if (mat.nNumTextures > 3) mat.nNumTextures = 3;
	for (int i = 0; i < mat.nNumTextures; i++) {
		auto texName = GetFBXTextureInFO2Style(fbxMaterial, i);
		mat.sTextureNames[i] = texName;

		if (i == 0 && (texName == "colormap.tga" || texName == "Colormap.tga")) {
			mat.nUseColormap = 1;

			// hack to load colormapped textures properly when the fbx has texture2 stripped
			if (mat.nNumTextures == 1) {
				mat.sTextureNames[1] = mat.sName + ".tga";
				if (mat.sTextureNames[1].starts_with("terrain_")) mat.sTextureNames[1] = "dm_" + mat.sTextureNames[1];
				mat.nNumTextures = 2;
				break;
			}
		}
	}
	// FOUC doesn't seem to support trees outside of plant_geom, replace their shaders to get around this
	if (bIsFOUCModel) {
		if (mat.nShaderId == 19 || mat.nShaderId == 20 || mat.nShaderId == 21) {
			if (mat.nShaderId != 19) mat._bIsCustomFOUCTree = true;
			mat.nShaderId = 0;
		}
	}
	WriteConsole("Creating new material " + mat.sName + " with shader " + GetShaderName(mat.nShaderId), LOG_ALL);
	return mat;
}

int GetMaterialSortPriority(aiMaterial* mat) {
	auto name = (std::string)mat->GetName().C_Str();
	if (name == "light_brake") return 1;
	if (name == "light_brake_l") return 1;
	if (name == "light_brake_r") return 1;
	if (name == "light_brake_b") return 2;
	if (name == "light_brake_l_b") return 2;
	if (name == "light_brake_r_b") return 2;
	return 0;
}

int GetBGMMaterialID(const std::string& name) {
	for (auto& material : aMaterials) {
		if (material.sName == name) return &material - &aMaterials[0];
	}
	return 0;
}

int GetBGMMaterialID(aiMaterial* material) {
	return GetBGMMaterialID(material->GetName().C_Str());
}

void CreateBGMSurfaceFromFBX(aiNode* node, int meshId, bool isCrash) {
	auto mesh = pParsedFBXScene->mMeshes[node->mMeshes[meshId]];
	int fbxMeshId = node->mMeshes[meshId];

	tSurface surface;
	surface.nIsVegetation = 0;
	surface.nVertexCount = mesh->mNumVertices;
	surface.nPolyCount = mesh->mNumFaces;
	surface.nNumIndicesUsed = mesh->mNumFaces * 3;
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

void FillBGMFromFBX() {
	WriteConsole("Creating BGM data...", LOG_ALWAYS);

	WriteConsole("Creating materials...", LOG_ALWAYS);

	// create materials
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < pParsedFBXScene->mNumMaterials; i++) {
			auto mat = pParsedFBXScene->mMaterials[i];
			if (GetMaterialSortPriority(mat) != j) continue;
			aMaterials.push_back(GetCarMaterialFromFBX(pParsedFBXScene->mMaterials[i]));
		}
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

		tBGMMesh bgmMesh;
		bgmMesh.sName1 = bodyNode->mName.C_Str();
		bgmMesh.nFlags = 0x0;
		bgmMesh.nGroup = -1;
		FBXMatrixToFO2Matrix(bodyNode->mTransformation, bgmMesh.mMatrix);

		for (int j = 0; j < bodyNode->mNumChildren; j++) {
			auto body001 = bodyNode->mChildren[j]; // body.001

			tModel model;
			model.sName = body001->mName.C_Str();

			float aabbMin[3] = {0, 0, 0};
			float aabbMax[3] = {0, 0, 0};

			for (int k = 0; k < body001->mNumMeshes; k++) {
				// Z inversion doesn't matter here - we're calculating a radius anyway
				auto mesh = pParsedFBXScene->mMeshes[body001->mMeshes[k]];
				auto meshName = (std::string)mesh->mName.C_Str();
				if (meshName.ends_with("_crash")) {
					model.aCrashSurfaces.push_back(aCrashSurfaces.size());
					CreateBGMSurfaceFromFBX(body001, k, true);
					continue;
				}

				if (aabbMin[0] > mesh->mAABB.mMin.x) aabbMin[0] = mesh->mAABB.mMin.x;
				if (aabbMin[1] > mesh->mAABB.mMin.y) aabbMin[1] = mesh->mAABB.mMin.y;
				if (aabbMin[2] > mesh->mAABB.mMin.z) aabbMin[2] = mesh->mAABB.mMin.z;
				if (aabbMax[0] < mesh->mAABB.mMax.x) aabbMax[0] = mesh->mAABB.mMax.x;
				if (aabbMax[1] < mesh->mAABB.mMax.y) aabbMax[1] = mesh->mAABB.mMax.y;
				if (aabbMax[2] < mesh->mAABB.mMax.z) aabbMax[2] = mesh->mAABB.mMax.z;

				model.aSurfaces.push_back(aSurfaces.size());
				CreateBGMSurfaceFromFBX(body001, k, false);
			}
			for (int k = 0; k < body001->mNumChildren; k++) {
				auto surface = body001->mChildren[k]; // Surface1
				auto nodeName = (std::string)surface->mName.C_Str();
				for (int l = 0; l < surface->mNumMeshes; l++) {
					// Z inversion doesn't matter here - we're calculating a radius anyway
					auto mesh = pParsedFBXScene->mMeshes[surface->mMeshes[l]];
					auto meshName = (std::string)mesh->mName.C_Str();
					if (nodeName.ends_with("_crash") || meshName.ends_with("_crash")) {
						model.aCrashSurfaces.push_back(aCrashSurfaces.size());
						CreateBGMSurfaceFromFBX(surface, l, true);
						continue;
					}
					if (aabbMin[0] > mesh->mAABB.mMin.x) aabbMin[0] = mesh->mAABB.mMin.x;
					if (aabbMin[1] > mesh->mAABB.mMin.y) aabbMin[1] = mesh->mAABB.mMin.y;
					if (aabbMin[2] > mesh->mAABB.mMin.z) aabbMin[2] = mesh->mAABB.mMin.z;
					if (aabbMax[0] < mesh->mAABB.mMax.x) aabbMax[0] = mesh->mAABB.mMax.x;
					if (aabbMax[1] < mesh->mAABB.mMax.y) aabbMax[1] = mesh->mAABB.mMax.y;
					if (aabbMax[2] < mesh->mAABB.mMax.z) aabbMax[2] = mesh->mAABB.mMax.z;

					model.aSurfaces.push_back(aSurfaces.size());
					CreateBGMSurfaceFromFBX(surface, l, false);
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
			if (!model.aCrashSurfaces.empty() && model.aCrashSurfaces.size() != model.aSurfaces.size()) {
				WriteConsole("ERROR: " + model.sName + " has crash models but not for every mesh! Skipping", LOG_ERRORS);
				model.aCrashSurfaces.clear();
			}
			// fRadius isn't required, game doesn't read it
			bgmMesh.aModels.push_back(aModels.size());
			aModels.push_back(model);
		}

		aBGMMeshes.push_back(bgmMesh);
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
		auto newMat = GetMapMaterialFromFBX(fbxMaterial, isStaticModel);
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
		auto mat = aMaterials[surface->nMaterialId];
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
	surface->nPolyMode = 4;
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

tSurface* ReplaceNextAvailableSurface(aiNode* node, aiMesh* mesh, bool allowModelSurfaces) {
	for (auto& surface: aSurfaces) {
		if (!surface._bIsReplacedMapSurface) continue;
		if (surface._pReplacedMapSurfaceMesh == mesh) return &surface;
	}

	for (auto& surface: aSurfaces) {
		if (!allowModelSurfaces && surface._nNumReferencesByType[SURFACE_REFERENCE_MODEL] > 0) continue;
		if (surface.nNumStreamsUsed != 2) continue;
		if (surface._bIsReplacedMapSurface) continue;

		ImportSurfaceFromFBX(&surface, node, !allowModelSurfaces, mesh);
		return &surface;
	}
	return nullptr;
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

	std::vector<tSurface*> surfaces;
	for (auto& mesh : aMeshes) {
		aabb.mMin.x = std::min(aabb.mMin.x, mesh->mAABB.mMin.x);
		aabb.mMin.y = std::min(aabb.mMin.y, mesh->mAABB.mMin.y);
		aabb.mMin.z = std::min(aabb.mMin.z, mesh->mAABB.mMin.z);
		aabb.mMax.x = std::max(aabb.mMax.x, mesh->mAABB.mMax.x);
		aabb.mMax.y = std::max(aabb.mMax.y, mesh->mAABB.mMax.y);
		aabb.mMax.z = std::max(aabb.mMax.z, mesh->mAABB.mMax.z);

		auto surface = ReplaceNextAvailableSurface(node, mesh, true);
		if (!surface) continue;
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
	for (auto& surface : surfaces) {
		model.aSurfaces.push_back(surface - &aSurfaces[0]);
	}
	aModels.push_back(model);
	return &aModels[aModels.size() - 1];
}

std::string GetPropDynamicObjectByName(const std::string& propName) {
	if (nExportFileVersion >= 0x20000) {
		if (propName.starts_with("dyn_streetlight")) return "metal_trafficlightpole";
		if (propName.ends_with("traffictraficlightpole_")) return "metal_trafficlightpole";
		if (propName.ends_with("traffictraficlightpole_01")) return "metal_trafficlightpole";
		if (propName.ends_with("traffictraficlightpole_02")) return "metal_trafficlightpole";
		if (propName.starts_with("dyn_traficlightpole") && propName.ends_with("lights_")) return "metal_light";
		if (propName.starts_with("dyn_traficlightpole") && propName.ends_with("lights_01")) return "metal_light";
		if (propName.starts_with("dyn_traficlightpole") && propName.ends_with("lights_02")) return "metal_light";
		if (propName.starts_with("dyn_parkingmeter")) return "metal_light";
		if (propName.find("ladder") != std::string::npos) return "metal_medium";
		if (propName.starts_with("dyn_cardboardbox")) return "wood_light";
		if (propName.starts_with("dyn_tent_table")) return "wood_light";
		if (propName.find("bench") != std::string::npos) return "wood_light";
		if (propName.starts_with("dyn_marketsign")) return "wood_light";
		if (propName.starts_with("dyn_markettable")) return "wood_light";
		if (propName.starts_with("dyn_woodbox")) return "wood_light";
		if (propName.starts_with("dyn_fruit")) return "rubber_cone";
		if (propName.starts_with("dyn_chair")) return "plastic_light";
		if (propName.starts_with("dyn_trashcan")) return "metal_medium";
		if (propName.starts_with("dyn_vendingmachine")) return "metal_heavy";
		if (propName.starts_with("obj_barn")) return "wood_obstacle";
		if (propName.find("rustplate") != std::string::npos) return "wood_light";
		if (propName.find("_pole_") != std::string::npos) return "wood_obstacle";
		if (propName.find("gaspump") != std::string::npos) return "explosive_gaspump";
		if (propName.find("_tank_") != std::string::npos) return "metal_watertank";
		if (propName.starts_with("dyna_watertankhut")) return "metal_medium";
		if (propName.starts_with("dyn_fence")) return "fence_wood";
		if (propName.starts_with("dyn_electricpole")) return "wood_electricpole";
		if (propName.starts_with("dyn_car_")) return "metal_car";
		if (propName.starts_with("dyn_trashbag")) return "plastic_light";
		if (propName.starts_with("dyn_parasol")) return "plastic_light";
		if (propName.starts_with("dyn_sunscreen")) return "plastic_light";
		if (propName.starts_with("dyn_gasbottle")) return "exploding_gasbottle";
		if (propName.starts_with("dyn_roadblock_")) return "metal_light";
		if (propName.find("shelter") != std::string::npos) return "metal_medium";
		if (propName.find("_plank_") != std::string::npos) return "wood_obstacle";
		if (propName.find("metalbar") != std::string::npos) return "metal_light";
		if (propName.starts_with("dyn_bucket")) return "metal_light";
		if (propName.starts_with("dyn_warning_triangle")) return "wood_light";
		if (propName.starts_with("dyna_flap")) return "plastic_light";
		if (propName.starts_with("dyn_shovel")) return "metal_light";
		if (propName.starts_with("dyn_wheelbarrow")) return "metal_light";
		if (propName.starts_with("dyn_trafficsign")) return "metal_light";
		if (propName.starts_with("obj_toolshed")) return "wood_light";
		if (propName.starts_with("dyn_sign_")) return "wood_light";
		if (propName.starts_with("dyn_postbox")) return "metal_light";
		if (propName.find("porche") != std::string::npos) return "wood_light";
		if (propName.starts_with("dyn_watertower") && propName.find("_leg_") != std::string::npos) return "metal_obstacle";
		if (propName.find("_step_") != std::string::npos) return "wood_light";
		if (propName.starts_with("dyn_marketpost")) return "wood_light";
		if (propName.find("_tire_") != std::string::npos) return "rubber_tire";
		if (propName.find("_bumper_") != std::string::npos) return "metal_medium";
		if (propName.starts_with("obj_house_")) return "wood_light";
		if (propName.find("_garagedoor_") != std::string::npos) return "wood_heavy";
		if (propName.starts_with("dyn_motelsignpost")) return "metal_medium";
		if (propName.starts_with("dyna_house_")) return "wood_light";
	}
	else {
		if (propName.starts_with("dyn_streetlight")) return "metal_streetlight";
		if (propName.ends_with("traffictraficlightpole_")) return "metal_streetlight";
		if (propName.ends_with("traffictraficlightpole_01")) return "metal_streetlight";
		if (propName.ends_with("traffictraficlightpole_02")) return "metal_streetlight";
		if (propName.starts_with("dyn_traficlightpole") && propName.ends_with("lights_")) return "metal_streetlight_box";
		if (propName.starts_with("dyn_traficlightpole") && propName.ends_with("lights_01")) return "metal_streetlight_box";
		if (propName.starts_with("dyn_traficlightpole") && propName.ends_with("lights_02")) return "metal_streetlight_box";
		if (propName.starts_with("dyn_parkingmeter")) return "metal_parkingmeter";
		if (propName.find("ladder") != std::string::npos) return "metal_ladder_medium";
		if (propName.starts_with("dyn_cardboardbox")) return "wood_market_cardboardbox_medium";
		if (propName.starts_with("dyn_tent_table")) return "wood_table";
		if (propName.find("bench") != std::string::npos) return "wood_bench";
		if (propName.starts_with("dyn_marketsign")) return "wood_market_sign_pole";
		if (propName.starts_with("dyn_markettable")) return "wood_market_table_top";
		if (propName.starts_with("dyn_woodbox")) return "wood_market_fruitholder";
		if (propName.starts_with("dyn_fruit")) return "wood_market_fruit_medium";
		if (propName.starts_with("dyn_chair")) return "wood_market_chair";
		if (propName.starts_with("dyn_trashcan")) return "metal_trashcan_medium";
		if (propName.starts_with("dyn_vendingmachine")) return "metal_vendingmachine";
		if (propName.starts_with("obj_barn")) return "wood_board_small";
		if (propName.find("rustplate") != std::string::npos) return "wood_plank_light";
		if (propName.find("_pole_") != std::string::npos) return "wood_pole_tall";
		if (propName.find("gaspump") != std::string::npos) return "metal_gas_pump";
		if (propName.find("_tank_") != std::string::npos) return "metal_watertank";
		if (propName.starts_with("dyna_watertankhut")) return "metal_watertank_leg";
		if (propName.starts_with("dyn_fence")) return "wood_board_small";
		if (propName.starts_with("dyn_electricpole")) return "wood_electricpole_tilt";
		if (propName.starts_with("dyn_car_")) return "metal_car_roadrunner";
		if (propName.starts_with("dyn_trashbag")) return "plastic_trashbag";
		if (propName.starts_with("dyn_parasol")) return "wood_market_parasol_top";
		if (propName.starts_with("dyn_sunscreen")) return "wood_market_marquee";
		if (propName.starts_with("dyn_gasbottle")) return "metal_gasbottle_medium";
		if (propName.starts_with("dyn_roadblock_")) return "metal_fence_leg_light";
		if (propName.find("shelter") != std::string::npos) return "metal_motel_roof_medium";
		if (propName.find("_plank_") != std::string::npos) return "wood_plank_medium";
		if (propName.find("metalbar") != std::string::npos) return "metal_stairs_tiny";
		if (propName.starts_with("dyn_bucket")) return "metal_bucket";
		if (propName.starts_with("dyn_warning_triangle")) return "wood_board_medium";
		if (propName.starts_with("dyna_flap")) return "plastic_flap";
		if (propName.starts_with("dyn_shovel")) return "metal_shovel";
		if (propName.starts_with("dyn_wheelbarrow")) return "metal_wheelbarrow";
		if (propName.starts_with("dyn_trafficsign")) return "metal_trafficsign";
		if (propName.starts_with("obj_toolshed")) return "wood_ladder_large";
		if (propName.starts_with("dyn_sign_")) return "wood_board_medium";
		if (propName.starts_with("dyn_postbox")) return "metal_postbox";
		if (propName.find("porche") != std::string::npos) return "wood_board_large";
		if (propName.starts_with("dyn_watertower") && propName.find("_leg_") != std::string::npos) return "metal_pole_huge";
		if (propName.find("_step_") != std::string::npos) return "wood_board_large";
		if (propName.starts_with("dyn_marketpost")) return "wood_market_legs";
		if (propName.find("_tire_") != std::string::npos) return "rubber_tire_extra";
		if (propName.find("_bumper_") != std::string::npos) return "metal_truck_bumper";
		if (propName.starts_with("obj_house_")) return "wood_ladder_small";
		if (propName.find("_garagedoor_") != std::string::npos) return "wood_garagedoor";
		if (propName.starts_with("dyn_motelsignpost")) return "metal_sign_huge";
		if (propName.starts_with("dyna_house_")) return "wood_board_medium";
	}
	if (propName.starts_with("dyna_workshop_")) return "metal_gate_180";
	if (propName.starts_with("dyn_concrete_block")) return "rock_obstacle";
	if (propName.starts_with("dyn_scaffold")) return "metal_light";
	if (propName.starts_with("dyn_trackside_advert")) return "metal_light";
	if (propName.starts_with("box")) return "metal_light";
	if (propName.starts_with("dyn_barrel")) return "metal_light";
	if (propName.starts_with("dyn_tire")) return "rubber_tire";
	if (propName.starts_with("dyn_cone")) return "rubber_cone";
	WriteConsole("WARNING: Prop " + propName + " has unrecognized prefix, defaulting to metal_light", LOG_WARNINGS);
	return "metal_light";
}

void WriteW32(uint32_t exportMapVersion) {
	WriteConsole("Writing output w32 file...", LOG_ALWAYS);

	nExportFileVersion = exportMapVersion;
	if ((nExportFileVersion == 0x20002 || nImportFileVersion == 0x20002 || bIsFOUCModel) && nImportFileVersion != nExportFileVersion) {
		WriteConsole("ERROR: FOUC conversions are currently not supported!", LOG_ERRORS);
		return;
	}

	std::ofstream file(sFileNameNoExt.string() + "_out.w32", std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	file.write((char*)&nExportFileVersion, 4);
	if (nExportFileVersion >= 0x20000) file.write((char*)&nSomeMapValue, 4);

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
				if (!ReplaceNextAvailableSurface(data.node, data.mesh, false)) unmatchedCount++;
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
		auto node = GetFBXNodeForObjectsArray();
		for (int i = 0; i < node->mNumChildren; i++) {
			auto prop = node->mChildren[i];
			tObject object;
			object.sName1 = prop->mName.C_Str();
			object.nFlags = 0xE0F9;
			FBXMatrixToFO2Matrix(GetFullMatrixFromCompactMeshObject(prop), object.mMatrix);
			aObjects.push_back(object);
		}
	}

	if (bImportAllPropsFromFBX) {
		aMeshDamageAssoc.clear();
		aCollidableModels.clear();
		aCompactMeshes.clear();

		auto node = GetFBXNodeForCompactMeshArray();
		for (int i = 0; i < node->mNumChildren; i++) {
			auto prop = node->mChildren[i];
			if (auto model = CreateModelFromMesh(prop)) {
				tCompactMesh mesh;
				mesh.sName1 = prop->mName.C_Str();
				mesh.sName2 = GetPropDynamicObjectByName(mesh.sName1);
				FBXMatrixToFO2Matrix(GetFullMatrixFromCompactMeshObject(prop), mesh.mMatrix);
				mesh.nGroup = -1;
				mesh.nFlags = 0xE000;
				mesh.nUnk1 = 1;
				mesh.aLODMeshIds.push_back(model - &aModels[0]);
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

				WriteConsole("Created new prop " + mesh.sName1 + " with properties from " + mesh.sName2, LOG_ALL);
			}
		}
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
		for (int i = 0; i < 16; i++) {
			file.write((char*)&aUnknownArray3[i], 4);
		}
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
				auto child = node->mChildren[i];
				if (GetCompactMeshByName(child->mName.C_Str())) continue;
				auto tmpName = (std::string)child->mName.C_Str();
				while (!GetCompactMeshByName(tmpName) && !tmpName.empty()) {
					tmpName.pop_back();
				}
				if (tmpName.empty()) continue;
				auto baseMesh = GetCompactMeshByName(tmpName);
				if (!baseMesh) continue;
				auto baseName = baseMesh->sName1; // not copying this out beforehand for the console log makes the tool crash????????

				tCompactMesh newMesh = *baseMesh;
				newMesh.nGroup = -1;
				newMesh.sName1 = child->mName.C_Str();
				FBXMatrixToFO2Matrix(GetFullMatrixFromCompactMeshObject(child), newMesh.mMatrix);
				aCompactMeshes.push_back(newMesh);
				compactMeshCount++;

				WriteConsole("Cloning " + baseName + " for new prop placement " + newMesh.sName1, LOG_ALL);
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

	uint32_t carMeshCount = aBGMMeshes.size();
	file.write((char*)&carMeshCount, 4);
	for (auto& mesh : aBGMMeshes) {
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

void WriteCrashDat(uint32_t exportMapVersion) {
	if (aCrashData.empty()) {
		WriteConsole("No crash data found, skipping crash.dat export", LOG_ALWAYS);
		return;
	}

	WriteConsole("Writing output crash.dat file...", LOG_ALWAYS);

	nExportFileVersion = exportMapVersion;

	std::ofstream file(sFileNameNoExt.string() + "_out_crash.dat", std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	int numModels = 0;
	for (auto& model : aModels) {
		if (!model.aCrashSurfaces.empty()) numModels++;
	}

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
				exit(0);
			}
			auto crashVBuffer = FindVertexBuffer(crashSurface.nStreamId[0]);
			if (!crashVBuffer) {
				WriteConsole("ERROR: Failed to find damage vertex buffer for " + model.sName, LOG_ERRORS);
				exit(0);
			}
			if (baseVBuffer->vertexSize != crashVBuffer->vertexSize) {
				WriteConsole("ERROR: " + model.sName + " has damage model with a mismatching vertex size!", LOG_ERRORS);
				exit(0);
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