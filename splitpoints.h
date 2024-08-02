struct tSplitpoint {
	aiVector3D pos;
	aiVector3D left;
	aiVector3D right;
};
std::vector<tSplitpoint> aSplitpoints;

struct tStartpoint {
	float mMatrix[4*4];
};
std::vector<tStartpoint> aStartpoints;

bool IsBedLine(std::string& line, const std::string& value) {
	if (line.starts_with(value)) {
		line.erase(line.begin(), line.begin() + value.length());
		return true;
	}
	return false;
}

void ParseSplitpointsLine(const std::string& line) {
	static tSplitpoint addSplitpoint;

	auto tmp = line;
	if (IsBedLine(tmp, "Count = ")) {
		aSplitpoints.reserve(std::stoi(tmp));
		return;
	}
	if (IsBedLine(tmp, "\t\tPosition = { ")) {
		addSplitpoint.pos.x = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addSplitpoint.pos.y = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addSplitpoint.pos.z = std::stof(tmp);
		return;
	}
	if (IsBedLine(tmp, "\t\tLeft = { ")) {
		addSplitpoint.left.x = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addSplitpoint.left.y = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addSplitpoint.left.z = std::stof(tmp);
		return;
	}
	if (IsBedLine(tmp, "\t\tRight = { ")) {
		addSplitpoint.right.x = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addSplitpoint.right.y = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addSplitpoint.right.z = std::stof(tmp);
		aSplitpoints.push_back(addSplitpoint);
		return;
	}
}

void ParseStartpointsLine(const std::string& line) {
	static tStartpoint addStartpoint;
	addStartpoint.mMatrix[3] = 0;
	addStartpoint.mMatrix[7] = 0;
	addStartpoint.mMatrix[11] = 0;
	addStartpoint.mMatrix[15] = 1;

	auto tmp = line;
	if (IsBedLine(tmp, "Count = ")) {
		aStartpoints.reserve(std::stoi(tmp));
		return;
	}
	if (IsBedLine(tmp, "\t\tPosition = { ")) {
		addStartpoint.mMatrix[12] = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addStartpoint.mMatrix[13] = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addStartpoint.mMatrix[14] = std::stof(tmp);
		return;
	}
	if (IsBedLine(tmp, "\t\t\t[\"x\"]={")) {
		addStartpoint.mMatrix[0] = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addStartpoint.mMatrix[1] = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addStartpoint.mMatrix[2] = std::stof(tmp);
		return;
	}
	if (IsBedLine(tmp, "\t\t\t[\"y\"]={")) {
		addStartpoint.mMatrix[4] = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addStartpoint.mMatrix[5] = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addStartpoint.mMatrix[6] = std::stof(tmp);
		return;
	}
	if (IsBedLine(tmp, "\t\t\t[\"z\"]={")) {
		addStartpoint.mMatrix[8] = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addStartpoint.mMatrix[9] = std::stof(tmp);
		tmp.erase(tmp.begin(), tmp.begin() + tmp.find(',') + 1);
		addStartpoint.mMatrix[10] = std::stof(tmp);
		aStartpoints.push_back(addStartpoint);
		return;
	}
}

bool ParseSplitpoints(const std::filesystem::path& fileName) {
	if (fileName.extension() != ".bed") {
		return false;
	}

	WriteConsole("Parsing split points...", LOG_ALWAYS);

	std::ifstream fin(fileName, std::ios::in );
	if (!fin.is_open()) return false;

	for (std::string line; std::getline(fin, line); ) {
		ParseSplitpointsLine(line);
	}

	return true;
}

bool ParseStartpoints(const std::filesystem::path& fileName) {
	if (fileName.extension() != ".bed") {
		return false;
	}

	WriteConsole("Parsing start points...", LOG_ALWAYS);

	std::ifstream fin(fileName, std::ios::in );
	if (!fin.is_open()) return false;

	for (std::string line; std::getline(fin, line); ) {
		ParseStartpointsLine(line);
	}

	return true;
}

void WriteSplitpoints() {
	if (aSplitpoints.empty()) {
		WriteConsole("WARNING: No splitpoints.bed data loaded, checkpoint data will not be exported", LOG_WARNINGS);
		return;
	}

	WriteConsole("Writing output splitpoints.bed file...", LOG_ALWAYS);

	auto outFileName = sFileNameNoExt.string() + "_splitpoints.bed";
	if (bW32UseVanillaNames) outFileName = sFileFolder.string() + "splitpoints.bed";
	std::ofstream fout(outFileName, std::ios::out);
	if (!fout.is_open()) return;

	fout << "Count = ";
	fout << aSplitpoints.size();
	fout << "\n\nSplitpoints = {";
	for (auto& point : aSplitpoints) {
		fout << std::format("\n\t[{}] = {{", (&point - &aSplitpoints[0]) + 1);
		fout << std::format("\n\t\tPosition = {{ {}, {}, {} }},", point.pos.x, point.pos.y, point.pos.z);
		fout << std::format("\n\t\tLeft = {{ {}, {}, {} }},", point.left.x, point.left.y, point.left.z);
		fout << std::format("\n\t\tRight = {{ {}, {}, {} }},", point.right.x, point.right.y, point.right.z);
		fout << "\n\n\t},";
	}
	fout << "\n}";
}

void WriteStartpoints() {
	if (aStartpoints.empty()) {
		WriteConsole("WARNING: No startpoints.bed data loaded, spawn position data will not be exported", LOG_WARNINGS);
		return;
	}

	WriteConsole("Writing output startpoints.bed file...", LOG_ALWAYS);

	auto outFileName = sFileNameNoExt.string() + "_startpoints.bed";
	if (bW32UseVanillaNames) outFileName = sFileFolder.string() + "startpoints.bed";
	std::ofstream fout(outFileName, std::ios::out);
	if (!fout.is_open()) return;

	fout << "Count = ";
	fout << aStartpoints.size();
	fout << "\n\nStartpoints = {";
	for (auto& point : aStartpoints) {
		for (int i = 0; i < 16; i++) {
			if (std::abs(point.mMatrix[i]) < 0.001) point.mMatrix[i] = 0;
		}

		fout << std::format("\n\t[{}] = {{", (&point - &aStartpoints[0]) + 1);
		fout << std::format("\n\t\tPosition = {{ {}, {}, {} }},", point.mMatrix[12], point.mMatrix[13], point.mMatrix[14]);
		fout << "\n\t\tOrientation = {";
		fout << std::format("\n\t\t\t[\"x\"]={{{},{},{}}},", point.mMatrix[0], point.mMatrix[1], point.mMatrix[2]);
		fout << std::format("\n\t\t\t[\"y\"]={{{},{},{}}},", point.mMatrix[4], point.mMatrix[5], point.mMatrix[6]);
		fout << std::format("\n\t\t\t[\"z\"]={{{},{},{}}},", point.mMatrix[8], point.mMatrix[9], point.mMatrix[10]);
		fout << "\n\t\t},";
		fout << "\n\n\t},";
	}
	fout << "\n}";
}