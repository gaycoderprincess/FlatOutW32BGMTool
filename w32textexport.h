void WriteW32MaterialsToText() {
	WriteFile("");
	WriteFile("Materials begin");
	WriteFile("Count: " + std::to_string(aMaterials.size()));
	WriteFile("");
	for (auto& material : aMaterials) {
		WriteFile("Material " + std::to_string(&material - &aMaterials[0]));
		WriteFile("Name: " + material.sName);
		WriteFile("nAlpha: " + std::to_string(material.nAlpha));
		if (bDumpMaterialData) {
			WriteFile("nUnknown1: " + std::to_string(material.v92));
			WriteFile("nNumTextures: " + std::to_string(material.nNumTextures));
		}
		WriteFile("nShaderId: " + std::to_string(material.nShaderId) + " (" + GetShaderName(material.nShaderId) + ")");
		WriteFile("nUseColormap: " + std::to_string(material.nUseColormap));
		if (bDumpMaterialData) {
			WriteFile("nUnknown4: " + std::to_string(material.v74));
			WriteFile("nUnknown5: " + std::to_string(material.v108[0]) + ", " + std::to_string(material.v108[1]) + ", " + std::to_string(material.v108[2]));
			WriteFile("nUnknown6: " + std::to_string(material.v109[0]) + ", " + std::to_string(material.v109[1]) + ", " + std::to_string(material.v109[2]));
			WriteFile("nUnknown7: " + std::to_string(material.v98[0]) + ", " + std::to_string(material.v98[1]) + ", " + std::to_string(material.v98[2]) + ", " + std::to_string(material.v98[3]));
			WriteFile("nUnknown8: " + std::to_string(material.v99[0]) + ", " + std::to_string(material.v99[1]) + ", " + std::to_string(material.v99[2]) + ", " + std::to_string(material.v99[3]));
			WriteFile("nUnknown9: " + std::to_string(material.v100[0]) + ", " + std::to_string(material.v100[1]) + ", " + std::to_string(material.v100[2]) + ", " + std::to_string(material.v100[3]));
			WriteFile("nUnknown10: " + std::to_string(material.v101[0]) + ", " + std::to_string(material.v101[1]) + ", " + std::to_string(material.v101[2]) + ", " + std::to_string(material.v101[3]));
			WriteFile("nUnknown11: " + std::to_string(material.v102));
		}
		WriteFile("Texture 1: " + material.sTextureNames[0]);
		WriteFile("Texture 2: " + material.sTextureNames[1]);
		WriteFile("Texture 3: " + material.sTextureNames[2]);
		WriteFile("Total references: " + std::to_string(material._nNumReferences));
		WriteFile("");
	}
	WriteFile("Materials end");
	WriteFile("");
}

