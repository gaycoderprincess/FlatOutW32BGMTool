struct tCDB2Header {
	uint32_t vBBMin[3];
	uint32_t vBBMax[3];
	float vCoordMultipliers[3];
	float vCoordMultipliersInv[3];
	uint32_t nTriOffset;
	uint32_t nVertOffset;
};
static_assert(sizeof(tCDB2Header) == 64 - 8);


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
		uint8_t nMaterial : 6; // +1
		uint8_t nUnk2 : 2; // +1 breaks shadows and tire collision if nulled
		uint8_t nUnk3; // +1 breaks shadows and tire collision if nulled
		tInt24 nVertex1; // +3
		tInt24 nVertex2; // +6
		tInt24 nVertex3; // +9

		tCDBPoly() {
			memset(this,0,sizeof(*this));
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
				if (poly.nMaterial != i) continue;
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
			mesh->mMaterialIndex = region->aPolys[0].nMaterial;

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
				WriteFile(std::format("{} {} {} flags {} material {} unk {} {}", nData.nVertex1.Get(), nData.nVertex2.Get(), nData.nVertex3.Get(), nData.nFlags, (int)nData.nMaterial, (int)nData.nUnk2, nData.nUnk3));
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
			tCDBPoly poly;
			poly.nFlags = 27; // todo trees 1 objects 3 water 6 ground 27
			poly.nMaterial = 1; // todo
			poly.nVertex1.Set(aFBXVertices.size());
			poly.nVertex2.Set(aFBXVertices.size()+3);
			poly.nVertex3.Set(aFBXVertices.size()+6);
			aFBXPolys.push_back(poly);

			aFBXVertices.push_back(-mesh->mVertices[mesh->mFaces[i].mIndices[0]].x);
			aFBXVertices.push_back(mesh->mVertices[mesh->mFaces[i].mIndices[0]].y);
			aFBXVertices.push_back(mesh->mVertices[mesh->mFaces[i].mIndices[0]].z);
			aFBXVertices.push_back(-mesh->mVertices[mesh->mFaces[i].mIndices[1]].x);
			aFBXVertices.push_back(mesh->mVertices[mesh->mFaces[i].mIndices[1]].y);
			aFBXVertices.push_back(mesh->mVertices[mesh->mFaces[i].mIndices[1]].z);
			aFBXVertices.push_back(-mesh->mVertices[mesh->mFaces[i].mIndices[2]].x);
			aFBXVertices.push_back(mesh->mVertices[mesh->mFaces[i].mIndices[2]].y);
			aFBXVertices.push_back(mesh->mVertices[mesh->mFaces[i].mIndices[2]].z);
		}
	}

	void FillFromFBX() {
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

		// first region
		tCDBRegion region;
		region.nFlags.SetIndex(1);
		region.nXPosition = 0;
		region.nYPosition = 0;
		region.nZPosition = 0;
		region.nXSize = 32767;
		region.nYSize = 32767;
		region.nZSize = 32767;
		aFBXRegions.push_back(region);

		auto min = GetAABBMin();
		auto max = GetAABBMax();
		auto radius = max;
		if (std::abs(min.x) > radius.x) radius.x = std::abs(min.x);
		if (std::abs(min.y) > radius.y) radius.y = std::abs(min.y);
		if (std::abs(min.z) > radius.z) radius.z = std::abs(min.z);

		auto mult2 = radius * (1.0 / 32767.0);

		// todo this could be done better
		int currPoly = 0;
		int numPolysLeft = aFBXPolys.size();
		while (numPolysLeft) {
			int numPolysToAdd = numPolysLeft;
			if (numPolysToAdd > 0x7F) numPolysToAdd = 0x7F;

			numPolysLeft -= numPolysToAdd;

			if (numPolysLeft) {
				// node
				tCDBRegion region1;
				region1.nFlags.SetIndex(2);
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
			region2.nFlags.SetUnknownFlag(0xF);
			region2.nFlags.SetHasPolys(true);
			region2.nFlags.SetIndex(currPoly);
			region2.nFlags.SetPolyCount(numPolysToAdd);
			auto v = &aFBXVertices[aFBXPolys[currPoly].nVertex1.Get()];
			region2.SetPosition({v[0], v[1], v[2]}, &mult2.x);
			region2.SetSize({radius, radius, radius}, &mult2.x); // todo
			aFBXRegions.push_back(region2);
			currPoly += numPolysToAdd;
		}

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