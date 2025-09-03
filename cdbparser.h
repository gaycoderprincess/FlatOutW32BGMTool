bool bCDBIsRetroDemo = false;

struct tCDB2Header {
	uint32_t vBBMin[3];
	uint32_t vBBMax[3];
	float vCoordMultipliers[3];
	float vCoordMultipliersInv[3];
	uint32_t nTriOffset;
	uint32_t nVertOffset;
};
static_assert(sizeof(tCDB2Header) == 64 - 8);

struct __attribute__((packed)) tFO1CollisionIndex {
	struct tInt24 {
		uint8_t data[3];

		const uint32_t Get() {
			uint32_t value = data[0];
			value += data[1] * 256;
			value += data[2] * 65536;
			return value;
		}
		void Set(const uint32_t value) {
			memcpy(data, &value, 3);
		}
	};

	// unknownid:
	// byte_68BDF4[v7 >> 11]
	// byte_68BDF4[(v7 >> 6) & 0x1F]

	uint8_t nFlags; // +0
	uint16_t nUnknownId; // +1 breaks shadows and tire collision if nulled
	tInt24 nVertex1; // +3
	tInt24 nVertex2; // +6
	tInt24 nVertex3; // +9

	int GetFirstVertex() {
		auto ptr8 = (uint8_t*)this;
		auto ptr32 = (uint32_t*)this;
		return (ptr8[3] | ((uint16_t)(ptr32[1] << 8)));
	}
	int GetSecondVertex() {
		auto ptr16 = (uint16_t*)this;
		auto ptr32 = (uint32_t*)this;
		return (ptr16[3] | ((uint8_t)(ptr32[2] << 16)));
	}
	int GetThirdVertex() {
		auto ptr32 = (uint32_t*)this;
		return ((ptr32[2] >> 6) & 0x3FFFFFC) / 4;
	}
};
static_assert(sizeof(tFO1CollisionIndex) == 0xC);

/*
-15.397871 -35.542915 -305.60184 10526 49537
-35.601063 -305.89038 -15.711217 26493 49678
*/

struct tFO1CollisionVertex {
	float fPosition[3]; // +0
	uint16_t fMultipliers[2]; // +C * 0.000015259022
};
static_assert(sizeof(tFO1CollisionVertex) == 0x10);

struct tFO1CollisionRegion {
	// v77 = ((nFlags >> 5) & 0x7F) + 1
	// v24 = v76 + 12 * (nFlags >> 12);

	// 0887007B
	// gets me 00000004 for count and 8870 for id
	// 23A40020 23AA6560

	// 0886C07B
	// shr 00443603
	// and 00000003

	// first byte is 3 bytes
	struct __attribute__((packed)) tFlags {
		uint8_t indexCount : 3;
		uint8_t hasIndices : 1; // (v20 & 0x10) != 0 otherwise used as next region index
		uint8_t unk1 : 4;
		uint16_t index;
		uint8_t unk2;
	} nFlags;
	uint16_t nExtents[6]; // 4 6 8 10 12 14
};
static_assert(sizeof(tFO1CollisionRegion) == 0x10);

struct tExportCollisionRegion {
	std::vector<tFO1CollisionIndex> aIndices;
};
std::vector<tExportCollisionRegion> aCollisionRegions;
void ReadCollisionRegion(const tFO1CollisionRegion* region, const tFO1CollisionRegion* regions, const tFO1CollisionIndex* indices, const tFO1CollisionVertex* vertices) {
	if (region->nFlags.hasIndices) {
		tExportCollisionRegion tmp;
		for (int i = 0; i < region->nFlags.indexCount + 1; i++) {
			auto index = indices[region->nFlags.index + i];
			tmp.aIndices.push_back(index);
		}
		aCollisionRegions.push_back(tmp);
	}
	else {
		ReadCollisionRegion(&regions[region->nFlags.index], regions, indices, vertices);
		ReadCollisionRegion(&regions[region->nFlags.index+1], regions, indices, vertices);
	}
}

