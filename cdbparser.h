struct tCDB2Header {
	uint32_t vBBMin[3];
	uint32_t vBBMax[3];
	float vCoordMultipliers[3];
	float vCoordMultipliersInv[3];
	uint32_t nTriOffset;
	uint32_t nVertOffset;
};
static_assert(sizeof(tCDB2Header) == 64 - 8);

// 4C09A5 updates car light level

namespace FO1CDB {
	const char* aMaterialNames[] = {
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
		"Bush (Object)",
		"Rubber (Object)",
		"Water",
		"No Camera Col",
		"Reset",
		"Camera only col",
	};

	enum eMaterials {
		NOCOLLISION = 1,
		TARMAC_ROAD,
		TARMAC_MARK_ROAD,
		HARD_ROAD,
		HARD_MARK_ROAD,
		MEDIUM_ROAD,
		MEDIUM_MARK_ROAD,
		SOFT_ROAD,
		SOFT_MARK_ROAD,
		ICE_ROAD,
		ICE_MARK_ROAD,
		SNOW_ROAD,
		SNOW_MARK_ROAD,
		BANK_SAND_TERRAIN,
		GRASS_TERRAIN,
		FOREST_TERRAIN,
		SAND_TERRAIN,
		ROCK_TERRAIN,
		MOULD_TERRAIN,
		SNOW_TERRAIN,
		CONCRETE_OBJECT,
		ROCK_OBJECT,
		METAL_OBJECT,
		WOOD_OBJECT,
		TREE_OBJECT,
		BUSH_OBJECT,
		RUBBER_OBJECT,
		WATER,
		NO_CAMERA_COL,
		RESET,
		CAMERA_ONLY_COL,
	};

    uint8_t nWeirdPolyMatchupThing[] = {
        0,
        1,
        2,
        4,
        5,
        6,
        8,
        9,
        0xA,
        0x10,
        0x11,
        0x12,
        0x14,
        0x15,
        0x16,
        0x18,
        0x19,
        0x1A,
        0x20,
        0x21,
        0x22,
        0x24,
        0x25,
        0x26,
        0x28,
        0x29,
        0x2A,
    };
	const int nNumWeirdPolyMatchupThing = sizeof(nWeirdPolyMatchupThing)/sizeof(nWeirdPolyMatchupThing[0]);

	struct __attribute__((packed)) tCDBPoly {
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
		uint16_t nMaterialAndOtherStuff; // +1
		tInt24 nVertex1; // +3
		tInt24 nVertex2; // +6
		tInt24 nVertex3; // +9

		tCDBPoly() {
			memset(this,0,sizeof(*this));
		}

		int GetMaterial() {
			return nMaterialAndOtherStuff & 0x1F;
		}

		void SetMaterial(int i) {
			nMaterialAndOtherStuff &= ~0x1F;
			nMaterialAndOtherStuff |= i & 0x1F;
		}

		int GetPolyMinMatchup() {
			return (nMaterialAndOtherStuff >> 6) & 0x1F;
		}

		int GetPolyMaxMatchup() {
			return nMaterialAndOtherStuff >> 11;
		}

		void SetPolyMinMatchup(int i) {
			nMaterialAndOtherStuff &= ~(0x1F << 6);
			nMaterialAndOtherStuff |= (i & 0x1F) << 6;
		}
		void SetPolyMaxMatchup(int i) {
			nMaterialAndOtherStuff &= ~(0xFF << 11);
			nMaterialAndOtherStuff |= i << 11;
		}

		auto GetAABBMin(float* vertices) {
			float* coords[3] = {&vertices[nVertex1.Get()], &vertices[nVertex2.Get()], &vertices[nVertex3.Get()]};

			float x = std::min(coords[0][0], std::min(coords[1][0], coords[2][0]));
			float y = std::min(coords[0][1], std::min(coords[1][1], coords[2][1]));
			float z = std::min(coords[0][2], std::min(coords[1][2], coords[2][2]));
			return NyaVec3(x, y, z);
		}

		auto GetAABBMax(float* vertices) {
			float* coords[3] = {&vertices[nVertex1.Get()], &vertices[nVertex2.Get()], &vertices[nVertex3.Get()]};

			float x = std::max(coords[0][0], std::max(coords[1][0], coords[2][0]));
			float y = std::max(coords[0][1], std::max(coords[1][1], coords[2][1]));
			float z = std::max(coords[0][2], std::max(coords[1][2], coords[2][2]));
			return NyaVec3(x, y, z);
		}

		bool IsPolyMinMatchupValid(float* vertices) {
			auto poly1 = GetPolyMinMatchup();
			auto poly2 = GetPolyMaxMatchup();
			if (poly1 >= nNumWeirdPolyMatchupThing || poly2 >= nNumWeirdPolyMatchupThing) {
				WriteConsole(std::format("WARNING: poly matchup overflow!! {} {}, max {}", poly1, poly2, nNumWeirdPolyMatchupThing), LOG_WARNINGS);
				return true;
			}

			auto v20 = nWeirdPolyMatchupThing[poly1];
			auto x2 = v20 & 3;
			auto y2 = (v20 >> 2) & 3;
			auto z2 = (v20 >> 4) & 3;

			float* coords[3] = {&vertices[nVertex1.Get()], &vertices[nVertex2.Get()], &vertices[nVertex3.Get()]};

			float minX = std::min(coords[0][0], std::min(coords[1][0], coords[2][0]));
			float minY = std::min(coords[0][1], std::min(coords[1][1], coords[2][1]));
			float minZ = std::min(coords[0][2], std::min(coords[1][2], coords[2][2]));

			float fx2 = coords[x2][0];
			float fy2 = coords[y2][1];
			float fz2 = coords[z2][2];

			if (fx2 != minX) return false;
			if (fy2 != minY) return false;
			if (fz2 != minZ) return false;
			return true;
		}

