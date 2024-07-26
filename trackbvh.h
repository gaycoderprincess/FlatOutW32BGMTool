bool ReadAndEmptyTrackBVH() {
	if (!sFileName.ends_with(".gen")) {
		return false;
	}

	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	std::ofstream fout(sFileNameNoExt + "_out.gen", std::ios::out | std::ios::binary);
	if (fin.is_open() && fout.is_open()) {
		int header[3]; // 0 and 1 are unused, 2 is bvh count
		ReadFromFile(fin, header, sizeof(header));
		fout.write((char*)header, sizeof(header));
		WriteFile(std::format("0x{:08X}", (uint32_t)header[0]));
		WriteFile(std::to_string(header[1]));

		WriteFile("First chunk begin");
		WriteFile("Count: " + std::to_string(header[2]));
		WriteFile(""); // newline

		for (int i = 0; i < header[2]; i++) {
			float data[3];
			ReadFromFile(fin, data, 4 * 3);
			WriteFile(std::to_string(data[0]));
			WriteFile(std::to_string(data[1]));
			WriteFile(std::to_string(data[2]));
			fout.write((char*)data, sizeof(data));

			ReadFromFile(fin, data, 4 * 3);
			WriteFile(std::to_string(data[0]));
			WriteFile(std::to_string(data[1]));
			WriteFile(std::to_string(data[2]));
			data[0] = 10000;
			data[1] = 10000;
			data[2] = 10000;
			fout.write((char*)data, sizeof(data));

			int unkValues[2];
			ReadFromFile(fin, unkValues, sizeof(unkValues));
			WriteFile(std::to_string(unkValues[0]));
			WriteFile(std::to_string(unkValues[1]));
			fout.write((char*)unkValues, sizeof(unkValues));

			WriteFile(""); // newline
		}

		WriteFile("First chunk end");

		WriteFile(""); // newline

		WriteFile("Second chunk begin");

		int chunk2Count;
		ReadFromFile(fin, &chunk2Count, 4);
		WriteFile("Count: " + std::to_string(chunk2Count)); // count
		fout.write((char*)&chunk2Count, 4);

		WriteFile(""); // newline

		for (int i = 0; i < chunk2Count; i++) {
			float data[3];
			ReadFromFile(fin, data, 4 * 3);
			WriteFile(std::to_string(data[0]));
			WriteFile(std::to_string(data[1]));
			WriteFile(std::to_string(data[2]));
			fout.write((char*)data, sizeof(data));

			ReadFromFile(fin, data, 4 * 3);
			WriteFile(std::to_string(data[0]));
			WriteFile(std::to_string(data[1]));
			WriteFile(std::to_string(data[2]));
			data[0] = 100000;
			data[1] = 100000;
			data[2] = 100000;
			fout.write((char*)data, sizeof(data));

			int unkValues[2];
			ReadFromFile(fin, unkValues, sizeof(unkValues));
			WriteFile(std::to_string(unkValues[0]));
			WriteFile(std::to_string(unkValues[1]));
			fout.write((char*)unkValues, sizeof(unkValues));

			WriteFile(""); // newline
		}

		WriteFile("Second chunk end");

		return true;
	}
	return false;
}