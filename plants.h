struct tPlantData {
	float data[6];
	uint32_t surfaceId;
	uint32_t id;
};

bool ParsePlantVDB() {
	if (sFileName.extension() != ".gen") {
		return false;
	}

	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	int data[4];
	ReadFromFile(fin, data, sizeof(data));
	if (data[0] != 0x62647370) return false; // "psdb"
	WriteFile("data[1]: " + std::to_string(data[1])); // ignored
	WriteFile("data[2]: " + std::to_string(data[2])); // ignored
	WriteFile("Count: " + std::to_string(data[3]));
	WriteFile("");
	for (int i = 0; i < data[3]; i++) {
		tPlantData plant;
		ReadFromFile(fin, &plant, sizeof(plant));
		WriteFile("data[0]:" + std::to_string(plant.data[0]));
		WriteFile("data[1]:" + std::to_string(plant.data[1]));
		WriteFile("data[2]:" + std::to_string(plant.data[2]));
		WriteFile("data[3]:" + std::to_string(plant.data[3]));
		WriteFile("data[4]:" + std::to_string(plant.data[4]));
		WriteFile("data[5]:" + std::to_string(plant.data[5]));
		WriteFile("nSurfaceId:" + std::to_string(plant.surfaceId));
		WriteFile("nPlantId:" + std::to_string(plant.id));
		WriteFile("");
	}
	WriteFile("");
	uint32_t someData;
	ReadFromFile(fin, &someData, sizeof(someData));
	WriteFile("someData: " + std::to_string(someData)); // ignored
	float arrays[3];
	ReadFromFile(fin, arrays, sizeof(arrays));
	WriteFile("arrays[0]: " + std::to_string(arrays[0]));
	WriteFile("arrays[1]: " + std::to_string(arrays[1]));
	WriteFile("arrays[2]: " + std::to_string(arrays[2]));
	ReadFromFile(fin, arrays, sizeof(arrays));
	WriteFile("arrays2[0]: " + std::to_string(arrays[0]));
	WriteFile("arrays2[1]: " + std::to_string(arrays[1]));
	WriteFile("arrays2[2]: " + std::to_string(arrays[2]));

	ReadFromFile(fin, &someData, sizeof(someData));
	WriteFile("Count: " + std::to_string(someData));
	WriteFile("");
	for (int i = 0; i < someData; i++) {
		float tmp;
		ReadFromFile(fin, &tmp, sizeof(tmp));
		WriteFile(std::to_string(tmp));
	}

	return true;
}

void WriteEmptyPlantVDB() {
	std::ofstream file("plant_vdb_empty.gen", std::ios::out | std::ios::binary );
	if (!file.is_open()) return;

	uint32_t identifier = 0x62647370;
	file.write((char*)&identifier, 4);
	int tmp = 0;
	file.write((char*)&tmp, 4); // ignored
	file.write((char*)&tmp, 4); // ignored
	file.write((char*)&tmp, 4); // count
	file.write((char*)&tmp, 4); // ignored
	file.write((char*)&tmp, 4); // array[0]
	file.write((char*)&tmp, 4); // array[1]
	file.write((char*)&tmp, 4); // array[2]
	file.write((char*)&tmp, 4); // array2[0]
	file.write((char*)&tmp, 4); // array2[1]
	file.write((char*)&tmp, 4); // array2[2]
	file.write((char*)&tmp, 4); // count2
}

bool ParsePlantGeom() {
	if (sFileName.extension() != ".w32") {
		return false;
	}

	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	uint32_t identifier;
	ReadFromFile(fin, &identifier, 4);
	if (identifier != 0x62647370) return false; // "psdb"
	uint32_t someCount;
	ReadFromFile(fin, &someCount, 4);
	WriteFile("someCount: " + std::to_string(someCount));
	float data[8];
	ReadFromFile(fin, data, sizeof(data));
	WriteFile("data[0]: " + std::to_string(data[0]));
	WriteFile("data[1]: " + std::to_string(data[1]));
	WriteFile("data[2]: " + std::to_string(data[2]));
	WriteFile("data[3]: " + std::to_string(data[3]));
	WriteFile("data[4]: " + std::to_string(data[4]));
	WriteFile("data[5]: " + std::to_string(data[5]));
	WriteFile("data[6]: " + std::to_string(data[6]));
	WriteFile("data[7]: " + std::to_string(data[7]));

	ReadFromFile(fin, data, sizeof(data));
	WriteFile("data[0]: " + std::to_string(data[0]));
	WriteFile("data[1]: " + std::to_string(data[1]));
	WriteFile("data[2]: " + std::to_string(data[2]));
	WriteFile("data[3]: " + std::to_string(data[3]));
	WriteFile("data[4]: " + std::to_string(data[4]));
	WriteFile("data[5]: " + std::to_string(data[5]));
	WriteFile("data[6]: " + std::to_string(data[6]));
	WriteFile("data[7]: " + std::to_string(data[7]));

	float data2[2];
	ReadFromFile(fin, data2, sizeof(data2));
	WriteFile("data2[0]: " + std::to_string(data2[0]));
	WriteFile("data2[1]: " + std::to_string(data2[1]));
	ReadFromFile(fin, data2, sizeof(data2));
	WriteFile("data2[0]: " + std::to_string(data2[0]));
	WriteFile("data2[1]: " + std::to_string(data2[1]));
	ReadFromFile(fin, data2, sizeof(data2));
	WriteFile("data2[0]: " + std::to_string(data2[0]));
	WriteFile("data2[1]: " + std::to_string(data2[1]));

	WriteFile("");

	ReadFromFile(fin, &someCount, 4);
	WriteFile("someCount: " + std::to_string(someCount));
	WriteFile("");
	for (int i = 0; i < someCount; i++) {
		uint32_t values[2];
		ReadFromFile(fin, values, sizeof(values));
		WriteFile(std::format("values[0]: 0x{:X}", values[0]));
		WriteFile(std::format("values[1]: 0x{:X}", values[1]));
	}

	WriteFile("");

	ReadFromFile(fin, &someCount, 4);
	WriteFile("someCount: " + std::to_string(someCount));
	WriteFile("");
	for (int i = 0; i < someCount; i++) {
		uint32_t values[2];
		ReadFromFile(fin, values, sizeof(values));
		WriteFile("values[0]: " + std::to_string(values[0]));
		WriteFile("values[1]: " + std::to_string(values[1]));
	}

	return true;
}