bool ParseTrackCDB(const std::filesystem::path& fileName) {
	WriteConsole("Parsing CDB data...", LOG_ALWAYS);

	if (fileName.extension() != ".gen") {
		return false;
	}

	std::ifstream fin(fileName, std::ios::in | std::ios::binary);
	if (!fin.is_open()) return false;

	uint32_t identifier;
	uint32_t dateIdentifier;
	fin.read((char*)&identifier, 4);
	fin.read((char*)&dateIdentifier, 4);
	WriteFile(std::format("nIdentifier: {:X}", identifier));
	WriteFile(std::format("nDateIdentifier: {:X}", dateIdentifier));

	if (identifier == 0x62626161) { // "aabb"
		uint32_t tmp;
		fin.read((char*)&tmp, 4);
		WriteFile(std::format("nValue1: {:X}", tmp));
		fin.read((char*)&tmp, 4);
		WriteFile(std::format("nValue2: {:X}", tmp));
		fin.read((char*)&tmp, 4);
		WriteFile(std::format("nVertexSize: {:X}", tmp));

		auto vertices = new uint8_t[tmp];
		fin.read((char*)vertices, tmp);

		if (tmp % sizeof(tFO1CollisionVertex) != 0) {
			WriteConsole(std::format("Truncated {} bytes from vertex array", (tmp % sizeof(tFO1CollisionVertex))), LOG_ALWAYS);
			tmp -= (tmp % sizeof(tFO1CollisionVertex));
		}

		for (int i = 0; i < tmp / sizeof(tFO1CollisionVertex); i++) {
			auto fData = (tFO1CollisionVertex*)vertices;
			WriteFile(std::format("{} {} {} {} {}", fData[i].fPosition[0], fData[i].fPosition[1], fData[i].fPosition[2], fData[i].fMultipliers[0], fData[i].fMultipliers[1]));
		}
		uint32_t numVertices = tmp / sizeof(tFO1CollisionVertex);

		fin.read((char*)&tmp, 4);
		WriteFile(std::format("nIndexSize: {:X}", tmp));

		uint32_t data2Size = tmp * 12;
		if (bCDBIsRetroDemo) data2Size = tmp * 16;
		auto indices = new uint8_t[data2Size];
		fin.read((char*)indices, data2Size);

		uint32_t numIndices = tmp;
		if (bCDBIsRetroDemo) {
			for (int i = 0; i < data2Size / sizeof(float); i++) {
				auto nData = (uint32_t*)indices;
				WriteFile(std::format("{:X}", nData[i]));
			}
		}
		else {
			for (int i = 0; i < numIndices; i++) {
				auto nData = (tFO1CollisionIndex*)indices;
				auto vertData = (tFO1CollisionVertex*)vertices;
				WriteFile(std::format("{} {} {} flags {} unk {}", nData[i].nVertex1.Get(), nData[i].nVertex2.Get(), nData[i].nVertex3.Get(), nData[i].nFlags, (int)nData[i].nUnknownId));
			}
		}

		float values[6];
		fin.read((char*)values, sizeof(values));
		WriteFile(std::format("vCenter.x: {}", values[0]));
		WriteFile(std::format("vCenter.y: {}", values[1]));
		WriteFile(std::format("vCenter.z: {}", values[2]));
		WriteFile(std::format("vRadius.x: {}", values[3]));
		WriteFile(std::format("vRadius.y: {}", values[4]));
		WriteFile(std::format("vRadius.z: {}", values[5]));

		float values2[3];
		fin.read((char*)values2, sizeof(values2));
		WriteFile(std::format("vCoordMultipliers1.x: {}", values2[0]));
		WriteFile(std::format("vCoordMultipliers1.y: {}", values2[1]));
		WriteFile(std::format("vCoordMultipliers1.z: {}", values2[2]));
		fin.read((char*)values2, sizeof(values2));
		WriteFile(std::format("vCoordMultipliers2.x: {}", values2[0]));
		WriteFile(std::format("vCoordMultipliers2.y: {}", values2[1]));
		WriteFile(std::format("vCoordMultipliers2.z: {}", values2[2]));
		fin.read((char*)values2, sizeof(values2));
		WriteFile(std::format("vCoordMultipliersInv1.x: {}", values2[0]));
		WriteFile(std::format("vCoordMultipliersInv1.y: {}", values2[1]));
		WriteFile(std::format("vCoordMultipliersInv1.z: {}", values2[2]));
		fin.read((char*)values2, sizeof(values2));
		WriteFile(std::format("vCoordMultipliersInv2.x: {}", values2[0]));
		WriteFile(std::format("vCoordMultipliersInv2.y: {}", values2[1]));
		WriteFile(std::format("vCoordMultipliersInv2.z: {}", values2[2]));

		fin.read((char*)&tmp, 4);
		WriteFile(std::format("nSomeData3Size: {:X}", tmp));

		auto region = new uint8_t[tmp * 16];
		fin.read((char*)region, tmp * 16);
		for (int i = 0; i < tmp; i++) {
			auto nData = (tFO1CollisionRegion*)region;
			WriteFile(std::format("indexCount {} hasIndices {} index {} unk1 {} unk2 {}", (int)nData[i].nFlags.indexCount, (int)nData[i].nFlags.hasIndices, (int)nData[i].nFlags.index, (int)nData[i].nFlags.unk1, nData[i].nFlags.unk2));
			WriteFile(std::format("extents {} {} {} {} {} {}", nData[i].nExtents[0], nData[i].nExtents[1], nData[i].nExtents[2], nData[i].nExtents[3], nData[i].nExtents[4], nData[i].nExtents[5]));
		}

		ReadCollisionRegion((tFO1CollisionRegion*)region, (tFO1CollisionRegion*)region, (tFO1CollisionIndex*)indices, (tFO1CollisionVertex*)vertices);

		aiScene scene;
		scene.mRootNode = new aiNode();

		// materials
		scene.mMaterials = new aiMaterial*[1];
		scene.mMaterials[0] = new aiMaterial();
		scene.mNumMaterials = 1;

		scene.mMeshes = new aiMesh*[aCollisionRegions.size()];
		scene.mNumMeshes = aCollisionRegions.size();

		for (int i = 0; i < aCollisionRegions.size(); i++) {
			auto region = &aCollisionRegions[i];
			if (region->aIndices.empty()) {
				WriteConsole(std::format("ERROR: region is empty!"), LOG_ERRORS);
				return false;
			}

			auto mesh = new aiMesh;
			scene.mMeshes[i] = mesh;
			mesh->mVertices = new aiVector3D[numVertices];
			mesh->mNumVertices = numVertices;
			for (int j = 0; j < numVertices; j++) {
				auto fData = (tFO1CollisionVertex*)vertices;
				mesh->mVertices[j].x = fData[j].fPosition[0];
				mesh->mVertices[j].y = fData[j].fPosition[1];
				mesh->mVertices[j].z = fData[j].fPosition[2];
			}

			mesh->mFaces = new aiFace[region->aIndices.size()];
			mesh->mNumFaces = region->aIndices.size();
			for (int j = 0; j < region->aIndices.size(); j++) {
				auto index = region->aIndices[j];
				mesh->mFaces[j].mIndices = new uint32_t[3];
				mesh->mFaces[j].mIndices[0] = index.GetFirstVertex();
				mesh->mFaces[j].mIndices[1] = index.GetSecondVertex();
				mesh->mFaces[j].mIndices[2] = index.GetThirdVertex();
				mesh->mFaces[j].mNumIndices = 3;

				if (mesh->mFaces[j].mIndices[0] < 0 || mesh->mFaces[j].mIndices[0] >= mesh->mNumVertices) {
					WriteConsole(std::format("ERROR: Face {} is out of bounds! (index 0 is {}/{})", j, mesh->mFaces[j].mIndices[0], mesh->mNumVertices), LOG_ERRORS);
					WaitAndExitOnFail();
				}
				if (mesh->mFaces[j].mIndices[1] < 0 || mesh->mFaces[j].mIndices[1] >= mesh->mNumVertices) {
					WriteConsole(std::format("ERROR: Face {} is out of bounds! (index 1 is {}/{})", j, mesh->mFaces[j].mIndices[1], mesh->mNumVertices), LOG_ERRORS);
					WaitAndExitOnFail();
				}
				if (mesh->mFaces[j].mIndices[2] < 0 || mesh->mFaces[j].mIndices[2] >= mesh->mNumVertices) {
					WriteConsole(std::format("ERROR: Face {} is out of bounds! (index 1 is {}/{})", j, mesh->mFaces[j].mIndices[2], mesh->mNumVertices), LOG_ERRORS);
					WaitAndExitOnFail();
				}
			}

			if (auto node = new aiNode()) {
				node->mName = std::format("Mesh{}", i+1);
				node->mMeshes = new uint32_t[1];
				node->mNumMeshes = 1;
				node->mMeshes[0] = i;
				scene.mRootNode->addChildren(1, &node);
			}
		}

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
	else {
		tCDB2Header header;
		fin.read((char*)&header, sizeof(header));

		WriteFile(std::format("vBBMin.x: {:X}", header.vBBMin[0]));
		WriteFile(std::format("vBBMin.y: {:X}", header.vBBMin[1]));
		WriteFile(std::format("vBBMin.z: {:X}", header.vBBMin[2]));
		WriteFile(std::format("vBBMax.x: {:X}", header.vBBMax[0]));
		WriteFile(std::format("vBBMax.y: {:X}", header.vBBMax[1]));
		WriteFile(std::format("vBBMax.z: {:X}", header.vBBMax[2]));
		WriteFile(std::format("vCoordMultipliers.x: {}", header.vCoordMultipliers[0]));
		WriteFile(std::format("vCoordMultipliers.y: {}", header.vCoordMultipliers[1]));
		WriteFile(std::format("vCoordMultipliers.z: {}", header.vCoordMultipliers[2]));
		WriteFile(std::format("vCoordMultipliersInv.x: {}", header.vCoordMultipliersInv[0]));
		WriteFile(std::format("vCoordMultipliersInv.y: {}", header.vCoordMultipliersInv[1]));
		WriteFile(std::format("vCoordMultipliersInv.z: {}", header.vCoordMultipliersInv[2]));
		WriteFile(std::format("nTriOffset: {:X}", header.nTriOffset));
		WriteFile(std::format("nVertOffset: {:X}", header.nVertOffset));
	}

	return true;
}