void WriteW32StreamsToText() {
	WriteFile("Streams begin");
	uint32_t numStreams = aVertexBuffers.size() + aIndexBuffers.size();
	WriteFile("Count: " + std::to_string(numStreams));
	WriteFile("");
	for (int i = 0; i < numStreams; i++) {
		WriteFile("Stream " + std::to_string(i));
		for (auto& buf : aVertexBuffers) {
			if (buf.id == i) {
				WriteFile(buf.isVegetation ? "Vegetation vertex buffer" : "Vertex buffer");
				if (bIsFOUCModel) WriteFile(std::format("foucExtraFormat: {}", buf.foucExtraFormat));
				WriteFile(std::format("Vertex Size: {}", buf.vertexSize));
				WriteFile(std::format("Vertex Count: {}", buf.vertexCount));
				std::string uvFlagsReadable;
				if ((buf.flags & VERTEX_POSITION) != 0) uvFlagsReadable += "Position ";
				if ((buf.flags & VERTEX_NORMAL) != 0) uvFlagsReadable += "Normals ";
				if ((buf.flags & VERTEX_COLOR) != 0) uvFlagsReadable += "VertexColor ";
				if ((buf.flags & VERTEX_UV) != 0) uvFlagsReadable += "UVMap ";
				if ((buf.flags & VERTEX_UV2) != 0) uvFlagsReadable += "DoubleUVMap ";
				if ((buf.flags & VERTEX_INT16) != 0) uvFlagsReadable += "Int16 ";
				WriteFile(std::format("nFlags: 0x{:X} {}", buf.flags, uvFlagsReadable));

				if ((buf.flags & VERTEX_INT16) != 0) {
					if (bDumpFOUCOffsetedStreams && !buf._coordsAfterFOUCMult.empty()) {
						int counter = 0;
						std::string out;
						for (auto& pos : buf._coordsAfterFOUCMult) {
							out += std::to_string(pos);
							out += " ";
							counter++;
							if (counter == 3) {
								WriteFile(out);
								counter = 0;
								out = "";
							}
						}
					}
					else if (bDumpStreams) {
						if (buf.vertexSize == 32) {
							auto data = (tVertexDataFOUC*)buf.data;
							if (buf.origDataForFOUCExport) data = (tVertexDataFOUC*)buf.origDataForFOUCExport;
							for (int k = 0; k < buf.vertexCount; k++) {
								auto &value = data[k];
								std::string out;
								out += std::format("0x{:04X} ", (uint16_t) value.vPos[0]);
								out += std::format("0x{:04X} ", (uint16_t) value.vPos[1]);
								out += std::format("0x{:04X} ", (uint16_t) value.vPos[2]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps[0]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps[1]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps[2]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps[3]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps2[0]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps2[1]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps2[2]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps2[3]);
								out += std::format("0x{:02X} ", value.vNormals[0]);
								out += std::format("0x{:02X} ", value.vNormals[1]);
								out += std::format("0x{:02X} ", value.vNormals[2]);
								out += std::format("0x{:02X} ", value.vNormals[3]);
								out += std::format("0x{:02X} ", value.vVertexColors[0]);
								out += std::format("0x{:02X} ", value.vVertexColors[1]);
								out += std::format("0x{:02X} ", value.vVertexColors[2]);
								out += std::format("0x{:02X} ", value.vVertexColors[3]);
								out += std::format("0x{:04X} ", value.vUV1[0]);
								out += std::format("0x{:04X} ", value.vUV1[1]);
								out += std::format("0x{:04X} ", value.vUV2[0]);
								out += std::format("0x{:04X} ", value.vUV2[1]);
								WriteFile(out);
							}
						} else if (buf.vertexSize == 24) {
							auto data = (tVertexDataFOUC24*)buf.data;
							if (buf.origDataForFOUCExport) data = (tVertexDataFOUC24*)buf.origDataForFOUCExport;
							for (int k = 0; k < buf.vertexCount; k++) {
								auto &value = data[k];
								std::string out;
								out += std::format("0x{:04X} ", (uint16_t)value.vPos[0]);
								out += std::format("0x{:04X} ", (uint16_t)value.vPos[1]);
								out += std::format("0x{:04X} ", (uint16_t)value.vPos[2]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps[0]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps[1]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps[2]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps[3]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps2[0]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps2[1]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps2[2]);
								out += std::format("0x{:02X} ", value.vUnknownProllyBumpmaps2[3]);
								out += std::format("0x{:02X} ", value.vNormals[0]);
								out += std::format("0x{:02X} ", value.vNormals[1]);
								out += std::format("0x{:02X} ", value.vNormals[2]);
								out += std::format("0x{:02X} ", value.vNormals[3]);
								out += std::format("0x{:04X} ", value.vUV1[0]);
								out += std::format("0x{:04X} ", value.vUV1[1]);
								WriteFile(out);
							}
						}
						else {
							auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(uint8_t));
							auto data = (uint8_t*)buf.data;
							if (buf.origDataForFOUCExport) data = (uint8_t*)buf.origDataForFOUCExport;

							size_t j = 0;
							while (j < dataSize) {
								std::string out;
								for (int k = 0; k < buf.vertexSize / sizeof(uint8_t); k++) {
									out += std::format("0x{:02X}", *(uint8_t*)&data[j]);
									out += " ";
									j++;
								}
								WriteFile(out);
							}
						}
					}
				}
				else if (bDumpStreams) {
					auto dataSize = buf.vertexCount * (buf.vertexSize / sizeof(float));

					int nVertexColorOffset = -1;
					if ((buf.flags & 0x40) != 0) {
						nVertexColorOffset = 3;
						if ((buf.flags & 0x10) != 0) {
							nVertexColorOffset = 6;
						}
					}

					size_t j = 0;
					while (j < dataSize) {
						std::string out;
						for (int k = 0; k < buf.vertexSize / sizeof(float); k++) {
							if (k == nVertexColorOffset) {
								out += std::format("0x{:X}", *(uint32_t*)&buf.data[j]);
							} else {
								out += std::to_string(buf.data[j]);
							}
							out += " ";
							j++;
						}
						WriteFile(out);
					}
				}
			}
		}
		for (auto& buf : aIndexBuffers) {
			if (buf.id == i) {
				WriteFile("Index buffer");
				if (bIsFOUCModel) WriteFile(std::format("foucExtraFormat: {}", buf.foucExtraFormat));
				WriteFile(std::format("Index Count: {}", buf.indexCount));
				if (bDumpStreams) {
					for (int j = 0; j < buf.indexCount; j++) {
						WriteFile(std::to_string(buf.data[j]));
					}
				}
			}
		}
		WriteFile("");
	}
	WriteFile("Streams end");
	WriteFile("");
}

