void WriteMaterialToFile(std::ofstream& file, const tMaterial& material) {
	file.write((char*)&material.identifier, 4);
	file.write(material.sName.c_str(), material.sName.length() + 1);
	file.write((char*)&material.nAlpha, 4);
	file.write((char*)&material.v92, 4);
	file.write((char*)&material.nNumTextures, 4);
	file.write((char*)&material.v73, 4);
	file.write((char*)&material.v75, 4);
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
	if ((flags & 0x10) == 0) return false;
	if ((flags & 0x40) == 0) return false;
	return true;
}

void WriteVertexBufferToFile(std::ofstream& file, tVertexBuffer& buf) {
	bool bRemoveNormals = false;
	if (nExportMapVersion < 0x20000 && nExportMapVersion != nImportMapVersion && IsBufferReductionRequiredForFO1(buf.flags)) {
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
			file.write((char*)buf.data, buf.vertexCount * buf.vertexSize);
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
	if (nExportMapVersion < 0x20000 && nExportMapVersion != nImportMapVersion && IsBufferReductionRequiredForFO1(surface.nFlags)) {
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
	if (nExportMapVersion < 0x20000) {
		file.write((char*)surface.vAbsoluteCenter, 12);
		file.write((char*)surface.vRelativeCenter, 12);
	}
	if (nExportMapVersion >= 0x20002) {
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
	if (nExportMapVersion >= 0x20000) {
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
	if (nExportMapVersion >= 0x20002) {
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
			FBXMatrixToFO2Matrix( GetFullMatrixFromCompactMeshObject(fbx), mesh.mMatrix);
		}
	}

	file.write((char*)&mesh.identifier, 4);
	file.write(mesh.sName1.c_str(), mesh.sName1.length() + 1);
	file.write(mesh.sName2.c_str(), mesh.sName2.length() + 1);
	file.write((char*)&mesh.nFlags, 4);
	file.write((char*)&mesh.nGroup, 4);
	file.write((char*)mesh.mMatrix, sizeof(mesh.mMatrix));
	if (nExportMapVersion >= 0x20000) {
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

void WriteW32(uint32_t exportMapVersion) {
	WriteConsole("Writing output w32 file...");

	nExportMapVersion = exportMapVersion;
	if ((nExportMapVersion >= 0x20002 || nImportMapVersion >= 0x20002) && nImportMapVersion != nExportMapVersion) {
		WriteConsole("ERROR: FOUC conversions are currently not supported!");
		return;
	}

	std::ofstream file(sFileNameNoExt + "_out.w32", std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	file.write((char*)&nExportMapVersion, 4);
	if (nExportMapVersion >= 0x20000) file.write((char*)&nSomeMapValue, 4);

	uint32_t materialCount = aMaterials.size();
	file.write((char*)&materialCount, 4);
	for (auto& material : aMaterials) {
		WriteMaterialToFile(file, material);
	}

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
		if (bImportSurfaceDeletionFromFBX && !FindFBXNodeForSurface(&surface - &aSurfaces[0]) && !surface._nNumReferencesByType[SURFACE_REFERENCE_MODEL]) {
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

	if (nExportMapVersion < 0x20002) {
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

	if (nExportMapVersion >= 0x10004) {
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
		if (nExportMapVersion >= 0x20000) {
			file.write((char*)&tmpCount, 4); // bbox
			file.write((char*)&tmpCount, 4); // bbox assoc
		}
		file.write((char*)&tmpCount, 4); // compactmesh groups
		file.write((char*)&tmpCount, 4); // compactmesh
	}
	else {
		if (nExportMapVersion >= 0x20000) {
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
		file.write((char*)&nCompactMeshGroupCount, 4);
		file.write((char*)&compactMeshCount, 4);
		for (auto& mesh : aCompactMeshes) {
			WriteCompactMeshToFile(file, mesh);
		}
	}

	file.flush();

	WriteConsole("W32 export finished");
}