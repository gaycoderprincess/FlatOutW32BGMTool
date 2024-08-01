struct tTrackBVHPrimitive {
	float vPos[3];
	float vRadius[3];

	// in staticbatch and treemesh, id1 is surfaceid, id2 is id in array
	int nId1;
	int nId2;
};
struct tTrackBVHNode {
	float vPos[3];
	float vRadius[3];
	int nUnk1;
	int nUnk2;
};
std::vector<tTrackBVHPrimitive> aBVHPrimitives;
std::vector<tTrackBVHNode> aBVHNodes;

bool ParseTrackBVH(const std::filesystem::path& fileName) {
	if (fileName.extension() != ".gen") {
		return false;
	}

	std::ifstream fin(fileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	uint32_t identifier;
	ReadFromFile(fin, &identifier, 4);
	if (identifier != 0xDEADC0DE) return false;
	uint32_t tmp;
	ReadFromFile(fin, &tmp, 4);
	if (tmp != 1) return false; // xbox beta asserts if this isn't 1
	uint32_t primCount;
	ReadFromFile(fin, &primCount, 4);
	aBVHPrimitives.reserve(primCount);
	for (int i = 0; i < primCount; i++) {
		tTrackBVHPrimitive prim;
		ReadFromFile(fin, &prim, sizeof(prim));
		aBVHPrimitives.push_back(prim);
	}
	uint32_t nodeCount;
	ReadFromFile(fin, &nodeCount, 4);
	aBVHPrimitives.reserve(nodeCount);
	for (int i = 0; i < nodeCount; i++) {
		tTrackBVHNode node;
		ReadFromFile(fin, &node, sizeof(node));
		aBVHNodes.push_back(node);
	}
	return true;
}

void WriteTrackBVH() {
	if (aBVHPrimitives.empty() && aBVHNodes.empty()) {
		WriteConsole("WARNING: No track_bvh.gen data loaded, culling data will not be exported", LOG_WARNINGS);
		return;
	}

	WriteConsole("Writing output track_bvh file...", LOG_ALWAYS);

	std::ofstream fout(sFileNameNoExt.string() + "_bvh.gen", std::ios::out | std::ios::binary);
	if (!fout.is_open()) return;

	uint32_t identifier = 0xDEADC0DE;
	fout.write((char*)&identifier, 4);
	uint32_t tmp = 1;
	fout.write((char*)&tmp, 4);
	uint32_t primCount = aBVHPrimitives.size();
	fout.write((char*)&primCount, 4);
	for (auto& prim : aBVHPrimitives) {
		fout.write((char*)prim.vPos, sizeof(prim.vPos));
		fout.write((char*)prim.vRadius, sizeof(prim.vRadius));
		fout.write((char*)&prim.nId1, sizeof(prim.nId1));
		fout.write((char*)&prim.nId2, sizeof(prim.nId2));
	}
	uint32_t nodeCount = aBVHNodes.size();
	fout.write((char*)&nodeCount, 4);
	for (auto& node : aBVHNodes) {
		fout.write((char*)node.vPos, sizeof(node.vPos));
		fout.write((char*)node.vRadius, sizeof(node.vRadius));
		fout.write((char*)&node.nUnk1, sizeof(node.nUnk1));
		fout.write((char*)&node.nUnk2, sizeof(node.nUnk2));
	}

	WriteConsole("track_bvh export finished", LOG_ALWAYS);
}

void WriteTrackBVHToText() {
	if (aBVHPrimitives.empty() && aBVHNodes.empty()) return;

	WriteFile("");
	WriteFile("BVH data begin");
	WriteFile("Primitive Count: " + std::to_string(aBVHPrimitives.size()));
	for (auto& prim : aBVHPrimitives) {
		WriteFile("vPos.x: " + std::to_string(prim.vPos[0]));
		WriteFile("vPos.y: " + std::to_string(prim.vPos[1]));
		WriteFile("vPos.z: " + std::to_string(prim.vPos[2]));
		WriteFile("vRadius.x: " + std::to_string(prim.vRadius[0]));
		WriteFile("vRadius.y: " + std::to_string(prim.vRadius[1]));
		WriteFile("vRadius.z: " + std::to_string(prim.vRadius[2]));
		WriteFile("nId1: " + std::to_string(prim.nId1));
		WriteFile("nId2: " + std::to_string(prim.nId2));
		WriteFile("");
	}
	WriteFile("");
	WriteFile("Node Count: " + std::to_string(aBVHPrimitives.size()));
	for (auto& node : aBVHNodes) {
		WriteFile("vPos.x: " + std::to_string(node.vPos[0]));
		WriteFile("vPos.y: " + std::to_string(node.vPos[1]));
		WriteFile("vPos.z: " + std::to_string(node.vPos[2]));
		WriteFile("vRadius.x: " + std::to_string(node.vRadius[0]));
		WriteFile("vRadius.y: " + std::to_string(node.vRadius[1]));
		WriteFile("vRadius.z: " + std::to_string(node.vRadius[2]));
		WriteFile("nUnknown1: " + std::to_string(node.nUnk1));
		WriteFile("nUnknown2: " + std::to_string(node.nUnk1));
		WriteFile("");
	}
}

bool ReadAndEmptyTrackBVH() {
	if (sFileName.extension() != ".gen") {
		return false;
	}

	if (!ParseTrackBVH(sFileName)) return false;

	for (auto& prim : aBVHPrimitives) {
		prim.vRadius[0] = 10000;
		prim.vRadius[1] = 10000;
		prim.vRadius[2] = 10000;
	}
	for (auto& node : aBVHNodes) {
		node.vRadius[0] = 10000;
		node.vRadius[1] = 10000;
		node.vRadius[2] = 10000;
	}

	WriteTrackBVH();
	if (bDumpIntoTextFile) {
		WriteTrackBVHToText();
	}

	return true;
}

tTrackBVHPrimitive* GetBVHPrimitiveForIDs(int id1, int id2) {
	for (auto& prim : aBVHPrimitives) {
		if (prim.nId1 == id1 && prim.nId2 == id2) return &prim;
	}
	return nullptr;
}

void UpdateTrackBVH() {
	if (aBVHPrimitives.empty() && aBVHNodes.empty()) return;

	for (auto& batch : aStaticBatches) {
		auto prim = GetBVHPrimitiveForIDs(batch.nBVHId1, batch.nBVHId2);
		if (!prim) {
			WriteConsole("ERROR: Failed to find BVH primitive for StaticBatch " + std::to_string(&batch - &aStaticBatches[0]), LOG_ERRORS);
			continue;
		}
		memcpy(prim->vPos, batch.vCenter, sizeof(prim->vPos));
		memcpy(prim->vRadius, batch.vRadius, sizeof(prim->vRadius));
	}
	for (auto& tree : aTreeMeshes) {
		auto prim = GetBVHPrimitiveForIDs(tree.nBVHId1, tree.nBVHId2);
		if (!prim) {
			WriteConsole("ERROR: Failed to find BVH primitive for TreeMesh " + std::to_string(&tree - &aTreeMeshes[0]), LOG_ERRORS);
			continue;
		}
		auto surfId = tree.nBranchSurfaceId;
		if (surfId < 0 || surfId >= aSurfaces.size()) continue;

		auto& surface = aSurfaces[surfId];
		if (surface.vRadius[0] == 0 && surface.vRadius[1] == 0 && surface.vRadius[2] == 0) continue;
		memcpy(prim->vPos, surface.vCenter, sizeof(prim->vPos));
		memcpy(prim->vRadius, surface.vRadius, sizeof(prim->vRadius));
	}

	for (auto& node : aBVHNodes) {
		node.vRadius[0] = 10000;
		node.vRadius[1] = 10000;
		node.vRadius[2] = 10000;
	}
}