void WriteW32SurfacesToText() {
	WriteFile("Surfaces begin");
	WriteFile("Count: " + std::to_string(aSurfaces.size()));
	WriteFile("");
	for (auto& surface : aSurfaces) {
		WriteFile("Surface " + std::to_string(&surface - &aSurfaces[0]));
		WriteFile("nIsVegetation: " + std::to_string(surface.nIsVegetation));
		WriteFile("nMaterialId: " + std::to_string(surface.nMaterialId));
		WriteFile("nVertexCount: " + std::to_string(surface.nVertexCount));
		WriteFile(std::format("nFormat: 0x{:X}", surface.nFlags));
		WriteFile("nPolyCount: " + std::to_string(surface.nPolyCount));
		WriteFile("nPolyMode: " + std::to_string(surface.nPolyMode)); // 4-triindx or 5-tristrip
		WriteFile("nNumIndicesUsed: " + std::to_string(surface.nNumIndicesUsed));
		WriteFile("vCenter.x: " + std::to_string(surface.vCenter[0]));
		WriteFile("vCenter.y: " + std::to_string(surface.vCenter[1]));
		WriteFile("vCenter.z: " + std::to_string(surface.vCenter[2]));
		WriteFile("vRadius.x: " + std::to_string(surface.vRadius[0]));
		WriteFile("vRadius.y: " + std::to_string(surface.vRadius[1]));
		WriteFile("vRadius.z: " + std::to_string(surface.vRadius[2]));
		if (bIsFOUCModel) {
			WriteFile("foucVertexMultiplier.x: " + std::to_string(surface.foucVertexMultiplier[0]));
			WriteFile("foucVertexMultiplier.y: " + std::to_string(surface.foucVertexMultiplier[1]));
			WriteFile("foucVertexMultiplier.z: " + std::to_string(surface.foucVertexMultiplier[2]));
			WriteFile("foucVertexMultiplier.w: " + std::to_string(surface.foucVertexMultiplier[3]));
		}
		WriteFile("nNumStreamsUsed: " + std::to_string(surface.nNumStreamsUsed));
		for (int j = 0; j < surface.nNumStreamsUsed; j++) {
			WriteFile("nStreamId: " + std::to_string(surface.nStreamId[j]));
			WriteFile(std::format("nStreamOffset: 0x{:X}", surface.nStreamOffset[j]));
		}
		WriteFile("Total references: " + std::to_string(surface._nNumReferences));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_STATICBATCH]) WriteFile("Static batch references: " + std::to_string(num));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_MODEL]) WriteFile("Prop model references: " + std::to_string(num));
		//if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_2]) WriteFile("Tree mesh surface 2 references: " + std::to_string(num));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_3]) WriteFile("Tree trunk references: " + std::to_string(num));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_4]) WriteFile("Tree branch references: " + std::to_string(num));
		if (auto num = surface._nNumReferencesByType[SURFACE_REFERENCE_TREEMESH_5]) WriteFile("Tree leaf references: " + std::to_string(num));
		WriteFile("");
	}
	WriteFile("Surfaces end");
	WriteFile("");
}

