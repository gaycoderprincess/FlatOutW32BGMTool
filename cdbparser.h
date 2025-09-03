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

	// nFlags:
	// trees 1
	// objects 3
	// water 6
	// ground 27

	uint8_t nFlags; // +0
	uint8_t nMaterial : 6; // +1
	uint8_t nUnk2 : 2; // +1 breaks shadows and tire collision if nulled
	uint8_t nUnk3; // +1 breaks shadows and tire collision if nulled
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
	int16_t nXPosition;
	int16_t nXSize;
	int16_t nYPosition;
	int16_t nYSize;
	int16_t nZPosition;
	int16_t nZSize;
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
	//else {
	//	ReadCollisionRegion(&regions[region->nFlags.index], regions, indices, vertices);
	//	ReadCollisionRegion(&regions[region->nFlags.index+1], regions, indices, vertices);
	//}
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
		WriteFile(std::format("nValue1: 0x{:X}", tmp));
		fin.read((char*)&tmp, 4);
		WriteFile(std::format("nValue2: 0x{:X}", tmp));
		fin.read((char*)&tmp, 4);
		WriteFile(std::format("nVertexSize: 0x{:X}", tmp));

		auto vertices = new uint8_t[tmp];
		fin.read((char*)vertices, tmp);

		for (int i = 0; i < tmp / sizeof(tFO1CollisionVertex); i++) {
			auto fData = (tFO1CollisionVertex*)vertices;
			WriteFile(std::format("{} {} {} {} {}", fData[i].fPosition[0], fData[i].fPosition[1], fData[i].fPosition[2], fData[i].fMultipliers[0], fData[i].fMultipliers[1]));
		}

		fin.read((char*)&tmp, 4);
		WriteFile(std::format("nIndexCount: {}", tmp));

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
				WriteFile(std::format("{} {} {} flags {} material {} unk {} {}", nData[i].nVertex1.Get(), nData[i].nVertex2.Get(), nData[i].nVertex3.Get(), nData[i].nFlags, (int)nData[i].nMaterial, (int)nData[i].nUnk2, nData[i].nUnk3));
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
		WriteFile(std::format("nRegionCount: {}", tmp));

		auto region = new uint8_t[tmp * 16];
		fin.read((char*)region, tmp * 16);
		for (int i = 0; i < tmp; i++) {
			auto nData = (tFO1CollisionRegion*)region;
			WriteFile(std::format("indexCount {} hasIndices {} index {} unk1 {} unk2 {}", (int)nData[i].nFlags.indexCount, (int)nData[i].nFlags.hasIndices, (int)nData[i].nFlags.index, (int)nData[i].nFlags.unk1, nData[i].nFlags.unk2));
			WriteFile(std::format("extents {} {} {} {} {} {}", nData[i].nXPosition, nData[i].nXSize, nData[i].nYPosition, nData[i].nYSize, nData[i].nZPosition, nData[i].nZSize));
		}

		int numMaterials = 31;

		//tExportCollisionRegion oneRegion;
		//for (int i = 0; i < numIndices; i++) {
		//	auto nData = (tFO1CollisionIndex*)indices;
		//	if (nData->nFlags > 64) {
		//		WriteConsole(std::format("flags found {}", nData->nFlags), LOG_ALWAYS);
		//	}
		//	//oneRegion.aIndices.push_back(nData[i]);
		//}
		for (int i = 0; i < numMaterials; i++) {
			tExportCollisionRegion matRegion;
			for (int j = 0; j < numIndices; j++) {
				auto nData = (tFO1CollisionIndex*)indices;
				if (nData[j].nMaterial != i) continue;
				matRegion.aIndices.push_back(nData[j]);
			}
			if (matRegion.aIndices.empty()) continue;
			aCollisionRegions.push_back(matRegion);
		}
		//aCollisionRegions.push_back(oneRegion);

		//for (int i = 0; i < tmp; i++) {
		//	auto nData = (tFO1CollisionRegion*)region;
		//	ReadCollisionRegion(&nData[i], (tFO1CollisionRegion*)region, (tFO1CollisionIndex*)indices, (tFO1CollisionVertex*)vertices);
		//}

		//vCoordMultipliers1.x: 90.79545
		//vCoordMultipliers1.y: 715.3839
		//vCoordMultipliers1.z: 91.06016
		//vCoordMultipliers2.x: 90.79268
		//vCoordMultipliers2.y: 715.3621
		//vCoordMultipliers2.z: 91.05737
		//vCoordMultipliersInv1.x: 0.011013768
		//vCoordMultipliersInv1.y: 0.0013978508
		//vCoordMultipliersInv1.z: 0.010981752
		//vCoordMultipliersInv2.x: 0.011014104
		//vCoordMultipliersInv2.y: 0.0013978934
		//vCoordMultipliersInv2.z: 0.010982087

		aiScene scene;
		scene.mRootNode = new aiNode();

		// materials
		scene.mMaterials = new aiMaterial*[numMaterials];
		for (int i = 0; i < numMaterials; i++) {
			const char* materialNames[] = {
				"NoCollision",
				"Tarmac (Road)",
				"Tarmac Mark (Road)",
				"Hard (Road)",
				"Hard Mark (Road)",
				"Medium (Road)",
				"Medium Mark (Road)",
				"Soft (Road)",
				"Soft Mark (Road)",
				"Ice (Road)",
				"Ice Mark (Road)",
				"Snow (Road)",
				"Snow Mark (Road)",
				"Bank Sand (terrain)",
				"Grass (terrain)",
				"Forest (terrain)",
				"Sand (terrain)",
				"Rock (terrain)",
				"Mould (terrain)",
				"Snow  (terrain)",
				"Concrete (Object)",
				"Rock (Object)",
				"Metal (Object)",
				"Wood (Object)",
				"Tree (Object)",
				"Bush",
				"Rubber (Object)",
				"Water",
				"No Camera Col",
				"Reset",
				"Camera only col",
			};

			scene.mMaterials[i] = new aiMaterial();
			aiString matName(materialNames[i]);
			scene.mMaterials[i]->AddProperty(&matName, AI_MATKEY_NAME);
		}
		scene.mNumMaterials = numMaterials;

		scene.mMeshes = new aiMesh*[aCollisionRegions.size()];
		scene.mNumMeshes = aCollisionRegions.size();

		for (int i = 0; i < aCollisionRegions.size(); i++) {
			auto region = &aCollisionRegions[i];
			if (region->aIndices.empty()) {
				WriteConsole(std::format("ERROR: region is empty!"), LOG_ERRORS);
				return false;
			}

			int numVertices = region->aIndices.size()*3;
			int currentVertex = 0;

			auto mesh = new aiMesh;
			scene.mMeshes[i] = mesh;
			mesh->mVertices = new aiVector3D[numVertices];
			mesh->mNumVertices = numVertices;
			mesh->mMaterialIndex = region->aIndices[0].nMaterial;

			mesh->mFaces = new aiFace[region->aIndices.size()];
			mesh->mNumFaces = region->aIndices.size();
			for (int j = 0; j < region->aIndices.size(); j++) {
				auto index = region->aIndices[j];
				auto vertex1 = (float*)(&vertices[index.nVertex1.Get()*4]);
				auto vertex2 = (float*)(&vertices[index.nVertex2.Get()*4]);
				auto vertex3 = (float*)(&vertices[index.nVertex3.Get()*4]);
				mesh->mVertices[currentVertex].x = vertex1[0];
				mesh->mVertices[currentVertex].y = vertex1[1];
				mesh->mVertices[currentVertex].z = vertex1[2];
				mesh->mVertices[currentVertex + 1].x = vertex2[0];
				mesh->mVertices[currentVertex + 1].y = vertex2[1];
				mesh->mVertices[currentVertex + 1].z = vertex2[2];
				mesh->mVertices[currentVertex + 2].x = vertex3[0];
				mesh->mVertices[currentVertex + 2].y = vertex3[1];
				mesh->mVertices[currentVertex + 2].z = vertex3[2];

				mesh->mFaces[j].mIndices = new uint32_t[3];
				mesh->mFaces[j].mIndices[0] = currentVertex++;
				mesh->mFaces[j].mIndices[1] = currentVertex++;
				mesh->mFaces[j].mIndices[2] = currentVertex++;
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