		bool IsPolyMaxMatchupValid(float* vertices) {
			auto poly1 = GetPolyMinMatchup();
			auto poly2 = GetPolyMaxMatchup();
			if (poly1 >= nNumWeirdPolyMatchupThing || poly2 >= nNumWeirdPolyMatchupThing) return true;

			auto v19 = nWeirdPolyMatchupThing[poly2];
			auto x1 = v19 & 3;
			auto y1 = (v19 >> 2) & 3;
			auto z1 = (v19 >> 4) & 3;

			float* coords[3] = {&vertices[nVertex1.Get()], &vertices[nVertex2.Get()], &vertices[nVertex3.Get()]};

			float maxX = std::max(coords[0][0], std::max(coords[1][0], coords[2][0]));
			float maxY = std::max(coords[0][1], std::max(coords[1][1], coords[2][1]));
			float maxZ = std::max(coords[0][2], std::max(coords[1][2], coords[2][2]));

			float fx1 = coords[x1][0];
			float fy1 = coords[y1][1];
			float fz1 = coords[z1][2];

			if (fx1 != maxX) return false;
			if (fy1 != maxY) return false;
			if (fz1 != maxZ) return false;
			return true;
		}
	};
	static_assert(sizeof(tCDBPoly) == 0xC);

	/*
	-15.397871 -35.542915 -305.60184 10526 49537
	-35.601063 -305.89038 -15.711217 26493 49678
	*/

	struct tCDBVertex {
		float fPosition[3]; // +0
		uint16_t fMultipliers[2]; // +C * 0.000015259022

		tCDBVertex() {
			memset(this,0,sizeof(*this));
		}
	};
	static_assert(sizeof(tCDBVertex) == 0x10);

	struct tCDBRegion {
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
			uint32_t value;

			int GetUnknownFlag() {
				return (*(uint8_t*)&value) & 0xF;
			}

			void SetUnknownFlag(int i) {
				*(uint8_t*)&value |= i & 0xF;
			}

			int GetPolyCount() {
				return ((value >> 5) & 0x7F) + 1;
			}

			bool HasPolys() {
				return (value & 0x10) != 0;
			}

			int GetIndex() {
				return value >> 12;
			}

			void SetPolyCount(int i) {
				if (i <= 0) return;
				value |= ((i - 1) << 5) & 0x7F;
			}

			void SetHasPolys(bool b) {
				if (b) {
					value |= 0x10;
				}
				else {
					value &= ~0x10;
				}
			}

			void SetIndex(int i) {
				value &= ~(0xFFFFFFFF << 12);
				value |= i << 12;
			}
		} nFlags;
		int16_t nXPosition;
		int16_t nXSize;
		int16_t nYPosition;
		int16_t nYSize;
		int16_t nZPosition;
		int16_t nZSize;

		tCDBRegion() {
			memset(this,0,sizeof(*this));
		}

		NyaVec3 GetPosition(float coordMult[3]) {
			return NyaVec3(nXPosition * coordMult[0], nYPosition * coordMult[1], nZPosition * coordMult[2]);
		}

		NyaVec3 GetSize(float coordMult[3]) {
			return NyaVec3(nXSize * coordMult[0], nYSize * coordMult[1], nZSize * coordMult[2]);
		}

		void SetPosition(NyaVec3 v, float coordMult[3]) {
			nXPosition = v.x / coordMult[0];
			nYPosition = v.y / coordMult[1];
			nZPosition = v.z / coordMult[2];
		}