void WriteW32StaticBatchesToText() {
	WriteFile("Static Batches begin");
	WriteFile("Count: " + std::to_string(aStaticBatches.size()));
	WriteFile("");
	for (auto& staticBatch : aStaticBatches) {
		WriteFile("nId1: " + std::to_string(staticBatch.nId1));
		WriteFile("nBVHId1_nSurfaceId: " + std::to_string(staticBatch.nBVHId1));
		WriteFile("nBVHId2: " + std::to_string(staticBatch.nBVHId2));
		WriteFile("nUnk: " + std::to_string(staticBatch.nUnk));
		WriteFile("vCenter.x: " + std::to_string(staticBatch.vCenter[0]));
		WriteFile("vCenter.y: " + std::to_string(staticBatch.vCenter[1]));
		WriteFile("vCenter.z: " + std::to_string(staticBatch.vCenter[2]));
		WriteFile("vRadius.x: " + std::to_string(staticBatch.vRadius[0]));
		WriteFile("vRadius.y: " + std::to_string(staticBatch.vRadius[1]));
		WriteFile("vRadius.z: " + std::to_string(staticBatch.vRadius[2]));
		WriteFile("");
	}
	WriteFile("Static Batches end");
	WriteFile("");
}

void WriteW32TreeColorsToText() {
	WriteFile("Tree Colors begin");
	WriteFile("Count: " + std::to_string(aTreeColors.size()));
	WriteFile("");
	for (auto& value : aTreeColors) {
		WriteFile(std::format("0x{:X}", value));
	}
	WriteFile("");
	WriteFile("Tree Colors end");
	WriteFile("");
}

void WriteW32TreeLODsToText() {
	WriteFile("Tree LODs begin");
	WriteFile("Count: " + std::to_string(aTreeLODs.size()));
	WriteFile("");
	for (auto& treeLod : aTreeLODs) {
		WriteFile("vPos.x: " + std::to_string(treeLod.vPos[0]));
		WriteFile("vPos.y: " + std::to_string(treeLod.vPos[1]));
		WriteFile("vPos.z: " + std::to_string(treeLod.vPos[2]));
		WriteFile("fScale.x: " + std::to_string(treeLod.fScale[0]));
		WriteFile("fScale.y: " + std::to_string(treeLod.fScale[1]));
		WriteFile(std::format("nUnknown[0]: 0x{:X}", treeLod.nValues[0]));
		WriteFile(std::format("nUnknown[1]: 0x{:X}", treeLod.nValues[1]));
		WriteFile("");
	}
	WriteFile("Tree LODs end");
	WriteFile("");
}

