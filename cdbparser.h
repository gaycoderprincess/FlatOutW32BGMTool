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
		"Bush",
		"Rubber (Object)",
		"Water",
		"No Camera Col",
		"Reset",
		"Camera only col",
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

	float* aVertices = nullptr;
	tCDBPoly* aPolys = nullptr;
	tCDBRegion* aRegions = nullptr;
	int nNumVertices = 0;
	int nNumPolys = 0;
	int nNumRegions = 0;
	float fPosMultiplier[3] = {};

	void ReadCollisionRegion(tCDBRegion* region) {
		if (region->nFlags.HasPolys()) {
			tExportCollisionRegion tmp;
			tmp.originalId = region - aRegions;
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

	void WriteToFBX() {
		int numMaterials = sizeof(aMaterialNames)/sizeof(aMaterialNames[0]);

		// use to read per material
		for (int i = 0; i < numMaterials; i++) {
			tExportCollisionRegion matRegion;
			for (int j = 0; j < nNumPolys; j++) {
				auto poly = aPolys[j];
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

	bool Parse(std::ifstream& fin) {
		uint32_t tmp;
		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nValue1: 0x{:X}", tmp));
		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nValue2: 0x{:X}", tmp));
		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nVertexSize: 0x{:X}", tmp));

		aVertices = new float[tmp/4];
		fin.read((char*)aVertices, tmp);
		nNumVertices = tmp/4;

		//for (int i = 0; i < tmp / sizeof(tFO1CollisionVertex); i++) {
		//	auto fData = aFO1CDBVertices;
		//	WriteFile(std::format("{} {} {} {} {}", fData[i].fPosition[0], fData[i].fPosition[1], fData[i].fPosition[2], fData[i].fMultipliers[0], fData[i].fMultipliers[1]));
		//}

		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nIndexCount: {}", tmp));

		aPolys = new tCDBPoly[tmp];
		fin.read((char*)aPolys, tmp*sizeof(tCDBPoly));
		nNumPolys = tmp;

		if (bDumpIntoTextFile) {
			for (int i = 0; i < nNumPolys; i++) {
				auto nData = aPolys[i];
				auto poly1 = nData.GetPolyMinMatchup();
				auto poly2 = nData.GetPolyMaxMatchup();
				if (!nData.IsPolyMinMatchupValid(aVertices) || !nData.IsPolyMaxMatchupValid(aVertices)) {
					WriteConsole(std::format("WARNING: {} poly matchup invalid", i), LOG_WARNINGS);
				}
				auto v20 = nWeirdPolyMatchupThing[poly1];
				auto v19 = nWeirdPolyMatchupThing[poly2];
				auto x1 = v19 & 3;
				auto x2 = v20 & 3;
				auto y1 = (v19 >> 2) & 3;
				auto y2 = (v20 >> 2) & 3;
				auto z1 = (v19 >> 4) & 3;
				auto z2 = (v20 >> 4) & 3;
				WriteFile(std::format("{} {} {} flags {} material {} unk {} {} ({} {} {} {} {} {})", nData.nVertex1.Get(), nData.nVertex2.Get(), nData.nVertex3.Get(), nData.nFlags, nData.GetMaterial(), poly1, poly2, x1, x2, y1, y2, z1, z2));

				float* coords[] = {&aVertices[nData.nVertex1.Get()], &aVertices[nData.nVertex2.Get()], &aVertices[nData.nVertex3.Get()]};

				float fx1 = coords[x1][0];
				float fx2 = coords[x2][0];
				float fy1 = coords[y1][1];
				float fy2 = coords[y2][1];
				float fz1 = coords[z1][2];
				float fz2 = coords[z2][2];

				auto v1 = NyaVec3(aVertices[nData.nVertex1.Get()], aVertices[nData.nVertex1.Get()+1], aVertices[nData.nVertex1.Get()+2]);
				auto v2 = NyaVec3(aVertices[nData.nVertex2.Get()], aVertices[nData.nVertex2.Get()+1], aVertices[nData.nVertex2.Get()+2]);
				auto v3 = NyaVec3(aVertices[nData.nVertex3.Get()], aVertices[nData.nVertex3.Get()+1], aVertices[nData.nVertex3.Get()+2]);

				WriteFile(std::format("{} {} {} {} {} {} {} {} {}", v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z));
				WriteFile(std::format("min {} {} {} max {} {} {}", fx1, fy1, fz1, fx2, fy2, fz2));
			}
		}

		float values[6];
		fin.read((char*)values, sizeof(values));
		if (bDumpIntoTextFile) {
			WriteFile(std::format("vCenter.x: {}", values[0]));
			WriteFile(std::format("vCenter.y: {}", values[1]));
			WriteFile(std::format("vCenter.z: {}", values[2]));
			WriteFile(std::format("vRadius.x: {}", values[3]));
			WriteFile(std::format("vRadius.y: {}", values[4]));
			WriteFile(std::format("vRadius.z: {}", values[5]));
		}

		float values2[3];
		fin.read((char*)values2, sizeof(values2));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers1.x: {}", values2[0]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers1.y: {}", values2[1]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers1.z: {}", values2[2]));
		fin.read((char*)values2, sizeof(values2));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers2.x: {}", values2[0]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers2.y: {}", values2[1]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliers2.z: {}", values2[2]));
		fin.read((char*)values2, sizeof(values2));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv1.x: {}", values2[0]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv1.y: {}", values2[1]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv1.z: {}", values2[2]));
		fin.read((char*)fPosMultiplier, sizeof(fPosMultiplier));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv2.x: {}", fPosMultiplier[0]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv2.y: {}", fPosMultiplier[1]));
		if (bDumpIntoTextFile) WriteFile(std::format("vCoordMultipliersInv2.z: {}", fPosMultiplier[2]));

		fin.read((char*)&tmp, 4);
		if (bDumpIntoTextFile) WriteFile(std::format("nRegionCount: {}", tmp));

		aRegions = new tCDBRegion[tmp];
		fin.read((char*)aRegions, tmp * sizeof(tCDBRegion));
		nNumRegions = tmp;
		if (bDumpIntoTextFile) {
			for (int i = 0; i < nNumRegions; i++) {
				auto nData = aRegions[i];
				WriteFile(std::format("Region {}", i));
				WriteFile(std::format("polyCount {} hasPolys {} index {} unk {}", nData.nFlags.GetPolyCount(), (int)nData.nFlags.HasPolys(), nData.nFlags.GetIndex(), nData.nFlags.GetUnknownFlag()));
				auto pos = nData.GetPosition(fPosMultiplier);
				auto size = nData.GetSize(fPosMultiplier);
				WriteFile(std::format("pos {} {} {}", pos.x, pos.y, pos.z));
				WriteFile(std::format("size {} {} {}", size.x, size.y, size.z));
			}
		}
		return true;
	}

	NyaVec3 GetAABBMin() {
		NyaVec3 min = {9999, 9999, 9999};
		for (int i = 0; i < nNumPolys; i++) {
			auto poly = aPolys[i];
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
		for (int i = 0; i < nNumPolys; i++) {
			auto poly = aPolys[i];
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
		if (nNumVertices <= 0) return;
		if (nNumPolys <= 0) return;
		if (nNumRegions <= 0) return;

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

		int count = nNumVertices*4;
		file.write((char*)&count, 4);
		for (int i = 0; i < nNumVertices; i++) {
			file.write((char*)&aVertices[i], sizeof(aVertices[i]));
		}

		file.write((char*)&nNumPolys, 4);
		for (int i = 0; i < nNumPolys; i++) {
			file.write((char*)&aPolys[i], sizeof(aPolys[i]));
		}

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

		file.write((char*)&nNumRegions, 4);
		for (int i = 0; i < nNumRegions; i++) {
			file.write((char*)&aRegions[i], sizeof(aRegions[i]));
		}

		file.flush();

		WriteConsole("CDB export finished", LOG_ALWAYS);
	}

	std::vector<float> aFBXVertices;
	std::vector<tCDBPoly> aFBXPolys;
	std::vector<tCDBRegion> aFBXRegions;
	void CreateCDBFromMesh(aiMesh* mesh) {
		for (int i = 0; i < mesh->mNumFaces; i++) {
			auto face = &mesh->mFaces[i];
			tCDBPoly poly;
			poly.nFlags = 27; // todo trees 1 objects 3 water 6 ground 27
			poly.SetMaterial(1); // todo
			poly.nVertex1.Set(aFBXVertices.size());
			poly.nVertex2.Set(aFBXVertices.size()+3);
			poly.nVertex3.Set(aFBXVertices.size()+6);

			aFBXVertices.push_back(-mesh->mVertices[face->mIndices[2]].x);
			aFBXVertices.push_back(mesh->mVertices[face->mIndices[2]].y);
			aFBXVertices.push_back(mesh->mVertices[face->mIndices[2]].z);
			aFBXVertices.push_back(-mesh->mVertices[face->mIndices[1]].x);
			aFBXVertices.push_back(mesh->mVertices[face->mIndices[1]].y);
			aFBXVertices.push_back(mesh->mVertices[face->mIndices[1]].z);
			aFBXVertices.push_back(-mesh->mVertices[face->mIndices[0]].x);
			aFBXVertices.push_back(mesh->mVertices[face->mIndices[0]].y);
			aFBXVertices.push_back(mesh->mVertices[face->mIndices[0]].z);

			int tmp = 0;
			do {
				poly.SetPolyMinMatchup(tmp++);
			} while (!poly.IsPolyMinMatchupValid(&aFBXVertices[0]));
			tmp = 0;
			do {

				poly.SetPolyMaxMatchup(tmp++);
			} while (!poly.IsPolyMaxMatchupValid(&aFBXVertices[0]));

			aFBXPolys.push_back(poly);
		}
	}

	struct tCDBNodeTree {
		tCDBRegion data;
		std::vector<tCDBNodeTree> children;
		std::vector<tCDBRegion> meshChildren;

		int GetTotalChildren() {
			int count = 0;
			for (auto& child : children) {
				count += child.GetTotalChildren() + 1;
			}
			return count;
		}

		void GenerateMeshChildren(float* vertices, float coordMult[3]) {
			auto min = data.GetPosition(coordMult) - data.GetSize(coordMult);
			auto max = data.GetPosition(coordMult) + data.GetSize(coordMult);
			for (auto& poly : aFBXPolys) {
				auto polyMin = poly.GetAABBMin(vertices);
				auto polyMax = poly.GetAABBMax(vertices);
				if (polyMin.x < min.x || polyMin.y < min.y || polyMin.z < min.z) continue;
				if (polyMax.x > max.x || polyMax.y > max.y || polyMax.z > max.z) continue;

				tCDBRegion region;
				region.nFlags.SetIndex(&poly-&aFBXPolys[0]);
				region.nFlags.SetHasPolys(true);
				region.nFlags.SetUnknownFlag(0xF);
				region.nFlags.SetPolyCount(1);
				meshChildren.push_back(region);
			}
		}

		void Generate(const NyaVec3& position, const NyaVec3& size, float* vertices, float coordMult[3]) {
			data.nFlags.SetIndex(1);
			data.nFlags.SetUnknownFlag(0xF); // todo no idea what this is
			data.SetPosition(position, coordMult);
			data.SetSize(size, coordMult);

			if (data.nXSize < 16 || size.x < 8) {
				if (data.nYSize < 16 || size.y < 8) {
					GenerateMeshChildren(vertices, coordMult);
					return;
				}

				// halve y
				auto sizeHalf = size;
				sizeHalf.y /= 2.0;
				auto topLeft = position;
				topLeft.y -= sizeHalf.y;

				children.push_back({});
				auto child = &children[children.size()-1];
				child->Generate(topLeft, sizeHalf, vertices, coordMult);
				children.push_back({});
				child = &children[children.size()-1];
				topLeft.y += sizeHalf.y * 2;
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
				tCDBRegion regionEnd;
				regionEnd.nFlags.SetHasPolys(true);
				regionEnd.nFlags.SetPolyCount(1);
				regionEnd.nFlags.SetIndex(0);
				regionEnd.nFlags.SetUnknownFlag(0);
				if (meshChildren.empty()) {
					// two empty dummy regions, this is a dead end
					return 2;
				}
				return meshChildren.size()*2;
			}
			else {
				return children[0].GetNumRegionsAdded() + children[1].GetNumRegionsAdded();
			}
		}

		void CreateRegions() {
			if (!children.empty() && children.size() != 2) {
				WriteConsole("ERROR: region child count != 2", LOG_ERRORS);
				return;
			}
			if (children.empty()) {
				tCDBRegion regionEnd;
				regionEnd.nFlags.SetHasPolys(true);
				regionEnd.nFlags.SetPolyCount(1);
				regionEnd.nFlags.SetIndex(0);
				regionEnd.nFlags.SetUnknownFlag(0);
				if (meshChildren.empty()) {
					// two empty dummy regions, this is a dead end
					aFBXRegions.push_back(regionEnd);
					aFBXRegions.push_back(regionEnd);
				}
				else {
					for (auto& mesh : meshChildren) {
						aFBXRegions.push_back(mesh);
						// end of meshes, dead end
						if (&mesh == &meshChildren[meshChildren.size()-1]) {
							aFBXRegions.push_back(regionEnd);
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
							aFBXRegions.push_back(region);
						}
					}
				}
			}
			else {

			}
			/*for (auto& child : children) {
				if (child.meshChildren.empty()) {
					tCDBRegion region = child.data;
					region.nFlags.SetIndex(2);
					region.nFlags.SetUnknownFlag(0xF);
					aFBXRegions.push_back(region);

					// dumb dummy data region so i dont have to do weird redirects like vanilla
					region.nXSize = 0;
					region.nYSize = 0;
					region.nXSize = 0;
					region.nFlags.SetIndex(0);
					region.nFlags.SetHasPolys(true);
					region.nFlags.SetPolyCount(1);
					region.nFlags.SetUnknownFlag(0);
					aFBXRegions.push_back(region);
				}
				else {
					for (auto& mesh : child.meshChildren) {
						//bool last = &mesh == &child.meshChildren[child.meshChildren.size()-1];
						//if (last) {
						//	// dumb dummy data region so i dont have to do weird redirects like vanilla
						//	region = child.data;
						//	region.nXSize = 0;
						//	region.nYSize = 0;
						//	region.nXSize = 0;
						//	region.nFlags.SetIndex(0);
						//	region.nFlags.SetHasPolys(true);
						//	region.nFlags.SetPolyCount(1);
						//	region.nFlags.SetUnknownFlag(0);
						//	aFBXRegions.push_back(region);
						//}
						//else {
						auto region = child.data;
						region.nFlags.SetIndex(2);
						region.nFlags.SetUnknownFlag(0xF);
						region.nFlags.SetHasPolys(false);
						region.nFlags.SetPolyCount(1);
						aFBXRegions.push_back(region);
						//}

						region = child.data;
						aFBXRegions.push_back(region);
					}
				}
			}*/
		}
	};
	tCDBNodeTree gFBXRootNode;

	void FillFromFBX() {
		WriteConsole(std::format("Processing {} meshes", pParsedFBXScene->mNumMeshes), LOG_ALWAYS);

		for (int i = 0; i < pParsedFBXScene->mNumMeshes; i++) {
			CreateCDBFromMesh(pParsedFBXScene->mMeshes[i]);
		}

		nNumVertices = aFBXVertices.size();
		nNumPolys = aFBXPolys.size();

		aVertices = new float[nNumVertices];
		for (int i = 0; i < nNumVertices; i++) {
			aVertices[i] = aFBXVertices[i];
		}

		aPolys = new tCDBPoly[nNumPolys];
		for (int i = 0; i < nNumPolys; i++) {
			aPolys[i] = aFBXPolys[i];
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
		gFBXRootNode.Generate(gFBXRootNode.data.GetPosition(&mult2.x), gFBXRootNode.data.GetSize(&mult2.x), aVertices, &mult2.x);

		if (!gFBXRootNode.meshChildren.empty()) {
			WriteConsole("ERROR: Collision model too small!", LOG_ERRORS);
			return;
		}

		// todo increase region sizes if any polys are out of bounds
		tCDBRegion root = gFBXRootNode.data;
		aFBXRegions.push_back(root);
		// todo add null last one
		for (auto& child : gFBXRootNode.children) {
			tCDBRegion region = child.data;
			region.nFlags.SetIndex(2);
			region.nFlags.SetUnknownFlag(0xF);
			aFBXRegions.push_back(region);

			if (child.meshChildren.empty()) {
				// dumb dummy data region so i dont have to do weird redirects like vanilla
				region.nXSize = 0;
				region.nYSize = 0;
				region.nXSize = 0;
				region.nFlags.SetIndex(0);
				region.nFlags.SetHasPolys(true);
				region.nFlags.SetPolyCount(1);
				region.nFlags.SetUnknownFlag(0);
				aFBXRegions.push_back(region);
			}
			else {
				for (auto& mesh : child.meshChildren) {
					//bool last = &mesh == &child.meshChildren[child.meshChildren.size()-1];
					//if (last) {
					//	// dumb dummy data region so i dont have to do weird redirects like vanilla
					//	region = child.data;
					//	region.nXSize = 0;
					//	region.nYSize = 0;
					//	region.nXSize = 0;
					//	region.nFlags.SetIndex(0);
					//	region.nFlags.SetHasPolys(true);
					//	region.nFlags.SetPolyCount(1);
					//	region.nFlags.SetUnknownFlag(0);
					//	aFBXRegions.push_back(region);
					//}
					//else {
						region = child.data;
						region.nFlags.SetIndex(2);
						region.nFlags.SetUnknownFlag(0xF);
						region.nFlags.SetHasPolys(false);
						region.nFlags.SetPolyCount(1);
						aFBXRegions.push_back(region);
					//}

					tCDBRegion region = child.data;
					aFBXRegions.push_back(region);
				}
			}
		}

		// todo this needs to be divided into segments, the game can't handle it as-is
		/*int currPoly = 0;
		int numPolysLeft = aFBXPolys.size();
		while (numPolysLeft) {
			int numPolysToAdd = numPolysLeft;
			if (numPolysToAdd > 0x7F) numPolysToAdd = 0x7F;

			numPolysLeft -= numPolysToAdd;

			if (numPolysLeft) {
				// node
				tCDBRegion region1;
				region1.nFlags.SetIndex(2);
				region1.nFlags.SetUnknownFlag(0xF); // todo no idea what this is
				region1.nXPosition = 0;
				region1.nYPosition = 0;
				region1.nZPosition = 0;
				region1.nXSize = 32767;
				region1.nYSize = 32767;
				region1.nZSize = 32767;
				aFBXRegions.push_back(region1);
			}

			//float radius = 10;
			float radius = 2;

			// data
			tCDBRegion region2;
			region2.nFlags.SetUnknownFlag(0xF); // todo no idea what this is
			region2.nFlags.SetHasPolys(true);
			region2.nFlags.SetIndex(currPoly);
			region2.nFlags.SetPolyCount(numPolysToAdd);
			auto v = &aFBXVertices[aFBXPolys[currPoly].nVertex1.Get()];
			region2.SetPosition({v[0], v[1], v[2]}, &mult2.x);
			region2.SetSize({radius, radius, radius}, &mult2.x); // todo
			aFBXRegions.push_back(region2);
			currPoly += numPolysToAdd;
		}*/

		nNumRegions = aFBXRegions.size();
		aRegions = new tCDBRegion[nNumRegions];
		for (int i = 0; i < nNumRegions; i++) {
			aRegions[i] = aFBXRegions[i];
		}
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