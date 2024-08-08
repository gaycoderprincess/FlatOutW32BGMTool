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
		WriteFile(std::format("nSomeDataSize: {:X}", tmp));

		auto data = new uint8_t[tmp];
		fin.read((char*)data, tmp);
		for (int i = 0; i < tmp / sizeof(float); i++) {
			auto fData = (float*)data;
			WriteFile(std::to_string(fData[i]));
		}

		fin.read((char*)&tmp, 4);
		WriteFile(std::format("nSomeData2Size: {:X}", tmp));

		uint32_t data2Size = tmp * 12;
		if (bCDBIsRetroDemo) data2Size = tmp * 16;
		data = new uint8_t[data2Size];
		fin.read((char*)data, data2Size);
		for (int i = 0; i < data2Size / sizeof(float); i++) {
			auto nData = (uint32_t*)data;
			WriteFile(std::format("{:X}", nData[i]));
		}

		float values[6];
		fin.read((char*)values, sizeof(values));
		WriteFile(std::format("vBBMin.x: {}", values[0]));
		WriteFile(std::format("vBBMin.y: {}", values[1]));
		WriteFile(std::format("vBBMin.z: {}", values[2]));
		WriteFile(std::format("vBBMax.x: {}", values[3]));
		WriteFile(std::format("vBBMax.y: {}", values[4]));
		WriteFile(std::format("vBBMax.z: {}", values[5]));

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

		data = new uint8_t[tmp * 16];
		fin.read((char*)data, tmp * 16);
		for (int i = 0; i < tmp * 16 / sizeof(uint32_t); i++) {
			auto nData = (uint32_t*)data;
			WriteFile(std::format("{:X}", nData[i]));
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