void WriteW32TreeMeshesToText() {
	WriteFile("Tree Meshes begin");
	WriteFile("Count: " + std::to_string(aTreeMeshes.size()));
	WriteFile("");
	for (auto& treeMesh : aTreeMeshes) {
		WriteFile("nIsBush: " + std::to_string(treeMesh.nIsBush));
		WriteFile("nUnknown2: " + std::to_string(treeMesh.nUnk2Unused));
		WriteFile("nBVHId1_nLeafSurfaceId: " + std::to_string(treeMesh.nBVHId1));
		WriteFile("nBVHId2: " + std::to_string(treeMesh.nBVHId2));
		WriteFile("mMatrix: ");
		WriteFile(std::format("{}, {}, {}, {}", treeMesh.mMatrix[0], treeMesh.mMatrix[1], treeMesh.mMatrix[2], treeMesh.mMatrix[3]));
		WriteFile(std::format("{}, {}, {}, {}", treeMesh.mMatrix[4], treeMesh.mMatrix[5], treeMesh.mMatrix[6], treeMesh.mMatrix[7]));
		WriteFile(std::format("{}, {}, {}, {}", treeMesh.mMatrix[8], treeMesh.mMatrix[9], treeMesh.mMatrix[10], treeMesh.mMatrix[11]));
		WriteFile(std::format("{}, {}, {}, {}", treeMesh.mMatrix[12], treeMesh.mMatrix[13], treeMesh.mMatrix[14], treeMesh.mMatrix[15]));
		WriteFile("fScale.x: " + std::to_string(treeMesh.fScale[0]));
		WriteFile("fScale.y: " + std::to_string(treeMesh.fScale[1]));
		WriteFile("fScale.z: " + std::to_string(treeMesh.fScale[2]));
		if (bIsFOUCModel) {
			WriteFile("nMaterialId: " + std::to_string(treeMesh.foucExtraData1[0]));
			WriteFile("foucData1[1]: " + std::to_string(treeMesh.foucExtraData1[1]));
			WriteFile("foucData1[2]: " + std::to_string(treeMesh.foucExtraData1[2]));
			WriteFile("foucData1[3]: " + std::to_string(treeMesh.foucExtraData1[3]));
			WriteFile("foucData1[4]: " + std::to_string(treeMesh.foucExtraData1[4]));
			WriteFile("foucData1[5]: " + std::to_string(treeMesh.foucExtraData1[5]));
			WriteFile("foucData1[6]: " + std::to_string(treeMesh.foucExtraData1[6]));
			WriteFile("foucData1[7]: " + std::to_string(treeMesh.foucExtraData1[7]));
			WriteFile("foucData1[8]: " + std::to_string(treeMesh.foucExtraData1[8]));
			WriteFile("foucData2[0]: " + std::to_string(treeMesh.foucExtraData2[0]));
			WriteFile("foucData2[1]: " + std::to_string(treeMesh.foucExtraData2[1]));
			WriteFile("foucData2[2]: " + std::to_string(treeMesh.foucExtraData2[2]));
			WriteFile("nSomeId1: " + std::to_string(treeMesh.foucExtraData2[3]));
			WriteFile(std::format("nSomeOffset1: {:X}", treeMesh.foucExtraData2[4]));
			WriteFile("nSomeId2: " + std::to_string(treeMesh.foucExtraData2[5]));
			WriteFile(std::format("nSomeOffset2: {:X}", treeMesh.foucExtraData2[6]));
			WriteFile("nSomeId3: " + std::to_string(treeMesh.foucExtraData2[7]));
			WriteFile(std::format("nSomeOffset3: {:X}", treeMesh.foucExtraData2[8]));
			WriteFile("nMaterialId2: " + std::to_string(treeMesh.foucExtraData3[0]));
			WriteFile("foucData3[1]: " + std::to_string(treeMesh.foucExtraData3[1]));
			WriteFile("nSomeId4: " + std::to_string(treeMesh.foucExtraData3[2]));
			WriteFile(std::format("nSomeOffset4: {:X}", (uint32_t)treeMesh.foucExtraData3[3]));
			WriteFile("nTrunkSurfaceId: " + std::to_string(treeMesh.nTrunkSurfaceId));
			WriteFile("nBranchSurfaceId: " + std::to_string(treeMesh.nBranchSurfaceId));
			WriteFile("nLeafSurfaceId: " + std::to_string(treeMesh.nLeafSurfaceId));
		}
		else {
			WriteFile("nTrunkSurfaceId: " + std::to_string(treeMesh.nTrunkSurfaceId));
			WriteFile("nBranchSurfaceId: " + std::to_string(treeMesh.nBranchSurfaceId));
			WriteFile("nLeafSurfaceId: " + std::to_string(treeMesh.nLeafSurfaceId));
			WriteFile("nColorId: " + std::to_string(treeMesh.nColorId));
			WriteFile("nLodId: " + std::to_string(treeMesh.nLodId));
			WriteFile("nMaterialId: " + std::to_string(treeMesh.nMaterialId));
		}
		WriteFile("");
	}
	WriteFile("Tree Meshes end");
	WriteFile("");
}

