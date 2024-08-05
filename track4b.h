struct BITMAPHEADER {
	uint16_t identifier = 0x4D42;
	uint32_t fileSize = 0;
	uint32_t unused = 0;
	uint32_t dataOffset = 0;
} __attribute__((packed, aligned(1)));
static_assert(sizeof(BITMAPHEADER) == 14);

struct BITMAPINFOHEADER {
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};
static_assert(sizeof(BITMAPINFOHEADER) == 40);

uint8_t* a4BData = nullptr;
size_t n4BDataSize = 0;
uint8_t* a4BBMPData = nullptr;
size_t n4BBMPDataSize = 0;

bool Parse4B() {
	WriteConsole("Parsing 4B map...", LOG_ALWAYS);

	if (sFileName.extension() != ".4b") {
		return false;
	}

	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return false;

	fin.seekg(0, std::ios::end);
	n4BDataSize = fin.tellg();
	fin.seekg(0, std::ios::beg);

	a4BData = new uint8_t[n4BDataSize];
	fin.read((char*)a4BData, n4BDataSize);
	return true;
}

bool Write4BFromBMP() {
	WriteConsole("Parsing BMP...", LOG_ALWAYS);

	if (sFileName.extension() != ".bmp") {
		return false;
	}

	std::ifstream fin(sFileName, std::ios::in | std::ios::binary );
	std::ofstream fout(sFileNameNoExt.string() + "_out.4b", std::ios::out | std::ios::binary );
	if (!fin.is_open() || !fout.is_open()) return false;

	BITMAPHEADER header;
	fin.read((char*)&header, sizeof(header));
	BITMAPINFOHEADER infoHeader;
	fin.read((char*)&infoHeader, sizeof(infoHeader));
	if (header.fileSize <= 0) return false;
	if (infoHeader.biHeight != 256 || infoHeader.biWidth != 256) {
		WriteConsole("ERROR: Invalid image size for BMP to 4B conversion!", LOG_ERRORS);
		return false;
	}
	if (infoHeader.biBitCount != 24) {
		WriteConsole("ERROR: Invalid bit count for BMP to 4B conversion!", LOG_ERRORS);
		return false;
	}
	n4BBMPDataSize = header.fileSize - sizeof(header) - sizeof(infoHeader);
	if (infoHeader.biSizeImage != n4BBMPDataSize) {
		WriteConsole("ERROR: Invalid file size for BMP to 4B conversion, compression is not supported!", LOG_ERRORS);
		return false;
	}
	if (n4BBMPDataSize % 6 != 0) {
		WriteConsole("ERROR: File size for BMP not divisible by 6 to convert to the 4B format!", LOG_ERRORS);
		return false;
	}
	a4BBMPData = new uint8_t[n4BBMPDataSize];
	fin.read((char*)a4BBMPData, n4BBMPDataSize);

	for (int i = 0; i < n4BBMPDataSize; i += 6) {
		fout.write((char*)&a4BBMPData[i], 1);
	}

	WriteConsole("4B export finished", LOG_ALWAYS);
	return true;
}

bool Write4BToBMP() {
	WriteConsole("Exporting 4B map to .bmp...", LOG_ALWAYS);

	std::ofstream fout(sFileNameNoExt.string() + "_out.bmp", std::ios::out | std::ios::binary );
	if (!fout.is_open()) return false;

	uint32_t bmpFileSize = n4BDataSize * 6;

	BITMAPHEADER baseHeader;
	baseHeader.fileSize = bmpFileSize + sizeof(BITMAPHEADER) + sizeof(BITMAPINFOHEADER);
	baseHeader.dataOffset = sizeof(BITMAPHEADER) + sizeof(BITMAPINFOHEADER);

	BITMAPINFOHEADER header;
	header.biSize = sizeof(header);
	header.biWidth = 256;
	header.biHeight = 256;
	header.biPlanes = 1;
	header.biBitCount = 24;
	header.biCompression = 0;
	header.biSizeImage = bmpFileSize;
	header.biXPelsPerMeter = 256;
	header.biYPelsPerMeter = 256;
	header.biClrUsed = 0;
	header.biClrImportant = 0;
	fout.write((char*)&baseHeader, sizeof(baseHeader));
	fout.write((char*)&header, sizeof(header));
	for (int i = 0; i < n4BDataSize; i++) {
		// duplicate data 3 times for 24 bit, and twice afterwards to make it square
		for (int j = 0; j < 6; j++) {
			fout.write((char*)&a4BData[i], 1);
		}
	}

	return true;
}