		void SetSize(NyaVec3 v, float coordMult[3]) {
			nXSize = v.x / coordMult[0];
			nYSize = v.y / coordMult[1];
			nZSize = v.z / coordMult[2];
		}
	};
	static_assert(sizeof(tCDBRegion) == 0x10);

	struct tExportCollisionRegion {
		std::vector<tCDBPoly> aPolys;
		int originalId = 0;
	};
	std::vector<tExportCollisionRegion> aCollisionRegions;

	std::vector<float> aVertices;
	std::vector<tCDBPoly> aPolys;
	std::vector<tCDBRegion> aRegions;

	void ReadCollisionRegion(tCDBRegion* region) {
		if (region->nFlags.HasPolys()) {
			tExportCollisionRegion tmp;
			tmp.originalId = region - &aRegions[0];
			for (int i = 0; i < region->nFlags.GetPolyCount(); i++) {
				auto index = aPolys[region->nFlags.GetIndex() + i];
				tmp.aPolys.push_back(index);
			}
			aCollisionRegions.push_back(tmp);
		}
		else {
			// adds an offset to the current region index
			ReadCollisionRegion(&region[region->nFlags.GetIndex()]);
			ReadCollisionRegion(&region[region->nFlags.GetIndex()+1]);
		}
	}

	int ConvertRetroDemoMaterial(int id) {
		switch (id) {
			case 1: return NOCOLLISION;
			case 2: return TARMAC_ROAD;
			case 3: return TARMAC_MARK_ROAD;
			case 4: return HARD_ROAD;
			case 5: return HARD_MARK_ROAD;
			case 6: return MEDIUM_ROAD;
			case 7: return MEDIUM_MARK_ROAD;
			case 8: return SOFT_ROAD;
			case 9: return SOFT_MARK_ROAD;
			case 10: return ICE_ROAD;
			case 11: return ICE_MARK_ROAD;
			case 12: return SNOW_ROAD;
			case 13: return SNOW_MARK_ROAD;
			case 14: return BANK_SAND_TERRAIN;
			case 15: return GRASS_TERRAIN;
			case 16: return FOREST_TERRAIN;
			case 17: return SAND_TERRAIN;
			case 18: return ROCK_TERRAIN;
			case 19: return MOULD_TERRAIN;
			case 20: return CONCRETE_OBJECT;
			case 21: return SNOW_TERRAIN;
			case 22: return ROCK_OBJECT;
			case 23: return METAL_OBJECT;
			case 24: return WOOD_OBJECT;
			case 25: return TREE_OBJECT;
			case 26: return BUSH_OBJECT;
			case 27: return RUBBER_OBJECT;
			case 28: return WATER;
			case 29: return RESET;
			default:
				return TARMAC_ROAD;
		}
	}

	void WriteToFBX() {
		int numMaterials = sizeof(aMaterialNames)/sizeof(aMaterialNames[0]);

		// use to read per material
		for (int i = 0; i < numMaterials; i++) {
			tExportCollisionRegion matRegion;
			for (auto& poly : aPolys) {
				if (poly.GetMaterial() != i) continue;
				matRegion.aPolys.push_back(poly);
			}
			if (matRegion.aPolys.empty()) continue;
			aCollisionRegions.push_back(matRegion);
		}
		// use this instead to read per collision region, for debugging
		//ReadCollisionRegion(aRegions);

		aiScene scene;
		scene.mRootNode = new aiNode();

		// materials
		scene.mMaterials = new aiMaterial*[numMaterials];
		for (int i = 0; i < numMaterials; i++) {
			scene.mMaterials[i] = new aiMaterial();
			aiString matName(aMaterialNames[i]);
			scene.mMaterials[i]->AddProperty(&matName, AI_MATKEY_NAME);
		}
		scene.mNumMaterials = numMaterials;

		scene.mMeshes = new aiMesh*[aCollisionRegions.size()];
		scene.mNumMeshes = aCollisionRegions.size();

		for (int i = 0; i < aCollisionRegions.size(); i++) {
			auto region = &aCollisionRegions[i];
			if (region->aPolys.empty()) {
				WriteConsole(std::format("ERROR: region {} is empty!", region->originalId), LOG_ERRORS);
				return;
			}

			int numVertices = region->aPolys.size()*3;
			int currentVertex = 0;

			auto mesh = new aiMesh;
			scene.mMeshes[i] = mesh;
			mesh->mVertices = new aiVector3D[numVertices];
			mesh->mNumVertices = numVertices;
			mesh->mMaterialIndex = region->aPolys[0].GetMaterial();

			mesh->mFaces = new aiFace[region->aPolys.size()];
			mesh->mNumFaces = region->aPolys.size();
			for (int j = 0; j < region->aPolys.size(); j++) {
				auto poly = region->aPolys[j];
				auto vertex1 = &aVertices[poly.nVertex1.Get()];
				auto vertex2 = &aVertices[poly.nVertex2.Get()];
				auto vertex3 = &aVertices[poly.nVertex3.Get()];
				mesh->mVertices[currentVertex].x = -vertex1[0];
				mesh->mVertices[currentVertex].y = vertex1[1];
				mesh->mVertices[currentVertex].z = vertex1[2];
				mesh->mVertices[currentVertex + 1].x = -vertex2[0];
				mesh->mVertices[currentVertex + 1].y = vertex2[1];
				mesh->mVertices[currentVertex + 1].z = vertex2[2];
				mesh->mVertices[currentVertex + 2].x = -vertex3[0];
				mesh->mVertices[currentVertex + 2].y = vertex3[1];
				mesh->mVertices[currentVertex + 2].z = vertex3[2];

				mesh->mFaces[j].mIndices = new uint32_t[3];
				mesh->mFaces[j].mIndices[2] = currentVertex++;
				mesh->mFaces[j].mIndices[1] = currentVertex++;
				mesh->mFaces[j].mIndices[0] = currentVertex++;
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
				//node->mName = std::format("Mesh{}", region->originalId);
				node->mName = std::format("Mesh{}", i+1);
				node->mMeshes = new uint32_t[1];
				node->mNumMeshes = 1;
				node->mMeshes[0] = i;
				// testing
				//auto pos = region->position;
				//node->mTransformation.a4 = pos.x;
				//node->mTransformation.b4 = pos.y;
				//node->mTransformation.c4 = pos.z;
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

	float vCenter[3];
	float vRadius[3];
	float vCoordMultipliers1[3];
	float vCoordMultipliers2[3];
	float vCoordMultipliersInv1[3];
	float vCoordMultipliersInv2[3];

	bool Parse(std::ifstream& fin) {
		uint32_t tmp;
		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nValue1: 0x{:X}", tmp));
		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nValue2: 0x{:X}", tmp));
		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nVertexSize: 0x{:X}", tmp));

		aVertices.reserve(tmp/4);
		for (int i = 0; i < tmp/4; i++) {
			float value;
			fin.read((char*)&value, 4);
			aVertices.push_back(value);
		}

		//for (int i = 0; i < tmp / sizeof(tFO1CollisionVertex); i++) {
		//	auto fData = aFO1CDBVertices;
		//	WriteFile(std::format("{} {} {} {} {}", fData[i].fPosition[0], fData[i].fPosition[1], fData[i].fPosition[2], fData[i].fMultipliers[0], fData[i].fMultipliers[1]));
		//}

		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nIndexCount: {}", tmp));

		if (bIsRetroDemoCDB) {
			aPolys.reserve(tmp);
			for (int i = 0; i < tmp; i++) {
				struct tTemp {
					uint8_t nFlags; // +0
					uint8_t nMaterial; // +1
					uint16_t nUnknown; // +2
					uint32_t nVertexOffset[3]; // +4

					int GetVertexOffset(int id) {
						return nVertexOffset[id] / 4;
					}
				} retro;
				static_assert(sizeof(tTemp) == 0x10);

				fin.read((char*)&retro, sizeof(retro));

				tCDBPoly value;
				value.nFlags = retro.nFlags;
				value.SetMaterial(ConvertRetroDemoMaterial(retro.nMaterial));
				value.nVertex1.Set(retro.GetVertexOffset(0));
				value.nVertex2.Set(retro.GetVertexOffset(1));
				value.nVertex3.Set(retro.GetVertexOffset(2));

				int tmp = 0;
				do {
					value.SetPolyMinMatchup(tmp++);
				} while (!value.IsPolyMinMatchupValid(&aVertices[0]));
				tmp = 0;
				do {

					value.SetPolyMaxMatchup(tmp++);
				} while (!value.IsPolyMaxMatchupValid(&aVertices[0]));

				aPolys.push_back(value);
			}
		}
		else {
			aPolys.reserve(tmp);
			for (int i = 0; i < tmp; i++) {
				tCDBPoly value;
				fin.read((char*)&value, sizeof(tCDBPoly));
				aPolys.push_back(value);
			}
		}

		if (bDumpIntoTextFile) {
			for (auto& nData : aPolys) {
				WriteFile(std::format("{} {} {} flags {} material {} unk {}", nData.nVertex1.Get(), nData.nVertex2.Get(), nData.nVertex3.Get(), nData.nFlags, nData.GetMaterial(), (uint32_t)nData.nMaterialAndOtherStuff));
			}
		}

		fin.read((char*)vCenter, sizeof(vCenter));
		fin.read((char*)vRadius, sizeof(vRadius));
		if (bDumpIntoTextFile) {
			WriteFile(std::format("vCenter.x: {}", vCenter[0]));
			WriteFile(std::format("vCenter.y: {}", vCenter[1]));
			WriteFile(std::format("vCenter.z: {}", vCenter[2]));
			WriteFile(std::format("vRadius.x: {}", vRadius[0]));
			WriteFile(std::format("vRadius.y: {}", vRadius[1]));
			WriteFile(std::format("vRadius.z: {}", vRadius[2]));
		}

		fin.read((char*)vCoordMultipliers1, sizeof(vCoordMultipliers1));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers1.x: {}", vCoordMultipliers1[0]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers1.y: {}", vCoordMultipliers1[1]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers1.z: {}", vCoordMultipliers1[2]));
		fin.read((char*)vCoordMultipliers2, sizeof(vCoordMultipliers2));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers2.x: {}", vCoordMultipliers2[0]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers2.y: {}", vCoordMultipliers2[1]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers2.z: {}", vCoordMultipliers2[2]));
		fin.read((char*)vCoordMultipliersInv1, sizeof(vCoordMultipliersInv1));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv1.x: {}", vCoordMultipliersInv1[0]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv1.y: {}", vCoordMultipliersInv1[1]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv1.z: {}", vCoordMultipliersInv1[2]));
		fin.read((char*)vCoordMultipliersInv2, sizeof(vCoordMultipliersInv2));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv2.x: {}", vCoordMultipliersInv2[0]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv2.y: {}", vCoordMultipliersInv2[1]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv2.z: {}", vCoordMultipliersInv2[2]));

		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nRegionCount: {}", tmp));

		aRegions.reserve(tmp);
		for (int i = 0; i < tmp; i++) {
			tCDBRegion value;
			fin.read((char*)&value, sizeof(tCDBRegion));
			aRegions.push_back(value);
		}

		if (bDumpIntoTextFile) {
			for (auto& nData : aRegions) {
				WriteFile(std::format("Region {}", &nData - &aRegions[0]));
				WriteFile(std::format("polyCount {} hasPolys {} index {} unk {}", nData.nFlags.GetPolyCount(), (int)nData.nFlags.HasPolys(), nData.nFlags.GetIndex(), nData.nFlags.GetUnknownFlag()));
				auto pos = nData.GetPosition(vCoordMultipliersInv2);
				auto size = nData.GetSize(vCoordMultipliersInv2);
				WriteFile(std::format("pos {} {} {}", pos.x, pos.y, pos.z));
				WriteFile(std::format("size {} {} {}", size.x, size.y, size.z));
			}
		}
		return true;
	}

	NyaVec3 GetAABBMin() {
		NyaVec3 min = {9999, 9999, 9999};
		for (auto& poly : aPolys) {
			auto vertex1 = &aVertices[poly.nVertex1.Get()];
			auto vertex2 = &aVertices[poly.nVertex2.Get()];
			auto vertex3 = &aVertices[poly.nVertex3.Get()];
			if (vertex1[0] < min.x) min.x = vertex1[0];
			if (vertex1[1] < min.y) min.y = vertex1[1];
			if (vertex1[2] < min.z) min.z = vertex1[2];
			if (vertex2[0] < min.x) min.x = vertex2[0];
			if (vertex2[1] < min.y) min.y = vertex2[1];
			if (vertex2[2] < min.z) min.z = vertex2[2];
			if (vertex3[0] < min.x) min.x = vertex3[0];
			if (vertex3[1] < min.y) min.y = vertex3[1];
			if (vertex3[2] < min.z) min.z = vertex3[2];
		}
		return min;
	}

	NyaVec3 GetAABBMax() {
		NyaVec3 max = {-9999, -9999, -9999};
		for (auto& poly : aPolys) {
			auto vertex1 = &aVertices[poly.nVertex1.Get()];
			auto vertex2 = &aVertices[poly.nVertex2.Get()];
			auto vertex3 = &aVertices[poly.nVertex3.Get()];
			if (vertex1[0] > max.x) max.x = vertex1[0];
			if (vertex1[1] > max.y) max.y = vertex1[1];
			if (vertex1[2] > max.z) max.z = vertex1[2];
			if (vertex2[0] > max.x) max.x = vertex2[0];
			if (vertex2[1] > max.y) max.y = vertex2[1];
			if (vertex2[2] > max.z) max.z = vertex2[2];
			if (vertex3[0] > max.x) max.x = vertex3[0];
			if (vertex3[1] > max.y) max.y = vertex3[1];
			if (vertex3[2] > max.z) max.z = vertex3[2];
		}
		return max;
	}

	void WriteToFile() {
		if (aVertices.empty()) return;
		if (aPolys.empty()) return;
		if (aRegions.empty()) return;

		WriteConsole("Writing output cdb file...", LOG_ALWAYS);

		std::ofstream file(sFileNameNoExt.string() + "_out.gen", std::ios::out | std::ios::binary );
		if (!file.is_open()) return;

		uint32_t nIdentifier = 0x62626161;
		uint32_t nDateIdentifier = 0x20020722;
		uint32_t nValue1 = 0;
		file.write((char*)&nIdentifier, 4);
		file.write((char*)&nDateIdentifier, 4);
		file.write((char*)&nValue1, 4);
		file.write((char*)&nValue1, 4); // unused

		int count = aVertices.size()*4;
		file.write((char*)&count, 4);
		file.write((char*)&aVertices[0], sizeof(aVertices[0])*aVertices.size());

		count = aPolys.size();
		file.write((char*)&count, 4);
		file.write((char*)&aPolys[0], sizeof(aPolys[0])*aPolys.size());

		if (bIsRetroDemoCDB && bConvertToFO1) {
			file.write((char*)vCenter, sizeof(vCenter));
			file.write((char*)vRadius, sizeof(vRadius));
			file.write((char*)vCoordMultipliers1, sizeof(vCoordMultipliers1));
			file.write((char*)vCoordMultipliers2, sizeof(vCoordMultipliers2));
			file.write((char*)vCoordMultipliersInv1, sizeof(vCoordMultipliersInv1));
			file.write((char*)vCoordMultipliersInv2, sizeof(vCoordMultipliersInv2));
		}
		else {
			file.write((char*)&nValue1, 4); // center x
			file.write((char*)&nValue1, 4); // center y
			file.write((char*)&nValue1, 4); // center z

			auto min = GetAABBMin();
			auto max = GetAABBMax();
			auto radius = max;
			if (std::abs(min.x) > radius.x) radius.x = std::abs(min.x);
			if (std::abs(min.y) > radius.y) radius.y = std::abs(min.y);
			if (std::abs(min.z) > radius.z) radius.z = std::abs(min.z);
			file.write((char*)&radius, sizeof(radius));

			auto mult2 = radius * (1.0 / 32767.0);
			auto mult1 = 1.0 / mult2;
			file.write((char*)&mult1, sizeof(mult1));
			file.write((char*)&mult1, sizeof(mult1));
			file.write((char*)&mult2, sizeof(mult2));
			file.write((char*)&mult2, sizeof(mult2));
		}

		count = aRegions.size();
		file.write((char*)&count, 4);
		file.write((char*)&aRegions[0], sizeof(aRegions[0])*aRegions.size());

		file.flush();

		WriteConsole("CDB export finished", LOG_ALWAYS);
	}

	struct tFO2MaterialMatchup {
		std::string matName;
		int materialId;
	};
	std::vector<tFO2MaterialMatchup> aFO2Materials = {
		{"col_1_", NOCOLLISION},
		{"col_2_", TARMAC_ROAD},
		{"col_3_", TARMAC_MARK_ROAD},
		{"col_4_", TARMAC_ROAD}, // Asphalt (Road)
		{"col_5_", TARMAC_MARK_ROAD}, // Asphalt Mark (Road)
		{"col_6_", TARMAC_ROAD}, // Cement Mark (Road)
		{"col_7_", TARMAC_MARK_ROAD}, // Cement Mark (Road)
		{"col_8_", HARD_ROAD},
		{"col_9_", HARD_MARK_ROAD},
		{"col_10_", MEDIUM_ROAD},
		{"col_11_", MEDIUM_MARK_ROAD},
		{"col_12_", SOFT_ROAD},
		{"col_13_", SOFT_MARK_ROAD},
		{"col_14_", ICE_ROAD},
		{"col_15_", ICE_MARK_ROAD},
		{"col_16_", SNOW_ROAD},
		{"col_17_", SNOW_MARK_ROAD},
		{"col_18_", MEDIUM_ROAD}, // Dirt (Road)
		{"col_19_", MEDIUM_MARK_ROAD}, // Dirt Mark (Road)
		{"col_20_", TARMAC_ROAD}, // Bridge Metal (Road)
		{"col_21_", TARMAC_ROAD}, // Bridge Wooden (Road)
		{"col_22_", TARMAC_ROAD}, // Curb (Terrain)
		{"col_23_", BANK_SAND_TERRAIN},
		{"col_24_", GRASS_TERRAIN},
		{"col_25_", FOREST_TERRAIN},
		{"col_26_", SAND_TERRAIN},
		{"col_27_", ROCK_TERRAIN},
		{"col_28_", MOULD_TERRAIN},
		{"col_29_", SNOW_TERRAIN},
		{"col_30_", FOREST_TERRAIN}, // Field (Terrain)
		{"col_31_", MOULD_TERRAIN}, // Wet (Terrain)
		{"col_32_", CONCRETE_OBJECT},
		{"col_33_", ROCK_OBJECT},
		{"col_34_", METAL_OBJECT},
		{"col_35_", WOOD_OBJECT},
		{"col_36_", TREE_OBJECT},
		{"col_37_", BUSH_OBJECT},
		{"col_38_", RUBBER_OBJECT},
		{"col_39_", WATER},
		{"col_40_", WATER}, // River (Water)
		{"col_41_", WATER}, // Puddle (Water)
		{"col_42_", NO_CAMERA_COL},
		{"col_43_", CAMERA_ONLY_COL},
		{"col_44_", RESET},
		// todo stunt materials
		{"col_49_", TARMAC_ROAD}, // Stunt Tarmac -> Tarmac (Road)
	};

	void CreateCDBFromMesh(aiMesh* mesh, aiMaterial* material) {
		int materialId = -1;
		std::string matName = material->GetName().C_Str();
		for (auto& material : aMaterialNames) {
			if (matName == material) materialId = &material - &aMaterialNames[0];
		}
		for (auto& material : aFO2Materials) {
			if (matName.starts_with(material.matName)) materialId = material.materialId;
		}
		if (materialId == -1) {
			WriteConsole(std::format("WARNING: Failed to find material for {}, defaulting to tarmac", matName), LOG_WARNINGS);
			materialId = 1;
		}
		for (int i = 0; i < mesh->mNumFaces; i++) {
			auto face = &mesh->mFaces[i];
			tCDBPoly poly;
			poly.nFlags = 27;
			if (materialId >= 21 && materialId <= 27) poly.nFlags = 3; // object
			if (materialId == 25) poly.nFlags = 1; // tree
			if (materialId == 28) poly.nFlags = 6; // water
			poly.SetMaterial(materialId); // todo
			poly.nVertex1.Set(aVertices.size());
			poly.nVertex2.Set(aVertices.size()+3);
			poly.nVertex3.Set(aVertices.size()+6);

			aVertices.push_back(mesh->mVertices[face->mIndices[2]].x);
			aVertices.push_back(mesh->mVertices[face->mIndices[2]].y);
			aVertices.push_back(-mesh->mVertices[face->mIndices[2]].z);
			aVertices.push_back(mesh->mVertices[face->mIndices[1]].x);
			aVertices.push_back(mesh->mVertices[face->mIndices[1]].y);
			aVertices.push_back(-mesh->mVertices[face->mIndices[1]].z);
			aVertices.push_back(mesh->mVertices[face->mIndices[0]].x);
			aVertices.push_back(mesh->mVertices[face->mIndices[0]].y);
			aVertices.push_back(-mesh->mVertices[face->mIndices[0]].z);

			int tmp = 0;
			do {
				poly.SetPolyMinMatchup(tmp++);
			} while (!poly.IsPolyMinMatchupValid(&aVertices[0]));
			tmp = 0;
			do {

				poly.SetPolyMaxMatchup(tmp++);
			} while (!poly.IsPolyMaxMatchupValid(&aVertices[0]));

			aPolys.push_back(poly);
		}
	}

	bool* vertsAdded = nullptr;
	struct tCDBNodeTree {
		tCDBRegion data;
		std::vector<tCDBNodeTree> children;
		std::vector<tCDBRegion> meshChildren;
		tCDBNodeTree* parent = nullptr;

		int GetTotalChildren() {
			int count = 0;
			for (auto& child : children) {
				count += child.GetTotalChildren() + 1;
			}
			return count;
		}

		bool IsPolyIntersecting(NyaVec3 currentMin, NyaVec3 currentMax, NyaVec3 otherMin, NyaVec3 otherMax) {
			return (currentMin.x < otherMax.x) && (currentMax.x > otherMin.x) &&
			//(currentMin.y < otherMax.y) && (currentMax.y > otherMin.y) &&
			(currentMin.z < otherMax.z) && (currentMax.z > otherMin.z);
		}

		bool RecalculateAABB(const NyaVec3& polyMin, const NyaVec3& polyMax, float coordMult[3]) {
			auto pos = data.GetPosition(coordMult);
			auto min = pos - data.GetSize(coordMult);
			auto max = pos + data.GetSize(coordMult);
			auto newMin = min;
			auto newMax = max;
			newMin.x = std::min(polyMin.x, min.x);
			newMin.z = std::min(polyMin.z, min.z);
			newMax.x = std::max(polyMax.x, max.x);
			newMax.z = std::max(polyMax.z, max.z);
			if (newMin.x != min.x || newMax.x != max.x || newMin.z != min.z || newMax.z != max.z) {
				auto newSize = newMax - newMin;
				data.SetSize(newSize / 2, coordMult); // todo is the / 2 required
				data.nXSize += 8;
				data.nYSize += 8;
				data.nZSize += 8;
				return true;
			}
			return false;
		}

		void GenerateMeshChildren(float* vertices, float coordMult[3]) {
			if (!vertsAdded) {
				vertsAdded = new bool[aPolys.size()];
				memset(vertsAdded, 0, aPolys.size());
			}

			auto pos = data.GetPosition(coordMult);
			auto min = pos - data.GetSize(coordMult);
			auto max = pos + data.GetSize(coordMult);
			for (auto& poly : aPolys) {
				auto polyMin = poly.GetAABBMin(vertices);
				auto polyMax = poly.GetAABBMax(vertices);
				//if (vertsAdded[&poly-&aFBXPolys[0]]) continue;
				if (!IsPolyIntersecting(polyMin, polyMax, min, max)) continue;
				vertsAdded[&poly-&aPolys[0]] = true;

				//if (RecalculateAABB(polyMin, polyMax, coordMult)) {
				//	auto pParent = parent;
				//	while (pParent) {
				//		if (!pParent->RecalculateAABB(polyMin, polyMax, coordMult)) break;
				//		pParent = pParent->parent;
				//	}
				//}

				auto polyMid = polyMin;
				polyMid.x = std::lerp(polyMin.x, polyMax.x, 0.5);
				polyMid.y = std::lerp(polyMin.y, polyMax.y, 0.5);
				polyMid.z = std::lerp(polyMin.z, polyMax.z, 0.5);

				NyaVec3 polySize = {polyMax.x - polyMin.x, polyMax.y - polyMin.y, polyMax.z - polyMin.z};

				tCDBRegion region;
				region.nFlags.SetHasPolys(true);
				region.nFlags.SetIndex(&poly-&aPolys[0]);
				region.nFlags.SetUnknownFlag(0xF);
				region.nFlags.SetPolyCount(1);
				region.SetPosition(polyMid, coordMult);
				region.SetSize(polySize, coordMult); // todo is this big enough
				meshChildren.push_back(region);
			}
		}

		void Generate(const NyaVec3& position, const NyaVec3& size, float* vertices, float coordMult[3]) {
			data.nFlags.SetIndex(1);
			data.nFlags.SetUnknownFlag(0xF); // todo no idea what this is
			data.SetPosition(position, coordMult);
			data.SetSize(size, coordMult);

			if (data.nXSize < 16 || size.x < 4) {
				if (data.nZSize < 16 || size.z < 4) {
					GenerateMeshChildren(vertices, coordMult);
					// add a bit of leeway, todo this is kinda hacky
					//auto newSize = size;
					//newSize.x *= 2;
					//newSize.z *= 2;
					//data.SetSize(newSize, coordMult);
					return;
				}

				// halve y
				auto sizeHalf = size;
				sizeHalf.z /= 2.0;
				auto topLeft = position;
				topLeft.z -= sizeHalf.z;

				children.push_back({});
				children.push_back({});
				auto child = &children[children.size()-2];
				child->parent = this;
				child->Generate(topLeft, sizeHalf, vertices, coordMult);
				child = &children[children.size()-1];
				topLeft.z += sizeHalf.z * 2;
				child->parent = this;
				child->Generate(topLeft, sizeHalf, vertices, coordMult);
			}
			else {
				// halve x
				auto sizeHalf = size;
				sizeHalf.x /= 2.0;
				auto topLeft = position;
				topLeft.x -= sizeHalf.x;

				children.push_back({});
				auto child = &children[children.size()-1];
				child->Generate(topLeft, sizeHalf, vertices, coordMult);
				children.push_back({});
				child = &children[children.size()-1];
				topLeft.x += sizeHalf.x * 2;
				child->Generate(topLeft, sizeHalf, vertices, coordMult);
			}
		}

		int GetNumRegionsAdded() {
			if (children.empty()) {
				if (meshChildren.empty()) {
					// two empty dummy regions, this is a dead end
					return 2;
				}
				return meshChildren.size()*2;
			}
			else {
				return children[0].GetNumRegionsAdded() + children[1].GetNumRegionsAdded() + 2;
			}
		}

		void CreateRegions() {
			if (!children.empty() && children.size() != 2) {
				WriteConsole("ERROR: region child count != 2", LOG_ERRORS);
				return;
			}
			tCDBRegion regionEnd;
			regionEnd.nFlags.SetHasPolys(true);
			regionEnd.nFlags.SetPolyCount(1);
			regionEnd.nFlags.SetIndex(0);
			regionEnd.nFlags.SetUnknownFlag(0);
			if (children.empty()) {
				if (meshChildren.empty()) {
					// two empty dummy regions, this is a dead end
					aRegions.push_back(regionEnd);
					aRegions.push_back(regionEnd);
				}
				else {
					for (auto& mesh : meshChildren) {
						aRegions.push_back(mesh);
						// end of meshes, dead end
						if (&mesh == &meshChildren[meshChildren.size()-1]) {
							aRegions.push_back(regionEnd);
						}
						else {
							// go to next
							tCDBRegion region;
							region.nFlags.SetHasPolys(false);
							region.nFlags.SetIndex(1);
							region.nFlags.SetUnknownFlag(0xF);
							region.nXPosition = data.nXPosition;
							region.nYPosition = data.nYPosition;
							region.nZPosition = data.nZPosition;
							region.nXSize = data.nXSize;
							region.nYSize = data.nYSize;
							region.nZSize = data.nZSize;
							aRegions.push_back(region);
						}
					}
				}
			}
			else {
				tCDBRegion region = data;
				region.nFlags.SetUnknownFlag(0xF);
				region.nFlags.SetHasPolys(false);
				region.nFlags.SetIndex(children[0].GetNumRegionsAdded()+2);
				aRegions.push_back(region);
				region = data;
				region.nFlags.SetUnknownFlag(0xF);
				region.nFlags.SetHasPolys(false);
				region.nFlags.SetIndex(1);
				aRegions.push_back(region);
				children[0].CreateRegions();
				children[1].CreateRegions();
			}
		}
	};
	tCDBNodeTree gFBXRootNode;

	void FillFromFBX() {
		WriteConsole(std::format("Processing {} meshes", pParsedFBXScene->mNumMeshes), LOG_ALWAYS);

		for (int i = 0; i < pParsedFBXScene->mNumMeshes; i++) {
			CreateCDBFromMesh(pParsedFBXScene->mMeshes[i], pParsedFBXScene->mMaterials[pParsedFBXScene->mMeshes[i]->mMaterialIndex]);
		}

		auto min = GetAABBMin();
		auto max = GetAABBMax();
		auto radius = max;
		if (std::abs(min.x) > radius.x) radius.x = std::abs(min.x);
		if (std::abs(min.y) > radius.y) radius.y = std::abs(min.y);
		if (std::abs(min.z) > radius.z) radius.z = std::abs(min.z);

		auto mult2 = radius * (1.0 / 32767.0);

		// first region
		gFBXRootNode.data.nFlags.SetIndex(1);
		gFBXRootNode.data.nFlags.SetUnknownFlag(0xF); // todo no idea what this is
		gFBXRootNode.data.nXPosition = 0;
		gFBXRootNode.data.nYPosition = 0;
		gFBXRootNode.data.nZPosition = 0;
		gFBXRootNode.data.nXSize = 32767;
		gFBXRootNode.data.nYSize = 32767;
		gFBXRootNode.data.nZSize = 32767;
		gFBXRootNode.Generate(gFBXRootNode.data.GetPosition(&mult2.x), gFBXRootNode.data.GetSize(&mult2.x), &aVertices[0], &mult2.x);

		if (!gFBXRootNode.meshChildren.empty()) {
			WriteConsole("ERROR: Collision model too small!", LOG_ERRORS);
			return;
		}

		aRegions.push_back(gFBXRootNode.data);

		// todo increase region sizes if any polys are out of bounds
		gFBXRootNode.CreateRegions();

		int count = 0;
		for (int i = 0; i < aPolys.size(); i++) {
			if (!vertsAdded[i]) {
				count++;
			}
		}
		WriteConsole(std::format("{}/{} verts missed", count, aPolys.size()), LOG_ALWAYS);
	}
}

bool ParseTrackCDB(const std::filesystem::path& fileName) {
	WriteConsole("Parsing CDB data...", LOG_ALWAYS);

	if (fileName.extension() != ".gen" && fileName.extension() != ".cdb") {
		return false;
	}

	std::ifstream fin(fileName, std::ios::in | std::ios::binary);
	if (!fin.is_open()) return false;

	uint32_t identifier;
	uint32_t dateIdentifier;
	fin.read((char*)&identifier, 4);
	fin.read((char*)&dateIdentifier, 4);
	if (bDumpIntoTextFile) {
		WriteFile(std::format("nIdentifier: {:X}", identifier));
		WriteFile(std::format("nDateIdentifier: {:X}", dateIdentifier));
	}

	if (identifier == 0x62626161) { // "aabb"
		FO1CDB::Parse(fin);
	}
	else {
		tCDB2Header header;
		fin.read((char*)&header, sizeof(header));

		if (bDumpIntoTextFile) {
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
	}

	return true;
}