void WriteW32UnknownArray3ToText() {
	WriteFile("Unknown Array 3 begin");
	WriteFile("Count: " + std::to_string(aUnknownArray3.size()));
	WriteFile("");
	for (auto& value : aUnknownArray3) {
		WriteFile(std::to_string(value));
	}
	WriteFile("");
	WriteFile("Unknown Array 3 end");
	WriteFile("");
}

void WriteW32ModelsToText() {
	WriteFile("Models begin");
	WriteFile("Count: " + std::to_string(aModels.size()));
	WriteFile("");
	for (auto& model : aModels) {
		WriteFile("nUnknown1: " + std::to_string(model.nUnk));
		WriteFile("sName: " + model.sName);
		WriteFile("vCenter.x: " + std::to_string(model.vCenter[0]));
		WriteFile("vCenter.y: " + std::to_string(model.vCenter[1]));
		WriteFile("vCenter.z: " + std::to_string(model.vCenter[2]));
		WriteFile("vRadius.x: " + std::to_string(model.vRadius[0]));
		WriteFile("vRadius.y: " + std::to_string(model.vRadius[1]));
		WriteFile("vRadius.z: " + std::to_string(model.vRadius[2]));
		WriteFile("fRadius: " + std::to_string(model.fRadius)); // this is entirely skipped in the reader and instead calculated
		WriteFile("nNumSurfaces: " + std::to_string(model.aSurfaces.size()));
		for (auto& surface : model.aSurfaces) {
			WriteFile(std::to_string(surface));
		}
		WriteFile("");
	}
	WriteFile("Models end");
	WriteFile("");
}

void WriteW32ObjectsToText() {
	WriteFile("Objects begin");
	WriteFile("Count: " + std::to_string(aObjects.size()));
	WriteFile("");
	for (auto& object : aObjects) {
		WriteFile("sName: " + object.sName1);
		WriteFile("sUnknown: " + object.sName2);
		WriteFile(std::format("nFlags: 0x{:X}", object.nFlags));
		WriteFile("mMatrix: ");
		WriteFile(std::format("{}, {}, {}, {}", object.mMatrix[0], object.mMatrix[1], object.mMatrix[2], object.mMatrix[3]));
		WriteFile(std::format("{}, {}, {}, {}", object.mMatrix[4], object.mMatrix[5], object.mMatrix[6], object.mMatrix[7]));
		WriteFile(std::format("{}, {}, {}, {}", object.mMatrix[8], object.mMatrix[9], object.mMatrix[10], object.mMatrix[11]));
		WriteFile(std::format("{}, {}, {}, {}", object.mMatrix[12], object.mMatrix[13], object.mMatrix[14], object.mMatrix[15]));
		WriteFile("");
	}
	WriteFile("Objects end");
	WriteFile("");
}

void WriteW32CollidableModelsToText() {
	WriteFile("Collidable Models begin");
	WriteFile("Count: " + std::to_string(aCollidableModels.size()));
	WriteFile("");
	for (auto& col : aCollidableModels) {
		WriteFile("Model count: " + std::to_string(col.aModels.size()));
		for (auto& model : col.aModels) {
			WriteFile(std::to_string(model));
		}
		WriteFile("vCenter.x: " + std::to_string(col.vCenter[0]));
		WriteFile("vCenter.y: " + std::to_string(col.vCenter[1]));
		WriteFile("vCenter.z: " + std::to_string(col.vCenter[2]));
		WriteFile("vRadius.x: " + std::to_string(col.vRadius[0]));
		WriteFile("vRadius.y: " + std::to_string(col.vRadius[1]));
		WriteFile("vRadius.z: " + std::to_string(col.vRadius[2]));
		WriteFile("");
	}
	WriteFile("Collidable Models end");
	WriteFile("");
}

void WriteW32MeshDamageAssocToText() {
	WriteFile("Mesh Damage Associations begin");
	WriteFile("Count: " + std::to_string(aMeshDamageAssoc.size()));
	WriteFile("");
	for (auto& assoc : aMeshDamageAssoc) {
		WriteFile("sName: " + assoc.sName);
		WriteFile("nBaseModel: " + std::to_string(assoc.nIds[0]));
		WriteFile("nDamageModel: " + std::to_string(assoc.nIds[1]));
		WriteFile("");
	}
	WriteFile("Mesh Damage Associations end");
	WriteFile("");
}

