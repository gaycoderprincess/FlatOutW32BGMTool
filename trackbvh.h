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

bool ParseTrackBVH() {
	if (sFileName.extension() != ".gen") {
		return false;
	}

	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
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
}

bool ReadAndEmptyTrackBVH() {
	if (sFileName.extension() != ".gen") {
		return false;
	}

	if (!ParseTrackBVH()) return false;

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

	return true;
}