void WriteW32CompactMeshesToText() {
	WriteFile("Compact Meshes begin");
	WriteFile("Group Count: " + std::to_string(nCompactMeshGroupCount));
	WriteFile("Count: " + std::to_string(aCompactMeshes.size()));
	WriteFile("");
	for (auto& mesh : aCompactMeshes) {
		WriteFile("sObjectName: " + mesh.sName1);
		WriteFile("sDynamicName: " + mesh.sName2);
		WriteFile(std::format("nFlags: 0x{:X}", mesh.nFlags));
		WriteFile("nGroup: " + std::to_string(mesh.nGroup));
		WriteFile("mMatrix: ");
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[0], mesh.mMatrix[1], mesh.mMatrix[2], mesh.mMatrix[3]));
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[4], mesh.mMatrix[5], mesh.mMatrix[6], mesh.mMatrix[7]));
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[8], mesh.mMatrix[9], mesh.mMatrix[10], mesh.mMatrix[11]));
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[12], mesh.mMatrix[13], mesh.mMatrix[14], mesh.mMatrix[15]));
		if (nImportFileVersion >= 0x20000) {
			WriteFile("nUnk1: " + std::to_string(mesh.nUnk1));
			WriteFile("nDamageAssocId: " + std::to_string(mesh.nDamageAssocId));
		}
		WriteFile("nNumLODs: " + std::to_string(mesh.aLODMeshIds.size()));
		for (auto unkValue : mesh.aLODMeshIds) {
			auto model = aModels[unkValue];
			WriteFile(std::to_string(unkValue) + " - " + model.sName);
		}
		WriteFile("");
	}
	WriteFile("Compact Meshes end");
	WriteFile("");
}

void WriteBGMMeshesToText() {
	WriteFile("BGM Meshes begin");
	WriteFile("Count: " + std::to_string(aBGMMeshes.size()));
	WriteFile("");
	for (auto& mesh : aBGMMeshes) {
		WriteFile("sObjectName: " + mesh.sName1);
		WriteFile("sModelName: " + mesh.sName2);
		WriteFile(std::format("nFlags: 0x{:X}", mesh.nFlags));
		WriteFile("nGroup: " + std::to_string(mesh.nGroup));
		WriteFile("mMatrix: ");
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[0], mesh.mMatrix[1], mesh.mMatrix[2], mesh.mMatrix[3]));
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[4], mesh.mMatrix[5], mesh.mMatrix[6], mesh.mMatrix[7]));
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[8], mesh.mMatrix[9], mesh.mMatrix[10], mesh.mMatrix[11]));
		WriteFile(std::format("{}, {}, {}, {}", mesh.mMatrix[12], mesh.mMatrix[13], mesh.mMatrix[14], mesh.mMatrix[15]));
		WriteFile("nNumModels: " + std::to_string(mesh.aModels.size()));
		for (auto unkValue : mesh.aModels) {
			auto model = aModels[unkValue];
			WriteFile(std::to_string(unkValue) + " - " + model.sName);
		}
		WriteFile("");
	}
	WriteFile("Car Meshes end");
	WriteFile("");
}

void WriteW32ToText() {
	WriteConsole("Writing text file...", LOG_ALWAYS);

	WriteFile(std::format("nFileVersion: 0x{:X} {}", nImportFileVersion, GetFileVersion(nImportFileVersion)));
	if (nImportFileVersion > 0x20000) {
		WriteFile("nSomeMapValue: " + std::to_string(nSomeMapValue));
	}

	WriteW32MaterialsToText();
	WriteW32StreamsToText();
	WriteW32SurfacesToText();
	WriteW32StaticBatchesToText();
	WriteW32TreeColorsToText();
	WriteW32TreeLODsToText();
	WriteW32TreeMeshesToText();
	WriteW32UnknownArray3ToText();
	WriteW32ModelsToText();
	WriteW32ObjectsToText();
	WriteW32CollidableModelsToText();
	WriteW32MeshDamageAssocToText();
	WriteW32CompactMeshesToText();

	WriteConsole("Text file export finished", LOG_ALWAYS);
}

void WriteBGMToText() {
	WriteConsole("Writing text file...", LOG_ALWAYS);

	WriteFile(std::format("nFileVersion: 0x{:X} {}", nImportFileVersion, GetFileVersion(nImportFileVersion)));

	for (auto& surface : aSurfaces) {
		if (!surface._nNumReferences) {
			WriteConsole("WARNING: Surface " + std::to_string(&surface - &aSurfaces[0]) + " goes unused! The game will not like this!!", LOG_WARNINGS);
		}
	}

	WriteW32MaterialsToText();
	WriteW32StreamsToText();
	WriteW32SurfacesToText();
	WriteW32ModelsToText();
	WriteBGMMeshesToText();
	WriteW32ObjectsToText();

	if (!aCrashData.empty()) {
		WriteFile("Crash data begin");
		WriteFile("Count: " + std::to_string(aCrashData.size()));
		WriteFile("");
		for (auto& crashData: aCrashData) {
			WriteFile("sName: " + crashData.sName);
			WriteFile("nNumSurfaces: " + std::to_string(crashData.aSurfaces.size()));
			WriteFile("");
			for (auto& surface: crashData.aSurfaces) {
				if (bIsFOUCModel) {
					auto& damage = surface.aCrashWeightsFOUC;
					WriteFile("Crash weights buffer");
					WriteFile(std::format("Vertex Count: {}", damage.size()));
					for (auto& value : damage) {
						std::string out;
						out += std::format("0x{:04X} ", (uint16_t)value.vCrashPos[0]);
						out += std::format("0x{:04X} ", (uint16_t)value.vCrashPos[1]);
						out += std::format("0x{:04X} ", (uint16_t)value.vCrashPos[2]);
						out += std::format("0x{:02X} ", value.vCrashUnknownProllyBumpmaps[0]);
						out += std::format("0x{:02X} ", value.vCrashUnknownProllyBumpmaps[1]);
						out += std::format("0x{:02X} ", value.vCrashUnknownProllyBumpmaps[2]);
						out += std::format("0x{:02X} ", value.vCrashUnknownProllyBumpmaps[3]);
						out += std::format("0x{:02X} ", value.vCrashUnknownProllyBumpmaps2[0]);
						out += std::format("0x{:02X} ", value.vCrashUnknownProllyBumpmaps2[1]);
						out += std::format("0x{:02X} ", value.vCrashUnknownProllyBumpmaps2[2]);
						out += std::format("0x{:02X} ", value.vCrashUnknownProllyBumpmaps2[3]);
						out += std::format("0x{:02X} ", value.vCrashNormals[0]);
						out += std::format("0x{:02X} ", value.vCrashNormals[1]);
						out += std::format("0x{:02X} ", value.vCrashNormals[2]);
						out += std::format("0x{:04X} ", value.vBaseUVs[0]);
						out += std::format("0x{:04X} ", value.vBaseUVs[1]);
						WriteFile(out);
					}
				}
				else {
					auto& buf = surface.vBuffer;
					WriteFile("Vertex buffer");
					WriteFile(std::format("Vertex Size: {}", buf.vertexSize));
					WriteFile(std::format("Vertex Count: {}", buf.vertexCount));
					if (bDumpStreams) {
						for (auto &weights: surface.aCrashWeights) {
							std::string out;
							for (int i = 0; i < 3; i++) {
								out += std::to_string(weights.vCrashPos[i]);
								out += " ";
							}
							for (int i = 0; i < 3; i++) {
								out += std::to_string(weights.vCrashNormal[i]);
								out += " ";
							}
							WriteFile(out);
						}
					}
				}
				WriteFile("");
			}
			WriteFile("");
		}
		WriteFile("Crash data end");
		WriteFile("");
	}

	WriteConsole("Text file export finished", LOG_ALWAYS);
}